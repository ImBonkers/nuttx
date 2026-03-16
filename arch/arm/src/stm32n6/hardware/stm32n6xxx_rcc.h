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

/* Core control and status registers */

#define STM32_RCC_CR_OFFSET            0x0000  /* Clock control register */
#define STM32_RCC_SR_OFFSET            0x0004  /* Clock status register */
#define STM32_RCC_CFGR1_OFFSET         0x0020  /* Clock configuration register 1 */
#define STM32_RCC_CFGR2_OFFSET         0x0024  /* Clock configuration register 2 */
#define STM32_RCC_HSICFGR_OFFSET       0x0048  /* HSI configuration register */

/* Atomic set/clear variants of CR (offset + 0x800 / + 0x1000) */

#define STM32_RCC_CSR_OFFSET           0x0800  /* Control Set Register (write-1-to-set CR) */
#define STM32_RCC_CCR_OFFSET           0x1000  /* Control Clear Register (write-1-to-clear CR) */

/* PLL1 configuration registers (3 per PLL, then a reserved gap) */

#define STM32_RCC_PLL1CFGR1_OFFSET     0x0080  /* PLL1 configuration register 1 */
#define STM32_RCC_PLL1CFGR2_OFFSET     0x0084  /* PLL1 configuration register 2 */
#define STM32_RCC_PLL1CFGR3_OFFSET     0x0088  /* PLL1 configuration register 3 */

/* PLL2 configuration registers */

#define STM32_RCC_PLL2CFGR1_OFFSET     0x0090  /* PLL2 configuration register 1 */
#define STM32_RCC_PLL2CFGR2_OFFSET     0x0094  /* PLL2 configuration register 2 */
#define STM32_RCC_PLL2CFGR3_OFFSET     0x0098  /* PLL2 configuration register 3 */

/* PLL3 configuration registers */

#define STM32_RCC_PLL3CFGR1_OFFSET     0x00a0  /* PLL3 configuration register 1 */
#define STM32_RCC_PLL3CFGR2_OFFSET     0x00a4  /* PLL3 configuration register 2 */
#define STM32_RCC_PLL3CFGR3_OFFSET     0x00a8  /* PLL3 configuration register 3 */

/* PLL4 configuration registers */

#define STM32_RCC_PLL4CFGR1_OFFSET     0x00b0  /* PLL4 configuration register 1 */
#define STM32_RCC_PLL4CFGR2_OFFSET     0x00b4  /* PLL4 configuration register 2 */
#define STM32_RCC_PLL4CFGR3_OFFSET     0x00b8  /* PLL4 configuration register 3 */

/* IC (Interconnect Clock) configuration registers (contiguous, 4 bytes each) */

#define STM32_RCC_IC1CFGR_OFFSET       0x00c4  /* IC1 configuration register */
#define STM32_RCC_IC2CFGR_OFFSET       0x00c8  /* IC2 configuration register */
#define STM32_RCC_IC3CFGR_OFFSET       0x00cc  /* IC3 configuration register */
#define STM32_RCC_IC4CFGR_OFFSET       0x00d0  /* IC4 configuration register */
#define STM32_RCC_IC5CFGR_OFFSET       0x00d4  /* IC5 configuration register */
#define STM32_RCC_IC6CFGR_OFFSET       0x00d8  /* IC6 configuration register */
#define STM32_RCC_IC7CFGR_OFFSET       0x00dc  /* IC7 configuration register */
#define STM32_RCC_IC8CFGR_OFFSET       0x00e0  /* IC8 configuration register */
#define STM32_RCC_IC9CFGR_OFFSET       0x00e4  /* IC9 configuration register */
#define STM32_RCC_IC10CFGR_OFFSET      0x00e8  /* IC10 configuration register */
#define STM32_RCC_IC11CFGR_OFFSET      0x00ec  /* IC11 configuration register */
#define STM32_RCC_IC12CFGR_OFFSET      0x00f0  /* IC12 configuration register */
#define STM32_RCC_IC13CFGR_OFFSET      0x00f4  /* IC13 configuration register */
#define STM32_RCC_IC14CFGR_OFFSET      0x00f8  /* IC14 configuration register */
#define STM32_RCC_IC15CFGR_OFFSET      0x00fc  /* IC15 configuration register */
#define STM32_RCC_IC16CFGR_OFFSET      0x0100  /* IC16 configuration register */
#define STM32_RCC_IC17CFGR_OFFSET      0x0104  /* IC17 configuration register */
#define STM32_RCC_IC18CFGR_OFFSET      0x0108  /* IC18 configuration register */
#define STM32_RCC_IC19CFGR_OFFSET      0x010c  /* IC19 configuration register */
#define STM32_RCC_IC20CFGR_OFFSET      0x0110  /* IC20 configuration register */

/* HSE Configuration Register */

#define STM32_RCC_HSECFGR_OFFSET       0x0054  /* HSE configuration register */

/* Peripheral Reset Registers */

#define STM32_RCC_AHB1RSTR_OFFSET      0x0210  /* AHB1 peripheral reset register */
#define STM32_RCC_AHB5RSTR_OFFSET      0x0220  /* AHB5 peripheral reset register */
#define STM32_RCC_APB1RSTR1_OFFSET     0x0224  /* APB1 peripheral reset register 1 */
#define STM32_RCC_APB1RSTR2_OFFSET     0x0228  /* APB1 peripheral reset register 2 */
#define STM32_RCC_APB2RSTR_OFFSET      0x022c  /* APB2 peripheral reset register */

/* Atomic SET variants of reset registers (offset + 0x800) */

#define STM32_RCC_AHB1RSTSR_OFFSET     0x0a10  /* AHB1 reset SET register */
#define STM32_RCC_AHB5RSTSR_OFFSET     0x0a20  /* AHB5 reset SET register */
#define STM32_RCC_APB1RSTSR1_OFFSET    0x0a24  /* APB1 reset SET register 1 */
#define STM32_RCC_APB1RSTSR2_OFFSET    0x0a28  /* APB1 reset SET register 2 */
#define STM32_RCC_APB2RSTSR_OFFSET     0x0a2c  /* APB2 reset SET register */

/* Atomic CLEAR variants of reset registers (offset + 0x1000) */

#define STM32_RCC_AHB1RSTCR_OFFSET     0x1210  /* AHB1 reset CLEAR register */
#define STM32_RCC_AHB5RSTCR_OFFSET     0x1220  /* AHB5 reset CLEAR register */
#define STM32_RCC_APB1RSTCR1_OFFSET    0x1224  /* APB1 reset CLEAR register 1 */
#define STM32_RCC_APB1RSTCR2_OFFSET    0x1228  /* APB1 reset CLEAR register 2 */
#define STM32_RCC_APB2RSTCR_OFFSET     0x122c  /* APB2 reset CLEAR register */

/* Misc enable registers (before clock enable registers) */

#define STM32_RCC_DIVENR_OFFSET        0x0240  /* IC dividers enable register */
#define STM32_RCC_BUSENR_OFFSET        0x0244  /* Embedded buses enable register */
#define STM32_RCC_MISCENR_OFFSET       0x0248  /* Miscellaneous enable register */
#define STM32_RCC_MEMENR_OFFSET        0x024c  /* Embedded memories enable register */

/* Atomic SET variants of misc enable registers (offset + 0x800) */

