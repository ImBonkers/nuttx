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
#elif defined(CONFIG_STM32N6_NPU_MODEL_PEOPLE_DET)
#include "people_det.h"
#else
#include "npu_test.h"
#endif

/* ST LL_ATON runtime init */

extern int LL_ATON_Init(void);

/* Forward declarations for generated model functions */

#ifdef CONFIG_STM32N6_NPU_MODEL_YOLOV8N192

extern const EpochBlock_ItemTypeDef *
  LL_ATON_EpochBlockItems_yolov8n192(void);
extern const LL_Buffer_InfoTypeDef *
  LL_ATON_Input_Buffers_Info_yolov8n192(void);
extern int LL_ATON_Set_User_Input_Buffer_yolov8n192(uint32_t num,
                                                    void *buffer,
                                                    uint32_t size);
extern int LL_ATON_Set_User_Output_Buffer_yolov8n192(uint32_t num,
                                                     void *buffer,
                                                     uint32_t size);
extern const LL_Buffer_InfoTypeDef *
  LL_ATON_Output_Buffers_Info_yolov8n192(void);
extern bool LL_ATON_EC_Network_Init_yolov8n192(void);
extern bool LL_ATON_EC_Inference_Init_yolov8n192(void);

#elif defined(CONFIG_STM32N6_NPU_MODEL_PEOPLE_DET)

extern const EpochBlock_ItemTypeDef *
  LL_ATON_EpochBlockItems_people_det(void);
extern const LL_Buffer_InfoTypeDef *
  LL_ATON_Input_Buffers_Info_people_det(void);
extern const LL_Buffer_InfoTypeDef *
  LL_ATON_Output_Buffers_Info_people_det(void);
extern bool LL_ATON_EC_Network_Init_people_det(void);
extern bool LL_ATON_EC_Inference_Init_people_det(void);
extern int LL_ATON_Set_User_Input_Buffer_people_det(uint32_t num,
                                                     void *buffer,
                                                     uint32_t size);
extern int LL_ATON_Set_User_Output_Buffer_people_det(uint32_t num,
                                                      void *buffer,
                                                      uint32_t size);

#else /* npu_test model */

extern const EpochBlock_ItemTypeDef *
  LL_ATON_EpochBlockItems_npu_test(void);
extern const LL_Buffer_InfoTypeDef *
  LL_ATON_Input_Buffers_Info_npu_test(void);
extern const LL_Buffer_InfoTypeDef *
  LL_ATON_Output_Buffers_Info_npu_test(void);
extern bool LL_ATON_EC_Network_Init_npu_test(void);
extern bool LL_ATON_EC_Inference_Init_npu_test(void);

#endif

/* Model-specific macros */

#ifdef CONFIG_STM32N6_NPU_MODEL_YOLOV8N192
#  define MODEL_IN_NUM          LL_ATON_YOLOV8N192_IN_NUM
#  define MODEL_OUT_NUM         LL_ATON_YOLOV8N192_OUT_NUM
#  define MODEL_IN_SIZE         LL_ATON_YOLOV8N192_IN_1_SIZE_BYTES
#  define MODEL_OUT_SIZE        LL_ATON_YOLOV8N192_OUT_1_SIZE_BYTES
#  define MODEL_IN_ALIGN        LL_ATON_YOLOV8N192_IN_1_ALIGNMENT
#  define MODEL_OUT_ALIGN       LL_ATON_YOLOV8N192_OUT_1_ALIGNMENT
#  define MODEL_NAME            "yolov8n192"
#elif defined(CONFIG_STM32N6_NPU_MODEL_PEOPLE_DET)
#  define MODEL_IN_NUM          LL_ATON_PEOPLE_DET_IN_NUM
#  define MODEL_OUT_NUM         LL_ATON_PEOPLE_DET_OUT_NUM
#  define MODEL_IN_SIZE         LL_ATON_PEOPLE_DET_IN_1_SIZE_BYTES
#  define MODEL_OUT_SIZE        LL_ATON_PEOPLE_DET_OUT_1_SIZE_BYTES
#  define MODEL_IN_ALIGN        LL_ATON_PEOPLE_DET_IN_1_ALIGNMENT
#  define MODEL_OUT_ALIGN       LL_ATON_PEOPLE_DET_OUT_1_ALIGNMENT
#  define MODEL_NAME            "people_det"
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
  bool initialized;
  bool cacheaxi_held;
