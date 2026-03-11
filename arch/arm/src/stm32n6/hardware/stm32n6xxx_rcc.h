/****************************************************************************
 * arch/arm/src/stm32n6/hardware/stm32n6xxx_rcc.h
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

#ifndef __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_RCC_H
#define __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_RCC_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Register Offsets *********************************************************/

#define STM32_RCC_CR_OFFSET            0x0000  /* Clock control register */
#define STM32_RCC_SR_OFFSET            0x0004  /* Clock status register (N6 has SR, not CFGR) */
#define STM32_RCC_CFGR1_OFFSET        0x0020  /* Clock configuration register 1 */
#define STM32_RCC_CFGR2_OFFSET        0x0024  /* Clock configuration register 2 */
#define STM32_RCC_HSICFGR_OFFSET      0x0048  /* HSI calibration register */

/* PLL1 configuration registers */

#define STM32_RCC_PLL1CFGR1_OFFSET    0x0080  /* PLL1 configuration register 1 */
#define STM32_RCC_PLL1CFGR2_OFFSET    0x0084  /* PLL1 configuration register 2 */
#define STM32_RCC_PLL1CFGR3_OFFSET    0x0088  /* PLL1 configuration register 3 */
#define STM32_RCC_PLL1CFGR4_OFFSET    0x008c  /* PLL1 configuration register 4 */
#define STM32_RCC_PLL1CFGR5_OFFSET    0x0090  /* PLL1 configuration register 5 */
#define STM32_RCC_PLL1CFGR6_OFFSET    0x0094  /* PLL1 configuration register 6 */
#define STM32_RCC_PLL1CFGR7_OFFSET    0x0098  /* PLL1 configuration register 7 */

/* PLL2 configuration registers */

#define STM32_RCC_PLL2CFGR1_OFFSET    0x00a0  /* PLL2 configuration register 1 */
#define STM32_RCC_PLL2CFGR2_OFFSET    0x00a4  /* PLL2 configuration register 2 */
#define STM32_RCC_PLL2CFGR3_OFFSET    0x00a8  /* PLL2 configuration register 3 */
#define STM32_RCC_PLL2CFGR4_OFFSET    0x00ac  /* PLL2 configuration register 4 */
#define STM32_RCC_PLL2CFGR5_OFFSET    0x00b0  /* PLL2 configuration register 5 */
#define STM32_RCC_PLL2CFGR6_OFFSET    0x00b4  /* PLL2 configuration register 6 */
#define STM32_RCC_PLL2CFGR7_OFFSET    0x00b8  /* PLL2 configuration register 7 */

/* PLL3 configuration register */

#define STM32_RCC_PLL3CFGR1_OFFSET    0x00c0  /* PLL3 configuration register 1 */

/* PLL4 configuration register */

#define STM32_RCC_PLL4CFGR1_OFFSET    0x00e0  /* PLL4 configuration register 1 */

/* IC (Interconnect Clock) configuration registers */

#define STM32_RCC_IC1CFGR_OFFSET      0x0100  /* IC1 configuration register */
#define STM32_RCC_IC2CFGR_OFFSET      0x0104  /* IC2 configuration register */
#define STM32_RCC_IC3CFGR_OFFSET      0x0108  /* IC3 configuration register */
#define STM32_RCC_IC4CFGR_OFFSET      0x010c  /* IC4 configuration register */
#define STM32_RCC_IC5CFGR_OFFSET      0x0110  /* IC5 configuration register */
#define STM32_RCC_IC6CFGR_OFFSET      0x0114  /* IC6 configuration register */
#define STM32_RCC_IC7CFGR_OFFSET      0x0118  /* IC7 configuration register */
#define STM32_RCC_IC8CFGR_OFFSET      0x011c  /* IC8 configuration register */
#define STM32_RCC_IC9CFGR_OFFSET      0x0120  /* IC9 configuration register */
#define STM32_RCC_IC10CFGR_OFFSET     0x0124  /* IC10 configuration register */
#define STM32_RCC_IC11CFGR_OFFSET     0x0128  /* IC11 configuration register */
#define STM32_RCC_IC12CFGR_OFFSET     0x012c  /* IC12 configuration register */
#define STM32_RCC_IC13CFGR_OFFSET     0x0130  /* IC13 configuration register */
#define STM32_RCC_IC14CFGR_OFFSET     0x0134  /* IC14 configuration register */
#define STM32_RCC_IC15CFGR_OFFSET     0x0138  /* IC15 configuration register */
#define STM32_RCC_IC16CFGR_OFFSET     0x013c  /* IC16 configuration register */
#define STM32_RCC_IC17CFGR_OFFSET     0x0140  /* IC17 configuration register */
#define STM32_RCC_IC18CFGR_OFFSET     0x0144  /* IC18 configuration register */
#define STM32_RCC_IC19CFGR_OFFSET     0x0148  /* IC19 configuration register */
#define STM32_RCC_IC20CFGR_OFFSET     0x014c  /* IC20 configuration register */

