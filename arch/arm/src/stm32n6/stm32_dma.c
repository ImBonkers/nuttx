/****************************************************************************
 * arch/arm/src/stm32n6/stm32_dma.c
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
#include <assert.h>
#include <debug.h>
#include <errno.h>
#include <inttypes.h>

#include <nuttx/irq.h>
#include <nuttx/arch.h>

#include "arm_internal.h"
#include "nvic.h"
#include "stm32_dma.h"
#include "hardware/stm32_gpdma.h"
#include "hardware/stm32n6xxx_memorymap.h"
#include "hardware/stm32n6xxx_rcc.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Per-channel register layout within a DMA controller.
 * Each channel occupies 0x80 bytes starting at offset 0x50 from the
 * controller base.
 */

#define CH_BASE_OFFSET(ch)   (0x80*(ch))
#define CH_CXLBAR_OFFSET      0x50
#define CH_CXCIDCFGR_OFFSET   0x54
#define CH_CXFCR_OFFSET       0x5C
#define CH_CXSR_OFFSET        0x60
#define CH_CXCR_OFFSET        0x64
#define CH_CXTR1_OFFSET       0x90
#define CH_CXTR2_OFFSET       0x94
#define CH_CXBR1_OFFSET       0x98
#define CH_CXSAR_OFFSET       0x9C
#define CH_CXDAR_OFFSET       0xA0
#define CH_CXTR3_OFFSET       0xA4
#define CH_CXBR2_OFFSET       0xA8
#define CH_CXLLR_OFFSET       0xCC

/* DMA instance identifiers */

#define DMA_INST_HPDMA1       1
#define DMA_INST_GPDMA1       2

/* Total number of channels in g_chan[] (computed at compile time) */

#define DMA_NCHANNELS         (sizeof(g_chan) / sizeof(struct gpdma_ch_s))

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct stm32_gpdma_lli_s
{
  uint32_t tr1;   /* GPDMA_CxTR1 value */
  uint32_t tr2;   /* GPDMA_CxTR2 value */
  uint32_t br1;   /* GPDMA_CxBR1 value (block size in bytes) */
  uint32_t sar;   /* GPDMA_CxSAR (source address) */
  uint32_t dar;   /* GPDMA_CxDAR (dest address) */
  uint32_t llr;   /* GPDMA_CxLLR (pointer+update bits) */
}
__attribute__ ((aligned(32)));

struct gpdma_ch_s
{
  uint8_t            dma_instance; /* DMA_INST_HPDMA1 or DMA_INST_GPDMA1 */
  uint8_t            channel;      /* Channel number within the instance */
  uint8_t            irq;          /* IRQ number for this channel */
  uint8_t            fifo_bytes;   /* FIFO size in bytes (8/16/32/64) */
  bool               has_2d;       /* 2D addressing capable (ch 12-15) */
  enum gpdma_ttype_e type;
  bool               free;         /* Is this channel free to use */
  uint32_t           base;         /* Channel base address */
  dma_callback_t     callback;
  void              *arg;
  struct stm32_gpdma_cfg_s cfg;    /* Configuration passed at setup */
  struct stm32_gpdma_lli_s lli[2]; /* Linked-list items for circular mode */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static inline uint32_t gpdmach_getreg(struct gpdma_ch_s *chan,
                                      uint32_t offset);
static inline void gpdmach_putreg(struct gpdma_ch_s *chan, uint32_t offset,
                                  uint32_t value);
static inline void gpdmach_modifyreg32(struct gpdma_ch_s *chan,
                                       uint32_t offset, uint32_t clrbits,
                                       uint32_t setbits);
static void gpdma_ch_abort(struct gpdma_ch_s *chan);
static void gpdma_ch_disable(struct gpdma_ch_s *chan);

static int gpdma_setup(struct gpdma_ch_s *chan,
                       struct stm32_gpdma_cfg_s *cfg);
static int gpdma_setup_circular(struct gpdma_ch_s *chan,
                                struct stm32_gpdma_cfg_s *cfg);
static int gpdma_dmainterrupt(int irq, void *context, void *arg);

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* Unified channel array: HPDMA1 channels first, then GPDMA1 channels.
 * Both controllers have 16 channels with identical register layout.
 *
 * Per-channel FIFO sizes (DS14791 Tables 8/9):
 *   HPDMA1 ch0-11:  16 bytes, no 2D
 *   HPDMA1 ch12-15: 64 bytes, 2D capable
 *   GPDMA1 ch0-11:   8 bytes, no 2D
 *   GPDMA1 ch12-15: 32 bytes, 2D capable
 */

static struct gpdma_ch_s g_chan[] =
{
#ifdef CONFIG_STM32N6_HPDMA1
  /* HPDMA1 channels 0-11 (AXI, 16-byte FIFO) */

