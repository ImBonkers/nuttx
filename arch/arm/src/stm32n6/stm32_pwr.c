/****************************************************************************
 * arch/arm/src/stm32n6/stm32_pwr.c
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
#include <stdbool.h>

#include "arm_internal.h"
#include "stm32_pwr.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_pwr_enablebkp
 *
 * Description:
 *   Enables write access to the backup domain (RTC registers, RTC backup
 *   data registers and backup SRAM).
 *
 * Input Parameters:
 *   writable - True: enable ability to write to backup domain registers
 *
 * Returned Value:
 *   True: The backup domain was previously writable.
 *
 ****************************************************************************/

bool stm32_pwr_enablebkp(bool writable)
{
  uint32_t regval;
  bool waswritable;

  /* Get the current state of the PWR_DBPCR register */

  regval = getreg32(STM32_PWR_DBPCR);
  waswritable = ((regval & PWR_DBPCR_DBP) != 0);

  /* Set or clear the DBP bit as requested */

  if (writable)
    {
      regval |= PWR_DBPCR_DBP;
    }
  else
    {
      regval &= ~PWR_DBPCR_DBP;
    }

  putreg32(regval, STM32_PWR_DBPCR);

  return waswritable;
}

/****************************************************************************
 * Name: stm32_pwr_enablevddio
 *
 * Description:
 *   Enable VddIO supply monitoring for all I/O voltage domains that are
 *   needed for GPIO and external peripheral access on STM32N6.
 *
 *   - VddIO2 + VddIO3: Required for GPIO port E (USART1 TX/RX on PE5/PE6)
 *   - VddIO4: Required for XSPI2 (external flash)
 *   - VddIO5: Required for XSPI1
 *
 *   These supply valid bits must be set before configuring GPIOs or
 *   accessing peripherals on the corresponding I/O domains.
 *
 ****************************************************************************/

void stm32_pwr_enablevddio(void)
{
  /* Enable VddIO2 and VddIO3 (for GPIOE used by USART1).
   * Also set voltage range to 1.8V for both — the Nucleo board runs
   * VDDIO2/3 at 1.8V. Without VRSEL, IO pads operate in "degraded mode"
   * (assuming 3.3V) and cannot reach maximum XSPI speed for OPI DTR.
   * SVMCR3 bits: 25=VDDIO2VRSEL, 26=VDDIO3VRSEL (1=1.8V, 0=3.3V).
   */

  modifyreg32(STM32_PWR_SVMCR3, 0,
              PWR_SVMCR3_VDDIO2SV | PWR_SVMCR3_VDDIO3SV |
              (1 << 25) | (1 << 26));

  /* Enable VddIO4 (for XSPI2) */

  modifyreg32(STM32_PWR_SVMCR1, 0, PWR_SVMCR1_VDDIO4SV);

  /* Enable VddIO5 (for XSPI1) */

  modifyreg32(STM32_PWR_SVMCR2, 0, PWR_SVMCR2_VDDIO5SV);
}