/* Misc enable registers (before clock enable registers) */

#define STM32_RCC_DIVENR_OFFSET       0x0240  /* IC dividers enable register */
#define STM32_RCC_BUSENR_OFFSET       0x0244  /* Embedded buses enable register */
#define STM32_RCC_MISCENR_OFFSET      0x0248  /* Miscellaneous enable register */
#define STM32_RCC_MEMENR_OFFSET       0x024c  /* Embedded memories enable register */

/* Embedded memories enable SET register (write-1-to-set, at MEMENR + 0x800) */

#define STM32_RCC_MEMENSR_OFFSET      0x0a4c  /* Embedded memories enable SET register */

/* Clock enable registers (read/write) */

#define STM32_RCC_AHB1ENR_OFFSET      0x0250  /* AHB1 peripheral clock enable register */
#define STM32_RCC_AHB2ENR_OFFSET      0x0254  /* AHB2 peripheral clock enable register */
#define STM32_RCC_AHB3ENR_OFFSET      0x0258  /* AHB3 peripheral clock enable register */
#define STM32_RCC_AHB4ENR_OFFSET      0x025c  /* AHB4 peripheral clock enable register */
#define STM32_RCC_AHB5ENR_OFFSET      0x0260  /* AHB5 peripheral clock enable register */
#define STM32_RCC_APB1ENR1_OFFSET     0x0264  /* APB1 peripheral clock enable register 1 */
#define STM32_RCC_APB1ENR2_OFFSET     0x0268  /* APB1 peripheral clock enable register 2 */
#define STM32_RCC_APB2ENR_OFFSET      0x026c  /* APB2 peripheral clock enable register */
#define STM32_RCC_APB4ENR_OFFSET      0x0274  /* APB4 peripheral clock enable register 1 */
#define STM32_RCC_APB5ENR_OFFSET      0x027c  /* APB5 peripheral clock enable register */

/* Clock enable SET registers (write-1-to-set, at ENR offset + 0x800) */

#define STM32_RCC_AHB1ENSR_OFFSET     0x0a50  /* AHB1 clock enable SET register */
#define STM32_RCC_AHB2ENSR_OFFSET     0x0a54  /* AHB2 clock enable SET register */
#define STM32_RCC_AHB3ENSR_OFFSET     0x0a58  /* AHB3 clock enable SET register */
#define STM32_RCC_AHB4ENSR_OFFSET     0x0a5c  /* AHB4 clock enable SET register */
#define STM32_RCC_AHB5ENSR_OFFSET     0x0a60  /* AHB5 clock enable SET register */
#define STM32_RCC_APB1ENSR1_OFFSET    0x0a64  /* APB1 clock enable SET register 1 */
#define STM32_RCC_APB1ENSR2_OFFSET    0x0a68  /* APB1 clock enable SET register 2 */
#define STM32_RCC_APB2ENSR_OFFSET     0x0a6c  /* APB2 clock enable SET register */
#define STM32_RCC_APB4ENSR_OFFSET     0x0a74  /* APB4 clock enable SET register 1 */
#define STM32_RCC_APB5ENSR_OFFSET     0x0a7c  /* APB5 clock enable SET register */

/* Clock enable CLEAR registers (write-1-to-clear, at ENR offset + 0x1000) */