  {
    .dma_instance = DMA_INST_HPDMA1, .channel = 0,
    .irq = STM32_IRQ_HPDMA1_CH0, .fifo_bytes = 16, .has_2d = false,
    .free = true, .base = STM32_HPDMA1_BASE + CH_BASE_OFFSET(0)
  },
  {
    .dma_instance = DMA_INST_HPDMA1, .channel = 1,
    .irq = STM32_IRQ_HPDMA1_CH1, .fifo_bytes = 16, .has_2d = false,
    .free = true, .base = STM32_HPDMA1_BASE + CH_BASE_OFFSET(1)
  },
  {
    .dma_instance = DMA_INST_HPDMA1, .channel = 2,
    .irq = STM32_IRQ_HPDMA1_CH2, .fifo_bytes = 16, .has_2d = false,
    .free = true, .base = STM32_HPDMA1_BASE + CH_BASE_OFFSET(2)
  },
  {
    .dma_instance = DMA_INST_HPDMA1, .channel = 3,
    .irq = STM32_IRQ_HPDMA1_CH3, .fifo_bytes = 16, .has_2d = false,
    .free = true, .base = STM32_HPDMA1_BASE + CH_BASE_OFFSET(3)
  },
  {
    .dma_instance = DMA_INST_HPDMA1, .channel = 4,
    .irq = STM32_IRQ_HPDMA1_CH4, .fifo_bytes = 16, .has_2d = false,
    .free = true, .base = STM32_HPDMA1_BASE + CH_BASE_OFFSET(4)
  },
  {
    .dma_instance = DMA_INST_HPDMA1, .channel = 5,
    .irq = STM32_IRQ_HPDMA1_CH5, .fifo_bytes = 16, .has_2d = false,
    .free = true, .base = STM32_HPDMA1_BASE + CH_BASE_OFFSET(5)
  },
  {
    .dma_instance = DMA_INST_HPDMA1, .channel = 6,
    .irq = STM32_IRQ_HPDMA1_CH6, .fifo_bytes = 16, .has_2d = false,
    .free = true, .base = STM32_HPDMA1_BASE + CH_BASE_OFFSET(6)
  },
  {
    .dma_instance = DMA_INST_HPDMA1, .channel = 7,
    .irq = STM32_IRQ_HPDMA1_CH7, .fifo_bytes = 16, .has_2d = false,
    .free = true, .base = STM32_HPDMA1_BASE + CH_BASE_OFFSET(7)
  },
  {
    .dma_instance = DMA_INST_HPDMA1, .channel = 8,
    .irq = STM32_IRQ_HPDMA1_CH8, .fifo_bytes = 16, .has_2d = false,
    .free = true, .base = STM32_HPDMA1_BASE + CH_BASE_OFFSET(8)
  },
  {
    .dma_instance = DMA_INST_HPDMA1, .channel = 9,
    .irq = STM32_IRQ_HPDMA1_CH9, .fifo_bytes = 16, .has_2d = false,
    .free = true, .base = STM32_HPDMA1_BASE + CH_BASE_OFFSET(9)
  },
  {
    .dma_instance = DMA_INST_HPDMA1, .channel = 10,
    .irq = STM32_IRQ_HPDMA1_CH10, .fifo_bytes = 16, .has_2d = false,
    .free = true, .base = STM32_HPDMA1_BASE + CH_BASE_OFFSET(10)
  },
  {
    .dma_instance = DMA_INST_HPDMA1, .channel = 11,
    .irq = STM32_IRQ_HPDMA1_CH11, .fifo_bytes = 16, .has_2d = false,
    .free = true, .base = STM32_HPDMA1_BASE + CH_BASE_OFFSET(11)
  },

  /* HPDMA1 channels 12-15 (AXI, 64-byte FIFO, 2D capable) */

  {
    .dma_instance = DMA_INST_HPDMA1, .channel = 12,
    .irq = STM32_IRQ_HPDMA1_CH12, .fifo_bytes = 64, .has_2d = true,
    .free = true, .base = STM32_HPDMA1_BASE + CH_BASE_OFFSET(12)
  },
  {
    .dma_instance = DMA_INST_HPDMA1, .channel = 13,
    .irq = STM32_IRQ_HPDMA1_CH13, .fifo_bytes = 64, .has_2d = true,
    .free = true, .base = STM32_HPDMA1_BASE + CH_BASE_OFFSET(13)
  },
  {
    .dma_instance = DMA_INST_HPDMA1, .channel = 14,
    .irq = STM32_IRQ_HPDMA1_CH14, .fifo_bytes = 64, .has_2d = true,
    .free = true, .base = STM32_HPDMA1_BASE + CH_BASE_OFFSET(14)
  },
  {
    .dma_instance = DMA_INST_HPDMA1, .channel = 15,
    .irq = STM32_IRQ_HPDMA1_CH15, .fifo_bytes = 64, .has_2d = true,
    .free = true, .base = STM32_HPDMA1_BASE + CH_BASE_OFFSET(15)
  },
#endif /* CONFIG_STM32N6_HPDMA1 */

#ifdef CONFIG_STM32N6_GPDMA1
  /* GPDMA1 channels 0-11 (AHB, 8-byte FIFO) */

