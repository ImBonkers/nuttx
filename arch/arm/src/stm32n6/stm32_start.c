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
 * Private Function Prototypes
 ****************************************************************************/

void stm32_early_fault_dump(uint32_t *frame);

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* SYSCFG (Secure alias) - I/O compensation cell registers ******************/

#define STM32_SYSCFG_BASE        0x56008000
#define STM32_SYSCFG_INITSVTORCR (STM32_SYSCFG_BASE + 0x10)
#define STM32_SYSCFG_VDDIO4CCCR (STM32_SYSCFG_BASE + 0x44)
#define STM32_SYSCFG_VDDIO5CCCR (STM32_SYSCFG_BASE + 0x4c)
#define STM32_SYSCFG_VDDIO2CCCR (STM32_SYSCFG_BASE + 0x54)
#define STM32_SYSCFG_VDDIO3CCCR (STM32_SYSCFG_BASE + 0x5c)
#define STM32_SYSCFG_VDDCCCR    (STM32_SYSCFG_BASE + 0x64)

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
 * Name: early_print_hex
 *
 * Description:
 *   Print a 32-bit value as hex via arm_lowputc (polling UART).
 *
 ****************************************************************************/

static void early_print_hex(uint32_t val)
{
  static const char hex[] = "0123456789ABCDEF";
  int i;

  for (i = 28; i >= 0; i -= 4)
    {
      arm_lowputc(hex[(val >> i) & 0xf]);
    }
}

/****************************************************************************
 * Name: early_print_str
 *
 * Description:
 *   Print a string via arm_lowputc (polling UART).
 *
 ****************************************************************************/

static void early_print_str(const char *s)
{
  while (*s)
    {
      arm_lowputc(*s++);
    }
}

/****************************************************************************
 * Name: stm32_early_fault_dump
 *
 * Description:
 *   Minimal fault handler that dumps CFSR, PC, LR, SP via polling UART.
 *   Called from the naked entry point with the exception stack frame.
 *
 ****************************************************************************/

void stm32_early_fault_dump(uint32_t *frame)
{
  uint32_t cfsr = getreg32(0xe000ed28);  /* CFSR */
  uint32_t hfsr = getreg32(0xe000ed2c);  /* HFSR */
  uint32_t bfar = getreg32(0xe000ed38);  /* BFAR */

  early_print_str("\r\n!FAULT C=");
  early_print_hex(cfsr);
  early_print_str(" H=");
  early_print_hex(hfsr);
  early_print_str(" PC=");
  early_print_hex(frame[6]);
  early_print_str(" LR=");
  early_print_hex(frame[5]);
  early_print_str(" B=");
  early_print_hex(bfar);
  early_print_str(" SP=");
  early_print_hex((uint32_t)frame);
  early_print_str("\r\n");

  for (; ; );
}

/****************************************************************************
 * Name: stm32_early_fault_entry
 *
 * Description:
 *   Naked entry point for the early fault handler.  Determines whether the
 *   exception frame is on MSP or PSP and passes it to the C dump function.
 *
 ****************************************************************************/