#define STM32_RCC_AHB1ENCR_OFFSET     0x1250  /* AHB1 clock enable CLEAR register */
#define STM32_RCC_AHB2ENCR_OFFSET     0x1254  /* AHB2 clock enable CLEAR register */
#define STM32_RCC_AHB3ENCR_OFFSET     0x1258  /* AHB3 clock enable CLEAR register */
#define STM32_RCC_AHB4ENCR_OFFSET     0x125c  /* AHB4 clock enable CLEAR register */
#define STM32_RCC_AHB5ENCR_OFFSET     0x1260  /* AHB5 clock enable CLEAR register */
#define STM32_RCC_APB1ENCR1_OFFSET    0x1264  /* APB1 clock enable CLEAR register 1 */
#define STM32_RCC_APB1ENCR2_OFFSET    0x1268  /* APB1 clock enable CLEAR register 2 */
#define STM32_RCC_APB2ENCR_OFFSET     0x126c  /* APB2 clock enable CLEAR register */
#define STM32_RCC_APB4ENCR_OFFSET     0x1274  /* APB4 clock enable CLEAR register 1 */
#define STM32_RCC_APB5ENCR_OFFSET     0x127c  /* APB5 clock enable CLEAR register */

/* RCC internal RIF security registers (per-bus access control) */

#define STM32_RCC_SECCFGR0_OFFSET     0x0780  /* Security config 0 (oscillators) */
#define STM32_RCC_SECCFGR1_OFFSET     0x0790  /* Security config 1 (PLLs) */
#define STM32_RCC_SECCFGR2_OFFSET     0x07a0  /* Security config 2 (dividers) */
#define STM32_RCC_SECCFGR3_OFFSET     0x07b0  /* Security config 3 (system) */
#define STM32_RCC_SECCFGR4_OFFSET     0x07c0  /* Security config 4 (bus clocks) */
#define STM32_RCC_PRIVCFGR4_OFFSET    0x07c4  /* Privilege config 4 (bus clocks) */
#define STM32_RCC_LOCKCFGR4_OFFSET    0x07c8  /* Lock config 4 (bus clocks) */
#define STM32_RCC_PUBCFGR4_OFFSET     0x07cc  /* Public config 4 (bus clocks) */

/* Register Addresses *******************************************************/

#define STM32_RCC_CR          (STM32_RCC_BASE + STM32_RCC_CR_OFFSET)
#define STM32_RCC_SR          (STM32_RCC_BASE + STM32_RCC_SR_OFFSET)
#define STM32_RCC_CFGR1       (STM32_RCC_BASE + STM32_RCC_CFGR1_OFFSET)
#define STM32_RCC_CFGR2       (STM32_RCC_BASE + STM32_RCC_CFGR2_OFFSET)
#define STM32_RCC_HSICFGR     (STM32_RCC_BASE + STM32_RCC_HSICFGR_OFFSET)

#define STM32_RCC_PLL1CFGR1   (STM32_RCC_BASE + STM32_RCC_PLL1CFGR1_OFFSET)
#define STM32_RCC_PLL1CFGR2   (STM32_RCC_BASE + STM32_RCC_PLL1CFGR2_OFFSET)
#define STM32_RCC_PLL1CFGR3   (STM32_RCC_BASE + STM32_RCC_PLL1CFGR3_OFFSET)
#define STM32_RCC_PLL1CFGR4   (STM32_RCC_BASE + STM32_RCC_PLL1CFGR4_OFFSET)
#define STM32_RCC_PLL1CFGR5   (STM32_RCC_BASE + STM32_RCC_PLL1CFGR5_OFFSET)
#define STM32_RCC_PLL1CFGR6   (STM32_RCC_BASE + STM32_RCC_PLL1CFGR6_OFFSET)
#define STM32_RCC_PLL1CFGR7   (STM32_RCC_BASE + STM32_RCC_PLL1CFGR7_OFFSET)

#define STM32_RCC_PLL2CFGR1   (STM32_RCC_BASE + STM32_RCC_PLL2CFGR1_OFFSET)
#define STM32_RCC_PLL2CFGR2   (STM32_RCC_BASE + STM32_RCC_PLL2CFGR2_OFFSET)
#define STM32_RCC_PLL2CFGR3   (STM32_RCC_BASE + STM32_RCC_PLL2CFGR3_OFFSET)
#define STM32_RCC_PLL2CFGR4   (STM32_RCC_BASE + STM32_RCC_PLL2CFGR4_OFFSET)
#define STM32_RCC_PLL2CFGR5   (STM32_RCC_BASE + STM32_RCC_PLL2CFGR5_OFFSET)
#define STM32_RCC_PLL2CFGR6   (STM32_RCC_BASE + STM32_RCC_PLL2CFGR6_OFFSET)
#define STM32_RCC_PLL2CFGR7   (STM32_RCC_BASE + STM32_RCC_PLL2CFGR7_OFFSET)

