/****************************************************************************
 * arch/arm/src/stm32n6/stm32_start.c
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
#include <assert.h>
#include <debug.h>

#include <nuttx/init.h>
#include <arch/board/board.h>

#include "arm_internal.h"
#include "nvic.h"

#include "stm32.h"
#include "stm32_gpio.h"
#include "stm32_pwr.h"
#include "stm32_start.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* STM32N6 Memory Map - DEV boot mode ***************************************/

/* In DEV boot mode, code is loaded directly to SRAM by ST-LINK.
 * There is NO internal flash on the STM32N6.
 *
 * 0x3400:0000 - Start of SRAM1 and start of .data (_sdata)
 *             - End of .data (_edata) and start of .bss (_sbss)
 *             - End of .bss (_ebss) and bottom of idle stack
 *             - _ebss + CONFIG_IDLETHREAD_STACKSIZE = end of idle stack,
 *               start of heap.
 * 0x3450:0000 - End of SRAM (4.2MB total: SRAM1 + SRAM2)
 */

#define HEAP_BASE  ((uintptr_t)_ebss + CONFIG_IDLETHREAD_STACKSIZE)

/* g_idle_topstack: _sbss is the start of the BSS region as defined by the
 * linker script. _ebss lies at the end of the BSS region. The idle task
 * stack starts at the end of BSS and is of size CONFIG_IDLETHREAD_STACKSIZE.
 * The IDLE thread is the thread that the system boots on and, eventually,
 * becomes the IDLE, do nothing task that runs only when there is nothing
 * else to run.  The heap continues from there until the end of memory.
 * g_idle_topstack is a read-only variable the provides this computed
 * address.
 */

const uintptr_t g_idle_topstack = HEAP_BASE;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: showprogress
 *
 * Description:
 *   Print a character on the UART to show boot status.
 *
 ****************************************************************************/

#ifdef CONFIG_DEBUG_FEATURES
#  define showprogress(c) arm_lowputc(c)
#else
#  define showprogress(c)
#endif

/****************************************************************************
 * Name: stm32_enable_lob
 *
 * Description:
 *   Enable Low-Overhead Branch (LOB) extensions on Cortex-M55.
 *   This enables hardware loop support via the WLS/DLS/LE instructions
 *   which are part of the ARMv8.1-M architecture.
 *
 ****************************************************************************/

static inline void stm32_enable_lob(void)
{
  uint32_t regval;

  /* Enable LOB by setting the LOB bit in the CCR (Configuration and Control
   * Register).  On Cortex-M55, bit 19 of CCR enables LOB.
   */

  regval = getreg32(NVIC_CFGCON);
  regval |= (1 << 19);  /* LOB enable bit */
  putreg32(regval, NVIC_CFGCON);

  /* Instruction Synchronization Barrier to ensure the LOB enable takes
   * effect before any LOB instructions are executed.
   */

  __asm__ volatile ("isb" ::: "memory");
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_ARMV8M_STACKCHECK
/* we need to get r10 set before we can allow instrumentation calls */

void __start(void) noinstrument_function;
#endif

/****************************************************************************
 * Name: __start
 *
 * Description:
 *   This is the reset entry point.
 *
 ****************************************************************************/

void __start(void)
{
  const uint32_t *src;
  uint32_t *dest;

  /* Set VTOR to point to our vector table.  In DEV boot mode the boot ROM
   * leaves VTOR pointing at 0x18000000 (ROM).  We must fix this before
   * any exception can fire so that our handlers are used.
   */

  putreg32((uint32_t)_vectors, NVIC_VECTAB);

  /* Enable FPU (CP10/CP11) immediately.  With hard-float ABI the compiler
   * may emit FPU instructions at any point, and the NVIC will attempt to
   * save FP context on exception entry.  Both require the FPU to be
   * enabled.  Must happen before any other function call.
   */

  arm_fpuconfig();

#ifdef CONFIG_ARMV8M_STACKCHECK
  /* Set the stack limit before we attempt to call any functions */

  __asm__ volatile
    ("sub r10, sp, %0" : : "r" (CONFIG_IDLETHREAD_STACKSIZE - 64) :);
#endif

  /* Clear .bss.  We'll do this inline (vs. calling memset) just to be
   * certain that there are no issues with the state of global variables.
   */

  for (dest = (uint32_t *)_sbss; dest < (uint32_t *)_ebss; )
    {
      *dest++ = 0;
    }

  /* Move the initialized data section from its temporary holding spot in
   * FLASH into the correct place in SRAM.  The correct place in SRAM is
   * give by _sdata and _edata.  The temporary location is in FLASH at the
   * end of all of the other read-only data (.text, .rodata) at _eronly.
   *
   * In DEV boot mode (running entirely from SRAM), _eronly == _sdata so
   * this loop body will not execute - the data is already in place.
   */

  for (src = (const uint32_t *)_eronly,
       dest = (uint32_t *)_sdata; dest < (uint32_t *)_edata;
      )
    {
      *dest++ = *src++;
    }

  /* Enable Low-Overhead Branch (LOB) extensions for Cortex-M55 */

  stm32_enable_lob();

  /* Configure clocks (enables PWR + GPIO clocks), then enable VddIO power
   * domains before any GPIO access.  VddIO2/3 are required for GPIOE
   * (USART1 TX/RX on PE5/PE6).
   */

  stm32_clockconfig();
  stm32_pwr_enablevddio();
  stm32_lowsetup();
  stm32_gpioinit();
  __asm volatile ("dsb sy");  /* Flush write buffer — catch deferred bus faults here */
  showprogress('A');

#ifdef CONFIG_ARMV8M_STACKCHECK
  arm_stack_check_init();
#endif

#ifdef CONFIG_ARCH_PERF_EVENTS
  up_perf_init((void *)STM32_SYSCLK_FREQUENCY);
#endif

  /* Perform early serial initialization */

#ifdef USE_EARLYSERIALINIT
  arm_earlyserialinit();
#endif
  __asm volatile ("dsb sy");  /* Flush write buffer */
  showprogress('B');

  /* Initialize onboard resources */

  stm32_board_initialize();
  __asm volatile ("dsb sy");  /* Flush write buffer */
  showprogress('C');

  /* Clear any sticky fault flags from early boot before entering NuttX.
   * This helps distinguish early-boot faults from runtime faults.
   */

  putreg32(0xffffffff, 0xe000ed28);  /* Clear CFSR */
  putreg32(0xffffffff, 0xe000ed2c);  /* Clear HFSR */

  /* NOTE: Do NOT enable BusFault/MemFault/UsageFault handlers here.
   * The irq_attach() calls that register the actual fault handlers happen
   * inside nx_start() -> up_irqinitialize().  If we enable fault handlers
   * in SHCSR before the vectors are attached, any fault during early
   * nx_start() will hit an unattached vector and panic as "unexpected IRQ"
   * instead of giving a proper fault dump.  Let up_irqinitialize() handle
   * SHCSR setup.
   */

  /* Final DSB + ISB to flush any deferred writes before entering NuttX */

  __asm volatile ("dsb sy");
  __asm volatile ("isb sy");

  /* Then start NuttX */

  showprogress('\r');
  showprogress('\n');

  nx_start();

  /* Shouldn't get here */

  for (; ; );
}
