/****************************************************************************
 * arch/arm/src/stm32n6/hardware/stm32n6xxx_pwr.h
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

#ifndef __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_PWR_H
#define __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_PWR_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Register Offsets *********************************************************/

#define STM32_PWR_CR1_OFFSET       0x0000  /* Power control register 1 */
#define STM32_PWR_CR2_OFFSET       0x0004  /* Power control register 2 */
#define STM32_PWR_CR3_OFFSET       0x0008  /* Power control register 3 */
#define STM32_PWR_CR4_OFFSET       0x000c  /* Power control register 4 */
#define STM32_PWR_VOSCR_OFFSET     0x0010  /* Voltage scaling control register */
#define STM32_PWR_VOSSR_OFFSET     0x0014  /* Voltage scaling status register */
#define STM32_PWR_SVMCR1_OFFSET    0x0034  /* Supply voltage monitoring control register 1 */
#define STM32_PWR_SVMCR2_OFFSET    0x0038  /* Supply voltage monitoring control register 2 */
#define STM32_PWR_SVMCR3_OFFSET    0x003c  /* Supply voltage monitoring control register 3 */

/* Register Addresses *******************************************************/

#define STM32_PWR_CR1          (STM32_PWR_BASE + STM32_PWR_CR1_OFFSET)
#define STM32_PWR_CR2          (STM32_PWR_BASE + STM32_PWR_CR2_OFFSET)
#define STM32_PWR_CR3          (STM32_PWR_BASE + STM32_PWR_CR3_OFFSET)
#define STM32_PWR_CR4          (STM32_PWR_BASE + STM32_PWR_CR4_OFFSET)
#define STM32_PWR_VOSCR        (STM32_PWR_BASE + STM32_PWR_VOSCR_OFFSET)
#define STM32_PWR_VOSSR        (STM32_PWR_BASE + STM32_PWR_VOSSR_OFFSET)
#define STM32_PWR_SVMCR1       (STM32_PWR_BASE + STM32_PWR_SVMCR1_OFFSET)
#define STM32_PWR_SVMCR2       (STM32_PWR_BASE + STM32_PWR_SVMCR2_OFFSET)
#define STM32_PWR_SVMCR3       (STM32_PWR_BASE + STM32_PWR_SVMCR3_OFFSET)

/* Register Bitfield Definitions ********************************************/

/* Voltage Scaling Control Register (PWR_VOSCR) */

#define PWR_VOSCR_VOS_SHIFT        (0)
#define PWR_VOSCR_VOS_MASK         (3 << PWR_VOSCR_VOS_SHIFT)
#  define PWR_VOSCR_VOS_RANGE3     (0 << PWR_VOSCR_VOS_SHIFT) /* Voltage scaling range 3 */
#  define PWR_VOSCR_VOS_RANGE2     (1 << PWR_VOSCR_VOS_SHIFT) /* Voltage scaling range 2 */
#  define PWR_VOSCR_VOS_RANGE1     (2 << PWR_VOSCR_VOS_SHIFT) /* Voltage scaling range 1 */
#  define PWR_VOSCR_VOS_RANGE0     (3 << PWR_VOSCR_VOS_SHIFT) /* Voltage scaling range 0 (highest perf) */

/* Voltage Scaling Status Register (PWR_VOSSR) */

#define PWR_VOSSR_VOSRDY           (1 << 3)  /* Bit 3: VOS Ready */
#define PWR_VOSSR_ACTVOSRDY        (1 << 13) /* Bit 13: Active VOS Ready */
#define PWR_VOSSR_ACTVOS_SHIFT     (14)
#define PWR_VOSSR_ACTVOS_MASK      (3 << PWR_VOSSR_ACTVOS_SHIFT)

/* Supply Voltage Monitoring Control Register 1 (PWR_SVMCR1) - VddIO4 */

#define PWR_SVMCR1_VDDIO4SV       (1 << 8)  /* Bit 8: VddIO4 supply valid */

/* Supply Voltage Monitoring Control Register 2 (PWR_SVMCR2) - VddIO5 */

#define PWR_SVMCR2_VDDIO5SV       (1 << 8)  /* Bit 8: VddIO5 supply valid */

/* Supply Voltage Monitoring Control Register 3 (PWR_SVMCR3) - VddIO2/3 */

#define PWR_SVMCR3_VDDIO2SV       (1 << 8)  /* Bit 8: VddIO2 supply valid */
#define PWR_SVMCR3_VDDIO3SV       (1 << 9)  /* Bit 9: VddIO3 supply valid */

#endif /* __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_PWR_H */