#define STM32_RCC_PLL3CFGR1   (STM32_RCC_BASE + STM32_RCC_PLL3CFGR1_OFFSET)
#define STM32_RCC_PLL4CFGR1   (STM32_RCC_BASE + STM32_RCC_PLL4CFGR1_OFFSET)

#define STM32_RCC_IC1CFGR     (STM32_RCC_BASE + STM32_RCC_IC1CFGR_OFFSET)
#define STM32_RCC_IC2CFGR     (STM32_RCC_BASE + STM32_RCC_IC2CFGR_OFFSET)
#define STM32_RCC_IC3CFGR     (STM32_RCC_BASE + STM32_RCC_IC3CFGR_OFFSET)
#define STM32_RCC_IC4CFGR     (STM32_RCC_BASE + STM32_RCC_IC4CFGR_OFFSET)
#define STM32_RCC_IC5CFGR     (STM32_RCC_BASE + STM32_RCC_IC5CFGR_OFFSET)
#define STM32_RCC_IC6CFGR     (STM32_RCC_BASE + STM32_RCC_IC6CFGR_OFFSET)
#define STM32_RCC_IC7CFGR     (STM32_RCC_BASE + STM32_RCC_IC7CFGR_OFFSET)
#define STM32_RCC_IC8CFGR     (STM32_RCC_BASE + STM32_RCC_IC8CFGR_OFFSET)
#define STM32_RCC_IC9CFGR     (STM32_RCC_BASE + STM32_RCC_IC9CFGR_OFFSET)
#define STM32_RCC_IC10CFGR    (STM32_RCC_BASE + STM32_RCC_IC10CFGR_OFFSET)
#define STM32_RCC_IC11CFGR    (STM32_RCC_BASE + STM32_RCC_IC11CFGR_OFFSET)
#define STM32_RCC_IC12CFGR    (STM32_RCC_BASE + STM32_RCC_IC12CFGR_OFFSET)
#define STM32_RCC_IC13CFGR    (STM32_RCC_BASE + STM32_RCC_IC13CFGR_OFFSET)
#define STM32_RCC_IC14CFGR    (STM32_RCC_BASE + STM32_RCC_IC14CFGR_OFFSET)
#define STM32_RCC_IC15CFGR    (STM32_RCC_BASE + STM32_RCC_IC15CFGR_OFFSET)
#define STM32_RCC_IC16CFGR    (STM32_RCC_BASE + STM32_RCC_IC16CFGR_OFFSET)
#define STM32_RCC_IC17CFGR    (STM32_RCC_BASE + STM32_RCC_IC17CFGR_OFFSET)
#define STM32_RCC_IC18CFGR    (STM32_RCC_BASE + STM32_RCC_IC18CFGR_OFFSET)
#define STM32_RCC_IC19CFGR    (STM32_RCC_BASE + STM32_RCC_IC19CFGR_OFFSET)
#define STM32_RCC_IC20CFGR    (STM32_RCC_BASE + STM32_RCC_IC20CFGR_OFFSET)

#define STM32_RCC_MEMENR      (STM32_RCC_BASE + STM32_RCC_MEMENR_OFFSET)
#define STM32_RCC_MEMENSR     (STM32_RCC_BASE + STM32_RCC_MEMENSR_OFFSET)

#define STM32_RCC_AHB1ENR     (STM32_RCC_BASE + STM32_RCC_AHB1ENR_OFFSET)
#define STM32_RCC_AHB2ENR     (STM32_RCC_BASE + STM32_RCC_AHB2ENR_OFFSET)
#define STM32_RCC_AHB3ENR     (STM32_RCC_BASE + STM32_RCC_AHB3ENR_OFFSET)
#define STM32_RCC_AHB4ENR     (STM32_RCC_BASE + STM32_RCC_AHB4ENR_OFFSET)
#define STM32_RCC_AHB5ENR     (STM32_RCC_BASE + STM32_RCC_AHB5ENR_OFFSET)
#define STM32_RCC_APB1ENR1    (STM32_RCC_BASE + STM32_RCC_APB1ENR1_OFFSET)
#define STM32_RCC_APB1ENR2    (STM32_RCC_BASE + STM32_RCC_APB1ENR2_OFFSET)
#define STM32_RCC_APB2ENR     (STM32_RCC_BASE + STM32_RCC_APB2ENR_OFFSET)
#define STM32_RCC_APB4ENR     (STM32_RCC_BASE + STM32_RCC_APB4ENR_OFFSET)
#define STM32_RCC_APB5ENR     (STM32_RCC_BASE + STM32_RCC_APB5ENR_OFFSET)

