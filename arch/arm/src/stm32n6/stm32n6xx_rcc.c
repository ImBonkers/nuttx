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
#define PLL2RDY_TIMEOUT (100 * CONFIG_BOARD_LOOPSPERMSEC)
#define PLL3RDY_TIMEOUT (100 * CONFIG_BOARD_LOOPSPERMSEC)

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
 * Name: stm32_configure_pll2_pll3
 *
 * Description:
 *   Configure PLL2 and PLL3 for NPU clocks.  Called from both DEV mode
 *   (full clock config) and FSBL mode (PLL1/CFGR1 already done).
 *
 *   PLL2: HSI/8 * 125 = 1000 MHz VCO, PDIV1=1, PDIV2=1 → 1000 MHz
 *         → IC6  /1  = 1000 MHz  (NPU core clock via SYSSW)
 *
 *   PLL3: HSI/8 * 225 = 1800 MHz VCO, PDIV1=1, PDIV2=2 → 900 MHz
 *         → IC11 /1  = 900 MHz   (NPU SRAM clock via SYSSW)
 *
 ****************************************************************************/

#ifdef CONFIG_STM32N6_NPU
static void stm32_configure_pll2_pll3(void)
{
  volatile int32_t timeout;
  uint32_t regval;

  /* --- PLL2: 1000 MHz for NPU core (IC6) --- */

  /* Disable IC6 before changing its source */

  putreg32(RCC_DIVENR_IC6EN, STM32_RCC_DIVENCR);

  /* Disable PLL2 */

  putreg32(RCC_CR_PLL2ON, STM32_RCC_CCR);

  for (timeout = PLL2RDY_TIMEOUT; timeout > 0; timeout--)
    {
      if ((getreg32(STM32_RCC_SR) & RCC_SR_PLL2RDY) == 0)
        {
          break;
        }
    }

  /* Configure PLL2CFGR1: source=HSI, M=8, N=125
   *   VCO_in  = 64 MHz / 8 = 8 MHz
   *   VCO_out = 8 MHz * 125 = 1000 MHz
   */

  regval = (RCC_PLL1CFGR1_SEL_HSI)
         | (8 << RCC_PLL1CFGR1_DIVM_SHIFT)
         | (125 << RCC_PLL1CFGR1_DIVN_SHIFT);
  putreg32(regval, STM32_RCC_PLL2CFGR1);

  /* Configure PLL2CFGR3: disable SS, PDIV1=1, PDIV2=1, enable post-div */

  regval = RCC_PLL1CFGR3_MODSSDIS
         | RCC_PLL1CFGR3_PDIVEN
         | (1 << RCC_PLL1CFGR3_PDIV1_SHIFT)
         | (1 << RCC_PLL1CFGR3_PDIV2_SHIFT);
  putreg32(regval, STM32_RCC_PLL2CFGR3);

  /* Enable PLL2 */

  putreg32(RCC_CR_PLL2ON, STM32_RCC_CSR);

  for (timeout = PLL2RDY_TIMEOUT; timeout > 0; timeout--)
    {
      if ((getreg32(STM32_RCC_SR) & RCC_SR_PLL2RDY) != 0)
        {
          break;
        }
    }

  /* --- PLL3: 900 MHz for NPU SRAM (IC11) --- */

  /* Disable IC11 before changing its source */

  putreg32(RCC_DIVENR_IC11EN, STM32_RCC_DIVENCR);

  /* Disable PLL3 */

  putreg32(RCC_CR_PLL3ON, STM32_RCC_CCR);

  for (timeout = PLL3RDY_TIMEOUT; timeout > 0; timeout--)
    {
      if ((getreg32(STM32_RCC_SR) & RCC_SR_PLL3RDY) == 0)
        {
          break;
        }
    }

  /* Configure PLL3CFGR1: source=HSI, M=8, N=225
   *   VCO_in  = 64 MHz / 8 = 8 MHz
   *   VCO_out = 8 MHz * 225 = 1800 MHz
   */

  regval = (RCC_PLL1CFGR1_SEL_HSI)
         | (8 << RCC_PLL1CFGR1_DIVM_SHIFT)
         | (225 << RCC_PLL1CFGR1_DIVN_SHIFT);
  putreg32(regval, STM32_RCC_PLL3CFGR1);

  /* Configure PLL3CFGR3: disable SS, PDIV1=1, PDIV2=2, enable post-div
   *   PLL3 output = VCO / PDIV2 = 1800 / 2 = 900 MHz
   */

  regval = RCC_PLL1CFGR3_MODSSDIS
         | RCC_PLL1CFGR3_PDIVEN
         | (1 << RCC_PLL1CFGR3_PDIV1_SHIFT)
         | (2 << RCC_PLL1CFGR3_PDIV2_SHIFT);
  putreg32(regval, STM32_RCC_PLL3CFGR3);

  /* Enable PLL3 */

  putreg32(RCC_CR_PLL3ON, STM32_RCC_CSR);

  for (timeout = PLL3RDY_TIMEOUT; timeout > 0; timeout--)
    {
      if ((getreg32(STM32_RCC_SR) & RCC_SR_PLL3RDY) != 0)
        {
          break;
        }
    }

  /* Re-source IC6 to PLL2/1 (1000 MHz) and IC11 to PLL3/1 (900 MHz).
   * ICxCFGR can be modified independently of CFGR1.
   */

  putreg32(RCC_ICCFGR_SEL_PLL2 | (0 << RCC_ICCFGR_INT_SHIFT),
           STM32_RCC_IC6CFGR);   /* IC6: PLL2/1 = 1000 MHz */
  putreg32(RCC_ICCFGR_SEL_PLL3 | (0 << RCC_ICCFGR_INT_SHIFT),
           STM32_RCC_IC11CFGR);  /* IC11: PLL3/1 = 900 MHz */

  /* Re-enable IC6 and IC11 */

  putreg32(RCC_DIVENR_IC6EN | RCC_DIVENR_IC11EN, STM32_RCC_DIVENSR);

  __asm volatile ("dsb sy");
  __asm volatile ("isb sy");
}
#endif /* CONFIG_STM32N6_NPU */

