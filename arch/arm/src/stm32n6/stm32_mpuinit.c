/****************************************************************************
 * arch/arm/src/stm32n6/stm32_mpuinit.c
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

#include "mpu.h"
#include "stm32_mpuinit.h"

#ifdef CONFIG_ARM_MPU

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define STM32N6_AXISRAM_BASE  0x34000000
#define STM32N6_AXISRAM_SIZE  (4 * 1024 * 1024)  /* 4 MB */


/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_mpuinitialize
 *
 * Description:
 *   Configure the MPU for the STM32N6.
 *
 *   Region 0: AXI-SRAM (0x34000000, 4 MB) as Normal, Non-shareable,
 *   Write-Back Read/Write-Allocate.  This provides explicit cacheable
 *   memory attributes so that MVA-based D-cache maintenance operations
 *   (DCCIMVAC, DCIMVAC, DCCMVAC) work correctly on Cortex-M55.  Without
 *   an MPU region, MVA cache lookups silently fail and the operations
 *   become NOPs.
 *
 *   PRIVDEFENA is set so that privileged accesses to addresses not covered
 *   by any MPU region use the default memory map.  This means peripherals,
 *   PPB (System Control Space), and other non-SRAM regions continue to
 *   work without explicit MPU regions.
 *
 ****************************************************************************/

void stm32_mpuinitialize(void)
{
  /* Show MPU information */

  mpu_showtype();

  /* Reset MPU if enabled */

  mpu_reset();

  /* Region 0: AXI-SRAM as Normal, Non-shareable, Write-Back R/W-Allocate.
   *
   * On Cortex-M55, MVA-based D-cache maintenance operations (DCCMVAC,
   * DCIMVAC, DCCIMVAC) require the target address to have explicit
   * cacheable memory attributes from an MPU region.  Without this,
   * the cache lookup for MVA ops silently fails and DMA coherency
   * breaks.  Write-Back gives best performance; DMA drivers must use
   * up_clean_dcache() before transfers and up_invalidate_dcache() after.
   *
   * flags1: Priv RW, Non-shareable, execute allowed (code runs from SRAM)
   * flags2: Write-Back R/W-Allocate, region enabled
   */

  mpu_configure_region(STM32N6_AXISRAM_BASE, STM32N6_AXISRAM_SIZE,
                       MPU_RBAR_AP_RWNO | MPU_RBAR_SH_NO,
                       MPU_RLAR_ENABLE | MPU_RLAR_WRITE_BACK);

  /* Enable MPU with PRIVDEFENA so accesses to addresses not covered by
   * any explicit region (peripherals, PPB, etc.) use the default memory
   * map attributes.  hfnmiena=false so NMI/HardFault bypass the MPU.
   */

  mpu_control(true, false, true);
}

#endif /* CONFIG_ARM_MPU */
