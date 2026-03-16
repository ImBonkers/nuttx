/****************************************************************************
 * arch/arm/src/stm32n6/stm32_lsi.c
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

#include "arm_internal.h"
#include "stm32_rcc.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_rcc_enablelsi
 *
 * Description:
 *   Enable the Internal Low-Speed (LSI) RC Oscillator.
 *
 *   STM32N6 uses split CR/SR registers (not a single CSR like STM32H7):
 *     - RCC_CR bit 0 (LSION) to enable
 *     - RCC_SR bit 0 (LSIRDY) to check ready status
 *
 ****************************************************************************/

void stm32_rcc_enablelsi(void)
{
  /* Enable the Internal Low-Speed (LSI) RC Oscillator by setting the LSION
   * bit in the RCC CR register.
   */

  modifyreg32(STM32_RCC_CR, 0, RCC_CR_LSION);

  /* Wait for the internal RC 32 kHz oscillator to be stable. */

  while ((getreg32(STM32_RCC_SR) & RCC_SR_LSIRDY) == 0);
}

/****************************************************************************
 * Name: stm32_rcc_disablelsi
 *
 * Description:
 *   Disable the Internal Low-Speed (LSI) RC Oscillator.
 *
 ****************************************************************************/

void stm32_rcc_disablelsi(void)
{
  /* Disable the Internal Low-Speed (LSI) RC Oscillator by clearing the
   * LSION bit in the RCC CR register.
   */

  modifyreg32(STM32_RCC_CR, RCC_CR_LSION, 0);

  /* LSIRDY should go low after 3 LSI clock cycles */
}
