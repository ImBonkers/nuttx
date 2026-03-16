/****************************************************************************
 * arch/arm/src/stm32n6/hardware/stm32n6xxx_wdg.h
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

#ifndef __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_WDG_H
#define __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_WDG_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include "chip.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Register Offsets *********************************************************/

#define STM32_IWDG_KR_OFFSET     0x0000  /* Key register */
#define STM32_IWDG_PR_OFFSET     0x0004  /* Prescaler register */
#define STM32_IWDG_RLR_OFFSET    0x0008  /* Reload register */
#define STM32_IWDG_SR_OFFSET     0x000c  /* Status register */
#define STM32_IWDG_WINR_OFFSET   0x0010  /* Window register */
#define STM32_IWDG_EWCR_OFFSET   0x0014  /* Early wakeup interrupt register */
#define STM32_IWDG_ICR_OFFSET    0x0018  /* Interrupt clear register */

/* Register Addresses *******************************************************/

#define STM32_IWDG_KR            (STM32_IWDG_BASE + STM32_IWDG_KR_OFFSET)
#define STM32_IWDG_PR            (STM32_IWDG_BASE + STM32_IWDG_PR_OFFSET)
#define STM32_IWDG_RLR           (STM32_IWDG_BASE + STM32_IWDG_RLR_OFFSET)
#define STM32_IWDG_SR            (STM32_IWDG_BASE + STM32_IWDG_SR_OFFSET)
#define STM32_IWDG_WINR          (STM32_IWDG_BASE + STM32_IWDG_WINR_OFFSET)
#define STM32_IWDG_EWCR          (STM32_IWDG_BASE + STM32_IWDG_EWCR_OFFSET)
#define STM32_IWDG_ICR           (STM32_IWDG_BASE + STM32_IWDG_ICR_OFFSET)

/* Register Bitfield Definitions ********************************************/

/* Key register */

#define IWDG_KR_KEY_SHIFT        (0)       /* Bits 15-0: Key value (write only) */
#define IWDG_KR_KEY_MASK         (0xffff << IWDG_KR_KEY_SHIFT)

#define IWDG_KR_KEY_ENABLE       (0x5555)  /* Enable register access */
#define IWDG_KR_KEY_DISABLE      (0x0000)  /* Disable register access */
#define IWDG_KR_KEY_RELOAD       (0xaaaa)  /* Reload the counter */
#define IWDG_KR_KEY_START        (0xcccc)  /* Start the watchdog */

/* Prescaler register — 4-bit field on STM32N6 (supports /4 through /1024) */

#define IWDG_PR_SHIFT            (0)       /* Bits 3-0: Prescaler divider */
#define IWDG_PR_MASK             (0xf << IWDG_PR_SHIFT)
#  define IWDG_PR_DIV4           (0 << IWDG_PR_SHIFT)  /* 0000: /4 */
#  define IWDG_PR_DIV8           (1 << IWDG_PR_SHIFT)  /* 0001: /8 */
#  define IWDG_PR_DIV16          (2 << IWDG_PR_SHIFT)  /* 0010: /16 */
#  define IWDG_PR_DIV32          (3 << IWDG_PR_SHIFT)  /* 0011: /32 */
#  define IWDG_PR_DIV64          (4 << IWDG_PR_SHIFT)  /* 0100: /64 */
#  define IWDG_PR_DIV128         (5 << IWDG_PR_SHIFT)  /* 0101: /128 */
#  define IWDG_PR_DIV256         (6 << IWDG_PR_SHIFT)  /* 0110: /256 */
#  define IWDG_PR_DIV512         (7 << IWDG_PR_SHIFT)  /* 0111: /512 */
#  define IWDG_PR_DIV1024        (8 << IWDG_PR_SHIFT)  /* 1000: /1024 */

/* Reload register */

#define IWDG_RLR_RL_SHIFT        (0)       /* Bits 11:0: Watchdog counter reload value */
#define IWDG_RLR_RL_MASK         (0x0fff << IWDG_RLR_RL_SHIFT)

#define IWDG_RLR_MAX             (0xfff)

/* Status register */

#define IWDG_SR_PVU              (1 << 0)  /* Bit 0: Prescaler value update */
#define IWDG_SR_RVU              (1 << 1)  /* Bit 1: Reload value update */
#define IWDG_SR_WVU              (1 << 2)  /* Bit 2: Window value update */
#define IWDG_SR_EWU              (1 << 3)  /* Bit 3: Early wakeup value update */
#define IWDG_SR_ONF              (1 << 8)  /* Bit 8: Watchdog enable status */
#define IWDG_SR_EWIF             (1 << 15) /* Bit 15: Early wakeup interrupt flag */

/* Window register */

#define IWDG_WINR_SHIFT          (0)       /* Bits 11:0: Watchdog counter window value */
#define IWDG_WINR_MASK           (0x0fff << IWDG_WINR_SHIFT)

/* Early wakeup interrupt register */

#define IWDG_EWCR_EWIT_SHIFT     (0)       /* Bits 11:0: Early wakeup comparator */
#define IWDG_EWCR_EWIT_MASK      (0x0fff << IWDG_EWCR_EWIT_SHIFT)
#define IWDG_EWCR_EWIE           (1 << 15) /* Bit 15: Early wakeup interrupt enable */

/* Interrupt clear register */

#define IWDG_ICR_EWIC            (1 << 15) /* Bit 15: Early wakeup interrupt clear */

#endif /* __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_WDG_H */