  {
    .dma_instance = DMA_INST_GPDMA1, .channel = 0,
    .irq = STM32_IRQ_GPDMA1_CH0, .fifo_bytes = 8, .has_2d = false,
    .free = true, .base = STM32_GPDMA1_BASE + CH_BASE_OFFSET(0)
  },
  {
    .dma_instance = DMA_INST_GPDMA1, .channel = 1,
    .irq = STM32_IRQ_GPDMA1_CH1, .fifo_bytes = 8, .has_2d = false,
    .free = true, .base = STM32_GPDMA1_BASE + CH_BASE_OFFSET(1)
  },
  {
    .dma_instance = DMA_INST_GPDMA1, .channel = 2,
    .irq = STM32_IRQ_GPDMA1_CH2, .fifo_bytes = 8, .has_2d = false,
    .free = true, .base = STM32_GPDMA1_BASE + CH_BASE_OFFSET(2)
  },
  {
    .dma_instance = DMA_INST_GPDMA1, .channel = 3,
    .irq = STM32_IRQ_GPDMA1_CH3, .fifo_bytes = 8, .has_2d = false,
    .free = true, .base = STM32_GPDMA1_BASE + CH_BASE_OFFSET(3)
  },
  {
    .dma_instance = DMA_INST_GPDMA1, .channel = 4,
    .irq = STM32_IRQ_GPDMA1_CH4, .fifo_bytes = 8, .has_2d = false,
    .free = true, .base = STM32_GPDMA1_BASE + CH_BASE_OFFSET(4)
  },
  {
    .dma_instance = DMA_INST_GPDMA1, .channel = 5,
    .irq = STM32_IRQ_GPDMA1_CH5, .fifo_bytes = 8, .has_2d = false,
    .free = true, .base = STM32_GPDMA1_BASE + CH_BASE_OFFSET(5)
  },
  {
    .dma_instance = DMA_INST_GPDMA1, .channel = 6,
    .irq = STM32_IRQ_GPDMA1_CH6, .fifo_bytes = 8, .has_2d = false,
    .free = true, .base = STM32_GPDMA1_BASE + CH_BASE_OFFSET(6)
  },
  {
    .dma_instance = DMA_INST_GPDMA1, .channel = 7,
    .irq = STM32_IRQ_GPDMA1_CH7, .fifo_bytes = 8, .has_2d = false,
    .free = true, .base = STM32_GPDMA1_BASE + CH_BASE_OFFSET(7)
  },
  {
    .dma_instance = DMA_INST_GPDMA1, .channel = 8,
    .irq = STM32_IRQ_GPDMA1_CH8, .fifo_bytes = 8, .has_2d = false,
    .free = true, .base = STM32_GPDMA1_BASE + CH_BASE_OFFSET(8)
  },
  {
    .dma_instance = DMA_INST_GPDMA1, .channel = 9,
    .irq = STM32_IRQ_GPDMA1_CH9, .fifo_bytes = 8, .has_2d = false,
    .free = true, .base = STM32_GPDMA1_BASE + CH_BASE_OFFSET(9)
  },
  {
    .dma_instance = DMA_INST_GPDMA1, .channel = 10,
    .irq = STM32_IRQ_GPDMA1_CH10, .fifo_bytes = 8, .has_2d = false,
    .free = true, .base = STM32_GPDMA1_BASE + CH_BASE_OFFSET(10)
  },
  {
    .dma_instance = DMA_INST_GPDMA1, .channel = 11,
    .irq = STM32_IRQ_GPDMA1_CH11, .fifo_bytes = 8, .has_2d = false,
    .free = true, .base = STM32_GPDMA1_BASE + CH_BASE_OFFSET(11)
  },

  /* GPDMA1 channels 12-15 (AHB, 32-byte FIFO, 2D capable) */