/* Clock enable SET register addresses (write-1-to-set) */

#define STM32_RCC_AHB1ENSR    (STM32_RCC_BASE + STM32_RCC_AHB1ENSR_OFFSET)
#define STM32_RCC_AHB2ENSR    (STM32_RCC_BASE + STM32_RCC_AHB2ENSR_OFFSET)
#define STM32_RCC_AHB3ENSR    (STM32_RCC_BASE + STM32_RCC_AHB3ENSR_OFFSET)
#define STM32_RCC_AHB4ENSR    (STM32_RCC_BASE + STM32_RCC_AHB4ENSR_OFFSET)
#define STM32_RCC_AHB5ENSR    (STM32_RCC_BASE + STM32_RCC_AHB5ENSR_OFFSET)
#define STM32_RCC_APB1ENSR1   (STM32_RCC_BASE + STM32_RCC_APB1ENSR1_OFFSET)
#define STM32_RCC_APB1ENSR2   (STM32_RCC_BASE + STM32_RCC_APB1ENSR2_OFFSET)
#define STM32_RCC_APB2ENSR    (STM32_RCC_BASE + STM32_RCC_APB2ENSR_OFFSET)
#define STM32_RCC_APB4ENSR    (STM32_RCC_BASE + STM32_RCC_APB4ENSR_OFFSET)
#define STM32_RCC_APB5ENSR    (STM32_RCC_BASE + STM32_RCC_APB5ENSR_OFFSET)

/* Clock enable CLEAR register addresses (write-1-to-clear) */

#define STM32_RCC_AHB1ENCR    (STM32_RCC_BASE + STM32_RCC_AHB1ENCR_OFFSET)
#define STM32_RCC_AHB2ENCR    (STM32_RCC_BASE + STM32_RCC_AHB2ENCR_OFFSET)
#define STM32_RCC_AHB3ENCR    (STM32_RCC_BASE + STM32_RCC_AHB3ENCR_OFFSET)
#define STM32_RCC_AHB4ENCR    (STM32_RCC_BASE + STM32_RCC_AHB4ENCR_OFFSET)
#define STM32_RCC_AHB5ENCR    (STM32_RCC_BASE + STM32_RCC_AHB5ENCR_OFFSET)
#define STM32_RCC_APB1ENCR1   (STM32_RCC_BASE + STM32_RCC_APB1ENCR1_OFFSET)
#define STM32_RCC_APB1ENCR2   (STM32_RCC_BASE + STM32_RCC_APB1ENCR2_OFFSET)
#define STM32_RCC_APB2ENCR    (STM32_RCC_BASE + STM32_RCC_APB2ENCR_OFFSET)
#define STM32_RCC_APB4ENCR    (STM32_RCC_BASE + STM32_RCC_APB4ENCR_OFFSET)
#define STM32_RCC_APB5ENCR    (STM32_RCC_BASE + STM32_RCC_APB5ENCR_OFFSET)

/* RCC internal RIF security register addresses */

#define STM32_RCC_SECCFGR4    (STM32_RCC_BASE + STM32_RCC_SECCFGR4_OFFSET)
#define STM32_RCC_PRIVCFGR4   (STM32_RCC_BASE + STM32_RCC_PRIVCFGR4_OFFSET)
#define STM32_RCC_LOCKCFGR4   (STM32_RCC_BASE + STM32_RCC_LOCKCFGR4_OFFSET)
#define STM32_RCC_PUBCFGR4    (STM32_RCC_BASE + STM32_RCC_PUBCFGR4_OFFSET)

/* Register Bitfield Definitions ********************************************/

/* Clock Control Register (RCC_CR) */

