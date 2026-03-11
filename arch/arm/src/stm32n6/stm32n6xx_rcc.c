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
  rcc_enableahb4();
  rcc_enableapb1();
  rcc_enableapb2();
}

/****************************************************************************
 * Name: stm32_stdclockconfig
 *
 * Description:
 *   Phase 1 clock configuration: just verify HSI is ready.
 *   The STM32N6 boots with HSI at 64 MHz by default.
 *   No PLL configuration is needed for initial bring-up.
 *
 ****************************************************************************/

void stm32_stdclockconfig(void)
{
  volatile int32_t timeout;

  /* The STM32N6 boots from HSI at 64 MHz.  HSI should already be enabled
   * and ready from reset.  Just verify it.
   */

  for (timeout = HSIRDY_TIMEOUT; timeout > 0; timeout--)
    {
      /* Check if the HSIRDY flag is set in the SR register.
       * Note: STM32N6 uses RCC_SR (status register) not RCC_CR for
       * ready flags.
       */

      if ((getreg32(STM32_RCC_SR) & RCC_SR_HSIRDY) != 0)
        {
          /* HSI is ready - break out with timeout > 0 */

          break;
        }
    }

  /* If timeout expired, we are in trouble.  But there is not much we can
   * do about it - the HSI should always be available after reset.
   */

  /* Clear AHB/APB prescalers.  The boot ROM in DEV mode sets HPRE to /2
   * (CFGR2 = 0x00100000), halving HCLK to 32 MHz.  We need all buses at
   * the full 64 MHz HSI rate so BRR and SysTick calculations are correct.
   */

  putreg32(0, STM32_RCC_CFGR2);
}