#define STM32_RCC_DIVENSR_OFFSET       0x0a40  /* IC dividers enable SET register */
#define STM32_RCC_BUSENSR_OFFSET       0x0a44  /* Embedded buses enable SET register */
#define STM32_RCC_MEMENSR_OFFSET       0x0a4c  /* Embedded memories enable SET register */

/* Clock enable registers (read/write) */

#define STM32_RCC_AHB1ENR_OFFSET       0x0250  /* AHB1 peripheral clock enable register */
#define STM32_RCC_AHB2ENR_OFFSET       0x0254  /* AHB2 peripheral clock enable register */
#define STM32_RCC_AHB3ENR_OFFSET       0x0258  /* AHB3 peripheral clock enable register */
#define STM32_RCC_AHB4ENR_OFFSET       0x025c  /* AHB4 peripheral clock enable register */
#define STM32_RCC_AHB5ENR_OFFSET       0x0260  /* AHB5 peripheral clock enable register */
#define STM32_RCC_APB1ENR1_OFFSET      0x0264  /* APB1 peripheral clock enable register 1 */
#define STM32_RCC_APB1ENR2_OFFSET      0x0268  /* APB1 peripheral clock enable register 2 */
#define STM32_RCC_APB2ENR_OFFSET       0x026c  /* APB2 peripheral clock enable register */
#define STM32_RCC_APB4ENR_OFFSET       0x0274  /* APB4 peripheral clock enable register 1 */
#define STM32_RCC_APB5ENR_OFFSET       0x027c  /* APB5 peripheral clock enable register */

/* Clock enable SET registers (write-1-to-set, at ENR offset + 0x800) */

#define STM32_RCC_AHB1ENSR_OFFSET      0x0a50  /* AHB1 clock enable SET register */
#define STM32_RCC_AHB2ENSR_OFFSET      0x0a54  /* AHB2 clock enable SET register */
#define STM32_RCC_AHB3ENSR_OFFSET      0x0a58  /* AHB3 clock enable SET register */
#define STM32_RCC_AHB4ENSR_OFFSET      0x0a5c  /* AHB4 clock enable SET register */
#define STM32_RCC_AHB5ENSR_OFFSET      0x0a60  /* AHB5 clock enable SET register */
#define STM32_RCC_APB1ENSR1_OFFSET     0x0a64  /* APB1 clock enable SET register 1 */
#define STM32_RCC_APB1ENSR2_OFFSET     0x0a68  /* APB1 clock enable SET register 2 */
#define STM32_RCC_APB2ENSR_OFFSET      0x0a6c  /* APB2 clock enable SET register */
#define STM32_RCC_APB4ENSR_OFFSET      0x0a74  /* APB4 clock enable SET register 1 */
#define STM32_RCC_APB5ENSR_OFFSET      0x0a7c  /* APB5 clock enable SET register */

/* Clock enable CLEAR registers (write-1-to-clear, at ENR offset + 0x1000) */

#define STM32_RCC_AHB1ENCR_OFFSET      0x1250  /* AHB1 clock enable CLEAR register */
#define STM32_RCC_AHB2ENCR_OFFSET      0x1254  /* AHB2 clock enable CLEAR register */
#define STM32_RCC_AHB3ENCR_OFFSET      0x1258  /* AHB3 clock enable CLEAR register */
#define STM32_RCC_AHB4ENCR_OFFSET      0x125c  /* AHB4 clock enable CLEAR register */
#define STM32_RCC_AHB5ENCR_OFFSET      0x1260  /* AHB5 clock enable CLEAR register */
#define STM32_RCC_APB1ENCR1_OFFSET     0x1264  /* APB1 clock enable CLEAR register 1 */
#define STM32_RCC_APB1ENCR2_OFFSET     0x1268  /* APB1 clock enable CLEAR register 2 */
#define STM32_RCC_APB2ENCR_OFFSET      0x126c  /* APB2 clock enable CLEAR register */
#define STM32_RCC_APB4ENCR_OFFSET      0x1274  /* APB4 clock enable CLEAR register 1 */
#define STM32_RCC_APB5ENCR_OFFSET      0x127c  /* APB5 clock enable CLEAR register */

/* Kernel Clock Configuration Registers */

#define STM32_RCC_CCIPR1_OFFSET        0x0144  /* Kernel clock config register 1 (ADC) */
#define STM32_RCC_CCIPR6_OFFSET        0x0158  /* Kernel clock config register 6 (USB PHY) */
#define STM32_RCC_CCIPR7_OFFSET        0x015c  /* Kernel clock config register 7 (RTC) */
#define STM32_RCC_CCIPR9_OFFSET        0x0164  /* Kernel clock config register 9 (SPI) */

/* Backup Domain Control Register */

#define STM32_RCC_BDCR_OFFSET          0x002c  /* Backup domain control register */

/* Sleep (LPEN) enable SET registers */

#define STM32_RCC_AHB3LPENSR_OFFSET    0x0a98  /* AHB3 sleep enable SET register */

/* RCC internal RIF security registers (per-bus access control) */

#define STM32_RCC_SECCFGR0_OFFSET      0x0780  /* Security config 0 (oscillators) */
#define STM32_RCC_SECCFGR1_OFFSET      0x0790  /* Security config 1 (PLLs) */
#define STM32_RCC_SECCFGR2_OFFSET      0x07a0  /* Security config 2 (dividers) */
#define STM32_RCC_SECCFGR3_OFFSET      0x07b0  /* Security config 3 (system) */
#define STM32_RCC_SECCFGR4_OFFSET      0x07c0  /* Security config 4 (bus clocks) */
#define STM32_RCC_PRIVCFGR4_OFFSET     0x07c4  /* Privilege config 4 (bus clocks) */
#define STM32_RCC_LOCKCFGR4_OFFSET     0x07c8  /* Lock config 4 (bus clocks) */
#define STM32_RCC_PUBCFGR4_OFFSET      0x07cc  /* Public config 4 (bus clocks) */

/* Register Addresses *******************************************************/

#define STM32_RCC_CR           (STM32_RCC_BASE + STM32_RCC_CR_OFFSET)
#define STM32_RCC_SR           (STM32_RCC_BASE + STM32_RCC_SR_OFFSET)
#define STM32_RCC_CFGR1        (STM32_RCC_BASE + STM32_RCC_CFGR1_OFFSET)
#define STM32_RCC_CFGR2        (STM32_RCC_BASE + STM32_RCC_CFGR2_OFFSET)
#define STM32_RCC_HSICFGR      (STM32_RCC_BASE + STM32_RCC_HSICFGR_OFFSET)

#define STM32_RCC_HSECFGR      (STM32_RCC_BASE + STM32_RCC_HSECFGR_OFFSET)
#define STM32_RCC_CSR          (STM32_RCC_BASE + STM32_RCC_CSR_OFFSET)
#define STM32_RCC_CCR          (STM32_RCC_BASE + STM32_RCC_CCR_OFFSET)

#define STM32_RCC_PLL1CFGR1    (STM32_RCC_BASE + STM32_RCC_PLL1CFGR1_OFFSET)
#define STM32_RCC_PLL1CFGR2    (STM32_RCC_BASE + STM32_RCC_PLL1CFGR2_OFFSET)
#define STM32_RCC_PLL1CFGR3    (STM32_RCC_BASE + STM32_RCC_PLL1CFGR3_OFFSET)

