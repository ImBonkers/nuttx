/****************************************************************************
 * arch/arm/src/stm32n6/stm32_npu.h
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

#ifndef __ARCH_ARM_SRC_STM32N6_STM32_NPU_H
#define __ARCH_ARM_SRC_STM32N6_STM32_NPU_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/aie/ai_engine.h>
#include <stdint.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Extended ioctl commands for NPU driver (beyond standard AIE_CMD_*).
 * These are passed through aie_ops_s->control().
 */

#define NPUIOC_RUN_SYNC   0x100   /* Run inference synchronously */
#define NPUIOC_GET_INFO   0x101   /* Get tensor info (shapes/sizes) */

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Tensor information returned by NPUIOC_GET_INFO */

struct npu_tensor_info_s
{
  uint32_t n_inputs;           /* Number of input tensors */
  uint32_t n_outputs;          /* Number of output tensors */
  uint32_t input_size_bytes;   /* Total input size in bytes */
  uint32_t output_size_bytes;  /* Total output size in bytes */
  int32_t  input_shape[4];     /* Input shape [N,C,H,W] */
  int32_t  output_shape[4];    /* Output shape [N,C,H,W] */
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef CONFIG_STM32N6_NPU

/****************************************************************************
 * Name: stm32_npu_initialize
 *
 * Description:
 *   Initialize NPU driver state and return an AIE lower-half driver
 *   instance.  Does NOT perform hardware initialization — that is done
 *   by stm32_npu_setup() in the board layer before this is called.
 *
 * Returned Value:
 *   Non-NULL pointer to aie_lowerhalf_s on success; NULL on failure.
 *
 ****************************************************************************/

FAR struct aie_lowerhalf_s *stm32_npu_initialize(void);

/****************************************************************************
 * Name: stm32_npu_setup
 *
 * Description:
 *   Board-level NPU setup: performs HW init (RAMCFG, RIFSC, ATON fabric,
 *   CACHEAXI, STRENG self-test), then initializes the driver and registers
 *   /dev/npu0.  Called from stm32_bringup().
 *
 ****************************************************************************/

int stm32_npu_setup(void);

#endif /* CONFIG_STM32N6_NPU */

#endif /* __ARCH_ARM_SRC_STM32N6_STM32_NPU_H */
