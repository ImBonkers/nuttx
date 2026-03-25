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
 * Uses ST's LL_ATON_RT_RunEpochBlock API in polling mode (RT_MODE=1).
 * This handles HW epochs, SW epochs (DequantizeLinear, Resize), and
 * Hybrid epochs (Concat, Slice, Transpose) with internal sub-epoch chains.
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
#include <nuttx/kmalloc.h>
#include <nuttx/aie/ai_engine.h>

#include "arm_internal.h"
#include "stm32_npu.h"
#include "stm32_xspi.h"
#include "hardware/stm32n6xxx_memorymap.h"

#include "stm32_aton.h"
#include "ll_aton_NN_interface.h"

#ifdef CONFIG_STM32N6_NPU_MODEL_YOLOV8N192
#include "yolov8n192.h"
#else
#include "npu_test.h"
#endif

/* ST LL_ATON runtime */

#include "ll_aton_rt_user_api.h"
#include "ll_aton_osal_user_impl.h"

extern int LL_ATON_Init(void);
extern void LL_ATON_RT_RuntimeInit(void);
extern void LL_ATON_RT_Reset_Network(NN_Instance_TypeDef *nn_instance);

/* Forward declarations handled by LL_ATON_DECLARE_NAMED_NN_INSTANCE_AND_INTERFACE */

/* Model-specific macros */

#ifdef CONFIG_STM32N6_NPU_MODEL_YOLOV8N192
#  define MODEL_IN_NUM          LL_ATON_YOLOV8N192_IN_NUM
#  define MODEL_OUT_NUM         LL_ATON_YOLOV8N192_OUT_NUM
#  define MODEL_IN_SIZE         LL_ATON_YOLOV8N192_IN_1_SIZE_BYTES
#  define MODEL_OUT_SIZE        LL_ATON_YOLOV8N192_OUT_1_SIZE_BYTES
#  define MODEL_IN_ALIGN        LL_ATON_YOLOV8N192_IN_1_ALIGNMENT
#  define MODEL_OUT_ALIGN       LL_ATON_YOLOV8N192_OUT_1_ALIGNMENT
#  define MODEL_NAME            "yolov8n192"
#else
#  define MODEL_IN_NUM          LL_ATON_NPU_TEST_IN_NUM
#  define MODEL_OUT_NUM         LL_ATON_NPU_TEST_OUT_NUM
#  define MODEL_IN_SIZE         LL_ATON_NPU_TEST_IN_1_SIZE_BYTES
#  define MODEL_OUT_SIZE        LL_ATON_NPU_TEST_OUT_1_SIZE_BYTES
#  define MODEL_IN_ALIGN        LL_ATON_NPU_TEST_IN_1_ALIGNMENT
#  define MODEL_OUT_ALIGN       LL_ATON_NPU_TEST_OUT_1_ALIGNMENT
#  define MODEL_NAME            "npu_test"
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Poll interval (microseconds) — short enough for low latency,
 * long enough to let other tasks run.
 */

#define NPU_POLL_INTERVAL_US  100

/* Timeout in milliseconds per epoch */