#define STM32_RCC_PLL2CFGR1    (STM32_RCC_BASE + STM32_RCC_PLL2CFGR1_OFFSET)
#define STM32_RCC_PLL2CFGR2    (STM32_RCC_BASE + STM32_RCC_PLL2CFGR2_OFFSET)
#define STM32_RCC_PLL2CFGR3    (STM32_RCC_BASE + STM32_RCC_PLL2CFGR3_OFFSET)

#define STM32_RCC_PLL3CFGR1    (STM32_RCC_BASE + STM32_RCC_PLL3CFGR1_OFFSET)
#define STM32_RCC_PLL4CFGR1    (STM32_RCC_BASE + STM32_RCC_PLL4CFGR1_OFFSET)

#define STM32_RCC_IC1CFGR      (STM32_RCC_BASE + STM32_RCC_IC1CFGR_OFFSET)
#define STM32_RCC_IC2CFGR      (STM32_RCC_BASE + STM32_RCC_IC2CFGR_OFFSET)
#define STM32_RCC_IC3CFGR      (STM32_RCC_BASE + STM32_RCC_IC3CFGR_OFFSET)
#define STM32_RCC_IC4CFGR      (STM32_RCC_BASE + STM32_RCC_IC4CFGR_OFFSET)
#define STM32_RCC_IC5CFGR      (STM32_RCC_BASE + STM32_RCC_IC5CFGR_OFFSET)
#define STM32_RCC_IC6CFGR      (STM32_RCC_BASE + STM32_RCC_IC6CFGR_OFFSET)
#define STM32_RCC_IC7CFGR      (STM32_RCC_BASE + STM32_RCC_IC7CFGR_OFFSET)
#define STM32_RCC_IC8CFGR      (STM32_RCC_BASE + STM32_RCC_IC8CFGR_OFFSET)
#define STM32_RCC_IC9CFGR      (STM32_RCC_BASE + STM32_RCC_IC9CFGR_OFFSET)
#define STM32_RCC_IC10CFGR     (STM32_RCC_BASE + STM32_RCC_IC10CFGR_OFFSET)
#define STM32_RCC_IC11CFGR     (STM32_RCC_BASE + STM32_RCC_IC11CFGR_OFFSET)
#define STM32_RCC_IC12CFGR     (STM32_RCC_BASE + STM32_RCC_IC12CFGR_OFFSET)
#define STM32_RCC_IC13CFGR     (STM32_RCC_BASE + STM32_RCC_IC13CFGR_OFFSET)
#define STM32_RCC_IC14CFGR     (STM32_RCC_BASE + STM32_RCC_IC14CFGR_OFFSET)
#define STM32_RCC_IC15CFGR     (STM32_RCC_BASE + STM32_RCC_IC15CFGR_OFFSET)
#define STM32_RCC_IC16CFGR     (STM32_RCC_BASE + STM32_RCC_IC16CFGR_OFFSET)
#define STM32_RCC_IC17CFGR     (STM32_RCC_BASE + STM32_RCC_IC17CFGR_OFFSET)
#define STM32_RCC_IC18CFGR     (STM32_RCC_BASE + STM32_RCC_IC18CFGR_OFFSET)
#define STM32_RCC_IC19CFGR     (STM32_RCC_BASE + STM32_RCC_IC19CFGR_OFFSET)
#define STM32_RCC_IC20CFGR     (STM32_RCC_BASE + STM32_RCC_IC20CFGR_OFFSET)

#define STM32_RCC_AHB1RSTR     (STM32_RCC_BASE + STM32_RCC_AHB1RSTR_OFFSET)
#define STM32_RCC_AHB1RSTSR    (STM32_RCC_BASE + STM32_RCC_AHB1RSTSR_OFFSET)
#define STM32_RCC_AHB1RSTCR    (STM32_RCC_BASE + STM32_RCC_AHB1RSTCR_OFFSET)

#define STM32_RCC_AHB5RSTR     (STM32_RCC_BASE + STM32_RCC_AHB5RSTR_OFFSET)
#define STM32_RCC_AHB5RSTSR    (STM32_RCC_BASE + STM32_RCC_AHB5RSTSR_OFFSET)
#define STM32_RCC_AHB5RSTCR    (STM32_RCC_BASE + STM32_RCC_AHB5RSTCR_OFFSET)

#define STM32_RCC_APB1RSTR1    (STM32_RCC_BASE + STM32_RCC_APB1RSTR1_OFFSET)
#define STM32_RCC_APB1RSTSR1   (STM32_RCC_BASE + STM32_RCC_APB1RSTSR1_OFFSET)
#define STM32_RCC_APB1RSTCR1   (STM32_RCC_BASE + STM32_RCC_APB1RSTCR1_OFFSET)
#define STM32_RCC_APB2RSTR     (STM32_RCC_BASE + STM32_RCC_APB2RSTR_OFFSET)
#define STM32_RCC_APB2RSTSR    (STM32_RCC_BASE + STM32_RCC_APB2RSTSR_OFFSET)
#define STM32_RCC_APB2RSTCR    (STM32_RCC_BASE + STM32_RCC_APB2RSTCR_OFFSET)

#define STM32_RCC_DIVENR       (STM32_RCC_BASE + STM32_RCC_DIVENR_OFFSET)
#define STM32_RCC_DIVENSR      (STM32_RCC_BASE + STM32_RCC_DIVENSR_OFFSET)
#define STM32_RCC_BUSENR       (STM32_RCC_BASE + STM32_RCC_BUSENR_OFFSET)
#define STM32_RCC_BUSENSR      (STM32_RCC_BASE + STM32_RCC_BUSENSR_OFFSET)
#define STM32_RCC_MEMENR       (STM32_RCC_BASE + STM32_RCC_MEMENR_OFFSET)
#define STM32_RCC_MEMENSR      (STM32_RCC_BASE + STM32_RCC_MEMENSR_OFFSET)

#define STM32_RCC_AHB1ENR      (STM32_RCC_BASE + STM32_RCC_AHB1ENR_OFFSET)
#define STM32_RCC_AHB2ENR      (STM32_RCC_BASE + STM32_RCC_AHB2ENR_OFFSET)
#define STM32_RCC_AHB3ENR      (STM32_RCC_BASE + STM32_RCC_AHB3ENR_OFFSET)
#define STM32_RCC_AHB4ENR      (STM32_RCC_BASE + STM32_RCC_AHB4ENR_OFFSET)
#define STM32_RCC_AHB5ENR      (STM32_RCC_BASE + STM32_RCC_AHB5ENR_OFFSET)
#define STM32_RCC_APB1ENR1     (STM32_RCC_BASE + STM32_RCC_APB1ENR1_OFFSET)
#define STM32_RCC_APB1ENR2     (STM32_RCC_BASE + STM32_RCC_APB1ENR2_OFFSET)
#define STM32_RCC_APB2ENR      (STM32_RCC_BASE + STM32_RCC_APB2ENR_OFFSET)
#define STM32_RCC_APB4ENR      (STM32_RCC_BASE + STM32_RCC_APB4ENR_OFFSET)
#define STM32_RCC_APB5ENR      (STM32_RCC_BASE + STM32_RCC_APB5ENR_OFFSET)

/* Clock enable SET register addresses (write-1-to-set) */