#ifdef CONFIG_STM32N6_NPU_MODEL_PEOPLE_DET
  uint8_t *io_input;            /* User-allocated input buffer */
  uint8_t *io_output;           /* User-allocated output buffer */
#endif
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

#ifdef CONFIG_STM32N6_NPU_MODEL_YOLOV8N192

  /* YOLOX-Nano: user-allocated input in SRAM1 (not activation pool).
   * NPU CACHEAXI caches AXISRAM reads — user buffer in SRAM1
   * avoids stale cached input between inferences.
   */

  {
    static uint8_t v8_in[110592]   aligned_data(32);
    static uint8_t v8_out[15120]   aligned_data(32);

    LL_ATON_Set_User_Input_Buffer_yolov8n192(0, v8_in, 110592);
    LL_ATON_Set_User_Output_Buffer_yolov8n192(0, v8_out, 15120);
  }

  priv->epoch_blocks = LL_ATON_EpochBlockItems_yolov8n192();
  priv->input_bufs   = LL_ATON_Input_Buffers_Info_yolov8n192();
  priv->output_bufs  = LL_ATON_Output_Buffers_Info_yolov8n192();

  LL_ATON_EC_Network_Init_yolov8n192();
  LL_ATON_EC_Inference_Init_yolov8n192();

#elif defined(CONFIG_STM32N6_NPU_MODEL_PEOPLE_DET)

  /* Allocate user I/O buffers (people_det uses user-allocated I/O) */

  priv->io_input = kmm_memalign(MODEL_IN_ALIGN, MODEL_IN_SIZE);
  if (priv->io_input == NULL)
    {
      syslog(LOG_ERR, "NPU: failed to alloc input (%d bytes)\n",
             MODEL_IN_SIZE);
      return -ENOMEM;
    }

  priv->io_output = kmm_memalign(MODEL_OUT_ALIGN, MODEL_OUT_SIZE);
  if (priv->io_output == NULL)
    {
      syslog(LOG_ERR, "NPU: failed to alloc output (%d bytes)\n",
             MODEL_OUT_SIZE);
      kmm_free(priv->io_input);
      priv->io_input = NULL;
      return -ENOMEM;
    }

  /* Register buffers with the model */

  LL_ATON_Set_User_Input_Buffer_people_det(0, priv->io_input,
                                           MODEL_IN_SIZE);
  LL_ATON_Set_User_Output_Buffer_people_det(0, priv->io_output,
                                            MODEL_OUT_SIZE);

  priv->epoch_blocks = LL_ATON_EpochBlockItems_people_det();
  priv->input_bufs   = LL_ATON_Input_Buffers_Info_people_det();
  priv->output_bufs  = LL_ATON_Output_Buffers_Info_people_det();

  LL_ATON_EC_Network_Init_people_det();
  LL_ATON_EC_Inference_Init_people_det();

#else /* npu_test */

  priv->epoch_blocks = LL_ATON_EpochBlockItems_npu_test();
  priv->input_bufs   = LL_ATON_Input_Buffers_Info_npu_test();
  priv->output_bufs  = LL_ATON_Output_Buffers_Info_npu_test();

  LL_ATON_EC_Network_Init_npu_test();
  LL_ATON_EC_Inference_Init_npu_test();

#endif

  if (priv->epoch_blocks == NULL || priv->input_bufs == NULL ||
      priv->output_bufs == NULL)
    {
      syslog(LOG_ERR, "NPU: failed to get model tables\n");
      return -EIO;
    }

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

#ifdef CONFIG_STM32N6_NPU_MODEL_PEOPLE_DET
  if (priv->io_input != NULL)
    {
      kmm_free(priv->io_input);
      priv->io_input = NULL;
    }

  if (priv->io_output != NULL)
    {
      kmm_free(priv->io_output);
      priv->io_output = NULL;
    }