  {
    .dma_instance = DMA_INST_GPDMA1, .channel = 12,
    .irq = STM32_IRQ_GPDMA1_CH12, .fifo_bytes = 32, .has_2d = true,
    .free = true, .base = STM32_GPDMA1_BASE + CH_BASE_OFFSET(12)
  },
  {
    .dma_instance = DMA_INST_GPDMA1, .channel = 13,
    .irq = STM32_IRQ_GPDMA1_CH13, .fifo_bytes = 32, .has_2d = true,
    .free = true, .base = STM32_GPDMA1_BASE + CH_BASE_OFFSET(13)
  },
  {
    .dma_instance = DMA_INST_GPDMA1, .channel = 14,
    .irq = STM32_IRQ_GPDMA1_CH14, .fifo_bytes = 32, .has_2d = true,
    .free = true, .base = STM32_GPDMA1_BASE + CH_BASE_OFFSET(14)
  },
  {
    .dma_instance = DMA_INST_GPDMA1, .channel = 15,
    .irq = STM32_IRQ_GPDMA1_CH15, .fifo_bytes = 32, .has_2d = true,
    .free = true, .base = STM32_GPDMA1_BASE + CH_BASE_OFFSET(15)
  },
#endif /* CONFIG_STM32N6_GPDMA1 */
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static inline uint32_t gpdmach_getreg(struct gpdma_ch_s *chan,
                                      uint32_t offset)
{
  return getreg32(chan->base + offset);
}

static inline void gpdmach_putreg(struct gpdma_ch_s *chan, uint32_t offset,
                                  uint32_t value)
{
  putreg32(value, chan->base + offset);
}

static inline void gpdmach_modifyreg32(struct gpdma_ch_s *chan,
                                       uint32_t offset, uint32_t clrbits,
                                       uint32_t setbits)
{
  modifyreg32(chan->base + offset, clrbits, setbits);
}

/****************************************************************************
 * Name: gpdma_dmainterrupt
 *
 * Description:
 *   DMA interrupt handler.
 *
 ****************************************************************************/

static int gpdma_dmainterrupt(int irq, void *context, void *arg)
{
  struct gpdma_ch_s *chan = (struct gpdma_ch_s *)arg;
  uint32_t status;

  DEBUGASSERT(chan != NULL);

  /* Get the interrupt status for this channel (bits 8-14 → 0-6) */

  status = (gpdmach_getreg(chan, CH_CXSR_OFFSET) >> 8) & 0x7f;

  if (chan->callback)
    {
      /* Interrupt-based mode: clear flags + DSB to prevent NVIC
       * re-latch, then invoke the callback if status is non-zero.
       */

      gpdmach_putreg(chan, CH_CXFCR_OFFSET, ~0);
      __asm volatile ("dsb sy" ::: "memory");

      if (status != 0)
        {
          chan->callback(chan, (uint8_t)status, chan->arg);
        }
    }
  else
    {
      /* Polled mode: disable interrupt enables to stop further IRQs,
       * but do NOT clear flags — the polled reader needs to see TCF.
       */

      gpdmach_modifyreg32(chan, CH_CXCR_OFFSET, GPDMA_CXCR_ALLINTS, 0);
      __asm volatile ("dsb sy" ::: "memory");
    }

  return 0;
}

/****************************************************************************
 * Name: gpdma_ch_abort
 *
 * Description:
 *   Suspend and abort any ongoing channel transfers.
 *
 ****************************************************************************/

static void gpdma_ch_abort(struct gpdma_ch_s *chan)
{
  int timeout;

  if ((gpdmach_getreg(chan, CH_CXCR_OFFSET) & GPDMA_CXCR_EN) == 0)
    {
      return;
    }

  /* Suspend the channel first.  For idle channels (after TC) this
   * completes immediately.  For active channels it waits for the
   * current beat to finish.
   */

  gpdmach_modifyreg32(chan, CH_CXCR_OFFSET, 0, GPDMA_CXCR_SUSP);

  timeout = 100000;
  while ((gpdmach_getreg(chan, CH_CXSR_OFFSET) & GPDMA_CXSR_SUSPF) == 0)
    {
      if (--timeout == 0)
        {
          break;
        }
    }

  /* Disable the channel by clearing CxCR entirely.  This avoids the
   * RESET operation which clears CxCIDCFGR and other per-channel
   * security state that is difficult to fully restore.
   */

  gpdmach_putreg(chan, CH_CXCR_OFFSET, 0);
}

/****************************************************************************
 * Name: gpdma_ch_disable
 *
 * Description:
 *   Disable the DMA channel.
 *
 ****************************************************************************/

static void gpdma_ch_disable(struct gpdma_ch_s *chan)
{
  int ext_irq;

  DEBUGASSERT(chan != NULL);

  gpdma_ch_abort(chan);

  /* Disable and clear all interrupts */

  gpdmach_modifyreg32(chan, CH_CXCR_OFFSET, GPDMA_CXCR_ALLINTS, 0);
  gpdmach_putreg(chan, CH_CXFCR_OFFSET, ~0);

  /* Clear NVIC pending bit for this channel's IRQ.  After a completed
   * transfer, the DMA hardware may latch a new interrupt between the
   * ISR clearing CxFCR and the channel being disabled.  Without this,
   * re-enabling the channel causes a stale interrupt to fire immediately,
   * reading empty status and consuming the semaphore before the real
   * transfer completes.
   */

  ext_irq = chan->irq - STM32_IRQ_FIRST;
  putreg32(1 << (ext_irq & 31), NVIC_IRQ_CLRPEND(ext_irq));

  /* Restore CxCIDCFGR — channel RESET clears this register.
   * Without CID filtering, the DMA channel may present CID=0 which
   * can cause RIFSC to block peripheral access on subsequent transfers.
   * CFEN=1, SCID=1 (Cortex-M55 CID).
   */

  gpdmach_putreg(chan, CH_CXCIDCFGR_OFFSET, 0x11);

  /* Ensure all register writes are committed and pipeline is flushed
   * before the channel can be reconfigured for the next transfer.
   */

  __asm volatile ("dsb sy" ::: "memory");
  __asm volatile ("isb sy" ::: "memory");
}

/****************************************************************************
 * Name: gpdma_setup
 *
 * Description:
 *   Standard linear DMA transfer setup.
 *
 ****************************************************************************/

static int gpdma_setup(struct gpdma_ch_s *chan,
                       struct stm32_gpdma_cfg_s *cfg)
{
  uint32_t reg;