#define STM32_RCC_AHB1ENSR     (STM32_RCC_BASE + STM32_RCC_AHB1ENSR_OFFSET)
#define STM32_RCC_AHB2ENSR     (STM32_RCC_BASE + STM32_RCC_AHB2ENSR_OFFSET)
#define STM32_RCC_AHB3ENSR     (STM32_RCC_BASE + STM32_RCC_AHB3ENSR_OFFSET)
#define STM32_RCC_AHB4ENSR     (STM32_RCC_BASE + STM32_RCC_AHB4ENSR_OFFSET)
#define STM32_RCC_AHB5ENSR     (STM32_RCC_BASE + STM32_RCC_AHB5ENSR_OFFSET)
#define STM32_RCC_APB1ENSR1    (STM32_RCC_BASE + STM32_RCC_APB1ENSR1_OFFSET)
#define STM32_RCC_APB1ENSR2    (STM32_RCC_BASE + STM32_RCC_APB1ENSR2_OFFSET)
#define STM32_RCC_APB2ENSR     (STM32_RCC_BASE + STM32_RCC_APB2ENSR_OFFSET)
#define STM32_RCC_APB4ENSR     (STM32_RCC_BASE + STM32_RCC_APB4ENSR_OFFSET)
#define STM32_RCC_APB5ENSR     (STM32_RCC_BASE + STM32_RCC_APB5ENSR_OFFSET)

/* Clock enable CLEAR register addresses (write-1-to-clear) */

#define STM32_RCC_AHB1ENCR     (STM32_RCC_BASE + STM32_RCC_AHB1ENCR_OFFSET)
#define STM32_RCC_AHB2ENCR     (STM32_RCC_BASE + STM32_RCC_AHB2ENCR_OFFSET)
#define STM32_RCC_AHB3ENCR     (STM32_RCC_BASE + STM32_RCC_AHB3ENCR_OFFSET)
#define STM32_RCC_AHB4ENCR     (STM32_RCC_BASE + STM32_RCC_AHB4ENCR_OFFSET)
#define STM32_RCC_AHB5ENCR     (STM32_RCC_BASE + STM32_RCC_AHB5ENCR_OFFSET)
#define STM32_RCC_APB1ENCR1    (STM32_RCC_BASE + STM32_RCC_APB1ENCR1_OFFSET)
#define STM32_RCC_APB1ENCR2    (STM32_RCC_BASE + STM32_RCC_APB1ENCR2_OFFSET)
#define STM32_RCC_APB2ENCR     (STM32_RCC_BASE + STM32_RCC_APB2ENCR_OFFSET)
#define STM32_RCC_APB4ENCR     (STM32_RCC_BASE + STM32_RCC_APB4ENCR_OFFSET)
#define STM32_RCC_APB5ENCR     (STM32_RCC_BASE + STM32_RCC_APB5ENCR_OFFSET)

/* Kernel clock configuration register addresses */

#define STM32_RCC_CCIPR1       (STM32_RCC_BASE + STM32_RCC_CCIPR1_OFFSET)
#define STM32_RCC_CCIPR6       (STM32_RCC_BASE + STM32_RCC_CCIPR6_OFFSET)
#define STM32_RCC_CCIPR7       (STM32_RCC_BASE + STM32_RCC_CCIPR7_OFFSET)
#define STM32_RCC_CCIPR9       (STM32_RCC_BASE + STM32_RCC_CCIPR9_OFFSET)
#define STM32_RCC_BDCR         (STM32_RCC_BASE + STM32_RCC_BDCR_OFFSET)

/* Sleep (LPEN) enable SET register addresses */

#define STM32_RCC_AHB3LPENSR  (STM32_RCC_BASE + STM32_RCC_AHB3LPENSR_OFFSET)

/* AHB3 Sleep Enable Register (RCC_AHB3LPENR) */

#define RCC_AHB3LPENR_RNGLPEN    (1 << 0)   /* Bit 0:  RNG sleep clock enable */

/* RCC internal RIF security register addresses */

#define STM32_RCC_SECCFGR4     (STM32_RCC_BASE + STM32_RCC_SECCFGR4_OFFSET)
#define STM32_RCC_PRIVCFGR4    (STM32_RCC_BASE + STM32_RCC_PRIVCFGR4_OFFSET)
#define STM32_RCC_LOCKCFGR4    (STM32_RCC_BASE + STM32_RCC_LOCKCFGR4_OFFSET)
#define STM32_RCC_PUBCFGR4     (STM32_RCC_BASE + STM32_RCC_PUBCFGR4_OFFSET)

/* Register Bitfield Definitions ********************************************/

/* Clock Control Register (RCC_CR) — also applies to CSR (set) and CCR (clear) */

#define RCC_CR_LSION              (1 << 0)   /* Bit 0:  LSI oscillator enable */
#define RCC_CR_LSEON              (1 << 1)   /* Bit 1:  LSE oscillator enable */
#define RCC_CR_MSION              (1 << 2)   /* Bit 2:  MSI oscillator enable */
#define RCC_CR_HSION              (1 << 3)   /* Bit 3:  HSI oscillator enable */
#define RCC_CR_HSEON              (1 << 4)   /* Bit 4:  HSE oscillator enable */
#define RCC_CR_PLL1ON             (1 << 8)   /* Bit 8:  PLL1 enable */
#define RCC_CR_PLL2ON             (1 << 9)   /* Bit 9:  PLL2 enable */
#define RCC_CR_PLL3ON             (1 << 10)  /* Bit 10: PLL3 enable */
#define RCC_CR_PLL4ON             (1 << 11)  /* Bit 11: PLL4 enable */

/* Clock Status Register (RCC_SR) — read-only mirror of ready flags */

#define RCC_SR_LSIRDY             (1 << 0)   /* Bit 0:  LSI clock ready flag */
#define RCC_SR_LSERDY             (1 << 1)   /* Bit 1:  LSE clock ready flag */
#define RCC_SR_MSIRDY             (1 << 2)   /* Bit 2:  MSI clock ready flag */
#define RCC_SR_HSIRDY             (1 << 3)   /* Bit 3:  HSI clock ready flag */
#define RCC_SR_HSERDY             (1 << 4)   /* Bit 4:  HSE clock ready flag */
#define RCC_SR_PLL1RDY            (1 << 8)   /* Bit 8:  PLL1 clock ready flag */
#define RCC_SR_PLL2RDY            (1 << 9)   /* Bit 9:  PLL2 clock ready flag */
#define RCC_SR_PLL3RDY            (1 << 10)  /* Bit 10: PLL3 clock ready flag */
#define RCC_SR_PLL4RDY            (1 << 11)  /* Bit 11: PLL4 clock ready flag */

/* Clock Configuration Register 1 (RCC_CFGR1) */

#define RCC_CFGR1_STOPWUCK        (1 << 0)   /* Bit 0: Wakeup clock after Stop */

#define RCC_CFGR1_CPUSW_SHIFT     (16)       /* Bits 17:16: CPU clock switch */
#define RCC_CFGR1_CPUSW_MASK      (3 << RCC_CFGR1_CPUSW_SHIFT)
#  define RCC_CFGR1_CPUSW_HSI     (0 << RCC_CFGR1_CPUSW_SHIFT) /* HSI selected */
#  define RCC_CFGR1_CPUSW_HSE     (1 << RCC_CFGR1_CPUSW_SHIFT) /* HSE selected */
#  define RCC_CFGR1_CPUSW_IC1     (3 << RCC_CFGR1_CPUSW_SHIFT) /* IC1 (PLL) selected */

