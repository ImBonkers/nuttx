/****************************************************************************
 * arch/arm/src/stm32n6/stm32_npu.c
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
 * STM32N6 NPU (ATON Neural-ART) lower-half AIE driver.
 *
 * Wraps native ATON epoch execution into NuttX's ai_engine upper/lower-half
 * framework, exposing /dev/npu0 with ioctl interface.
 *
 * Epoch wait strategy: yield-poll.  EN_BUFBL cannot be used because it
 * stalls the STRENG pipeline (it's a per-block synchronization mechanism
 * for CPU-in-the-loop SW epochs, not a passive completion notification).
 * Instead, we poll STRENG CTRL.RUNNING with nxsig_usleep() between polls
 * so the CPU yields to other NuttX tasks during inference.
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#ifdef CONFIG_STM32N6_NPU

#include <string.h>
#include <stdbool.h>
#include <debug.h>
#include <errno.h>

#include <nuttx/cache.h>
#include <nuttx/clock.h>
#include <nuttx/signal.h>
#include <nuttx/aie/ai_engine.h>

#include "arm_internal.h"
#include "stm32_npu.h"
#include "stm32_xspi.h"
#include "hardware/stm32n6xxx_memorymap.h"

#include "stm32_aton.h"
#include "stm32_aton_hw.h"
#include "npu_test.h"

/* Forward declarations for generated model functions (npu_test.c) */

extern const EpochBlock_ItemTypeDef *
  LL_ATON_EpochBlockItems_npu_test(void);
extern const LL_Buffer_InfoTypeDef *
  LL_ATON_Input_Buffers_Info_npu_test(void);
extern const LL_Buffer_InfoTypeDef *
  LL_ATON_Output_Buffers_Info_npu_test(void);
extern bool LL_ATON_EC_Network_Init_npu_test(void);
extern bool LL_ATON_EC_Inference_Init_npu_test(void);

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Poll interval (microseconds) — short enough for low latency,
 * long enough to let other tasks run.
 */

#define NPU_POLL_INTERVAL_US  100

/* Timeout in milliseconds per epoch */

