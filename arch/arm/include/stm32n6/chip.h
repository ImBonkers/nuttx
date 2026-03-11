/****************************************************************************
 * arch/arm/include/stm32n6/chip.h
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

#ifndef __ARCH_ARM_INCLUDE_STM32N6_CHIP_H
#define __ARCH_ARM_INCLUDE_STM32N6_CHIP_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Pre-processor Prototypes
 ****************************************************************************/

/* Memory sizes - STM32N6 has one unified SRAM (4.2MB) and no internal
 * flash.  Code must be loaded from external XSPI flash or SRAM.
 */

#define STM32N6_SRAM_SIZE         (4 * 1024 * 1024 + 192 * 1024) /* 4390912 bytes (4.2MB) */

#define STM32N6_NATIM                    (2)   /* Two advanced timers TIM1 and TIM8 */
#define STM32N6_NGTIM32                  (2)   /* 32-bit general timers TIM2 and TIM5 with DMA */
#define STM32N6_NGTIM16                  (2)   /* 16-bit general timers TIM3 and TIM4 with DMA */
#define STM32N6_NGTIMNDMA                (3)   /* 16-bit general timers TIM15-17 without DMA */
#define STM32N6_NBTIM                    (2)   /* Two basic timers, TIM6-7 */
#define STM32N6_NLPTIM                   (5)   /* Five low-power timers, LPTIM1-LPTIM5 */
#define STM32N6_NRNG                     (1)   /* Random number generator (RNG) */
#define STM32N6_NUART                    (2)   /* UART 4-5 */
#define STM32N6_NUSART                   (3)   /* USART 1-3 */
#define STM32N6_NLPUART                  (1)   /* LPUART 1 */
#define STM32N6_NSPI                     (6)   /* SPI1-SPI6 */
#define STM32N6_NI2C                     (4)   /* I2C1-I2C4 */
#define STM32N6_NXSPI                    (3)   /* XSPI1-XSPI3 */
#define STM32N6_NSDMMC                   (2)   /* SDMMC1-SDMMC2 */
#define STM32N6_NDMA                     (2)   /* HPDMA1 + GPDMA1 */
#define STM32N6_NPORTS                  (12)   /* 12 GPIO ports: A-H (8) + N, O, P, Q (4) */
#define STM32N6_NADC                     (2)   /* 12-bit ADC1-ADC2 */
#define STM32N6_NDAC                     (1)   /* 12-bit DAC1 */
#define STM32N6_NCRC                     (1)   /* CRC */
#define STM32N6_NCAN                     (3)   /* FDCAN1-FDCAN3 */
#define STM32N6_NSAI                     (2)   /* SAI1-SAI2 */
#define STM32N6_NETHERNET                (1)   /* Ethernet MAC */
#define STM32N6_NUSBOTGHS                (2)   /* USB1 OTG HS + USB2 OTG HS */

/* NVIC priority levels *****************************************************/

/* 16 Programmable interrupt levels (4-bit priority) */

#define NVIC_SYSH_PRIORITY_MIN     0xf0 /* All bits set in minimum priority */
#define NVIC_SYSH_PRIORITY_DEFAULT 0x80 /* Midpoint is the default */
#define NVIC_SYSH_PRIORITY_MAX     0x00 /* Zero is maximum priority */
#define NVIC_SYSH_PRIORITY_STEP    0x10 /* Four bits of interrupt priority used */

#endif /* __ARCH_ARM_INCLUDE_STM32N6_CHIP_H */