#define RCC_CFGR1_CPUSWS_SHIFT    (20)       /* Bits 21:20: CPU clock switch status */
#define RCC_CFGR1_CPUSWS_MASK     (3 << RCC_CFGR1_CPUSWS_SHIFT)
#  define RCC_CFGR1_CPUSWS_HSI    (0 << RCC_CFGR1_CPUSWS_SHIFT)
#  define RCC_CFGR1_CPUSWS_HSE    (1 << RCC_CFGR1_CPUSWS_SHIFT)
#  define RCC_CFGR1_CPUSWS_IC1    (3 << RCC_CFGR1_CPUSWS_SHIFT)

#define RCC_CFGR1_SYSSW_SHIFT     (24)       /* Bits 25:24: System bus clock switch */
#define RCC_CFGR1_SYSSW_MASK      (3 << RCC_CFGR1_SYSSW_SHIFT)
#  define RCC_CFGR1_SYSSW_HSI     (0 << RCC_CFGR1_SYSSW_SHIFT)
#  define RCC_CFGR1_SYSSW_HSE     (1 << RCC_CFGR1_SYSSW_SHIFT)
#  define RCC_CFGR1_SYSSW_IC2_IC6_IC11 (3 << RCC_CFGR1_SYSSW_SHIFT)

#define RCC_CFGR1_SYSSWS_SHIFT    (28)       /* Bits 29:28: System bus clock switch status */
#define RCC_CFGR1_SYSSWS_MASK     (3 << RCC_CFGR1_SYSSWS_SHIFT)
#  define RCC_CFGR1_SYSSWS_HSI    (0 << RCC_CFGR1_SYSSWS_SHIFT)
#  define RCC_CFGR1_SYSSWS_HSE    (1 << RCC_CFGR1_SYSSWS_SHIFT)
#  define RCC_CFGR1_SYSSWS_IC2_IC6_IC11 (3 << RCC_CFGR1_SYSSWS_SHIFT)

/* Clock Configuration Register 2 (RCC_CFGR2) — bus prescalers */

/* Clock Configuration Register 2 (RCC_CFGR2) — bus prescalers
 *
 * NOTE: STM32N6 uses simple power-of-2 encoding for prescalers:
 *   0=/1, 1=/2, 2=/4, 3=/8, 4=/16, 5=/32, 6=/64, 7=/128
 * This differs from older STM32 families that used the "4+x" encoding.
 */

#define RCC_CFGR2_PPRE1_SHIFT     (0)        /* Bits 2:0: APB1 prescaler */
#define RCC_CFGR2_PPRE1_MASK      (7 << RCC_CFGR2_PPRE1_SHIFT)
#  define RCC_CFGR2_PPRE1_HCLK    (0 << RCC_CFGR2_PPRE1_SHIFT) /* /1 */
#  define RCC_CFGR2_PPRE1_HCLKd2  (1 << RCC_CFGR2_PPRE1_SHIFT) /* /2 */
#  define RCC_CFGR2_PPRE1_HCLKd4  (2 << RCC_CFGR2_PPRE1_SHIFT) /* /4 */
#  define RCC_CFGR2_PPRE1_HCLKd8  (3 << RCC_CFGR2_PPRE1_SHIFT) /* /8 */
#  define RCC_CFGR2_PPRE1_HCLKd16 (4 << RCC_CFGR2_PPRE1_SHIFT) /* /16 */

#define RCC_CFGR2_PPRE2_SHIFT     (4)        /* Bits 6:4: APB2 prescaler */
#define RCC_CFGR2_PPRE2_MASK      (7 << RCC_CFGR2_PPRE2_SHIFT)
#  define RCC_CFGR2_PPRE2_HCLK    (0 << RCC_CFGR2_PPRE2_SHIFT) /* /1 */
#  define RCC_CFGR2_PPRE2_HCLKd2  (1 << RCC_CFGR2_PPRE2_SHIFT) /* /2 */
#  define RCC_CFGR2_PPRE2_HCLKd4  (2 << RCC_CFGR2_PPRE2_SHIFT) /* /4 */
#  define RCC_CFGR2_PPRE2_HCLKd8  (3 << RCC_CFGR2_PPRE2_SHIFT) /* /8 */
#  define RCC_CFGR2_PPRE2_HCLKd16 (4 << RCC_CFGR2_PPRE2_SHIFT) /* /16 */

#define RCC_CFGR2_PPRE4_SHIFT     (12)       /* Bits 14:12: APB4 prescaler */
#define RCC_CFGR2_PPRE4_MASK      (7 << RCC_CFGR2_PPRE4_SHIFT)

#define RCC_CFGR2_PPRE5_SHIFT     (16)       /* Bits 18:16: APB5 prescaler */
#define RCC_CFGR2_PPRE5_MASK      (7 << RCC_CFGR2_PPRE5_SHIFT)

#define RCC_CFGR2_HPRE_SHIFT      (20)       /* Bits 22:20: AHB prescaler */
#define RCC_CFGR2_HPRE_MASK       (7 << RCC_CFGR2_HPRE_SHIFT)
#  define RCC_CFGR2_HPRE_SYSCLK    (0 << RCC_CFGR2_HPRE_SHIFT) /* /1 */
#  define RCC_CFGR2_HPRE_SYSCLKd2  (1 << RCC_CFGR2_HPRE_SHIFT) /* /2 */
#  define RCC_CFGR2_HPRE_SYSCLKd4  (2 << RCC_CFGR2_HPRE_SHIFT) /* /4 */
#  define RCC_CFGR2_HPRE_SYSCLKd8  (3 << RCC_CFGR2_HPRE_SHIFT) /* /8 */
#  define RCC_CFGR2_HPRE_SYSCLKd16 (4 << RCC_CFGR2_HPRE_SHIFT) /* /16 */

/* HSI Configuration Register (RCC_HSICFGR) */

#define RCC_HSICFGR_HSIDIV_SHIFT  (7)        /* Bits 8:7: HSI divider */
#define RCC_HSICFGR_HSIDIV_MASK   (3 << RCC_HSICFGR_HSIDIV_SHIFT)
#  define RCC_HSICFGR_HSIDIV_1    (0 << RCC_HSICFGR_HSIDIV_SHIFT) /* /1 = 64 MHz */
#  define RCC_HSICFGR_HSIDIV_2    (1 << RCC_HSICFGR_HSIDIV_SHIFT) /* /2 = 32 MHz */
#  define RCC_HSICFGR_HSIDIV_4    (2 << RCC_HSICFGR_HSIDIV_SHIFT) /* /4 = 16 MHz */
#  define RCC_HSICFGR_HSIDIV_8    (3 << RCC_HSICFGR_HSIDIV_SHIFT) /* /8 = 8 MHz */

/* PLL1 Configuration Register 1 (RCC_PLL1CFGR1) */

#define RCC_PLL1CFGR1_DIVN_SHIFT  (8)        /* Bits 19:8: PLL1 VCO multiplier (N) */
#define RCC_PLL1CFGR1_DIVN_MASK   (0xfff << RCC_PLL1CFGR1_DIVN_SHIFT)

#define RCC_PLL1CFGR1_DIVM_SHIFT  (20)       /* Bits 25:20: PLL1 input divider (M) */
#define RCC_PLL1CFGR1_DIVM_MASK   (0x3f << RCC_PLL1CFGR1_DIVM_SHIFT)

#define RCC_PLL1CFGR1_BYP         (1 << 27)  /* Bit 27: PLL1 bypass */

