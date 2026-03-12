/****************************************************************************
 * arch/arm/src/stm32n6/stm32_timerisr.c
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

#include <stdint.h>
#include <time.h>
#include <debug.h>
#include <nuttx/arch.h>
#include <arch/board/board.h>

#include "nvic.h"
#include "clock/clock.h"
#include "arm_internal.h"
#include "chip.h"
#include "stm32.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* On the STM32N6 Cortex-M55, WFI halts both the processor clock and the
 * external reference clock, stopping SysTick regardless of CLKSOURCE.
 * We use TIM2 (a general-purpose 32-bit timer on APB1) instead, because
 * the APB1 clock continues running during WFI sleep mode.
 *
 * TIM2 register offsets (standard STM32 general-purpose timer layout):
 */

#define STM32_TIM_CR1_OFFSET    0x00
#define STM32_TIM_DIER_OFFSET   0x0c
#define STM32_TIM_SR_OFFSET     0x10
#define STM32_TIM_EGR_OFFSET    0x14
#define STM32_TIM_CNT_OFFSET    0x24
#define STM32_TIM_PSC_OFFSET    0x28
#define STM32_TIM_ARR_OFFSET    0x2c

/* CR1 bits */

#define TIM_CR1_CEN             (1 << 0)  /* Counter enable */
#define TIM_CR1_ARPE            (1 << 7)  /* Auto-reload preload enable */

/* DIER bits */

#define TIM_DIER_UIE            (1 << 0)  /* Update interrupt enable */

/* SR bits */

#define TIM_SR_UIF              (1 << 0)  /* Update interrupt flag */

/* EGR bits */

#define TIM_EGR_UG              (1 << 0)  /* Update generation */

/* TIM2 auto-reload value for the desired tick rate.
 * APB1 timer clock = STM32_HCLK_FREQUENCY (no prescaler in Phase 1).
 * ARR = (timer_clock / CLK_TCK) - 1
 */

#define TIM2_RELOAD ((STM32_HCLK_FREQUENCY / CLK_TCK) - 1)

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Function:  stm32_timerisr
 *
 * Description:
 *   TIM2 update interrupt handler.  Clears the interrupt flag and
 *   processes the system timer tick.
 *
 ****************************************************************************/

static int stm32_timerisr(int irq, uint32_t *regs, void *arg)
{
  /* Clear the update interrupt flag */

  putreg32(~TIM_SR_UIF, STM32_TIM2_BASE + STM32_TIM_SR_OFFSET);

  /* Process timer interrupt */

  nxsched_process_timer();
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Function:  up_timer_initialize
 *
 * Description:
 *   This function is called during start-up to initialize the system
 *   timer interrupt using TIM2.
 *
 ****************************************************************************/

void up_timer_initialize(void)
{
  /* Enable TIM2 clock on APB1 using the atomic SET register */

  putreg32(RCC_APB1ENR1_TIM2EN, STM32_RCC_APB1ENSR1);

  /* Stop the timer while configuring */

  putreg32(0, STM32_TIM2_BASE + STM32_TIM_CR1_OFFSET);

  /* Set prescaler to 0 (timer runs at full APB1 clock = HCLK) */

  putreg32(0, STM32_TIM2_BASE + STM32_TIM_PSC_OFFSET);

  /* Set auto-reload value for the desired tick rate */

  putreg32(TIM2_RELOAD, STM32_TIM2_BASE + STM32_TIM_ARR_OFFSET);

  /* Generate an update event to load PSC and ARR immediately */

  putreg32(TIM_EGR_UG, STM32_TIM2_BASE + STM32_TIM_EGR_OFFSET);

  /* Clear the update interrupt flag (UG sets UIF) */

  putreg32(~TIM_SR_UIF, STM32_TIM2_BASE + STM32_TIM_SR_OFFSET);

  /* Attach the TIM2 interrupt handler */

  irq_attach(STM32_IRQ_TIM2, (xcpt_t)stm32_timerisr, NULL);

  /* Enable TIM2 interrupt in NVIC (priority is set to default by
   * up_irqinitialize before this function is called).
   */

  up_enable_irq(STM32_IRQ_TIM2);

  /* Enable the update interrupt in TIM2 */

  putreg32(TIM_DIER_UIE, STM32_TIM2_BASE + STM32_TIM_DIER_OFFSET);

  /* Start the timer with auto-reload preload enabled */

  putreg32(TIM_CR1_CEN | TIM_CR1_ARPE,
           STM32_TIM2_BASE + STM32_TIM_CR1_OFFSET);
}