__attribute__((naked)) void stm32_early_fault_entry(void)
{
  __asm__ volatile
    (
      "tst lr, #4\n\t"
      "ite eq\n\t"
      "mrseq r0, msp\n\t"
      "mrsne r0, psp\n\t"
      "b stm32_early_fault_dump\n\t"
    );
}

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

  /* Disable SysTick and clear any pending interrupt.  When booting via
   * FSBL, HAL_Init() starts SysTick at 1ms.  If it fires before NuttX
   * has attached its handlers, we get an "unexpected IRQ" assertion.
   */

  putreg32(0, NVIC_SYSTICK_CTRL);
  putreg32(NVIC_INTCTRL_PENDSTCLR, NVIC_INTCTRL);

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
   * In SRAM-only builds (DEV mode), _eronly == _sdata so data is already
   * in place.  Skip the copy to avoid corrupting .data when alignment
   * padding creates a gap between _eronly and _sdata.
   */

  if (_eronly != _sdata)
    {
      for (src = (const uint32_t *)_eronly,
           dest = (uint32_t *)_sdata; dest < (uint32_t *)_edata;
          )
        {
          *dest++ = *src++;
        }
    }

  /* Enable Low-Overhead Branch (LOB) extensions for Cortex-M55 */

  stm32_enable_lob();

  /* Configure clocks (enables PWR + GPIO clocks), then enable VddIO power
   * domains before any GPIO access.  VddIO2/3 are required for GPIOE
   * (USART1 TX/RX on PE5/PE6).
   */

  stm32_clockconfig();

  /* Enable BSECEN per STM32N6 errata ES0620: "Do not clear BSECEN bit
   * before entering Low-power mode."  If BSECEN (RCC_APB4ENR2 bit 1) is
   * not set, the Cortex-M55 delivers incorrect cpu_sleep_in signals to
   * the RCC, causing WFI/sleep mode to fail.  Use the SET register
   * (APB4ENSR2 at RCC + 0x0A78) to enable it atomically.
   */

  putreg32(0x03, STM32_RCC_BASE + 0x0a78);  /* RCC_APB4ENSR2: SYSCFGEN+BSECEN */

  /* Enable LPEN (Low-Power Enable) registers so peripheral and memory
   * clocks keep running during CSLEEP (WFI).  Without these, WFI halts
   * ALL clocks — TIM2, USART1, SRAM — and the system never wakes up.
   *
   * BUSLPENSR bits: ACLKN(0) ACLKNC(1) AHBM(2) AHB4(6) APB1(8) APB2(9)
   */

  putreg32(0x0347, STM32_RCC_BASE + 0x0a84);  /* BUSLPENSR */

  /* MEMLPENSR: enable all AXISRAM banks + CACHEAXIRAM for CSLEEP.
   * Bits: SRAM3(0) SRAM4(1) SRAM5(2) SRAM6(3) SRAM1(7) SRAM2(8) CACHE(10)
   */

  putreg32(0x058f, STM32_RCC_BASE + 0x0a8c);  /* MEMLPENSR */

  /* Peripheral-level LPEN: TIM2 on APB1, USART1 on APB2 */

  putreg32(0x01, STM32_RCC_BASE + 0x0aa4);    /* APB1LPENSR1: TIM2 */
  putreg32(0x10, STM32_RCC_BASE + 0x0aac);    /* APB2LPENSR: USART1 */

  stm32_pwr_enablevddio();

  /* Configure SYSCFG I/O compensation cells per errata ES0620.
   * The FSBL's SystemInit() does this for stable I/O drive on all VddIO
   * domains.  Without this, some GPIO pins (e.g. PG0, PG10) may not
   * drive correctly.  SYSCFG clock was enabled above (APB4ENSR2 bit 0).
   *
   * Value 0x287 matches the FSBL: RANSRC=7, RAPSRC=8, CS=1 (manual codes).
   * SYSCFG Secure base = 0x56008000 (APB4 + 0x8000).
   */

  putreg32(0x287, STM32_SYSCFG_VDDIO4CCCR);
  putreg32(0x287, STM32_SYSCFG_VDDIO5CCCR);
  putreg32(0x287, STM32_SYSCFG_VDDIO2CCCR);
  putreg32(0x287, STM32_SYSCFG_VDDIO3CCCR);
  putreg32(0x287, STM32_SYSCFG_VDDCCCR);

  /* Set INITSVTORCR to our vector table address */

  putreg32((uint32_t)_vectors, STM32_SYSCFG_INITSVTORCR);

  /* Read-back to ensure writes complete before proceeding */

  (void)getreg32(STM32_SYSCFG_VDDCCCR);

  /* Set USART1 kernel clock to HSI (64 MHz) so the UART baud rate is
   * independent of SYSSW changes.  RCC_CCIPR13 offset 0x0174,
   * USART1SEL[2:0] = 6 (HSI).
   */

  putreg32(0x06, STM32_RCC_BASE + 0x0174);  /* CCIPR13: USART1SEL=HSI */

  stm32_lowsetup();
  stm32_gpioinit();
  __asm volatile ("dsb sy");  /* Flush write buffer — catch deferred bus faults here */
  showprogress('A');

  /* Enable I-cache and D-cache.  The Cortex-M55 caches dramatically reduce
   * stalls from the 200 MHz AHB bus when running the CPU at 600 MHz.
   */

#ifdef CONFIG_ARMV8M_ICACHE
  up_enable_icache();
#endif
#ifdef CONFIG_ARMV8M_DCACHE
  up_enable_dcache();
#endif

  /* Install early fault handler now that UART is configured.  The vector
   * table is in writable SRAM, so we can patch HardFault (index 3) to
   * point at our minimal handler that dumps CFSR/PC via polling UART.
   */

  ((void (*volatile *)(void))_vectors)[3] = stm32_early_fault_entry;
  ((void (*volatile *)(void))_vectors)[4] = stm32_early_fault_entry;
  ((void (*volatile *)(void))_vectors)[5] = stm32_early_fault_entry;
  ((void (*volatile *)(void))_vectors)[6] = stm32_early_fault_entry;

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