  /* Disable linked list mode */

  gpdmach_modifyreg32(chan, CH_CXLLR_OFFSET, ~0, 0);
  gpdmach_putreg(chan, CH_CXLBAR_OFFSET, 0);

  /* Set source and destination addresses */

  gpdmach_putreg(chan, CH_CXSAR_OFFSET, cfg->src_addr);
  gpdmach_putreg(chan, CH_CXDAR_OFFSET, cfg->dest_addr);

  /* Set channel priority */

  gpdmach_modifyreg32(chan, CH_CXCR_OFFSET, GPDMA_CXCR_PRIO_MASK,
                      cfg->priority << GPDMA_CXCR_PRIO_SHIFT);

  /* Set TR1 register.  Always set SSEC and DSEC because we run in
   * Secure state and all SRAM is Secure.  Both HPDMA1 and GPDMA1
   * require Secure transactions to access Secure memory regions.
   */

  reg = cfg->tr1 | GPDMA_CXTR1_SSEC | GPDMA_CXTR1_DSEC;

  /* Set SAP/DAP for HPDMA1 P2M/M2P transfers.  HPDMA1 Port 0 = AXI
   * (memory), Port 1 = AHB (peripherals).  For GPDMA1 both ports are
   * AHB so port selection doesn't matter (leave at default 0).
   */

  if (chan->dma_instance == DMA_INST_HPDMA1)
    {
      if (chan->type == GPDMA_TTYPE_P2M)
        {
          reg |= GPDMA_CXTR1_SAP;   /* Source = AHB peripheral port */
        }
      else if (chan->type == GPDMA_TTYPE_M2P)
        {
          reg |= GPDMA_CXTR1_DAP;   /* Dest = AHB peripheral port */
        }
    }

  /* Auto-compute burst length for M2M if caller hasn't set SBL_1/DBL_1.
   * Max burst per channel is fifo_bytes/4 (in words), per DS14791
   * Tables 8/9.  For sub-word data widths, burst in beats could exceed
   * the hardware's word-based limit, so cap accordingly.
   * For P2M/M2P leave at caller's value since the peripheral may have
   * FIFO constraints.
   */

  if (chan->type == GPDMA_TTYPE_M2M_LINEAR &&
      (reg & (GPDMA_CXTR1_SBL_1_MASK | GPDMA_CXTR1_DBL_1_MASK)) == 0)
    {
      uint32_t max_burst = chan->fifo_bytes / 4;

      if (max_burst >= 4)
        {
          reg |= GPDMA_CXTR1_SBL_1(max_burst)
              |  GPDMA_CXTR1_DBL_1(max_burst);
        }
    }

  gpdmach_putreg(chan, CH_CXTR1_OFFSET, reg);

  /* Assemble TR2: request selection + direction bits */

  reg = (uint32_t)cfg->request &
        (GPDMA_CXTR2_REQSEL_MASK | GPDMA_CXTR2_DREQ | GPDMA_CXTR2_SWREQ);
  gpdmach_putreg(chan, CH_CXTR2_OFFSET, reg);

  /* Set block byte count */

  gpdmach_putreg(chan, CH_CXBR1_OFFSET, cfg->ntransfers);

  return 0;
}

/****************************************************************************
 * Name: gpdma_setup_circular
 *
 * Description:
 *   Circular DMA setup using a 2-element linked list.
 *
 ****************************************************************************/

static int gpdma_setup_circular(struct gpdma_ch_s *chan,
                                struct stm32_gpdma_cfg_s *cfg)
{
  struct stm32_gpdma_lli_s *lli = chan->lli;

  lli[0].tr1 = cfg->tr1 | GPDMA_CXTR1_SSEC | GPDMA_CXTR1_DSEC;
  lli[0].tr2 = (2U << GPDMA_CXTR2_TCEM_SHIFT)
             | (cfg->request & GPDMA_CXTR2_REQSEL_MASK);
  lli[0].br1 = cfg->ntransfers;
  lli[0].sar = cfg->src_addr;
  lli[0].dar = cfg->dest_addr;
  lli[0].llr = (GPDMA_CXLLR_UT1
             | GPDMA_CXLLR_UT2
             | GPDMA_CXLLR_UB1
             | GPDMA_CXLLR_USA
             | GPDMA_CXLLR_UDA
             | GPDMA_CXLLR_ULL)
             | (((uint32_t)&lli[1]) & GPDMA_CXLLR_LA_MASK);

