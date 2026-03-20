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

#define HSIRDY_TIMEOUT  (100 * CONFIG_BOARD_LOOPSPERMSEC)
#define PLL1RDY_TIMEOUT (100 * CONFIG_BOARD_LOOPSPERMSEC)
#define PLL2RDY_TIMEOUT (100 * CONFIG_BOARD_LOOPSPERMSEC)
#define PLL3RDY_TIMEOUT (100 * CONFIG_BOARD_LOOPSPERMSEC)
#define VOS_TIMEOUT     (10 * CONFIG_BOARD_LOOPSPERMSEC)

/* SMPS overdrive and VOS registers */

#define GPIOB_BASE_S   0x56020400
#define PWR_VOSCR      (STM32_PWR_BASE + 0x020)
#define PWR_VOSCR_VOS  (1 << 0)
#define PWR_VOSCR_RDY  (1 << 1)

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

  /* SMPS overdrive + VOS SCALE0 for high-frequency operation.
   * Required when CPU is at 800 MHz or NPU at 1 GHz.
   * At 600 MHz CPU + 900 MHz NPU, SCALE1 (default) is sufficient
   * but SMPS overdrive is still needed for NPU > 800 MHz.
   */

#ifdef CONFIG_STM32N6_CPU_800MHZ
  putreg32(RCC_AHB4ENR_GPIOBEN, STM32_RCC_AHB4ENSR);

  modifyreg32(GPIOB_BASE_S + 0x00, (3 << 24), (1 << 24));
  modifyreg32(GPIOB_BASE_S + 0x04, (1 << 12), 0);
  modifyreg32(GPIOB_BASE_S + 0x08, (3 << 24), (3 << 24));
  putreg32((1 << 12), GPIOB_BASE_S + 0x18);

  modifyreg32(PWR_VOSCR, 0, PWR_VOSCR_VOS);

  for (timeout = VOS_TIMEOUT; timeout > 0; timeout--)
    {
      if ((getreg32(PWR_VOSCR) & PWR_VOSCR_RDY) != 0)
        {
          break;
        }
    }
