/****************************************************************************
 * boards/arm/stm32n6/nucleo-n657x0-q/src/stm32_dmatest.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <nuttx/cache.h>

#include "arm_internal.h"
#include "stm32_dma.h"
#include "hardware/stm32_gpdma.h"
#include "hardware/stm32n6xxx_memorymap.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define DMA_TEST_NWORDS    64
#define DMA_TEST_NBYTES    (DMA_TEST_NWORDS * 4)
#define DMA_TEST_PATTERN   0xdead0000
#define DMA_TEST_TIMEOUT   1000000

/* DMA instance IDs (must match stm32_dma.c) */

#define DMA_INST_HPDMA1    1
#define DMA_INST_GPDMA1    2

/****************************************************************************
 * Private Data
 ****************************************************************************/

static volatile bool g_dma_done;
static volatile uint8_t g_dma_status;

static uint32_t g_dma_src[DMA_TEST_NWORDS]
  __attribute__((aligned(32)));
static uint32_t g_dma_dst[DMA_TEST_NWORDS]
  __attribute__((aligned(32)));

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void dmatest_callback(DMA_HANDLE handle, uint8_t status, void *arg)
{
  g_dma_status = status;
  g_dma_done = true;
}

static int dmatest_run_one(DMA_HANDLE handle, const char *name)
{
  struct stm32_gpdma_cfg_s cfg;
  int timeout;
  int i;
  int errors;

  printf("  %s M2M: %d bytes... ", name, DMA_TEST_NBYTES);

  /* Fill source with known pattern, clear destination */

  for (i = 0; i < DMA_TEST_NWORDS; i++)
    {
      g_dma_src[i] = DMA_TEST_PATTERN | i;
    }

  memset(g_dma_dst, 0, sizeof(g_dma_dst));

  up_clean_dcache((uintptr_t)g_dma_src,
                  (uintptr_t)g_dma_src + DMA_TEST_NBYTES);
  up_invalidate_dcache((uintptr_t)g_dma_dst,
                       (uintptr_t)g_dma_dst + DMA_TEST_NBYTES);

  memset(&cfg, 0, sizeof(cfg));
  cfg.src_addr   = (uint32_t)g_dma_src;
  cfg.dest_addr  = (uint32_t)g_dma_dst;
  cfg.tr1        = GPDMA_CXTR1_SDW_LOG2_WORD
                 | GPDMA_CXTR1_DDW_LOG2_WORD
                 | GPDMA_CXTR1_SINC
                 | GPDMA_CXTR1_DINC;
  cfg.request    = GPDMA_CXTR2_SWREQ;
  cfg.ntransfers = DMA_TEST_NBYTES;
  cfg.priority   = GPDMACFG_PRIO_LL;
  cfg.mode       = 0;

  g_dma_done = false;
  g_dma_status = 0;

  stm32_dmasetup(handle, &cfg);

  /* Debug: read back channel registers via base ptr in handle.
   * handle points to gpdma_ch_s: instance(u8), channel(u8), irq(u8),
   * type(enum=int), free(bool), base(u32) at offset ~12 bytes.
   * Just use the known channel 0 base directly.
   */

  {
    /* Extract base from handle struct (base is at a known offset).
     * Simpler: just compute from instance+channel.
     */

    uint8_t inst = *((uint8_t *)handle);     /* dma_instance */
    uint8_t ch   = *((uint8_t *)handle + 1); /* channel */
    uint32_t base = (inst == DMA_INST_HPDMA1)
                  ? STM32_HPDMA1_BASE : STM32_GPDMA1_BASE;
    base += 0x80 * ch;

    printf("    ch%d base=0x%08lx regs: SAR=0x%08lx DAR=0x%08lx "
           "BR1=0x%08lx TR1=0x%08lx TR2=0x%08lx CR=0x%08lx\n",
           ch, (unsigned long)base,
           (unsigned long)getreg32(base + 0x9C),
           (unsigned long)getreg32(base + 0xA0),
           (unsigned long)getreg32(base + 0x98),
           (unsigned long)getreg32(base + 0x90),
           (unsigned long)getreg32(base + 0x94),
           (unsigned long)getreg32(base + 0x64));
  }

  stm32_dmastart(handle, dmatest_callback, NULL, false);

  for (timeout = 0; timeout < DMA_TEST_TIMEOUT && !g_dma_done; timeout++)
    {
    }

  if (!g_dma_done)
    {
      printf("FAIL (timeout, residual=%d)\n",
             (int)stm32_dmaresidual(handle));
      stm32_dmastop(handle);
      stm32_dmafree(handle);
      return -ETIMEDOUT;
    }

  if (g_dma_status & DMA_STATUS_FATAL)
    {
      printf("FAIL (error status=0x%02x)\n", g_dma_status);
      stm32_dmafree(handle);
      return -EIO;
    }

  up_invalidate_dcache((uintptr_t)g_dma_dst,
                       (uintptr_t)g_dma_dst + DMA_TEST_NBYTES);

  errors = 0;
  for (i = 0; i < DMA_TEST_NWORDS; i++)
    {
      if (g_dma_dst[i] != g_dma_src[i])
        {
          errors++;
        }
    }

  stm32_dmafree(handle);

  if (errors > 0)
    {
      printf("FAIL (%d/%d mismatched)\n", errors, DMA_TEST_NWORDS);
      printf("    src=%p dst=%p status=0x%02x\n",
             g_dma_src, g_dma_dst, g_dma_status);
      printf("    dst[0..3]: 0x%08lx 0x%08lx 0x%08lx 0x%08lx\n",
             (unsigned long)g_dma_dst[0], (unsigned long)g_dma_dst[1],
             (unsigned long)g_dma_dst[2], (unsigned long)g_dma_dst[3]);

      /* Dump RISAF2 (AXISRAM1) illegal access registers.
       * RISAF2 Secure base = 0x54027000
       * CR     @ +0x000
       * IASR   @ +0x008
       * IAR[0].IAESR @ +0x020
       * IAR[0].IADDR @ +0x024
       * REG[0].CFGR  @ +0x040
       */

      printf("    RISAF2: CR=0x%08lx IASR=0x%08lx "
             "IAESR=0x%08lx IADDR=0x%08lx R0CFGR=0x%08lx\n",
             (unsigned long)getreg32(0x54027000),
             (unsigned long)getreg32(0x54027008),
             (unsigned long)getreg32(0x54027020),
             (unsigned long)getreg32(0x54027024),
             (unsigned long)getreg32(0x54027040));
      return -EIO;
    }

  printf("PASS\n");
  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int stm32_dmatest_main(int argc, char *argv[])
{
  DMA_HANDLE handle;
  int ret = OK;

  printf("DMA Memory-to-Memory Test\n");

#ifdef CONFIG_STM32N6_HPDMA1
  handle = stm32_dmachannel_inst(DMA_INST_HPDMA1,
                                 GPDMA_TTYPE_M2M_LINEAR);
  if (handle != NULL)
    {
      if (dmatest_run_one(handle, "HPDMA1") != OK)
        {
          ret = -EIO;
        }
    }
  else
    {
      printf("  HPDMA1: no channel available\n");
    }
#endif

#ifdef CONFIG_STM32N6_GPDMA1
  handle = stm32_dmachannel_inst(DMA_INST_GPDMA1,
                                 GPDMA_TTYPE_M2M_LINEAR);
  if (handle != NULL)
    {
      if (dmatest_run_one(handle, "GPDMA1") != OK)
        {
          ret = -EIO;
        }
    }
  else
    {
      printf("  GPDMA1: no channel available\n");
    }
#endif

  return ret;
}