#endif

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

  {
    static int fc;
    if (fc < 3)
      {
        syslog(LOG_ERR, "FEED dst=%p size=%lu\n",
               dst, (unsigned long)size);
        fc++;
      }
  }

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
  src  = LL_Buffer_addr_start(&priv->output_bufs[0]);
  size = MODEL_OUT_SIZE;

  up_flush_dcache_all();
  memcpy(user_buf, src, size);
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

          if (!priv->cacheaxi_held)
            {
              ret = stm32_xspi_mmap_lock();
              if (ret < 0)
                {
                  return ret;
                }

              priv->cacheaxi_held = true;
            }

          /* Invalidate + re-enable CACHEAXI before EVERY inference.
           * CACHEAXI caches NPU reads from SRAM (0x342e0000 input).
           * Without invalidation, the NPU reads stale cached input
           * from the first inference on all subsequent runs.
           */

          putreg32(0x02,
                   STM32_CACHEAXI_BASE + ATON_CACHEAXI_CR1);
          while (getreg32(STM32_CACHEAXI_BASE +
                          ATON_CACHEAXI_SR) & 1);
          putreg32(0x01 | 0x3f0f0000,
                   STM32_CACHEAXI_BASE + ATON_CACHEAXI_CR1);

#ifdef CONFIG_STM32N6_NPU_MODEL_YOLOV8N192
          /* Full ATON reset between inferences — the LL_ATON runtime
           * tracks epoch state internally.  Without LL_ATON_Init(),
           * STRENG/ARITH/SWITCH units retain stale config from the
           * previous inference's 120 epochs.
           */

          LL_ATON_Init();
          LL_ATON_EC_Inference_Init_yolov8n192();
#elif defined(CONFIG_STM32N6_NPU_MODEL_PEOPLE_DET)
          LL_ATON_EC_Inference_Init_people_det();
