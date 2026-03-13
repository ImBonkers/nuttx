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
      printf("FAIL (%d/%d mismatched, status=0x%02x)\n",
             errors, DMA_TEST_NWORDS, g_dma_status);
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
