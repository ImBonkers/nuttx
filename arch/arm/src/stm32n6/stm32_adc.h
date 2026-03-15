/****************************************************************************
 * arch/arm/src/stm32n6/stm32_adc.h
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

#ifndef __ARCH_ARM_SRC_STM32N6_STM32_ADC_H
#define __ARCH_ARM_SRC_STM32N6_STM32_ADC_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/analog/adc.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Maximum number of channels per ADC */

#define STM32_ADC_MAX_SAMPLES  16

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifndef __ASSEMBLY__
#ifdef __cplusplus
extern "C"
{
#endif

/****************************************************************************
 * Name: stm32n6_adc_initialize
 *
 * Description:
 *   Initialize the ADC.  See stm32_adc.c for more details.
 *
 * Input Parameters:
 *   intf      - ADC interface number (1 for ADC1, 2 for ADC2)
 *   chanlist  - Array of ADC channel numbers to configure
 *   nchannels - Number of channels in chanlist
 *
 * Returned Value:
 *   Valid ADC device structure reference on success; NULL on failure
 *
 ****************************************************************************/

struct adc_dev_s *stm32n6_adc_initialize(int intf,
                                         const uint8_t *chanlist,
                                         int nchannels);

#ifdef __cplusplus
}
#endif
#endif /* __ASSEMBLY__ */

#endif /* __ARCH_ARM_SRC_STM32N6_STM32_ADC_H */