#define RCC_PLL1CFGR1_SEL_SHIFT   (28)       /* Bits 30:28: PLL1 source */
#define RCC_PLL1CFGR1_SEL_MASK    (7 << RCC_PLL1CFGR1_SEL_SHIFT)
#  define RCC_PLL1CFGR1_SEL_HSI   (0 << RCC_PLL1CFGR1_SEL_SHIFT)
#  define RCC_PLL1CFGR1_SEL_HSE   (1 << RCC_PLL1CFGR1_SEL_SHIFT)
#  define RCC_PLL1CFGR1_SEL_MSI   (2 << RCC_PLL1CFGR1_SEL_SHIFT)

/* PLL1 Configuration Register 3 (RCC_PLL1CFGR3) — post-dividers & SS */

#define RCC_PLL1CFGR3_MODSSDIS    (1 << 2)   /* Bit 2: Spread spectrum disable */

#define RCC_PLL1CFGR3_PDIV2_SHIFT (24)       /* Bits 26:24: Post-divider level 2 */
#define RCC_PLL1CFGR3_PDIV2_MASK  (7 << RCC_PLL1CFGR3_PDIV2_SHIFT)

#define RCC_PLL1CFGR3_PDIV1_SHIFT (27)       /* Bits 29:27: Post-divider level 1 */
#define RCC_PLL1CFGR3_PDIV1_MASK  (7 << RCC_PLL1CFGR3_PDIV1_SHIFT)

#define RCC_PLL1CFGR3_PDIVEN     (1 << 30)   /* Bit 30: Post-divider + PLL output enable */

/* IC Divider Configuration (all IC1-IC20 share the same bit layout) */

#define RCC_ICCFGR_INT_SHIFT      (16)       /* Bits 23:16: Integer divider (reg = div-1) */
#define RCC_ICCFGR_INT_MASK       (0xff << RCC_ICCFGR_INT_SHIFT)

#define RCC_ICCFGR_SEL_SHIFT      (28)       /* Bits 29:28: Source selection */
#define RCC_ICCFGR_SEL_MASK       (3 << RCC_ICCFGR_SEL_SHIFT)
#  define RCC_ICCFGR_SEL_PLL1     (0 << RCC_ICCFGR_SEL_SHIFT)
#  define RCC_ICCFGR_SEL_PLL2     (1 << RCC_ICCFGR_SEL_SHIFT)
#  define RCC_ICCFGR_SEL_PLL3     (2 << RCC_ICCFGR_SEL_SHIFT)
#  define RCC_ICCFGR_SEL_PLL4     (3 << RCC_ICCFGR_SEL_SHIFT)

/* IC Dividers Enable Register (RCC_DIVENR) — also DIVENSR (set) */

#define RCC_DIVENR_IC1EN          (1 << 0)   /* Bit 0:  IC1 enable */
#define RCC_DIVENR_IC2EN          (1 << 1)   /* Bit 1:  IC2 enable */
#define RCC_DIVENR_IC3EN          (1 << 2)   /* Bit 2:  IC3 enable */
#define RCC_DIVENR_IC4EN          (1 << 3)   /* Bit 3:  IC4 enable */
#define RCC_DIVENR_IC5EN          (1 << 4)   /* Bit 4:  IC5 enable */
#define RCC_DIVENR_IC6EN          (1 << 5)   /* Bit 5:  IC6 enable */
#define RCC_DIVENR_IC7EN          (1 << 6)   /* Bit 6:  IC7 enable */
#define RCC_DIVENR_IC8EN          (1 << 7)   /* Bit 7:  IC8 enable */
#define RCC_DIVENR_IC9EN          (1 << 8)   /* Bit 8:  IC9 enable */
#define RCC_DIVENR_IC10EN         (1 << 9)   /* Bit 9:  IC10 enable */
#define RCC_DIVENR_IC11EN         (1 << 10)  /* Bit 10: IC11 enable */
#define RCC_DIVENR_IC12EN         (1 << 11)  /* Bit 11: IC12 enable */
#define RCC_DIVENR_IC13EN         (1 << 12)  /* Bit 12: IC13 enable */
#define RCC_DIVENR_IC14EN         (1 << 13)  /* Bit 13: IC14 enable */
#define RCC_DIVENR_IC15EN         (1 << 14)  /* Bit 14: IC15 enable */
#define RCC_DIVENR_IC16EN         (1 << 15)  /* Bit 15: IC16 enable */
#define RCC_DIVENR_IC17EN         (1 << 16)  /* Bit 16: IC17 enable */
#define RCC_DIVENR_IC18EN         (1 << 17)  /* Bit 17: IC18 enable */
#define RCC_DIVENR_IC19EN         (1 << 18)  /* Bit 18: IC19 enable */
#define RCC_DIVENR_IC20EN         (1 << 19)  /* Bit 19: IC20 enable */

/* AHB1 Peripheral Clock Enable Register (RCC_AHB1ENR) */

#define RCC_AHB1ENR_GPDMA1EN      (1 << 4)   /* Bit 4:  GPDMA1 clock enable */
#define RCC_AHB1ENR_ADC12EN       (1 << 5)   /* Bit 5:  ADC1/ADC2 clock enable */

/* AHB1 Peripheral Reset Register (RCC_AHB1RSTR) */

#define RCC_AHB1RSTR_ADC12RST     (1 << 5)   /* Bit 5:  ADC1/ADC2 reset */

/* AHB3 Peripheral Clock Enable Register (RCC_AHB3ENR) */

#define RCC_AHB3ENR_RNGEN         (1 << 0)   /* Bit 0:  RNG clock enable */
#define RCC_AHB3ENR_RIFSCEN       (1 << 9)   /* Bit 9:  RIFSC clock enable */

/* Embedded Buses Enable Register (RCC_BUSENR) */

#define RCC_BUSENR_ACLKNEN        (1 << 0)   /* Bit 0:  AXI interconnect clock enable */
#define RCC_BUSENR_ACLKNCEN       (1 << 1)   /* Bit 1:  AXI interconnect companion clock */
#define RCC_BUSENR_AHBMEN         (1 << 2)   /* Bit 2:  AHB matrix clock enable */
#define RCC_BUSENR_AHB5EN         (1 << 7)   /* Bit 7:  AHB5 bus clock enable */

/* AHB5 Peripheral Clock Enable Register (RCC_AHB5ENR) */

#define RCC_AHB5ENR_HPDMA1EN      (1 << 0)   /* Bit 0:  HPDMA1 clock enable */
#define RCC_AHB5ENR_OTG1EN        (1 << 26)  /* Bit 26: USB1 OTG controller clock enable */
#define RCC_AHB5ENR_OTGPHY1EN     (1 << 27)  /* Bit 27: USB1 PHY clock enable */
#define RCC_AHB5ENR_OTGPHY2EN     (1 << 28)  /* Bit 28: USB2 PHY clock enable */
#define RCC_AHB5ENR_OTG2EN        (1 << 29)  /* Bit 29: USB2 OTG controller clock enable */
#define RCC_AHB5ENR_CACHEAXIEN    (1 << 30)  /* Bit 30: CACHEAXI clock enable */

/* HSE Configuration Register (RCC_HSECFGR) */

#define RCC_HSECFGR_HSEDIV2SEL        (1 << 6)   /* Bit 6:  HSE/2 actually divides by 2 */