/****************************************************************************
 * Name: stm32_stdclockconfig
 *
 * Description:
 *   Configure PLL1 for 600 MHz CPU clock, matching the FSBL configuration.
 *
 *   Clock tree (base, without NPU):
 *     HSI 64 MHz -> PLL1 (M=4, N=75) -> VCO 1200 MHz -> PDIV1/1 -> 1200 MHz
 *       -> IC1  /2  = 600 MHz  (CPU clock via CPUSW)
 *       -> IC2  /3  = 400 MHz  \
 *       -> IC6  /4  = 300 MHz   > System bus via SYSSW
 *       -> IC11 /3  = 400 MHz  /
 *       -> HPRE /2  = 200 MHz  (HCLK)
 *       -> PPRE1 /1 = 200 MHz  (APB1 = PCLK1)
 *       -> PPRE2 /1 = 200 MHz  (APB2 = PCLK2)
 *
 *   With CONFIG_STM32N6_NPU, IC6/IC11 are re-sourced from PLL2/PLL3:
 *       -> IC6  = PLL2/1 = 1000 MHz (NPU core)
 *       -> IC11 = PLL3/1 = 900 MHz  (NPU SRAM)
 *
 *   IMPORTANT: CFGR1 locks after the first write — CPUSW and SYSSW must
 *   be written together in a single putreg32().  CFGR2 (bus prescalers)
 *   also locks after CFGR1 is written, so it must be set first.
 *
 ****************************************************************************/