#define RCC_CR_HSION              (1 << 0)   /* Bit 0:  HSI clock enable */
#define RCC_CR_HSIDIV_SHIFT       (3)        /* Bits 3-4: HSI clock divider */
#define RCC_CR_HSIDIV_MASK        (3 << RCC_CR_HSIDIV_SHIFT)
#  define RCC_CR_HSIDIV_1         (0 << RCC_CR_HSIDIV_SHIFT) /* HSI / 1 = 64 MHz */
#  define RCC_CR_HSIDIV_2         (1 << RCC_CR_HSIDIV_SHIFT) /* HSI / 2 = 32 MHz */
#  define RCC_CR_HSIDIV_4         (2 << RCC_CR_HSIDIV_SHIFT) /* HSI / 4 = 16 MHz */
#  define RCC_CR_HSIDIV_8         (3 << RCC_CR_HSIDIV_SHIFT) /* HSI / 8 = 8 MHz */
#define RCC_CR_CSION              (1 << 8)   /* Bit 8:  CSI clock enable */
#define RCC_CR_HSEON              (1 << 16)  /* Bit 16: HSE clock enable */
#define RCC_CR_HSEBYP             (1 << 17)  /* Bit 17: HSE clock bypass */
#define RCC_CR_PLL1ON             (1 << 24)  /* Bit 24: PLL1 enable */
#define RCC_CR_PLL2ON             (1 << 25)  /* Bit 25: PLL2 enable */
#define RCC_CR_PLL3ON             (1 << 26)  /* Bit 26: PLL3 enable */
#define RCC_CR_PLL4ON             (1 << 27)  /* Bit 27: PLL4 enable */

/* Clock Status Register (RCC_SR) */

#define RCC_SR_HSIRDY             (1 << 0)   /* Bit 0:  HSI clock ready flag */
#define RCC_SR_CSIRDY             (1 << 8)   /* Bit 8:  CSI clock ready flag */
#define RCC_SR_HSERDY             (1 << 16)  /* Bit 16: HSE clock ready flag */
#define RCC_SR_PLL1RDY            (1 << 24)  /* Bit 24: PLL1 clock ready flag */
#define RCC_SR_PLL2RDY            (1 << 25)  /* Bit 25: PLL2 clock ready flag */
#define RCC_SR_PLL3RDY            (1 << 26)  /* Bit 26: PLL3 clock ready flag */
#define RCC_SR_PLL4RDY            (1 << 27)  /* Bit 27: PLL4 clock ready flag */

/* Clock Configuration Register 1 (RCC_CFGR1) */

#define RCC_CFGR1_SW_SHIFT        (0)        /* Bits 0-2: System clock switch */
#define RCC_CFGR1_SW_MASK         (7 << RCC_CFGR1_SW_SHIFT)
#  define RCC_CFGR1_SW_HSI        (0 << RCC_CFGR1_SW_SHIFT) /* HSI selected as system clock */
#  define RCC_CFGR1_SW_CSI        (1 << RCC_CFGR1_SW_SHIFT) /* CSI selected as system clock */
#  define RCC_CFGR1_SW_HSE        (2 << RCC_CFGR1_SW_SHIFT) /* HSE selected as system clock */
#  define RCC_CFGR1_SW_IC1        (3 << RCC_CFGR1_SW_SHIFT) /* IC1 (PLL output) selected as system clock */

#define RCC_CFGR1_SWS_SHIFT       (4)        /* Bits 4-6: System clock switch status */
#define RCC_CFGR1_SWS_MASK        (7 << RCC_CFGR1_SWS_SHIFT)
#  define RCC_CFGR1_SWS_HSI       (0 << RCC_CFGR1_SWS_SHIFT) /* HSI oscillator used as system clock */
#  define RCC_CFGR1_SWS_CSI       (1 << RCC_CFGR1_SWS_SHIFT) /* CSI oscillator used as system clock */
#  define RCC_CFGR1_SWS_HSE       (2 << RCC_CFGR1_SWS_SHIFT) /* HSE oscillator used as system clock */
#  define RCC_CFGR1_SWS_IC1       (3 << RCC_CFGR1_SWS_SHIFT) /* IC1 (PLL output) used as system clock */

/* AHB3 Peripheral Clock Enable Register (RCC_AHB3ENR) */

#define RCC_AHB3ENR_RIFSCEN       (1 << 9)   /* Bit 9:  RIFSC clock enable */

/* RCC_SECCFGR4 - Bus security configuration (bit = 1 means secure-only) */