/* AHB5 Peripheral Reset Register (RCC_AHB5RSTR / RSTSR / RSTCR) */

#define RCC_AHB5RSTR_OTG1PHYCTLRST   (1 << 23)  /* Bit 23: OTG1 PHY controller reset */
#define RCC_AHB5RSTR_OTG2PHYCTLRST   (1 << 24)  /* Bit 24: OTG2 PHY controller reset */
#define RCC_AHB5RSTR_OTG1RST         (1 << 26)  /* Bit 26: USB1 OTG controller reset */
#define RCC_AHB5RSTR_OTGPHY1RST      (1 << 27)  /* Bit 27: USB1 PHY reset */
#define RCC_AHB5RSTR_OTGPHY2RST      (1 << 28)  /* Bit 28: USB2 PHY reset */
#define RCC_AHB5RSTR_OTG2RST         (1 << 29)  /* Bit 29: USB2 OTG controller reset */

/* APB1 Peripheral Reset Register 1 (RCC_APB1RSTR1) */

#define RCC_APB1RSTR1_TIM2RST     (1 << 0)   /* Bit 0:  TIM2 reset */
#define RCC_APB1RSTR1_TIM3RST     (1 << 1)   /* Bit 1:  TIM3 reset */
#define RCC_APB1RSTR1_TIM4RST     (1 << 2)   /* Bit 2:  TIM4 reset */
#define RCC_APB1RSTR1_TIM5RST     (1 << 3)   /* Bit 3:  TIM5 reset */
#define RCC_APB1RSTR1_TIM6RST     (1 << 4)   /* Bit 4:  TIM6 reset */
#define RCC_APB1RSTR1_TIM7RST     (1 << 5)   /* Bit 5:  TIM7 reset */

/* APB2 Peripheral Reset Register (RCC_APB2RSTR) */

#define RCC_APB2RSTR_TIM1RST      (1 << 0)   /* Bit 0:  TIM1 reset */
#define RCC_APB2RSTR_TIM8RST      (1 << 1)   /* Bit 1:  TIM8 reset */
#define RCC_APB2RSTR_TIM15RST     (1 << 16)  /* Bit 16: TIM15 reset */
#define RCC_APB2RSTR_TIM16RST     (1 << 17)  /* Bit 17: TIM16 reset */
#define RCC_APB2RSTR_TIM17RST     (1 << 18)  /* Bit 18: TIM17 reset */

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

/* APB4 Peripheral Clock Enable Register 1 (RCC_APB4ENR) */

#define RCC_APB4ENR_SPI6EN        (1 << 5)   /* Bit 5:  SPI6 clock enable */
#define RCC_APB4ENR_I2C4EN        (1 << 7)   /* Bit 7:  I2C4 clock enable */
#define RCC_APB4ENR_LPTIM2EN      (1 << 9)   /* Bit 9:  LPTIM2 clock enable */
#define RCC_APB4ENR_LPTIM3EN      (1 << 10)  /* Bit 10: LPTIM3 clock enable */
#define RCC_APB4ENR_LPTIM4EN      (1 << 11)  /* Bit 11: LPTIM4 clock enable */
#define RCC_APB4ENR_LPTIM5EN      (1 << 12)  /* Bit 12: LPTIM5 clock enable */
#define RCC_APB4ENR1_RTCAPBEN     (1 << 17)  /* Bit 17: RTC APB interface clock enable */

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

/* RCC_CCIPR9 - SPI kernel clock source selection
 *
 * Each SPIxSEL field is 3 bits wide:
 *   0 = PCLK (APB clock)
 *   1 = CLKP
 *   2 = IC8
 *   3 = IC9
 *   4 = MSI
 *   5 = HSI
 *   6 = I2S_CKIN (external)
 *   7 = Reserved
 */

#define RCC_CCIPR9_SPI1SEL_SHIFT      (0)
#define RCC_CCIPR9_SPI1SEL_MASK       (7 << RCC_CCIPR9_SPI1SEL_SHIFT)
#  define RCC_CCIPR9_SPI1SEL_PCLK     (0 << RCC_CCIPR9_SPI1SEL_SHIFT)
#  define RCC_CCIPR9_SPI1SEL_CLKP     (1 << RCC_CCIPR9_SPI1SEL_SHIFT)
#  define RCC_CCIPR9_SPI1SEL_IC8      (2 << RCC_CCIPR9_SPI1SEL_SHIFT)
#  define RCC_CCIPR9_SPI1SEL_IC9      (3 << RCC_CCIPR9_SPI1SEL_SHIFT)
#  define RCC_CCIPR9_SPI1SEL_MSI      (4 << RCC_CCIPR9_SPI1SEL_SHIFT)
#  define RCC_CCIPR9_SPI1SEL_HSI      (5 << RCC_CCIPR9_SPI1SEL_SHIFT)

#define RCC_CCIPR9_SPI2SEL_SHIFT      (4)
#define RCC_CCIPR9_SPI2SEL_MASK       (7 << RCC_CCIPR9_SPI2SEL_SHIFT)
#  define RCC_CCIPR9_SPI2SEL_PCLK     (0 << RCC_CCIPR9_SPI2SEL_SHIFT)
#  define RCC_CCIPR9_SPI2SEL_CLKP     (1 << RCC_CCIPR9_SPI2SEL_SHIFT)
#  define RCC_CCIPR9_SPI2SEL_IC8      (2 << RCC_CCIPR9_SPI2SEL_SHIFT)
#  define RCC_CCIPR9_SPI2SEL_IC9      (3 << RCC_CCIPR9_SPI2SEL_SHIFT)
#  define RCC_CCIPR9_SPI2SEL_MSI      (4 << RCC_CCIPR9_SPI2SEL_SHIFT)
#  define RCC_CCIPR9_SPI2SEL_HSI      (5 << RCC_CCIPR9_SPI2SEL_SHIFT)

#define RCC_CCIPR9_SPI3SEL_SHIFT      (8)
#define RCC_CCIPR9_SPI3SEL_MASK       (7 << RCC_CCIPR9_SPI3SEL_SHIFT)
#  define RCC_CCIPR9_SPI3SEL_PCLK     (0 << RCC_CCIPR9_SPI3SEL_SHIFT)
#  define RCC_CCIPR9_SPI3SEL_CLKP     (1 << RCC_CCIPR9_SPI3SEL_SHIFT)
#  define RCC_CCIPR9_SPI3SEL_IC8      (2 << RCC_CCIPR9_SPI3SEL_SHIFT)
#  define RCC_CCIPR9_SPI3SEL_IC9      (3 << RCC_CCIPR9_SPI3SEL_SHIFT)
#  define RCC_CCIPR9_SPI3SEL_MSI      (4 << RCC_CCIPR9_SPI3SEL_SHIFT)
#  define RCC_CCIPR9_SPI3SEL_HSI      (5 << RCC_CCIPR9_SPI3SEL_SHIFT)

#define RCC_CCIPR9_SPI4SEL_SHIFT      (12)
#define RCC_CCIPR9_SPI4SEL_MASK       (7 << RCC_CCIPR9_SPI4SEL_SHIFT)
#  define RCC_CCIPR9_SPI4SEL_PCLK     (0 << RCC_CCIPR9_SPI4SEL_SHIFT)
#  define RCC_CCIPR9_SPI4SEL_CLKP     (1 << RCC_CCIPR9_SPI4SEL_SHIFT)
#  define RCC_CCIPR9_SPI4SEL_IC8      (2 << RCC_CCIPR9_SPI4SEL_SHIFT)
#  define RCC_CCIPR9_SPI4SEL_IC9      (3 << RCC_CCIPR9_SPI4SEL_SHIFT)
#  define RCC_CCIPR9_SPI4SEL_MSI      (4 << RCC_CCIPR9_SPI4SEL_SHIFT)
#  define RCC_CCIPR9_SPI4SEL_HSI      (5 << RCC_CCIPR9_SPI4SEL_SHIFT)