#endif

          ebs = priv->epoch_blocks;


          for (i = 0; ; i++)
            {
              if (ebs[i].flags & EpochBlock_Flags_last_eb)
                {
                  break;
                }

              /* SW epochs have NULL start_epoch_block */

              if (ebs[i].start_epoch_block != NULL)
                {
                  ebs[i].start_epoch_block(&ebs[i]);

                  /* Debug: dump STRENG 6 ADDR after first epoch start */
#ifdef CONFIG_STM32N6_NPU_MODEL_YOLOV8N192
                  if (i == 0)
                    {
                      static int dbg;
                      if (dbg < 3)
                        {
                          /* STRENG 6 reads input. Base=NPU+0x5000+0x1000*6 */
                          uint32_t se6_addr = getreg32(0x580EB008);
                          uint32_t se7_addr = getreg32(0x580EC008);
                          uint32_t se3_addr = getreg32(0x580E8008);
                          syslog(LOG_ERR, "EP%d SE6=%08lx SE7=%08lx SE3=%08lx\n",
                                 i, (unsigned long)se6_addr,
                                 (unsigned long)se7_addr,
                                 (unsigned long)se3_addr);
                          dbg++;
                        }
                    }
#endif
                }

              /* Only wait if there are STRENGs to wait on */

              {

              if (ebs[i].wait_mask != 0)
                {
                  ret = streng_wait_yield(ebs[i].wait_mask);
                  if (ret < 0)
                    {
                      int j;
                      syslog(LOG_ERR,
                             "NPU: epoch %d timeout, mask=0x%08lx\n",
                             i, (unsigned long)ebs[i].wait_mask);
                      for (j = 0; j < ATON_STRENG_NUM; j++)
                        {
                          uint32_t ctrl = getreg32(
                              ATON_STRENG_BASE(j) + ATON_STRENG_CTRL);
                          uint32_t addr = getreg32(
                              ATON_STRENG_BASE(j) + ATON_STRENG_ADDR);
                          uint32_t evt = getreg32(
                              ATON_STRENG_BASE(j) + ATON_STRENG_EVENT);
                          uint32_t limen = getreg32(
                              ATON_STRENG_BASE(j) + ATON_STRENG_LIMITEN);
                          uint32_t limit = getreg32(
                              ATON_STRENG_BASE(j) + 0x34);
                          if (ctrl != 0)
                            {
                              uint32_t irq = getreg32(
                                  ATON_STRENG_BASE(j) + 0x3c);
                              syslog(LOG_ERR,
                                     "  SE%d: CTRL=%08lx ADDR=%08lx "
                                     "LIMEN=%08lx LIM=%lu IRQ=%08lx\n",
                                     j, (unsigned long)ctrl,
                                     (unsigned long)addr,
                                     (unsigned long)limen,
                                     (unsigned long)limit,
                                     (unsigned long)irq);
                            }
                        }

                      /* Dump ARITH, CONVACC, ACTIV, POOL CTRL regs */

                      for (j = 0; j < ATON_ARITH_NUM; j++)
                        {
                          uint32_t c = getreg32(
                              ATON_ARITH_BASE(j) + 0x00);
                          if (c != 0)
                            {
                              syslog(LOG_ERR, "  AR%d: CTRL=%08lx\n",
                                     j, (unsigned long)c);
                            }
                        }

                      for (j = 0; j < ATON_CONVACC_NUM; j++)
                        {
                          uint32_t c = getreg32(
                              ATON_CONVACC_BASE(j) + 0x00);
                          if (c != 0)
                            {
                              syslog(LOG_ERR, "  CA%d: CTRL=%08lx\n",
                                     j, (unsigned long)c);
                            }
                        }

                      for (j = 0; j < ATON_ACTIV_NUM; j++)
                        {
                          uint32_t c = getreg32(
                              ATON_ACTIV_BASE(j) + 0x00);
                          if (c != 0)
                            {
                              syslog(LOG_ERR, "  AV%d: CTRL=%08lx\n",
                                     j, (unsigned long)c);
                            }
                        }

                      for (j = 0; j < ATON_POOL_NUM; j++)
                        {
                          uint32_t c = getreg32(
                              ATON_POOL_BASE(j) + 0x00);
                          if (c != 0)
                            {
                              syslog(LOG_ERR, "  PL%d: CTRL=%08lx\n",
                                     j, (unsigned long)c);
                            }
                        }

                      cacheaxi_disable();
                      stm32_xspi_mmap_unlock();
                      return ret;
                    }
                }

              ebs[i].end_epoch_block(&ebs[i]);

#ifdef CONFIG_STM32N6_NPU_MODEL_YOLOV8N192
              /* Print activation pool checksum after every epoch (first run only) */
              {
                static bool traced;
                if (!traced)
                  {
                    volatile uint8_t *ap = (volatile uint8_t *)0x342e0000;
                    uint32_t cs = 0;
                    int k;
                    up_flush_dcache_all();
                    for (k = 0; k < 1024; k++)
                      {
                        cs += ap[k];
                      }
                    syslog(LOG_ERR, "E%d=%lu\n", i, (unsigned long)cs);
                    if (ebs[i].flags & EpochBlock_Flags_last_eb)
                      {
                        traced = true;
                      }
                  }
              }
#endif
              }
            }

          /* Keep CACHEAXI + XSPI mmap held for back-to-back
           * inferences.  Released on device close or unload.
           */

          return OK;
        }

      case NPUIOC_GET_INFO:
        {
          FAR struct npu_tensor_info_s *info =
            (FAR struct npu_tensor_info_s *)arg;

          info->n_inputs          = MODEL_IN_NUM;
          info->n_outputs         = MODEL_OUT_NUM;
          info->input_size_bytes  = MODEL_IN_SIZE;
          info->output_size_bytes = MODEL_OUT_SIZE;

#ifdef CONFIG_STM32N6_NPU_MODEL_YOLOV8N192
          info->input_shape[0]  = 1;
          info->input_shape[1]  = 192;
          info->input_shape[2]  = 192;
          info->input_shape[3]  = 3;
          info->output_shape[0] = 1;
          info->output_shape[1] = 5;
          info->output_shape[2] = 756;
          info->output_shape[3] = 0;
#elif defined(CONFIG_STM32N6_NPU_MODEL_PEOPLE_DET)
          info->input_shape[0]  = 1;
          info->input_shape[1]  = 3;
          info->input_shape[2]  = 224;
          info->input_shape[3]  = 224;
          info->output_shape[0] = 1;
          info->output_shape[1] = 7;
          info->output_shape[2] = 7;
          info->output_shape[3] = 30;
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
