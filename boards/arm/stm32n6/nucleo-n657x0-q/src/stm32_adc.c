/****************************************************************************
 * boards/arm/stm32n6/nucleo-n657x0-q/src/stm32_adc.c
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

#include <errno.h>
#include <debug.h>

#include <nuttx/analog/adc.h>

#include "stm32_gpio.h"
#include "stm32_adc.h"
#include <arch/board/board.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* ADC channel-to-pin mapping on NUCLEO-N657X0-Q (from DS14791 Table 18):
 *
 *   PF11 -> ADC1_INP2   (Arduino D11 / SPI5_MOSI — shared)
 *   PF12 -> ADC1_INP6   (Arduino D12 / SPI5_MISO — shared)
 *   PF6  -> ADC1_INP15  (Arduino A2)
 *   PF7  -> ADC1_INP9   (morpho)
 *   PF3  -> ADC1_INP16  (morpho)
 *   PF4  -> ADC1_INP18  (morpho)
 *
 * NOTE: PF5 (Arduino A0) has NO ADC function on STM32N6!
 *
 * For initial testing, use internal VREFINT (ch17, ~1.21V)
 * and one external pin: PF6 = ADC1_INP15 (Arduino A2).
 */

#define GPIO_ADC1_IN15  (GPIO_ANALOG | GPIO_PORTF | GPIO_PIN6)

#define ADC1_NCHANNELS  2

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const uint8_t g_adc1_chanlist[ADC1_NCHANNELS] =
{
  17,   /* VREFINT internal (~1.21V, needs CCR.VREFEN=1) */
  15,   /* A2: PF6 = ADC1_INP15 */
};

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_adc_setup
 *
 * Description:
 *   Configure ADC GPIO pins and register /dev/adc0.
 *   Called from stm32_bringup().
 *
 ****************************************************************************/

int stm32_adc_setup(void)
{
  struct adc_dev_s *adc;
  int ret;

  /* Configure analog GPIO pin for external channel */

  stm32_configgpio(GPIO_ADC1_IN15);

  /* Initialize ADC1 */

  adc = stm32n6_adc_initialize(1, g_adc1_chanlist, ADC1_NCHANNELS);
  if (adc == NULL)
    {
      aerr("ERROR: stm32n6_adc_initialize(1) failed\n");
      return -ENODEV;
    }

  /* Register as /dev/adc0 */

  ret = adc_register("/dev/adc0", adc);
  if (ret < 0)
    {
      aerr("ERROR: adc_register(/dev/adc0) failed: %d\n", ret);
      return ret;
    }

  ainfo("ADC1 registered at /dev/adc0\n");
  return OK;
}