#define RCC_SECCFGR4_AHB4SEC      (1 << 6)   /* Bit 6:  AHB4 secure */
#define RCC_SECCFGR4_APB1SEC      (1 << 8)   /* Bit 8:  APB1 secure */
#define RCC_SECCFGR4_APB2SEC      (1 << 9)   /* Bit 9:  APB2 secure */

/* RCC_PUBCFGR4 - Bus public configuration (bit = 1 means CID-agnostic) */

#define RCC_PUBCFGR4_ALL          0x00003fff  /* All bus bits */

/* AHB4 Peripheral Clock Enable Register (RCC_AHB4ENR) - GPIO clocks */

#define RCC_AHB4ENR_GPIOAEN       (1 << 0)   /* Bit 0:  IO port A clock enable */
#define RCC_AHB4ENR_GPIOBEN       (1 << 1)   /* Bit 1:  IO port B clock enable */
#define RCC_AHB4ENR_GPIOCEN       (1 << 2)   /* Bit 2:  IO port C clock enable */
#define RCC_AHB4ENR_GPIODEN       (1 << 3)   /* Bit 3:  IO port D clock enable */
#define RCC_AHB4ENR_GPIOEEN       (1 << 4)   /* Bit 4:  IO port E clock enable */
#define RCC_AHB4ENR_GPIOFEN       (1 << 5)   /* Bit 5:  IO port F clock enable */
#define RCC_AHB4ENR_GPIOGEN       (1 << 6)   /* Bit 6:  IO port G clock enable */
#define RCC_AHB4ENR_GPIOHEN       (1 << 7)   /* Bit 7:  IO port H clock enable */
#define RCC_AHB4ENR_GPIONEN       (1 << 13)  /* Bit 13: IO port N clock enable */
#define RCC_AHB4ENR_GPIOOEN       (1 << 14)  /* Bit 14: IO port O clock enable */
#define RCC_AHB4ENR_GPIOPEN       (1 << 15)  /* Bit 15: IO port P clock enable */
#define RCC_AHB4ENR_GPIOQEN       (1 << 16)  /* Bit 16: IO port Q clock enable */
#define RCC_AHB4ENR_PWREN         (1 << 18)  /* Bit 18: PWR interface clock enable */

/* APB2 Peripheral Clock Enable Register (RCC_APB2ENR) */

#define RCC_APB2ENR_TIM1EN        (1 << 0)   /* Bit 0:  TIM1 clock enable */
#define RCC_APB2ENR_TIM8EN        (1 << 1)   /* Bit 1:  TIM8 clock enable */
#define RCC_APB2ENR_USART1EN      (1 << 4)   /* Bit 4:  USART1 clock enable */
#define RCC_APB2ENR_SPI1EN        (1 << 12)  /* Bit 12: SPI1 clock enable */
#define RCC_APB2ENR_SPI4EN        (1 << 13)  /* Bit 13: SPI4 clock enable */
#define RCC_APB2ENR_TIM15EN       (1 << 16)  /* Bit 16: TIM15 clock enable */
#define RCC_APB2ENR_TIM16EN       (1 << 17)  /* Bit 17: TIM16 clock enable */
#define RCC_APB2ENR_TIM17EN       (1 << 18)  /* Bit 18: TIM17 clock enable */
#define RCC_APB2ENR_SPI5EN        (1 << 20)  /* Bit 20: SPI5 clock enable */

/* APB1 Peripheral Clock Enable Register 1 (RCC_APB1ENR1) */

