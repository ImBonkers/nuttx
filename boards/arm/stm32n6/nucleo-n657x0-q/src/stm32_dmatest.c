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

#define DMA_TEST_MAX_BYTES   4096
#define DMA_TEST_TIMEOUT     1000000

/* DMA instance IDs (must match stm32_dma.c) */

#define DMA_INST_HPDMA1      1
#define DMA_INST_GPDMA1      2

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct dma_test_case_s
{
  const char *label;
  int         instance;    /* DMA_INST_HPDMA1 or DMA_INST_GPDMA1 */
  int         ch_lo;       /* Channel range low (0-11 small, 12-15 large) */
  int         ch_hi;       /* Channel range high */
  uint32_t    sdw;         /* Source data width (GPDMA_CXTR1_SDW_LOG2_xxx) */
  uint32_t    ddw;         /* Dest data width (GPDMA_CXTR1_DDW_LOG2_xxx) */
  uint16_t    nbytes;      /* Transfer size in bytes */
  uint8_t     priority;    /* GPDMACFG_PRIO_xxx */
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static volatile bool g_dma_done;
static volatile uint8_t g_dma_status;

static uint8_t g_dma_src[DMA_TEST_MAX_BYTES]
  __attribute__((aligned(32)));
static uint8_t g_dma_dst[DMA_TEST_MAX_BYTES]
  __attribute__((aligned(32)));

/* Comprehensive test matrix:
 *
 * For each engine (HPDMA1, GPDMA1):
 *   Small FIFO channels (ch0-11): byte/halfword/word @ 256B, word @ 4B/4KB
 *   Large FIFO channels (ch12-15): byte/halfword/word @ 256B, word @ 4B/4KB
 *   Priority test: high priority on large FIFO
 */

static const struct dma_test_case_s g_tests[] =
{
#ifdef CONFIG_STM32N6_HPDMA1
  /* HPDMA1 small FIFO (ch0-11, 16B FIFO) */

  {
    "HPDMA1 sm byte  256B", DMA_INST_HPDMA1, 0, 11,
    GPDMA_CXTR1_SDW_LOG2_BYTE, GPDMA_CXTR1_DDW_LOG2_BYTE, 256, 0
  },
  {
    "HPDMA1 sm hw    256B", DMA_INST_HPDMA1, 0, 11,
    GPDMA_CXTR1_SDW_LOG2_HW, GPDMA_CXTR1_DDW_LOG2_HW, 256, 0
  },
  {
    "HPDMA1 sm word  256B", DMA_INST_HPDMA1, 0, 11,
    GPDMA_CXTR1_SDW_LOG2_WORD, GPDMA_CXTR1_DDW_LOG2_WORD, 256, 0
  },
  {
    "HPDMA1 sm word    4B", DMA_INST_HPDMA1, 0, 11,
    GPDMA_CXTR1_SDW_LOG2_WORD, GPDMA_CXTR1_DDW_LOG2_WORD, 4, 0
  },
  {
    "HPDMA1 sm word  4KB ", DMA_INST_HPDMA1, 0, 11,
    GPDMA_CXTR1_SDW_LOG2_WORD, GPDMA_CXTR1_DDW_LOG2_WORD, 4096, 0
  },

  /* HPDMA1 large FIFO (ch12-15, 64B FIFO) */

  {
    "HPDMA1 lg byte  256B", DMA_INST_HPDMA1, 12, 15,
    GPDMA_CXTR1_SDW_LOG2_BYTE, GPDMA_CXTR1_DDW_LOG2_BYTE, 256, 0
  },
  {
    "HPDMA1 lg hw    256B", DMA_INST_HPDMA1, 12, 15,
    GPDMA_CXTR1_SDW_LOG2_HW, GPDMA_CXTR1_DDW_LOG2_HW, 256, 0
  },
  {
    "HPDMA1 lg word  256B", DMA_INST_HPDMA1, 12, 15,
    GPDMA_CXTR1_SDW_LOG2_WORD, GPDMA_CXTR1_DDW_LOG2_WORD, 256, 0
  },
  {
    "HPDMA1 lg word    4B", DMA_INST_HPDMA1, 12, 15,
    GPDMA_CXTR1_SDW_LOG2_WORD, GPDMA_CXTR1_DDW_LOG2_WORD, 4, 0
  },
  {
    "HPDMA1 lg word  4KB ", DMA_INST_HPDMA1, 12, 15,
    GPDMA_CXTR1_SDW_LOG2_WORD, GPDMA_CXTR1_DDW_LOG2_WORD, 4096, 0
  },
  {
    "HPDMA1 lg hipri 256B", DMA_INST_HPDMA1, 12, 15,
    GPDMA_CXTR1_SDW_LOG2_WORD, GPDMA_CXTR1_DDW_LOG2_WORD, 256,
    GPDMACFG_PRIO_H
  },
#endif

#ifdef CONFIG_STM32N6_GPDMA1
  /* GPDMA1 small FIFO (ch0-11, 8B FIFO) */

  {
    "GPDMA1 sm byte  256B", DMA_INST_GPDMA1, 0, 11,
    GPDMA_CXTR1_SDW_LOG2_BYTE, GPDMA_CXTR1_DDW_LOG2_BYTE, 256, 0
  },
  {
    "GPDMA1 sm hw    256B", DMA_INST_GPDMA1, 0, 11,
    GPDMA_CXTR1_SDW_LOG2_HW, GPDMA_CXTR1_DDW_LOG2_HW, 256, 0
  },
  {
    "GPDMA1 sm word  256B", DMA_INST_GPDMA1, 0, 11,
    GPDMA_CXTR1_SDW_LOG2_WORD, GPDMA_CXTR1_DDW_LOG2_WORD, 256, 0
  },
  {
    "GPDMA1 sm word    4B", DMA_INST_GPDMA1, 0, 11,
    GPDMA_CXTR1_SDW_LOG2_WORD, GPDMA_CXTR1_DDW_LOG2_WORD, 4, 0
  },
  {
    "GPDMA1 sm word  4KB ", DMA_INST_GPDMA1, 0, 11,
    GPDMA_CXTR1_SDW_LOG2_WORD, GPDMA_CXTR1_DDW_LOG2_WORD, 4096, 0
  },

  /* GPDMA1 large FIFO (ch12-15, 32B FIFO) */

  {
    "GPDMA1 lg byte  256B", DMA_INST_GPDMA1, 12, 15,
    GPDMA_CXTR1_SDW_LOG2_BYTE, GPDMA_CXTR1_DDW_LOG2_BYTE, 256, 0
  },
  {
    "GPDMA1 lg hw    256B", DMA_INST_GPDMA1, 12, 15,
    GPDMA_CXTR1_SDW_LOG2_HW, GPDMA_CXTR1_DDW_LOG2_HW, 256, 0
  },
  {
    "GPDMA1 lg word  256B", DMA_INST_GPDMA1, 12, 15,
    GPDMA_CXTR1_SDW_LOG2_WORD, GPDMA_CXTR1_DDW_LOG2_WORD, 256, 0
  },
  {
    "GPDMA1 lg word    4B", DMA_INST_GPDMA1, 12, 15,
    GPDMA_CXTR1_SDW_LOG2_WORD, GPDMA_CXTR1_DDW_LOG2_WORD, 4, 0
  },
  {
    "GPDMA1 lg word  4KB ", DMA_INST_GPDMA1, 12, 15,
    GPDMA_CXTR1_SDW_LOG2_WORD, GPDMA_CXTR1_DDW_LOG2_WORD, 4096, 0
  },
  {
    "GPDMA1 lg hipri 256B", DMA_INST_GPDMA1, 12, 15,
    GPDMA_CXTR1_SDW_LOG2_WORD, GPDMA_CXTR1_DDW_LOG2_WORD, 256,
    GPDMACFG_PRIO_H
  },
#endif
};

#define DMA_NTESTS (sizeof(g_tests) / sizeof(g_tests[0]))

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void dmatest_callback(DMA_HANDLE handle, uint8_t status, void *arg)
{
  g_dma_status = status;
  g_dma_done = true;
}

static int dmatest_run_one(const struct dma_test_case_s *tc)
{
  struct stm32_gpdma_cfg_s cfg;
  DMA_HANDLE handle;
  int timeout;
  int i;
  int errors;
  int first_err;

  printf("  %-22s", tc->label);

  /* Allocate channel from specified instance and range */

  handle = stm32_dmachannel_range(tc->instance, tc->ch_lo, tc->ch_hi,
                                   GPDMA_TTYPE_M2M_LINEAR);
  if (handle == NULL)
    {
      printf("SKIP (no channel)\n");
      return OK;
    }

  /* Fill source with deterministic pattern, destination with 0xff */

  for (i = 0; i < tc->nbytes; i++)
    {
      g_dma_src[i] = (uint8_t)(i ^ 0xa5);
    }

  memset(g_dma_dst, 0xff, tc->nbytes);

  /* Flush source, invalidate destination for cache coherency */

  up_clean_dcache((uintptr_t)g_dma_src,
                  (uintptr_t)g_dma_src + tc->nbytes);
  up_invalidate_dcache((uintptr_t)g_dma_dst,
                       (uintptr_t)g_dma_dst + tc->nbytes);

  /* Configure DMA transfer */

  memset(&cfg, 0, sizeof(cfg));
  cfg.src_addr   = (uint32_t)g_dma_src;
  cfg.dest_addr  = (uint32_t)g_dma_dst;
  cfg.tr1        = tc->sdw | tc->ddw
                 | GPDMA_CXTR1_SINC
                 | GPDMA_CXTR1_DINC;
  cfg.request    = GPDMA_CXTR2_SWREQ;
  cfg.ntransfers = tc->nbytes;
  cfg.priority   = tc->priority;
  cfg.mode       = 0;

  g_dma_done = false;
  g_dma_status = 0;

  stm32_dmasetup(handle, &cfg);
  stm32_dmastart(handle, dmatest_callback, NULL, false);

  /* Poll for completion */

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
      printf("FAIL (DMA error 0x%02x)\n", g_dma_status);
      stm32_dmafree(handle);
      return -EIO;
    }

  /* Invalidate destination to see DMA result */

  up_invalidate_dcache((uintptr_t)g_dma_dst,
                       (uintptr_t)g_dma_dst + tc->nbytes);

  /* Verify byte-by-byte */

  errors = 0;
  first_err = -1;
  for (i = 0; i < tc->nbytes; i++)
    {
      if (g_dma_dst[i] != g_dma_src[i])
        {
          if (first_err < 0)
            {
              first_err = i;
            }

          errors++;
        }
    }

  stm32_dmafree(handle);

  if (errors > 0)
    {
      printf("FAIL (%d/%d err @%d: got 0x%02x exp 0x%02x)\n",
             errors, tc->nbytes, first_err,
             g_dma_dst[first_err], g_dma_src[first_err]);
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
  int pass = 0;
  int fail = 0;
  int skip = 0;
  int ret;
  int i;

  printf("\nDMA Comprehensive Test (%d cases)\n",
         (int)DMA_NTESTS);
  printf("  %-22s%s\n", "Test", "Result");
  printf("  %-22s%s\n", "----------------------",
         "------");

  for (i = 0; i < (int)DMA_NTESTS; i++)
    {
      ret = dmatest_run_one(&g_tests[i]);
      if (ret == OK)
        {
          pass++;
        }
      else if (ret == -ETIMEDOUT || ret == -EIO)
        {
          fail++;
        }
      else
        {
          skip++;
        }
    }

  printf("  ----------------------\n");
  printf("  Total: %d pass, %d fail, %d skip / %d\n\n",
         pass, fail, skip, (int)DMA_NTESTS);

  return fail > 0 ? -EIO : OK;
}
