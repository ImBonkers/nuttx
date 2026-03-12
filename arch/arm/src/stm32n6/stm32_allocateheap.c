/****************************************************************************
 * arch/arm/src/stm32n6/stm32_allocateheap.c
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

#include <sys/types.h>
#include <stdint.h>
#include <assert.h>
#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/board.h>
#include <nuttx/kmalloc.h>
#include <nuttx/userspace.h>

#include <arch/stm32n6/chip.h>
#include <arch/board/board.h>

#include "arm_internal.h"
#include "hardware/stm32_memorymap.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* The STM32N6 has no internal flash.  All code and data reside in SRAM.
 * The heap starts after the idle thread stack (at g_idle_topstack) and
 * extends to the end of SRAM.
 *
 * For Phase 1, we use a single contiguous heap region.
 * SRAM1 starts at 0x34000000 and is 1MB.
 * SRAM2 starts at 0x34100000 and is ~3.2MB.
 * Total: ~4.2MB
 *
 * The linker script determines how much SRAM is used for code/data/bss.
 * The remainder is available for heap.
 */

/* End of SRAM - this should match the linker script */

#define STM32_SRAM_END  (STM32_SRAM1_BASE + STM32N6_SRAM_SIZE)

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_allocate_heap
 *
 * Description:
 *   This function will be called to dynamically set aside the heap region.
 *
 *   For the kernel build (CONFIG_BUILD_PROTECTED=y) with both kernel- and
 *   user-space heaps (CONFIG_MM_KERNEL_HEAP=y), this function provides the
 *   size of the unprotected, user-space heap.
 *
 *   If a protected kernel-space heap is provided, the kernel heap must be
 *   allocated (and target_heapsize adjusted) by an analogous
 *   up_allocate_kheap().
 *
 ****************************************************************************/

void up_allocate_heap(void **heap_start, size_t *heap_size)
{
  /* Start the heap after the idle thread stack */

  *heap_start = (void *)g_idle_topstack;
  *heap_size  = STM32_SRAM_END - g_idle_topstack;
}
