/****************************************************************************
 * arch/arm/src/stm32n6/stm32_dma.h
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

#ifndef __ARCH_ARM_SRC_STM32N6_STM32_DMA_H
#define __ARCH_ARM_SRC_STM32N6_STM32_DMA_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <sys/types.h>

#include <stdint.h>

#include "hardware/stm32_gpdma.h"

#ifdef CONFIG_DEBUG_DMA_INFO
#  error "CONFIG_DEBUG_DMA_INFO not yet implemented."
#  undef CONFIG_DEBUG_DMA_INFO
#endif

#ifdef CONFIG_STM32N6_DMACAPABLE
#  error "CONFIG_STM32N6_DMACAPABLE not yet implemented."
#  undef CONFIG_STM32N6_DMACAPABLE
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* DMA callback status bits */

#define DMA_STATUS_TCF        (1 << 0) /* Transfer Complete */
#define DMA_STATUS_HTF        (1 << 1) /* Half Transfer */
#define DMA_STATUS_DTEF       (1 << 2) /* Data transfer error */
#define DMA_STATUS_ULEF       (1 << 3) /* Update link transfer error */
#define DMA_STATUS_USEF       (1 << 4) /* User setting error */
#define DMA_STATUS_SUSPF      (1 << 5) /* Completed suspension flag */
#define DMA_STATUS_TOF        (1 << 6) /* Trigger overrun flag */

#define DMA_STATUS_FATAL      (DMA_STATUS_DTEF | DMA_STATUS_ULEF | \
                               DMA_STATUS_USEF)
#define DMA_STATUS_SUCCESS    (DMA_STATUS_TCF | DMA_STATUS_HTF)

/* GPDMA Mode Flags */

#define GPDMACFG_MODE_CIRC    (1 << 0)  /* Enable Circular mode */
#define GPDMACFG_MODE_PFC     (1 << 1)  /* Peripheral flow control */
#define GPDMACFG_MODE_DB      (1 << 2)  /* Double buffer mode */

/* Channel priority level */

#define GPDMACFG_PRIO_LL      (0)   /* Low priority, low weight */
#define GPDMACFG_PRIO_LM      (1)   /* Low priority, mid weight */
#define GPMDACFG_PRIO_LH      (2)   /* Low priority, high weight */
#define GPDMACFG_PRIO_H       (3)   /* High priority */

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* GPDMA transfer type enumeration */

enum gpdma_ttype_e
{
  GPDMA_TTYPE_P2M = 0,       /* Peripheral-to-memory */
  GPDMA_TTYPE_M2P,           /* Memory-to-peripheral */
  GPDMA_TTYPE_M2M_LINEAR,    /* Memory-to-memory, linear */
  GPDMA_TTYPE_2D             /* 2D Addressing (not implemented) */
};

#ifdef CONFIG_DEBUG_DMA_INFO
struct stm32_gpdma_reg_s
{
  uint32_t cxlbar;
  uint32_t cxfcr;
  uint32_t cxsr;
  uint32_t cxcr;
  uint32_t cxtr1;
  uint32_t cxtr2;
  uint32_t cxbr1;
  uint32_t cxsar;
  uint32_t cxdar;
  uint32_t cxtr3;
  uint32_t cxbr2;
  uint32_t cxllr;
};
#endif

struct stm32_gpdma_cfg_s
{
  uint32_t src_addr;
  uint32_t dest_addr;

  /* CxTR1 register value for the channel. */

  uint32_t tr1;

  /* request: Accepts GPDMA_CXTR2_SWREQ, GPDMA_CXTR2_DREQ, and
   * GPDMA_CXTR2_REQSEL(r) for r given by GPDMA_REQ_x macros
   * defined in hardware/stm32n6xx_dmasigmap.h
   */

  uint16_t request;

  /* Number of transfers, in units of the data width specified in tr1. */

  uint16_t ntransfers;

  /* Priority level: refer to GPDMACFG_PRIO defines above */

  uint8_t  priority;

  /* mode flags, refer to GPDMACFG_MODE_X defines above. */

  uint8_t  mode;
};

/* DMA_HANDLE provides an opaque reference to a DMA channel. */

typedef void *DMA_HANDLE;

/* DMA completion callback */

typedef void (*dma_callback_t)(DMA_HANDLE handle, uint8_t status, void *arg);

/****************************************************************************
 * Public Data
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
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_dmachannel
 *
 * Description:
 *   Allocate a DMA channel based on provided transfer type.
 *
 ****************************************************************************/

DMA_HANDLE stm32_dmachannel(enum gpdma_ttype_e type);

/****************************************************************************
 * Name: stm32_dmachannel_inst
 *
 * Description:
 *   Allocate a DMA channel from a specific DMA instance.
 *   instance: 1 = HPDMA1, 2 = GPDMA1
 *
 ****************************************************************************/

DMA_HANDLE stm32_dmachannel_inst(int instance, enum gpdma_ttype_e type);

/****************************************************************************
 * Name: stm32_dmachannel_range
 *
 * Description:
 *   Allocate a DMA channel from a specific instance and channel range.
 *   instance: 1 = HPDMA1, 2 = GPDMA1
 *   ch_lo/ch_hi: channel number range (0-11 = small FIFO, 12-15 = large)
 *
 ****************************************************************************/

DMA_HANDLE stm32_dmachannel_range(int instance, int ch_lo, int ch_hi,
                                   enum gpdma_ttype_e type);

/****************************************************************************
 * Name: stm32_dmafree
 *
 * Description:
 *   Release a DMA channel.
 *
 ****************************************************************************/

void stm32_dmafree(DMA_HANDLE handle);

/****************************************************************************
 * Name: stm32_dmasetup
 *
 * Description:
 *   Configure DMA before using
 *
 ****************************************************************************/

void stm32_dmasetup(DMA_HANDLE handle, struct stm32_gpdma_cfg_s *cfg);

/****************************************************************************
 * Name: stm32_dmastart
 *
 * Description:
 *   Start the DMA transfer.
 *
 ****************************************************************************/

void stm32_dmastart(DMA_HANDLE handle, dma_callback_t callback, void *arg,
                    bool half);

/****************************************************************************
 * Name: stm32_dmastop
 *
 * Description:
 *   Cancel the DMA.
 *
 ****************************************************************************/

void stm32_dmastop(DMA_HANDLE handle);

/****************************************************************************
 * Name: stm32_dmaresidual
 *
 * Description:
 *   Returns the number of bytes remaining to be transferred
 *
 ****************************************************************************/

size_t stm32_dmaresidual(DMA_HANDLE handle);

/****************************************************************************
 * Name: stm32_dmacapable
 *
 * Description:
 *   Check if the DMA controller can transfer data to/from given memory
 *   address with the given configuration.
 *
 ****************************************************************************/

#ifdef CONFIG_STM32N6_DMACAPABLE
bool stm32_dmacapable(DMA_HANDLE handle, struct stm32_gpdma_cfg_s *cfg);
#else
#  define stm32_dmacapable(handle, cfg) (true)
#endif

#ifdef CONFIG_DEBUG_DMA_INFO
void stm32_dmasample(DMA_HANDLE handle, struct stm32_gpdma_reg_s *regs);
#else
#  define stm32_dmasample(handle,regs)
#endif

#ifdef CONFIG_DEBUG_DMA_INFO
void stm32_dmadump(DMA_HANDLE handle, const char *msg);
#else
#  define stm32_dmadump(handle,msg)
#endif

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* !__ASSEMBLY__*/
#endif /* __ARCH_ARM_SRC_STM32N6_STM32_DMA_H*/
