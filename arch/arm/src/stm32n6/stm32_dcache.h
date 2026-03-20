/****************************************************************************
 * arch/arm/src/stm32n6/stm32_dcache.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * D-cache maintenance for DMA on Cortex-M55 Secure state.
 *
 * MVA-based cache ops (DCIMVAC/DCCIMVAC via up_invalidate_dcache) fail
 * silently at certain buffer alignments on M55 in Secure state.
 * Use DCCISW (set/way clean+invalidate) instead — it operates on the
 * cache structure directly without MVA address translation.
 *
 * Cost: ~2μs for 32KB D-cache.  Acceptable for DMA transfers where
 * the peripheral latency dominates.
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_STM32N6_STM32_DCACHE_H
#define __ARCH_ARM_SRC_STM32N6_STM32_DCACHE_H

#include "arm_internal.h"

/* CCSIDR register (Cache Size ID) */

#define NVIC_CCSIDR_ADDR  0xe000ed80

/* DCCISW register (D-Cache Clean and Invalidate by Set/Way) */

#define NVIC_DCCISW_ADDR  0xe000ef74

/****************************************************************************
 * Name: stm32_dcache_clean_invalidate
 *
 * Description:
 *   Clean+invalidate entire D-cache by set/way (DCCISW).
 *   Safe for DMA: writes back dirty lines first, then invalidates all.
 *   Call before DMA TX (flush CPU writes) and after DMA RX (discard stale).
 *
 ****************************************************************************/

static inline void stm32_dcache_clean_invalidate(void)
{
  uint32_t ccsidr = getreg32(NVIC_CCSIDR_ADDR);
  uint32_t sets   = (ccsidr >> 13) & 0x7fff;
  uint32_t sshift = ((ccsidr & 7) + 2) + 2;
  uint32_t ways   = (ccsidr >> 3) & 0x3ff;
  uint32_t wshift = __builtin_clz(ways) & 0x1f;

  __asm volatile ("dsb sy");

  do
    {
      int32_t w = ways;
      do
        {
          putreg32((w << wshift) | (sets << sshift), NVIC_DCCISW_ADDR);
        }
      while (w--);
    }
  while (sets--);

  __asm volatile ("dsb sy");
  __asm volatile ("isb sy");
}

#endif /* __ARCH_ARM_SRC_STM32N6_STM32_DCACHE_H */