#define RCC_APB1ENR1_TIM2EN       (1 << 0)   /* Bit 0:  TIM2 clock enable */
#define RCC_APB1ENR1_TIM3EN       (1 << 1)   /* Bit 1:  TIM3 clock enable */
#define RCC_APB1ENR1_TIM4EN       (1 << 2)   /* Bit 2:  TIM4 clock enable */
#define RCC_APB1ENR1_TIM5EN       (1 << 3)   /* Bit 3:  TIM5 clock enable */
#define RCC_APB1ENR1_TIM6EN       (1 << 4)   /* Bit 4:  TIM6 clock enable */
#define RCC_APB1ENR1_TIM7EN       (1 << 5)   /* Bit 5:  TIM7 clock enable */
#define RCC_APB1ENR1_SPI2EN       (1 << 14)  /* Bit 14: SPI2 clock enable */
#define RCC_APB1ENR1_SPI3EN       (1 << 15)  /* Bit 15: SPI3 clock enable */
#define RCC_APB1ENR1_USART2EN     (1 << 17)  /* Bit 17: USART2 clock enable */
#define RCC_APB1ENR1_USART3EN     (1 << 18)  /* Bit 18: USART3 clock enable */
#define RCC_APB1ENR1_UART4EN      (1 << 19)  /* Bit 19: UART4 clock enable */
#define RCC_APB1ENR1_UART5EN      (1 << 20)  /* Bit 20: UART5 clock enable */
#define RCC_APB1ENR1_I2C1EN       (1 << 21)  /* Bit 21: I2C1 clock enable */
#define RCC_APB1ENR1_I2C2EN       (1 << 22)  /* Bit 22: I2C2 clock enable */
#define RCC_APB1ENR1_I2C3EN       (1 << 23)  /* Bit 23: I2C3 clock enable */

/* APB4 Peripheral Clock Enable Register (RCC_APB4ENR) */

#define RCC_APB4ENR_SPI6EN        (1 << 1)   /* Bit 1:  SPI6 clock enable */
#define RCC_APB4ENR_I2C4EN        (1 << 2)   /* Bit 2:  I2C4 clock enable */
#define RCC_APB4ENR_LPTIM2EN      (1 << 3)   /* Bit 3:  LPTIM2 clock enable */
#define RCC_APB4ENR_LPTIM3EN      (1 << 4)   /* Bit 4:  LPTIM3 clock enable */
#define RCC_APB4ENR_LPTIM4EN      (1 << 5)   /* Bit 5:  LPTIM4 clock enable */
#define RCC_APB4ENR_LPTIM5EN      (1 << 6)   /* Bit 6:  LPTIM5 clock enable */

/* APB5 Peripheral Clock Enable Register (RCC_APB5ENR) */

#define RCC_APB5ENR_RTCEN         (1 << 0)   /* Bit 0:  RTC clock enable */

/* Embedded Memories Enable Register (RCC_MEMENR) */

#define RCC_MEMENR_AXISRAM3EN     (1 << 0)   /* Bit 0:  AXISRAM3 clock enable */
#define RCC_MEMENR_AXISRAM4EN     (1 << 1)   /* Bit 1:  AXISRAM4 clock enable */
#define RCC_MEMENR_AXISRAM5EN     (1 << 2)   /* Bit 2:  AXISRAM5 clock enable */
#define RCC_MEMENR_AXISRAM6EN     (1 << 3)   /* Bit 3:  AXISRAM6 clock enable */
#define RCC_MEMENR_AHBSRAM1EN     (1 << 4)   /* Bit 4:  AHBSRAM1 clock enable */
#define RCC_MEMENR_AHBSRAM2EN     (1 << 5)   /* Bit 5:  AHBSRAM2 clock enable */
#define RCC_MEMENR_BKPSRAMEN      (1 << 6)   /* Bit 6:  Backup SRAM clock enable */
#define RCC_MEMENR_AXISRAM1EN     (1 << 7)   /* Bit 7:  AXISRAM1 clock enable */
#define RCC_MEMENR_AXISRAM2EN     (1 << 8)   /* Bit 8:  AXISRAM2 clock enable */
#define RCC_MEMENR_FLEXRAMEN      (1 << 9)   /* Bit 9:  FLEXRAM clock enable */
#define RCC_MEMENR_CACHEAXIRAMEN  (1 << 10)  /* Bit 10: CACHEAXIRAM clock enable */
#define RCC_MEMENR_VENCRAMEN      (1 << 11)  /* Bit 11: VENCRAM clock enable */
#define RCC_MEMENR_BOOTROMEN      (1 << 12)  /* Bit 12: Boot ROM clock enable */

/* All AXISRAM banks (1-6) */

#define RCC_MEMENR_ALLAXISRAM     (RCC_MEMENR_AXISRAM1EN | RCC_MEMENR_AXISRAM2EN | \
                                   RCC_MEMENR_AXISRAM3EN | RCC_MEMENR_AXISRAM4EN | \
                                   RCC_MEMENR_AXISRAM5EN | RCC_MEMENR_AXISRAM6EN)

#endif /* __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_RCC_H */