  lli[1].tr1 = lli[0].tr1;
  lli[1].tr2 = lli[0].tr2;
  lli[1].br1 = lli[0].br1;
  lli[1].sar = lli[0].sar;
  lli[1].dar = lli[0].dar;
  lli[1].llr = (lli[0].llr & ~GPDMA_CXLLR_LA_MASK)
             | (((uint32_t)&lli[0]) & GPDMA_CXLLR_LA_MASK);

  gpdmach_putreg(chan, CH_CXSAR_OFFSET,   lli[0].sar);
  gpdmach_putreg(chan, CH_CXDAR_OFFSET,   lli[0].dar);
  gpdmach_putreg(chan, CH_CXTR1_OFFSET,   lli[0].tr1);
  gpdmach_putreg(chan, CH_CXTR2_OFFSET,   lli[0].tr2);
  gpdmach_putreg(chan, CH_CXBR1_OFFSET,   lli[0].br1);
  gpdmach_putreg(chan, CH_CXLBAR_OFFSET,  (uint32_t)&lli[0]);
  gpdmach_putreg(chan, CH_CXLLR_OFFSET,   lli[0].llr);

  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: arm_dma_initialize
 *
 * Description:
 *   Initialize the DMA subsystem. Enable clocks for enabled DMA
 *   controllers and attach interrupt handlers for all channels.
 *
 ****************************************************************************/

void weak_function arm_dma_initialize(void)
{
  struct gpdma_ch_s *chan;
  int i;

  /* Enable DMA controller clocks using SET registers (STM32N6 pattern) */

#ifdef CONFIG_STM32N6_HPDMA1
  /* HPDMA1 uses an AXI master port to access AXISRAM.  The AXI
   * interconnect clock (ACLKN) and companion clock (ACLKNC) must be
   * enabled for HPDMA1 data transfers to reach memory.  Without
   * these, HPDMA1 completes transfers (TC fires) but writes nothing.
   */

  putreg32(RCC_BUSENR_ACLKNEN | RCC_BUSENR_ACLKNCEN,
           STM32_RCC_BUSENSR);
  putreg32(RCC_AHB5ENR_HPDMA1EN | RCC_AHB5ENR_CACHEAXIEN,
           STM32_RCC_AHB5ENSR);
#endif

#ifdef CONFIG_STM32N6_GPDMA1
  putreg32(RCC_AHB1ENR_GPDMA1EN, STM32_RCC_AHB1ENSR);
#endif

  /* Mark all channels as Secure and Privileged.  We run in Secure
   * Privileged state (TrustZone cannot be disabled on STM32N6), so
   * DMA channels must be configured as Secure+Privileged to access
   * Secure memory regions (SRAM at 0x34000000).
   */

#ifdef CONFIG_STM32N6_HPDMA1
  putreg32(0xffff, STM32_HPDMA1_BASE + STM32_GPDMA_SECCFGR_OFFSET);
  putreg32(0xffff, STM32_HPDMA1_BASE + STM32_GPDMA_PRIVCFGR_OFFSET);
#endif

#ifdef CONFIG_STM32N6_GPDMA1
  putreg32(0xffff, STM32_GPDMA1_BASE + STM32_GPDMA_SECCFGR_OFFSET);
  putreg32(0xffff, STM32_GPDMA1_BASE + STM32_GPDMA_PRIVCFGR_OFFSET);
#endif

  /* Initialize each DMA channel */

  for (i = 0; i < (int)DMA_NCHANNELS; i++)
    {
      chan = &g_chan[i];

      /* Enable CID filtering with static CID=1 (Cortex-M55) per ST HAL
       * recommendation: "It is recommended to always enable the isolation
       * feature (CFEN=1) of the HPDMA channel with default CID=1."
       * CCIDCFGR: bit 0 = CFEN, bits [6:4] = SCID.
       */

      gpdmach_putreg(chan, CH_CXCIDCFGR_OFFSET, 0x11);

      /* Attach DMA interrupt vector */

      irq_attach(chan->irq, gpdma_dmainterrupt, chan);

      /* Disable the DMA channel */

      gpdma_ch_disable(chan);

      /* Enable the IRQ at the NVIC */

      up_enable_irq(chan->irq);
    }
}

/****************************************************************************
 * Name: gpdma_alloc_first_free
 *
 * Description:
 *   Scan g_chan[] for a free channel matching instance/channel constraints.
 *   Returns the channel pointer if found, or NULL.
 *
 ****************************************************************************/

static struct gpdma_ch_s *gpdma_alloc_first_free(int instance,
                                                  int ch_lo, int ch_hi)
{
  int i;

  for (i = 0; i < (int)DMA_NCHANNELS; i++)
    {
      struct gpdma_ch_s *chan = &g_chan[i];

      if (chan->free &&
          (instance == 0 || chan->dma_instance == instance) &&
          chan->channel >= ch_lo && chan->channel <= ch_hi)
        {
          return chan;
        }
    }

  return NULL;
}

/****************************************************************************
 * Name: stm32_dmachannel
 *
 * Description:
 *   Allocate a DMA channel based on transfer type with type-aware selection:
 *
 *   P2M / M2P: Prefer GPDMA1 ch0-11 (small FIFO, low power on AHB),
 *     fall back to HPDMA1 ch0-11, then any remaining channel.
 *
 *   M2M_LINEAR: Prefer GPDMA1 ch12-15 (large FIFO), fall back to
 *     HPDMA1 ch12-15, then ch0-11 of either engine.
 *
 *   2D: Only channels 12-15 (has_2d == true) of either engine.
 *
 ****************************************************************************/

DMA_HANDLE stm32_dmachannel(enum gpdma_ttype_e type)
{
  struct gpdma_ch_s *chan = NULL;
  irqstate_t flags;

  flags = enter_critical_section();

  switch (type)
    {
      case GPDMA_TTYPE_P2M:
      case GPDMA_TTYPE_M2P:

        /* Prefer GPDMA1 small-FIFO channels (lower power, AHB) */

        chan = gpdma_alloc_first_free(DMA_INST_GPDMA1, 0, 11);
        if (chan == NULL)
          {
            chan = gpdma_alloc_first_free(DMA_INST_HPDMA1, 0, 11);
          }

        if (chan == NULL)
          {
            chan = gpdma_alloc_first_free(0, 0, 15);
          }

        break;

      case GPDMA_TTYPE_M2M_LINEAR:

        /* Prefer large-FIFO channels for bulk memory copies */

        chan = gpdma_alloc_first_free(DMA_INST_GPDMA1, 12, 15);
        if (chan == NULL)
          {
            chan = gpdma_alloc_first_free(DMA_INST_HPDMA1, 12, 15);
          }

        if (chan == NULL)
          {
            chan = gpdma_alloc_first_free(0, 0, 11);
          }

        break;

      case GPDMA_TTYPE_2D:

        /* 2D addressing only available on ch12-15 */

        chan = gpdma_alloc_first_free(DMA_INST_GPDMA1, 12, 15);
        if (chan == NULL)
          {
            chan = gpdma_alloc_first_free(DMA_INST_HPDMA1, 12, 15);
          }

        break;
    }

  if (chan != NULL)
    {
      chan->free = false;
      chan->type = type;
    }

  leave_critical_section(flags);

  if (chan == NULL)
    {
      dmaerr("No available DMA channel for transfer type=%d\n", type);
    }

  return (DMA_HANDLE)chan;
}

/****************************************************************************
 * Name: stm32_dmachannel_inst
 *
 * Description:
 *   Allocate a DMA channel from a specific DMA instance.
 *   instance: DMA_INST_HPDMA1 (1) or DMA_INST_GPDMA1 (2).
 *
 ****************************************************************************/

DMA_HANDLE stm32_dmachannel_inst(int instance, enum gpdma_ttype_e type)
{
  DMA_HANDLE handle = NULL;
  irqstate_t flags;
  int i;

  DEBUGASSERT(type != GPDMA_TTYPE_2D);

  flags = enter_critical_section();

  for (i = 0; i < (int)DMA_NCHANNELS; i++)
    {
      struct gpdma_ch_s *chan = &g_chan[i];

      if (chan->free && chan->dma_instance == instance)
        {
          chan->free = false;
          chan->type = type;
          handle = (DMA_HANDLE)chan;
          break;
        }
    }

  leave_critical_section(flags);
  return handle;
}

/****************************************************************************
 * Name: stm32_dmachannel_range
 *
 * Description:
 *   Allocate a DMA channel from a specific instance and channel range.
 *   Useful for testing or when a specific FIFO size is needed.
 *
 ****************************************************************************/

DMA_HANDLE stm32_dmachannel_range(int instance, int ch_lo, int ch_hi,
                                   enum gpdma_ttype_e type)
{
  struct gpdma_ch_s *chan;
  irqstate_t flags;

