/****************************************************************************
 * arch/arm/src/stm32n6/stm32_otg.h
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

#ifndef __ARCH_ARM_SRC_STM32N6_STM32_OTG_H
#define __ARCH_ARM_SRC_STM32N6_STM32_OTG_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>

#include "chip.h"
#include "hardware/stm32n6xxx_otg.h"

#if defined(CONFIG_STM32N6_USB1_OTG_HS) || \
    defined(CONFIG_STM32N6_USB2_OTG_HS)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* STM32N6 has 9 bidirectional endpoints (ep0-8) */

#define STM32_NENDPOINTS      (9)

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifndef __ASSEMBLY__

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Name: stm32_otghost_initialize
 *
 * Description:
 *   Initialize USB host device controller hardware.
 *
 * Input Parameters:
 *   controller -- If the device supports more than one USB host controller,
 *     then this identifies which controller is being initialized.
 *     Normally, this is just zero.
 *
 * Returned Value:
 *   An instance of the USB host interface.
 *
 ****************************************************************************/

#ifdef CONFIG_USBHOST
struct usbhost_connection_s;
struct usbhost_connection_s *stm32_otghost_initialize(int controller);
#endif

/****************************************************************************
 * Name:  stm32_usbsuspend
 *
 * Description:
 *   Board logic must provide the stm32_usbsuspend logic if the OTG
 *   device driver is used.  This function is called whenever the USB
 *   enters or leaves suspend mode.
 *
 ****************************************************************************/

#ifdef CONFIG_USBDEV
struct usbdev_s;
void stm32_usbsuspend(struct usbdev_s *dev, bool resume);
#endif

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __ASSEMBLY__ */
#endif /* CONFIG_STM32N6_USB1_OTG_HS || CONFIG_STM32N6_USB2_OTG_HS */
#endif /* __ARCH_ARM_SRC_STM32N6_STM32_OTG_H */