#define NPU_EPOCH_TIMEOUT_MS  5000

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct stm32_npu_s
{
  struct aie_lowerhalf_s lower;  /* Must be first (cast-compatible) */
  const EpochBlock_ItemTypeDef *epoch_blocks;
  const LL_Buffer_InfoTypeDef  *input_bufs;
  const LL_Buffer_InfoTypeDef  *output_bufs;
  bool initialized;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int stm32_npu_init(FAR struct aie_lowerhalf_s *lower,
                          uintptr_t model);
static int stm32_npu_deinit(FAR struct aie_lowerhalf_s *lower, int id);
static int stm32_npu_feed_input(FAR struct aie_lowerhalf_s *lower, int id,
                                uintptr_t input);
static int stm32_npu_get_output(FAR struct aie_lowerhalf_s *lower, int id,
                                uintptr_t output);
static int stm32_npu_control(FAR struct aie_lowerhalf_s *lower, int id,
                             int cmd, unsigned long arg);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct aie_ops_s g_npu_ops =
{
  .init       = stm32_npu_init,
  .deinit     = stm32_npu_deinit,
  .feed_input = stm32_npu_feed_input,
  .get_output = stm32_npu_get_output,
  .control    = stm32_npu_control,
};

static struct stm32_npu_s g_npu_dev;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void cacheaxi_disable(void)
{
  /* Full invalidate while still enabled, wait for quiescence, then off */

  putreg32(0x03 | 0x3f0f0000,
           STM32_CACHEAXI_BASE + ATON_CACHEAXI_CR1);
  while (getreg32(STM32_CACHEAXI_BASE + ATON_CACHEAXI_SR) & 0x09);
  putreg32(0x12, STM32_CACHEAXI_BASE + ATON_CACHEAXI_FCR);
  putreg32(0, STM32_CACHEAXI_BASE + ATON_CACHEAXI_CR1);
}

/****************************************************************************
 * Name: streng_wait_yield
 *
 * Description:
 *   Wait for STRENGs in mask to complete, yielding CPU between polls.
 *   Returns 0 on success, -ETIMEDOUT on timeout.
 *
 ****************************************************************************/

static int streng_wait_yield(uint32_t mask)
{
  clock_t deadline = clock_systime_ticks() +
                     MSEC2TICK(NPU_EPOCH_TIMEOUT_MS);
  int i;

  for (; ; )
    {
      uint32_t running = 0;

      for (i = 0; i < ATON_STRENG_NUM; i++)
        {
          if (mask & (1u << i))
            {
              running |= getreg32(ATON_STRENG_BASE(i) + ATON_STRENG_CTRL)
                         & STRENG_CTRL_RUNNING;
            }
        }

      if (!running)
        {
          return 0;
        }

      if ((int32_t)(clock_systime_ticks() - deadline) >= 0)
        {
          return -ETIMEDOUT;
        }

      /* Yield CPU to other tasks via sched_yield().  This lets NuttX
       * run ready tasks without the minimum-tick overhead of usleep.
       * If no other task is ready, returns immediately (tight poll).
       */

      sched_yield();
    }
}

/****************************************************************************
 * Name: stm32_npu_init
 ****************************************************************************/

static int stm32_npu_init(FAR struct aie_lowerhalf_s *lower, uintptr_t model)
{
  FAR struct stm32_npu_s *priv = (FAR struct stm32_npu_s *)lower;

  if (priv->initialized)
    {
      return -EBUSY;
    }

  priv->epoch_blocks = LL_ATON_EpochBlockItems_npu_test();
  priv->input_bufs   = LL_ATON_Input_Buffers_Info_npu_test();
  priv->output_bufs  = LL_ATON_Output_Buffers_Info_npu_test();

  if (priv->epoch_blocks == NULL || priv->input_bufs == NULL ||
      priv->output_bufs == NULL)
    {
      syslog(LOG_ERR, "NPU: failed to get model tables\n");
      return -EIO;
    }

  /* Reject models with SW or hybrid epochs — only pure HW epochs allowed */

  {
    const EpochBlock_ItemTypeDef *ebs = priv->epoch_blocks;
    int i;

    for (i = 0; !(ebs[i].flags & EpochBlock_Flags_last_eb); i++)
      {
        if (ebs[i].flags & (EpochBlock_Flags_pure_sw |
                            EpochBlock_Flags_hybrid))
          {
            syslog(LOG_ERR, "NPU: model has SW/hybrid epochs, rejected\n");
            return -ENOTSUP;
          }
      }
  }

  LL_ATON_EC_Network_Init_npu_test();
  LL_ATON_EC_Inference_Init_npu_test();

  priv->initialized = true;
  return 1;
}

/****************************************************************************
 * Name: stm32_npu_deinit
 ****************************************************************************/

static int stm32_npu_deinit(FAR struct aie_lowerhalf_s *lower, int id)
{
  FAR struct stm32_npu_s *priv = (FAR struct stm32_npu_s *)lower;

  priv->epoch_blocks = NULL;
  priv->input_bufs   = NULL;
  priv->output_bufs  = NULL;
  priv->initialized  = false;

  return OK;
}

/****************************************************************************
 * Name: stm32_npu_feed_input
 ****************************************************************************/

static int stm32_npu_feed_input(FAR struct aie_lowerhalf_s *lower, int id,
                                uintptr_t input)
{
  FAR struct stm32_npu_s *priv = (FAR struct stm32_npu_s *)lower;
  FAR const void *user_buf = (FAR const void *)input;
  uint8_t *dst;
  uint32_t size;

  if (!priv->initialized)
    {
      return -EINVAL;
    }

  dst  = LL_Buffer_addr_start(&priv->input_bufs[0]);
  size = LL_ATON_NPU_TEST_IN_1_SIZE_BYTES;

  memcpy(dst, user_buf, size);
  up_clean_dcache((uintptr_t)dst, (uintptr_t)dst + size);

  return OK;
}

/****************************************************************************
 * Name: stm32_npu_get_output
 ****************************************************************************/

static int stm32_npu_get_output(FAR struct aie_lowerhalf_s *lower, int id,
                                uintptr_t output)
{
  FAR struct stm32_npu_s *priv = (FAR struct stm32_npu_s *)lower;
  FAR void *user_buf = (FAR void *)output;
  uint8_t *src;
  uint32_t size;

  if (!priv->initialized)
    {
      return -EINVAL;
    }

  src  = LL_Buffer_addr_start(&priv->output_bufs[0]);
  size = LL_ATON_NPU_TEST_OUT_1_SIZE_BYTES;

  up_invalidate_dcache((uintptr_t)src, (uintptr_t)src + size);
  memcpy(user_buf, src, size);

  return OK;
}

/****************************************************************************
 * Name: stm32_npu_control
 ****************************************************************************/

static int stm32_npu_control(FAR struct aie_lowerhalf_s *lower, int id,
                             int cmd, unsigned long arg)
{
  FAR struct stm32_npu_s *priv = (FAR struct stm32_npu_s *)lower;
  const EpochBlock_ItemTypeDef *ebs;
  int i;

  if (!priv->initialized)
    {
      return -EINVAL;
    }

  switch (cmd)
    {
      case NPUIOC_RUN_SYNC:
        {
          int ret;

          ret = stm32_xspi_mmap_lock();
          if (ret < 0)
            {
              return ret;
            }

          putreg32(0x02, STM32_CACHEAXI_BASE + ATON_CACHEAXI_CR1);
          while (getreg32(STM32_CACHEAXI_BASE + ATON_CACHEAXI_SR) & 1);
          putreg32(0x01 | 0x3f0f0000,
                   STM32_CACHEAXI_BASE + ATON_CACHEAXI_CR1);

          ebs = priv->epoch_blocks;

          for (i = 0; ; i++)
            {
              if (ebs[i].flags & EpochBlock_Flags_last_eb)
                {
                  break;
                }

              ebs[i].start_epoch_block(&ebs[i]);

              ret = streng_wait_yield(ebs[i].wait_mask);
              if (ret < 0)
                {
                  syslog(LOG_ERR, "NPU: epoch %d timeout\n", i);
                  cacheaxi_disable();
                  stm32_xspi_mmap_unlock();
                  return ret;
                }

              ebs[i].end_epoch_block(&ebs[i]);
            }

          cacheaxi_disable();
          stm32_xspi_mmap_unlock();

          return OK;
        }

      case NPUIOC_GET_INFO:
        {
          FAR struct npu_tensor_info_s *info =
            (FAR struct npu_tensor_info_s *)arg;

          info->n_inputs          = LL_ATON_NPU_TEST_IN_NUM;
          info->n_outputs         = LL_ATON_NPU_TEST_OUT_NUM;
          info->input_size_bytes  = LL_ATON_NPU_TEST_IN_1_SIZE_BYTES;
          info->output_size_bytes = LL_ATON_NPU_TEST_OUT_1_SIZE_BYTES;

          info->input_shape[0]  = 1;
          info->input_shape[1]  = 3;
          info->input_shape[2]  = 32;
          info->input_shape[3]  = 32;
          info->output_shape[0] = 1;
          info->output_shape[1] = 16;
          info->output_shape[2] = 32;
          info->output_shape[3] = 32;
          return OK;
        }

      default:
        return -ENOSYS;
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

FAR struct aie_lowerhalf_s *stm32_npu_initialize(void)
{
  memset(&g_npu_dev, 0, sizeof(g_npu_dev));
  g_npu_dev.lower.ops = &g_npu_ops;

  return &g_npu_dev.lower;
}

#endif /* CONFIG_STM32N6_NPU */
