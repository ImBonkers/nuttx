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
#include <string.h>
#include <debug.h>
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

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_dmatest
 *
 * Description:
 *   Run a memory-to-memory DMA transfer test. Fills a source buffer with
 *   a known pattern, DMA-copies it to a destination buffer, and verifies
 *   the result. Prints PASS/FAIL to syslog.
 *
 * Returned Value:
 *   OK on success, negative errno on failure.
 *
 ****************************************************************************/

int stm32_dmatest(void)
{
  DMA_HANDLE handle;
  struct stm32_gpdma_cfg_s cfg;
  int timeout;
  int i;
  int errors;

  syslog(LOG_INFO, "DMA M2M test: starting (%d bytes)...\n",
         DMA_TEST_NBYTES);

  /* Fill source with known pattern, clear destination */

  for (i = 0; i < DMA_TEST_NWORDS; i++)
    {
      g_dma_src[i] = DMA_TEST_PATTERN | i;
    }

  memset(g_dma_dst, 0, sizeof(g_dma_dst));

  /* Disable D-cache before DMA.  This performs a clean+invalidate of
   * all cache lines via set/way operations (DCCISW), ensuring source
   * data is written back to SRAM.  MVA-based cache ops (DCCMVAC,
   * DCIMVAC) require an MPU Write-Back region to function on Cortex-M55.
   */

  up_disable_dcache();

  /* Allocate a DMA channel for M2M transfer */

  handle = stm32_dmachannel(GPDMA_TTYPE_M2M_LINEAR);
  if (handle == NULL)
    {
      syslog(LOG_ERR, "DMA M2M test: FAIL - no channel available\n");
      return -ENOMEM;
    }

  /* Configure the M2M transfer */

  memset(&cfg, 0, sizeof(cfg));
  cfg.src_addr   = (uint32_t)g_dma_src;
  cfg.dest_addr  = (uint32_t)g_dma_dst;
  cfg.tr1        = GPDMA_CXTR1_SDW_LOG2_WORD   /* 32-bit source width */
                 | GPDMA_CXTR1_DDW_LOG2_WORD    /* 32-bit dest width */
                 | GPDMA_CXTR1_SINC             /* Source increment */
                 | GPDMA_CXTR1_DINC;            /* Dest increment */
  cfg.request    = GPDMA_CXTR2_SWREQ;           /* Software-triggered M2M */
  cfg.ntransfers = DMA_TEST_NBYTES;             /* Block size in bytes */
  cfg.priority   = GPDMACFG_PRIO_LL;
  cfg.mode       = 0;                           /* Linear (non-circular) */

  g_dma_done = false;
  g_dma_status = 0;

  stm32_dmasetup(handle, &cfg);
  stm32_dmastart(handle, dmatest_callback, NULL, false);

  /* Busy-wait for completion with timeout */

  for (timeout = 0; timeout < DMA_TEST_TIMEOUT && !g_dma_done; timeout++)
    {
    }

  if (!g_dma_done)
    {
      syslog(LOG_ERR, "DMA M2M test: FAIL - timeout (residual=%d)\n",
             (int)stm32_dmaresidual(handle));
      stm32_dmastop(handle);
      stm32_dmafree(handle);
      return -ETIMEDOUT;
    }

  /* Check for DMA errors */

  if (g_dma_status & DMA_STATUS_FATAL)
    {
      syslog(LOG_ERR, "DMA M2M test: FAIL - DMA error status=0x%02x\n",
             g_dma_status);
      stm32_dmafree(handle);
      return -EIO;
    }

  /* Re-enable D-cache.  Since disable cleaned+invalidated all lines,
   * the cache is cold and CPU reads will fetch fresh SRAM data
   * written by DMA.
   */

  up_enable_dcache();

  /* Verify the transfer */

  errors = 0;
  for (i = 0; i < DMA_TEST_NWORDS; i++)
    {
      if (g_dma_dst[i] != g_dma_src[i])
        {
          if (errors < 4)
            {
              syslog(LOG_ERR,
                     "DMA M2M test: mismatch at [%d]: "
                     "got 0x%08lx expected 0x%08lx\n",
                     i, (unsigned long)g_dma_dst[i],
                     (unsigned long)g_dma_src[i]);
            }

          errors++;
        }
    }

  stm32_dmafree(handle);

  if (errors > 0)
    {
      syslog(LOG_ERR, "DMA M2M test: FAIL - %d/%d words mismatched\n",
             errors, DMA_TEST_NWORDS);
      return -EIO;
    }

  syslog(LOG_INFO,
         "DMA M2M test: PASS (%d bytes, status=0x%02x)\n",
         DMA_TEST_NBYTES, g_dma_status);
  return OK;
}
