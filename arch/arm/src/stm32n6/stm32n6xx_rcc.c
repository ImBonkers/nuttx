/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6xx_rcc.c
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
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <arch/stm32n6/chip.h>
#include <arch/board/board.h>

#include <assert.h>

#include "stm32_rcc.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Allow up to 100 milliseconds for the high speed clock to become ready.
 * that is a very long delay, but if the clock does not become ready we are
 * hosed anyway.
 */

#define HSIRDY_TIMEOUT (100 * CONFIG_BOARD_LOOPSPERMSEC)
#define PLL1RDY_TIMEOUT (100 * CONFIG_BOARD_LOOPSPERMSEC)

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: rcc_enableahb4
 *
 * Description:
 *   Enable selected AHB4 peripherals - primarily GPIO clocks and PWR.
 *   On STM32N6, GPIO ports A-H and N-Q are on AHB4.
 *
 ****************************************************************************/

static inline void rcc_enableahb4(void)
{
  /* Use the atomic SET register (ENSR) instead of read-modify-write on ENR.
   * The RCC's internal RIF security may block direct ENR writes but allows
   * ENSR writes.  This matches the ST HAL pattern (stm32n6xx_ll_bus.h).
   */

  putreg32(RCC_AHB4ENR_GPIOAEN
           | RCC_AHB4ENR_GPIOBEN
           | RCC_AHB4ENR_GPIOCEN
           | RCC_AHB4ENR_GPIODEN
           | RCC_AHB4ENR_GPIOEEN
           | RCC_AHB4ENR_GPIOFEN
           | RCC_AHB4ENR_GPIOGEN
           | RCC_AHB4ENR_GPIOHEN
           | RCC_AHB4ENR_GPIONEN
           | RCC_AHB4ENR_GPIOOEN
           | RCC_AHB4ENR_GPIOPEN
           | RCC_AHB4ENR_GPIOQEN
           | RCC_AHB4ENR_PWREN,
           STM32_RCC_AHB4ENSR);
}

/****************************************************************************
 * Name: rcc_enableapb1
 *
 * Description:
 *   Enable selected APB1 peripherals.
 *
 ****************************************************************************/

static inline void rcc_enableapb1(void)
{
  uint32_t regval = 0;

#ifdef CONFIG_STM32N6_USART2
  regval |= RCC_APB1ENR1_USART2EN;
#endif

#ifdef CONFIG_STM32N6_USART3
  regval |= RCC_APB1ENR1_USART3EN;
#endif

#ifdef CONFIG_STM32N6_UART4
  regval |= RCC_APB1ENR1_UART4EN;
#endif

#ifdef CONFIG_STM32N6_UART5
  regval |= RCC_APB1ENR1_UART5EN;
#endif

  if (regval != 0)
    {
      putreg32(regval, STM32_RCC_APB1ENSR1);
    }
}

/****************************************************************************
 * Name: rcc_enableapb2
 *
 * Description:
 *   Enable selected APB2 peripherals.
 *
 ****************************************************************************/