  flags = enter_critical_section();

  chan = gpdma_alloc_first_free(instance, ch_lo, ch_hi);
  if (chan != NULL)
    {
      chan->free = false;
      chan->type = type;
    }

  leave_critical_section(flags);
  return (DMA_HANDLE)chan;
}

/****************************************************************************
 * Name: stm32_dmafree
 *
 * Description:
 *   Release a DMA channel.
 *
 ****************************************************************************/

void stm32_dmafree(DMA_HANDLE handle)
{
  struct gpdma_ch_s *chan = (struct gpdma_ch_s *)handle;

  DEBUGASSERT(handle != NULL);

  chan->free = true;
}

/****************************************************************************
 * Name: stm32_dmasetup
 *
 * Description:
 *   Configure DMA before using.
 *
 ****************************************************************************/

void stm32_dmasetup(DMA_HANDLE handle, struct stm32_gpdma_cfg_s *cfg)
{
  struct gpdma_ch_s *chan = (struct gpdma_ch_s *)handle;

  DEBUGASSERT(handle != NULL);

  /* Store the configuration */

  chan->cfg = *cfg;

  /* Disable channel first */

  gpdma_ch_disable(chan);

  /* Clear any unhandled flags from previous transactions */

  gpdmach_putreg(chan, CH_CXFCR_OFFSET, ~0);

  if (cfg->mode & GPDMACFG_MODE_CIRC)
    {
      gpdma_setup_circular(chan, cfg);
    }
  else
    {
      gpdma_setup(chan, cfg);
    }
}

/****************************************************************************
 * Name: stm32_dmastart
 *
 * Description:
 *   Start the DMA transfer.
 *
 ****************************************************************************/

void stm32_dmastart(DMA_HANDLE handle, dma_callback_t callback, void *arg,
                    bool half)
{
  struct gpdma_ch_s *chan = (struct gpdma_ch_s *)handle;
  uint32_t cr;

  DEBUGASSERT(handle != NULL);

  /* Save the callback info */

  chan->callback = callback;
  chan->arg = arg;

  /* Build CR value: enable + interrupt bits.
   * Interrupt enables are ALWAYS set — the STM32N6 GPDMA appears to
   * require TCIE for the channel to actually run the data transfer.
   */

  cr = gpdmach_getreg(chan, CH_CXCR_OFFSET);
  cr |= GPDMA_CXCR_EN;

  if (chan->cfg.mode & GPDMACFG_MODE_CIRC)
    {
      cr |= ((half ? GPDMA_CXCR_HTIE : 0) |
              GPDMA_CXCR_TCIE |
              GPDMA_CXCR_DTEIE);
    }
  else
    {
      cr |= (half ? (GPDMA_CXCR_HTIE | GPDMA_CXCR_DTEIE |
                      GPDMA_CXCR_USEIE | GPDMA_CXCR_ULEIE) :
                     (GPDMA_CXCR_TCIE | GPDMA_CXCR_DTEIE |
                      GPDMA_CXCR_USEIE | GPDMA_CXCR_ULEIE));
    }

  gpdmach_putreg(chan, CH_CXCR_OFFSET, cr);
}

/****************************************************************************
 * Name: stm32_dmastop
 *
 * Description:
 *   Cancel the DMA.
 *
 ****************************************************************************/

void stm32_dmastop(DMA_HANDLE handle)
{
  struct gpdma_ch_s *chan = (struct gpdma_ch_s *)handle;
  gpdma_ch_disable(chan);
}

/****************************************************************************
 * Name: stm32_dmapollwait
 *
 * Description:
 *   Poll for DMA transfer completion (TCF).  Use this instead of
 *   interrupt-based notification for short transfers to avoid NVIC
 *   races.  The DMA channel must have been started with callback=NULL
 *   (no interrupt enables).
 *
 ****************************************************************************/

int stm32_dmapollwait(DMA_HANDLE handle, uint32_t timeout_us)
{
  struct gpdma_ch_s *chan = (struct gpdma_ch_s *)handle;
  uint32_t sr;

  DEBUGASSERT(handle != NULL);

  while (timeout_us > 0)
    {
      sr = gpdmach_getreg(chan, CH_CXSR_OFFSET);

      if (sr & GPDMA_CXSR_TCF)
        {
          gpdmach_putreg(chan, CH_CXFCR_OFFSET, ~0);
          return OK;
        }

      if (sr & (GPDMA_CXSR_DTEF | GPDMA_CXSR_ULEF | GPDMA_CXSR_USEF))
        {
          gpdmach_putreg(chan, CH_CXFCR_OFFSET, ~0);
          return -EIO;
        }

      up_udelay(1);
      timeout_us--;
    }

  return -ETIMEDOUT;
}

/****************************************************************************
 * Name: stm32_dmaresidual
 *
 * Description:
 *   Returns the number of data bytes remaining in the current block.
 *
 ****************************************************************************/

size_t stm32_dmaresidual(DMA_HANDLE handle)
{
  struct gpdma_ch_s *chan = (struct gpdma_ch_s *)handle;
  uint32_t br1 = getreg32(chan->base + CH_CXBR1_OFFSET);

  return (size_t)(br1 & GPDMA_CXBR1_BNDT_MASK);
}

#ifdef CONFIG_STM32N6_DMACAPABLE
bool stm32_dmacapable(DMA_HANDLE handle, struct stm32_gpdma_cfg_s *cfg)
{
  return false;
}
#endif

#ifdef CONFIG_DEBUG_DMA_INFO
void stm32_dmasample(DMA_HANDLE handle, struct stm32_gpdma_reg_s *regs)
{
}

void stm32_dmadump(DMA_HANDLE handle, const char *msg)
{
}
#endif