#endif

  /* --- PLL2: NPU core clock (IC6) --- */

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

  /* Configure PLL2CFGR1: source=HSI, M and N from board.h */

  regval = (RCC_PLL1CFGR1_SEL_HSI)
         | (STM32_PLL2_M << RCC_PLL1CFGR1_DIVM_SHIFT)
         | (STM32_PLL2_N << RCC_PLL1CFGR1_DIVN_SHIFT);
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

  /* Configure PLL3CFGR1: source=HSI, M and N from board.h */

  regval = (RCC_PLL1CFGR1_SEL_HSI)
         | (STM32_PLL3_M << RCC_PLL1CFGR1_DIVM_SHIFT)
         | (STM32_PLL3_N << RCC_PLL1CFGR1_DIVN_SHIFT);
  putreg32(regval, STM32_RCC_PLL3CFGR1);

  /* Configure PLL3CFGR3: disable SS, PDIV1=1, PDIV2 from board.h */

  regval = RCC_PLL1CFGR3_MODSSDIS
         | RCC_PLL1CFGR3_PDIVEN
         | (1 << RCC_PLL1CFGR3_PDIV1_SHIFT)
         | (STM32_PLL3_PDIV2 << RCC_PLL1CFGR3_PDIV2_SHIFT);
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
 *     HSI 64 MHz -> PLL1 (M=2, N=25) -> VCO 800 MHz -> PDIV1/1 -> 800 MHz
 *       -> IC1  /1  = 800 MHz  (CPU clock via CPUSW)
 *       -> IC2  /2  = 400 MHz  \
 *       -> IC6  /3  = 267 MHz   > System bus via SYSSW
 *       -> IC11 /2  = 400 MHz  /
 *       -> HPRE /2  = 200 MHz  (HCLK)
 *       -> PPRE1 /1 = 200 MHz  (APB1 = PCLK1)
 *       -> PPRE2 /1 = 200 MHz  (APB2 = PCLK2)
 *
 *   With CONFIG_STM32N6_NPU, IC6/IC11 are re-sourced from PLL2/PLL3:
 *       -> IC6  = PLL2/1 = 1000 MHz (NPU core)
 *       -> IC11 = PLL3/1 = 900 MHz  (NPU SRAM)
 *
 *   Requires VOS SCALE0 + SMPS overdrive (PB12 HIGH) for 800 MHz CPU.
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

  /* 1b. SMPS overdrive + VOS SCALE0 for 800 MHz CPU operation.
   *     GPIOB clock is not yet enabled (rcc_enableahb4 runs later),
   *     so enable it here for PB12 access.
   */

  putreg32(RCC_AHB4ENR_GPIOBEN, STM32_RCC_AHB4ENSR);
  modifyreg32(GPIOB_BASE_S + 0x00, (3 << 24), (1 << 24));  /* MODER: output */
  modifyreg32(GPIOB_BASE_S + 0x08, (3 << 24), (3 << 24));  /* OSPEEDR: VH */
  putreg32((1 << 12), GPIOB_BASE_S + 0x18);                 /* BSRR: set */

  modifyreg32(PWR_VOSCR, 0, PWR_VOSCR_VOS);

  for (timeout = VOS_TIMEOUT; timeout > 0; timeout--)
    {
      if ((getreg32(PWR_VOSCR) & PWR_VOSCR_RDY) != 0)
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

  /* 3. Configure PLL1CFGR1: source=HSI, M and N from board.h */

  regval = (RCC_PLL1CFGR1_SEL_HSI)
         | (STM32_PLL1_M << RCC_PLL1CFGR1_DIVM_SHIFT)
         | (STM32_PLL1_N << RCC_PLL1CFGR1_DIVN_SHIFT);
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
   *    IC1  = VCO / IC1_DIV  -> CPU clock (from board.h)
   *    IC2  = VCO / (IC1_DIV*2) -> SYSCLK (half of CPU)
   *    IC3  = VCO / (IC1_DIV*4) -> XSPI2 kernel clock
   *    IC6  = VCO / (IC1_DIV*3) -> AHB (re-sourced by NPU if enabled)
   *    IC11 = VCO / (IC1_DIV*2) -> APB (re-sourced by NPU if enabled)
   */

  putreg32(RCC_ICCFGR_SEL_PLL1
         | ((STM32_PLL1_IC1_DIV - 1) << RCC_ICCFGR_INT_SHIFT),
           STM32_RCC_IC1CFGR);
  putreg32(RCC_ICCFGR_SEL_PLL1
         | ((STM32_PLL1_IC1_DIV * 2 - 1) << RCC_ICCFGR_INT_SHIFT),
           STM32_RCC_IC2CFGR);
  putreg32(RCC_ICCFGR_SEL_PLL1
         | ((STM32_PLL1_IC1_DIV * 4 - 1) << RCC_ICCFGR_INT_SHIFT),
           STM32_RCC_IC3CFGR);
  putreg32(RCC_ICCFGR_SEL_PLL1
         | ((STM32_PLL1_IC1_DIV * 3 - 1) << RCC_ICCFGR_INT_SHIFT),
           STM32_RCC_IC6CFGR);
  putreg32(RCC_ICCFGR_SEL_PLL1
         | ((STM32_PLL1_IC1_DIV * 2 - 1) << RCC_ICCFGR_INT_SHIFT),
           STM32_RCC_IC11CFGR);

  /* 7. Enable IC1, IC2, IC3, IC6, IC11 */

  putreg32(RCC_DIVENR_IC1EN | RCC_DIVENR_IC2EN | RCC_DIVENR_IC3EN
         | RCC_DIVENR_IC6EN | RCC_DIVENR_IC11EN,
           STM32_RCC_DIVENSR);

  /* 8. Set bus prescalers BEFORE the CFGR1 switch — CFGR2 locks after
   *    CFGR1 is written.  HPRE = /2 gives HCLK = SYSCLK/2 = 200 MHz.
   *    PPRE1 = /1, PPRE2 = /1 → PCLK1 = PCLK2 = HCLK = 200 MHz.
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