static inline void rcc_enableapb2(void)
{
  uint32_t regval = 0;

#ifdef CONFIG_STM32N6_USART1
  regval |= RCC_APB2ENR_USART1EN;
#endif

  if (regval != 0)
    {
      putreg32(regval, STM32_RCC_APB2ENSR);
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_rcc_enableperipherals
 ****************************************************************************/

void stm32_rcc_enableperipherals(void)
{
  /* Enable all AXISRAM bank clocks.  The boot ROM only enables AXISRAM1/2
   * which is sufficient for code execution, but the NuttX heap extends
   * across all SRAM banks (up to AXISRAM5 at 0x34400000+).  Without these
   * clocks, mm_initialize writing the tail node at the heap end will cause
   * an IMPRECISERR bus fault.
   */

  putreg32(RCC_MEMENR_ALLAXISRAM | RCC_MEMENR_CACHEAXIRAMEN,
           STM32_RCC_MEMENSR);

  rcc_enableahb4();
  rcc_enableapb1();
  rcc_enableapb2();
}

/****************************************************************************
 * Name: stm32_stdclockconfig
 *
 * Description:
 *   Configure PLL1 for 600 MHz CPU clock, matching the FSBL configuration.
 *
 *   Clock tree:
 *     HSI 64 MHz -> PLL1 (M=4, N=75) -> VCO 1200 MHz -> PDIV1/1 -> 1200 MHz
 *       -> IC1  /2  = 600 MHz  (CPU clock via CPUSW)
 *       -> IC2  /3  = 400 MHz  \
 *       -> IC6  /4  = 300 MHz   > System bus via SYSSW
 *       -> IC11 /3  = 400 MHz  /
 *       -> HPRE /2  = 200 MHz  (HCLK)
 *       -> PPRE1 /1 = 200 MHz  (APB1 = PCLK1)
 *       -> PPRE2 /1 = 200 MHz  (APB2 = PCLK2)
 *
 ****************************************************************************/

void stm32_stdclockconfig(void)
{
  volatile int32_t timeout;
  uint32_t regval;

  /* 1. Verify HSI is ready (already running from reset) */

  for (timeout = HSIRDY_TIMEOUT; timeout > 0; timeout--)
    {
      if ((getreg32(STM32_RCC_SR) & RCC_SR_HSIRDY) != 0)
        {
          break;
        }
    }

  /* 2. Disable PLL1 via the Control Clear Register (atomic) */

  putreg32(RCC_CR_PLL1ON, STM32_RCC_CCR);

  for (timeout = PLL1RDY_TIMEOUT; timeout > 0; timeout--)
    {
      if ((getreg32(STM32_RCC_SR) & RCC_SR_PLL1RDY) == 0)
        {
          break;
        }
    }

  /* 3. Configure PLL1CFGR1: source=HSI, M=4, N=75
   *    VCO_in  = 64 MHz / 4 = 16 MHz
   *    VCO_out = 16 MHz * 75 = 1200 MHz
   */

  regval = (RCC_PLL1CFGR1_SEL_HSI)
         | (4 << RCC_PLL1CFGR1_DIVM_SHIFT)
         | (75 << RCC_PLL1CFGR1_DIVN_SHIFT);
  putreg32(regval, STM32_RCC_PLL1CFGR1);

  /* 4. Configure PLL1CFGR3: disable SS, PDIV1=1, PDIV2=1, enable post-div */

  regval = RCC_PLL1CFGR3_MODSSDIS
         | RCC_PLL1CFGR3_PDIVEN
         | (1 << RCC_PLL1CFGR3_PDIV1_SHIFT)
         | (1 << RCC_PLL1CFGR3_PDIV2_SHIFT);
  putreg32(regval, STM32_RCC_PLL1CFGR3);

  /* 5. Enable PLL1 via the Control Set Register (atomic) */

  putreg32(RCC_CR_PLL1ON, STM32_RCC_CSR);

  for (timeout = PLL1RDY_TIMEOUT; timeout > 0; timeout--)
    {
      if ((getreg32(STM32_RCC_SR) & RCC_SR_PLL1RDY) != 0)
        {
          break;
        }
    }

  /* 6. Configure IC dividers (source=PLL1, register value = divider - 1)
   *    DEBUG: Use slow dividers first (1200/20=60MHz) to test switch mechanism.
   */

  putreg32(RCC_ICCFGR_SEL_PLL1 | (19 << RCC_ICCFGR_INT_SHIFT),
           STM32_RCC_IC1CFGR);   /* IC1: 1200/20 = 60 MHz */
  putreg32(RCC_ICCFGR_SEL_PLL1 | (19 << RCC_ICCFGR_INT_SHIFT),
           STM32_RCC_IC2CFGR);   /* IC2: 1200/20 = 60 MHz */
  putreg32(RCC_ICCFGR_SEL_PLL1 | (19 << RCC_ICCFGR_INT_SHIFT),
           STM32_RCC_IC6CFGR);   /* IC6: 1200/20 = 60 MHz */
  putreg32(RCC_ICCFGR_SEL_PLL1 | (19 << RCC_ICCFGR_INT_SHIFT),
           STM32_RCC_IC11CFGR);  /* IC11: 1200/20 = 60 MHz */

  /* 7. Enable IC1, IC2, IC6, IC11 */

  putreg32(RCC_DIVENR_IC1EN | RCC_DIVENR_IC2EN
         | RCC_DIVENR_IC6EN | RCC_DIVENR_IC11EN,
           STM32_RCC_DIVENSR);

  /* For now, stay on HSI.  The clock switch will be attempted later
   * from stm32_pll1_switch() after the UART is up for diagnostics.
   */

  putreg32(0, STM32_RCC_CFGR2);  /* Clear prescalers (HPRE=/1) for HSI */
}