void stm32_stdclockconfig(void)
{
  volatile int32_t timeout;
  uint32_t regval;

  /* If clocks are already configured (e.g. FSBL set up PLL1 and switched
   * CPUSW to IC1), skip PLL1/CFGR1 reconfiguration.  CFGR1 locks after
   * the first write — a second write crashes the system (SRAM goes
   * offline).  However, PLL2/PLL3 for NPU still need to be configured.
   */

  regval = getreg32(STM32_RCC_CFGR1);
  if ((regval & RCC_CFGR1_CPUSWS_MASK) == RCC_CFGR1_CPUSWS_IC1 &&
      (regval & RCC_CFGR1_SYSSWS_MASK) == RCC_CFGR1_SYSSWS_IC2_IC6_IC11)
    {
#ifdef CONFIG_STM32N6_NPU
      stm32_configure_pll2_pll3();
#endif
      return;
    }

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
   *    IC1  = PLL1 / 2  = 600 MHz  (CPU)
   *    IC2  = PLL1 / 3  = 400 MHz  (SYSCLK)
   *    IC3  = PLL1 / 6  = 200 MHz  (XSPI2 kernel clock)
   *    IC6  = PLL1 / 4  = 300 MHz  (AHB — will be re-sourced if NPU)
   *    IC11 = PLL1 / 3  = 400 MHz  (APB — will be re-sourced if NPU)
   */

  putreg32(RCC_ICCFGR_SEL_PLL1 | (1 << RCC_ICCFGR_INT_SHIFT),
           STM32_RCC_IC1CFGR);   /* IC1: 1200/2 = 600 MHz */
  putreg32(RCC_ICCFGR_SEL_PLL1 | (2 << RCC_ICCFGR_INT_SHIFT),
           STM32_RCC_IC2CFGR);   /* IC2: 1200/3 = 400 MHz */
  putreg32(RCC_ICCFGR_SEL_PLL1 | (5 << RCC_ICCFGR_INT_SHIFT),
           STM32_RCC_IC3CFGR);   /* IC3: 1200/6 = 200 MHz */
  putreg32(RCC_ICCFGR_SEL_PLL1 | (3 << RCC_ICCFGR_INT_SHIFT),
           STM32_RCC_IC6CFGR);   /* IC6: 1200/4 = 300 MHz */
  putreg32(RCC_ICCFGR_SEL_PLL1 | (2 << RCC_ICCFGR_INT_SHIFT),
           STM32_RCC_IC11CFGR);  /* IC11: 1200/3 = 400 MHz */

  /* 7. Enable IC1, IC2, IC3, IC6, IC11 */

  putreg32(RCC_DIVENR_IC1EN | RCC_DIVENR_IC2EN | RCC_DIVENR_IC3EN
         | RCC_DIVENR_IC6EN | RCC_DIVENR_IC11EN,
           STM32_RCC_DIVENSR);

  /* 8. Set bus prescalers BEFORE the CFGR1 switch — CFGR2 locks after
   *    CFGR1 is written.  HPRE = /2 gives HCLK = IC6/2 = 150 MHz.
   *    PPRE1 = /1, PPRE2 = /1 → PCLK1 = PCLK2 = HCLK = 150 MHz.
   */

  putreg32(RCC_CFGR2_HPRE_SYSCLKd2, STM32_RCC_CFGR2);

  /* 9. Switch CPU and system bus clocks to PLL1-derived IC outputs.
   *    CFGR1 locks after the first write — both CPUSW and SYSSW must be
   *    written together in a single putreg32() call.
   */

  __asm volatile ("dsb sy");

  regval = getreg32(STM32_RCC_CFGR1);
  regval &= ~(RCC_CFGR1_CPUSW_MASK | RCC_CFGR1_SYSSW_MASK);
  regval |= RCC_CFGR1_CPUSW_IC1 | RCC_CFGR1_SYSSW_IC2_IC6_IC11;
  putreg32(regval, STM32_RCC_CFGR1);

  __asm volatile ("dsb sy");
  __asm volatile ("isb sy");

  /* Re-enable SRAM clocks defensively after clock domain switch */

  putreg32(RCC_MEMENR_ALLAXISRAM | RCC_MEMENR_CACHEAXIRAMEN,
           STM32_RCC_MEMENSR);

  /* Wait for switch confirmation */

  for (timeout = PLL1RDY_TIMEOUT; timeout > 0; timeout--)
    {
      if (((getreg32(STM32_RCC_CFGR1) & RCC_CFGR1_CPUSWS_MASK)
            == RCC_CFGR1_CPUSWS_IC1) &&
          ((getreg32(STM32_RCC_CFGR1) & RCC_CFGR1_SYSSWS_MASK)
            == RCC_CFGR1_SYSSWS_IC2_IC6_IC11))
        {
          break;
        }
    }

  /* 10. Configure PLL2/PLL3 for NPU if enabled.  This is done after
   *     CFGR1 is written so PLL1 is already driving the system bus.
   *     IC6/IC11 will be re-sourced from PLL2/PLL3.
   */

#ifdef CONFIG_STM32N6_NPU
  stm32_configure_pll2_pll3();
#endif
}