#define RCC_CCIPR9_SPI5SEL_SHIFT      (16)
#define RCC_CCIPR9_SPI5SEL_MASK       (7 << RCC_CCIPR9_SPI5SEL_SHIFT)
#  define RCC_CCIPR9_SPI5SEL_PCLK     (0 << RCC_CCIPR9_SPI5SEL_SHIFT)
#  define RCC_CCIPR9_SPI5SEL_CLKP     (1 << RCC_CCIPR9_SPI5SEL_SHIFT)
#  define RCC_CCIPR9_SPI5SEL_IC8      (2 << RCC_CCIPR9_SPI5SEL_SHIFT)
#  define RCC_CCIPR9_SPI5SEL_IC9      (3 << RCC_CCIPR9_SPI5SEL_SHIFT)
#  define RCC_CCIPR9_SPI5SEL_MSI      (4 << RCC_CCIPR9_SPI5SEL_SHIFT)
#  define RCC_CCIPR9_SPI5SEL_HSI      (5 << RCC_CCIPR9_SPI5SEL_SHIFT)

#define RCC_CCIPR9_SPI6SEL_SHIFT      (20)
#define RCC_CCIPR9_SPI6SEL_MASK       (7 << RCC_CCIPR9_SPI6SEL_SHIFT)
#  define RCC_CCIPR9_SPI6SEL_PCLK     (0 << RCC_CCIPR9_SPI6SEL_SHIFT)
#  define RCC_CCIPR9_SPI6SEL_CLKP     (1 << RCC_CCIPR9_SPI6SEL_SHIFT)
#  define RCC_CCIPR9_SPI6SEL_IC8      (2 << RCC_CCIPR9_SPI6SEL_SHIFT)
#  define RCC_CCIPR9_SPI6SEL_IC9      (3 << RCC_CCIPR9_SPI6SEL_SHIFT)
#  define RCC_CCIPR9_SPI6SEL_MSI      (4 << RCC_CCIPR9_SPI6SEL_SHIFT)
#  define RCC_CCIPR9_SPI6SEL_HSI      (5 << RCC_CCIPR9_SPI6SEL_SHIFT)

/* RCC_CCIPR6 - USB PHY clock source selection
 *
 * Two muxes per PHY:
 *   OTGPHY1SEL (bits 13:12) - kernel clock source
 *   OTGPHY1CKREFSEL (bit 16) - reference clock source
 */

#define RCC_CCIPR6_OTGPHY1SEL_SHIFT       (12)
#define RCC_CCIPR6_OTGPHY1SEL_MASK        (3 << RCC_CCIPR6_OTGPHY1SEL_SHIFT)
#  define RCC_CCIPR6_OTGPHY1SEL_HSE_DIV2  (0 << RCC_CCIPR6_OTGPHY1SEL_SHIFT) /* HSE/2 */
#  define RCC_CCIPR6_OTGPHY1SEL_CLKP      (1 << RCC_CCIPR6_OTGPHY1SEL_SHIFT) /* CLKP */
#  define RCC_CCIPR6_OTGPHY1SEL_IC15      (2 << RCC_CCIPR6_OTGPHY1SEL_SHIFT) /* IC15 */
#  define RCC_CCIPR6_OTGPHY1SEL_HSE_DIV2_OSC (3 << RCC_CCIPR6_OTGPHY1SEL_SHIFT) /* HSE/2 direct from oscillator */

#define RCC_CCIPR6_OTGPHY1CKREFSEL        (1 << 16) /* Bit 16: 0=from OTGPHY1SEL, 1=HSE/2 direct */

#define RCC_CCIPR6_OTGPHY2SEL_SHIFT       (20)
#define RCC_CCIPR6_OTGPHY2SEL_MASK        (3 << RCC_CCIPR6_OTGPHY2SEL_SHIFT)

#define RCC_CCIPR6_OTGPHY2CKREFSEL        (1 << 24) /* Bit 24: 0=from OTGPHY2SEL, 1=HSE/2 direct */

/* RCC_CCIPR1 - ADC kernel clock source selection
 *
 * ADC12SEL (bits 6:4): kernel clock for ADC1/ADC2
 *   0 = PCLK
 *   1 = CLKP
 *   2 = IC7
 *   3 = IC8
 *   4 = MSI
 *   5 = HSI
 *
 * ADCPRE (bits 15:8): ADC bus slave clock prescaler
 */

#define RCC_CCIPR1_ADC12SEL_SHIFT     (4)
#define RCC_CCIPR1_ADC12SEL_MASK      (7 << RCC_CCIPR1_ADC12SEL_SHIFT)
#  define RCC_CCIPR1_ADC12SEL_PCLK    (0 << RCC_CCIPR1_ADC12SEL_SHIFT)
#  define RCC_CCIPR1_ADC12SEL_CLKP    (1 << RCC_CCIPR1_ADC12SEL_SHIFT)
#  define RCC_CCIPR1_ADC12SEL_IC7     (2 << RCC_CCIPR1_ADC12SEL_SHIFT)
#  define RCC_CCIPR1_ADC12SEL_IC8     (3 << RCC_CCIPR1_ADC12SEL_SHIFT)
#  define RCC_CCIPR1_ADC12SEL_MSI     (4 << RCC_CCIPR1_ADC12SEL_SHIFT)
#  define RCC_CCIPR1_ADC12SEL_HSI     (5 << RCC_CCIPR1_ADC12SEL_SHIFT)

#define RCC_CCIPR1_ADCPRE_SHIFT       (8)
#define RCC_CCIPR1_ADCPRE_MASK        (0xff << RCC_CCIPR1_ADCPRE_SHIFT)

/* RCC_CCIPR7 - RTC kernel clock source selection
 *
 * RTCSEL field is bits [9:8]:
 *   00 = No clock
 *   01 = LSE
 *   10 = LSI
 *   11 = HSE/64
 */

#define RCC_CCIPR7_RTCSEL_SHIFT       (8)
#define RCC_CCIPR7_RTCSEL_MASK        (3 << RCC_CCIPR7_RTCSEL_SHIFT)
#  define RCC_CCIPR7_RTCSEL_NONE      (0 << RCC_CCIPR7_RTCSEL_SHIFT)
#  define RCC_CCIPR7_RTCSEL_LSE       (1 << RCC_CCIPR7_RTCSEL_SHIFT)
#  define RCC_CCIPR7_RTCSEL_LSI       (2 << RCC_CCIPR7_RTCSEL_SHIFT)
#  define RCC_CCIPR7_RTCSEL_HSE       (3 << RCC_CCIPR7_RTCSEL_SHIFT)

/* RCC_BDCR - Backup Domain Control Register
 *
 * Bit 31: VSWRST - Voltage switch reset (backup domain reset)
 */

#define RCC_BDCR_VSWRST               (1 << 31) /* Bit 31: Backup domain reset */

#endif /* __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_RCC_H */