#define NPU_EPOCH_TIMEOUT_MS  60000

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct stm32_npu_s
{
  struct aie_lowerhalf_s lower;  /* Must be first (cast-compatible) */
  const EpochBlock_ItemTypeDef *epoch_blocks;
  const LL_Buffer_InfoTypeDef  *input_bufs;
  const LL_Buffer_InfoTypeDef  *output_bufs;
  NN_Instance_TypeDef *nn_instance;
  bool initialized;
  bool cacheaxi_held;
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

/* NN interface + instance for the LL_ATON runtime API */

#ifdef CONFIG_STM32N6_NPU_MODEL_YOLOV8N192
LL_ATON_DECLARE_NAMED_NN_INSTANCE_AND_INTERFACE(yolov8n192);
#else
LL_ATON_DECLARE_NAMED_NN_INSTANCE_AND_INTERFACE(npu_test);
#endif

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
 * Name: stm32_npu_init
 ****************************************************************************/

static int stm32_npu_init(FAR struct aie_lowerhalf_s *lower, uintptr_t model)
{
  FAR struct stm32_npu_s *priv = (FAR struct stm32_npu_s *)lower;

  if (priv->initialized)
    {
      return -EBUSY;
    }

#ifdef CONFIG_STM32N6_NPU_MODEL_YOLOV8N192

  /* User-allocated I/O buffers in BSS (not activation pool) */

  {
    static uint8_t v8_in[110592]   aligned_data(32);
    static uint8_t v8_out[15120]   aligned_data(32);

    LL_ATON_Set_User_Input_Buffer_yolov8n192(0, v8_in, 110592);
    LL_ATON_Set_User_Output_Buffer_yolov8n192(0, v8_out, 15120);
  }

  priv->nn_instance = &NN_Instance_yolov8n192;
  priv->epoch_blocks = LL_ATON_EpochBlockItems_yolov8n192();
  priv->input_bufs   = LL_ATON_Input_Buffers_Info_yolov8n192();
  priv->output_bufs  = LL_ATON_Output_Buffers_Info_yolov8n192();

#else /* npu_test */

  priv->nn_instance = &NN_Instance_npu_test;
  priv->epoch_blocks = LL_ATON_EpochBlockItems_npu_test();
  priv->input_bufs   = LL_ATON_Input_Buffers_Info_npu_test();
  priv->output_bufs  = LL_ATON_Output_Buffers_Info_npu_test();

#endif

  if (priv->epoch_blocks == NULL || priv->input_bufs == NULL ||
      priv->output_bufs == NULL || priv->nn_instance == NULL)
    {
      syslog(LOG_ERR, "NPU: failed to get model tables\n");
      return -EIO;
    }

  /* Initialize the LL_ATON runtime and network instance.
   * RuntimeInit sets up the global scheduler.
   * Init_Network sets up exec_state (epoch block list, etc.).
   */

  LL_ATON_RT_RuntimeInit();

  /* RuntimeInit sets INTCTRL AND-mask to 0xFFFFFFFF (block all).
   * With USE_IRQ_OR_MASK, SetWaitMask only modifies OR-mask.
   * Clear AND-mask so OR-mask controls interrupt routing.
   */

  *(volatile uint32_t *)(0x580E1000 + 0x24) = 0;

  LL_ATON_RT_Init_Network(priv->nn_instance);

  priv->initialized = true;
  return 1;
}

/****************************************************************************
 * Name: stm32_npu_deinit
 ****************************************************************************/

static int stm32_npu_deinit(FAR struct aie_lowerhalf_s *lower, int id)
{
  FAR struct stm32_npu_s *priv = (FAR struct stm32_npu_s *)lower;

  if (priv->cacheaxi_held)
    {
      cacheaxi_disable();
      stm32_xspi_mmap_unlock();
      priv->cacheaxi_held = false;
    }

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
  size = MODEL_IN_SIZE;

  memcpy(dst, user_buf, size);
  up_flush_dcache_all();

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

#if defined(CONFIG_STM32N6_NPU_MODEL_YOLOV8N192)
  /* Read raw int8 Concat output [756, 5] from activation pool.
   * Dequantize on host (scale=0.00513, zp=-128).
   * Prepend 4-byte sync marker for USB desync detection.
   */

  {
    static uint32_t frame_seq;
    uint32_t marker = 0xAA550000 | (frame_seq & 0xFFFF);

    src  = (uint8_t *)(0x342e0000 + 15120);
    size = 3780;

    up_flush_dcache_all();
    memcpy(user_buf, &marker, 4);
    memcpy((uint8_t *)user_buf + 4, src, size);

    frame_seq++;
  }
#else
  src  = LL_Buffer_addr_start(&priv->output_bufs[0]);
  size = MODEL_OUT_SIZE;

  up_flush_dcache_all();
  memcpy(user_buf, src, size);
#endif

  return OK;
}

/****************************************************************************
 * Name: stm32_npu_control
 ****************************************************************************/

static int stm32_npu_control(FAR struct aie_lowerhalf_s *lower, int id,
                             int cmd, unsigned long arg)
{
  FAR struct stm32_npu_s *priv = (FAR struct stm32_npu_s *)lower;

  if (!priv->initialized)
    {
      return -EINVAL;
    }

  switch (cmd)
    {
      case NPUIOC_RUN_SYNC:
        {
          int ret;

          if (!priv->cacheaxi_held)
            {
              ret = stm32_xspi_mmap_lock();
              if (ret < 0)
                {
                  return ret;
                }

              priv->cacheaxi_held = true;
            }

          /* CACHEAXI: enable then invalidate for weight reads.
           * Must enable first (bit 0), THEN invalidate (bit 1).
           * Full invalidate ensures reads go to XSPI2 on first
           * access, populating cache with fresh data.
           */

          putreg32(0x01 | 0x3f0f0000,
                   STM32_CACHEAXI_BASE + ATON_CACHEAXI_CR1);
          putreg32(0x03 | 0x3f0f0000,
                   STM32_CACHEAXI_BASE + ATON_CACHEAXI_CR1);
          while (getreg32(STM32_CACHEAXI_BASE +
                          ATON_CACHEAXI_SR) & 1);
          putreg32(0x12, STM32_CACHEAXI_BASE + ATON_CACHEAXI_FCR);

          /* Run inference using LL_ATON runtime API (ASYNC mode).
           * RunEpochBlock returns:
           *   NO_WFE = call again immediately (SW step done)
           *   WFE    = HW epoch running, call OSAL_WFE (sem_wait)
           *   DONE   = inference complete
           *
           * In ASYNC mode, WFE blocks the calling task on a semaphore.
           * The NPU IRQ handler posts the semaphore when the epoch
           * completes, waking this task. Other NuttX tasks run while
           * we wait.
           */

          {
            LL_ATON_RT_RetValues_t rv;

            do
              {
                rv = LL_ATON_RT_RunEpochBlock(priv->nn_instance);
                if (rv == LL_ATON_RT_WFE)
                  {
                    LL_ATON_OSAL_WFE();
                  }
              }
            while (rv != LL_ATON_RT_DONE);

            /* Reset network state for next inference */

            LL_ATON_RT_Reset_Network(priv->nn_instance);
          }

          return OK;
        }

      case NPUIOC_GET_INFO:
        {
          FAR struct npu_tensor_info_s *info =
            (FAR struct npu_tensor_info_s *)arg;

          info->n_inputs          = MODEL_IN_NUM;
          info->n_outputs         = MODEL_OUT_NUM;
          info->input_size_bytes  = MODEL_IN_SIZE;
#ifdef CONFIG_STM32N6_NPU_MODEL_YOLOV8N192
          info->output_size_bytes = 3784;  /* 4B sync + int8 [756,5] */
#else
          info->output_size_bytes = MODEL_OUT_SIZE;
#endif

#ifdef CONFIG_STM32N6_NPU_MODEL_YOLOV8N192
          info->input_shape[0]  = 1;
          info->input_shape[1]  = 192;
          info->input_shape[2]  = 192;
          info->input_shape[3]  = 3;
          info->output_shape[0] = 1;
          info->output_shape[1] = 5;
          info->output_shape[2] = 756;
          info->output_shape[3] = 0;
#else
          info->input_shape[0]  = 1;
          info->input_shape[1]  = 3;
          info->input_shape[2]  = 32;
          info->input_shape[3]  = 32;
          info->output_shape[0] = 1;
          info->output_shape[1] = 16;
          info->output_shape[2] = 32;
          info->output_shape[3] = 32;
#endif
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

  /* Initialize ATON fabric via ST SDK runtime.
   * This replaces the manual board-level ATON init and ensures
   * all NPU units are properly reset and configured.
   */

  LL_ATON_Init();

  return &g_npu_dev.lower;
}

#endif /* CONFIG_STM32N6_NPU */
