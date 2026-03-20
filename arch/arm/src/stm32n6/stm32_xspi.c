/****************************************************************************
 * arch/arm/src/stm32n6/stm32_xspi.c
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

#include "stm32_xspi.h"

#include <nuttx/config.h>

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <debug.h>

#include <arch/board/board.h>

#include <nuttx/arch.h>
#include <nuttx/clock.h>
#include <nuttx/kmalloc.h>
#include <nuttx/mutex.h>
#include <nuttx/nuttx.h>
#include <nuttx/semaphore.h>
#include <nuttx/spi/qspi.h>

#include <arch/barriers.h>

#include "arm_internal.h"

#include "stm32_gpio.h"
#include "stm32_rcc.h"
#include "hardware/stm32n6xxx_xspi.h"

#ifdef CONFIG_STM32N6_XSPI2_DMA
#  include <nuttx/cache.h>
#  include "stm32_dma.h"
#  include "stm32_dcache.h"
#endif

#ifdef CONFIG_STM32N6_XSPI

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Sanity check that board.h defines requisite XSPI pinmap options */

#if !defined(GPIO_XSPI2_CS) || !defined(GPIO_XSPI2_CLK) || \
    !defined(GPIO_XSPI2_D0) || !defined(GPIO_XSPI2_D1) || \
    !defined(GPIO_XSPI2_D2) || !defined(GPIO_XSPI2_D3)
#  error "You must define GPIO_XSPI2_CS, GPIO_XSPI2_CLK, " \
         "GPIO_XSPI2_D0..D3 in your board.h"
#endif

#if !defined(CONFIG_STM32N6_XSPI_FLASH_SIZE) || \
    0 == CONFIG_STM32N6_XSPI_FLASH_SIZE
#  define CONFIG_STM32N6_XSPI_FLASH_SIZE (64 * 1024 * 1024)
#endif

#ifndef CONFIG_STM32N6_XSPI_FIFO_THESHOLD
#  define CONFIG_STM32N6_XSPI_FIFO_THESHOLD 4
#endif

#ifndef CONFIG_STM32N6_XSPI_CSHT
#  define CONFIG_STM32N6_XSPI_CSHT 3
#endif

/* XSPI2 clock: IC3 = PLL1/6 = 200 MHz (configured externally) */

#ifndef STM32_XSPI_FREQUENCY
#  define STM32_XSPI_FREQUENCY  200000000ul
#endif

#define XSPI_CLK_FREQUENCY  STM32_XSPI_FREQUENCY

#ifdef CONFIG_STM32N6_XSPI2_DMA
#  ifndef CONFIG_STM32N6_XSPI_DMATHRESHOLD
#    define CONFIG_STM32N6_XSPI_DMATHRESHOLD 4
#  endif
#  define XSPI_DMA_THRESHOLD  CONFIG_STM32N6_XSPI_DMATHRESHOLD
#  define XSPI_DMA_BUFSIZE    4096
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct stm32_xspidev_s
{
  struct qspi_dev_s xspi;       /* Externally visible QSPI interface */
  uint32_t base;                /* XSPI controller register base address */
  uint32_t frequency;           /* Requested clock frequency */
  uint32_t actual;              /* Actual clock frequency */
  uint8_t mode;                 /* Mode 0,3 */
  uint8_t nbits;                /* Width of word in bits (8) */
  uint8_t intf;                 /* XSPI controller number (0) */
  bool initialized;             /* TRUE: Controller has been initialized */
  mutex_t lock;                 /* Assures mutually exclusive access */
  bool memmap;                  /* TRUE: Controller is in memory mapped mode */
  bool mmap_saved;              /* TRUE: mmap config saved for auto-resume */
  struct qspi_meminfo_s mmap_info;  /* Saved mmap config for resume */
  uint32_t mmap_lpto;           /* Saved mmap timeout */

#ifdef CONFIG_STM32N6_XSPI_INTERRUPTS
  xcpt_t handler;               /* Interrupt handler */
  uint8_t irq;                  /* Interrupt number */
  sem_t op_sem;                 /* Block until complete */
  struct xspi_xctnspec_s *xctn; /* Context of transaction in progress */
#endif

#ifdef CONFIG_STM32N6_XSPI2_DMA
  DMA_HANDLE rxdma;             /* RX DMA channel handle */
  DMA_HANDLE txdma;             /* TX DMA channel handle */
  uint8_t rxdma_buf[XSPI_DMA_BUFSIZE] aligned_data(32);
  uint8_t txdma_buf[XSPI_DMA_BUFSIZE] aligned_data(32);
#endif
};

/* Transaction descriptor */

struct xspi_xctnspec_s
{
  uint8_t instrmode;      /* Instruction mode; 0=none, 1=single, 4=octal */
  uint8_t instr;          /* The 8-bit instruction */

  uint8_t addrmode;       /* Address mode; 0=none, 1=single, 4=octal */
  uint8_t addrsize;       /* Address size (n - 1); 0, 1, 2, 3 */
  uint32_t addr;          /* The address (1 to 4 bytes) */

  uint8_t altbytesmode;   /* Alt bytes mode */
  uint8_t altbytessize;   /* Alt bytes size */
  uint32_t altbytes;      /* The alt bytes */

  uint8_t dummycycles;    /* Number of dummy cycles; 0-31 */

  uint8_t datamode;       /* Data mode; 0=none, 1=single, 4=octal */
  uint32_t datasize;      /* Number of data bytes */
  void *buffer;           /* Data buffer */

  uint8_t isddr;          /* True if DDR */

#ifdef CONFIG_STM32N6_XSPI_INTERRUPTS
  uint8_t function;       /* Functional mode; to distinguish read/write */
  int8_t disposition;     /* Result of the transfer */
  uint32_t idxnow;        /* Index into data buffer */
#endif
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static inline uint32_t xspi_getreg(struct stm32_xspidev_s *priv,
                                   unsigned int offset);
static inline void xspi_putreg(struct stm32_xspidev_s *priv,
                               uint32_t value, unsigned int offset);

#ifdef CONFIG_STM32N6_XSPI_INTERRUPTS
static int xspi0_interrupt(int irq, void *context, void *arg);
#endif

static int      xspi_lock(struct qspi_dev_s *dev, bool lock);
static uint32_t xspi_setfrequency(struct qspi_dev_s *dev,
                                  uint32_t frequency);
static void     xspi_setmode(struct qspi_dev_s *dev,
                             enum qspi_mode_e mode);
static void     xspi_setbits(struct qspi_dev_s *dev, int nbits);
static int      xspi_command(struct qspi_dev_s *dev,
                             struct qspi_cmdinfo_s *cmdinfo);
static int      xspi_memory(struct qspi_dev_s *dev,
                            struct qspi_meminfo_s *meminfo);
static void    *xspi_alloc(struct qspi_dev_s *dev, size_t buflen);
static void     xspi_free(struct qspi_dev_s *dev, void *buffer);

static int      xspi_hw_initialize(struct stm32_xspidev_s *priv);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct qspi_ops_s g_xspi0ops =
{
  .lock              = xspi_lock,
  .setfrequency      = xspi_setfrequency,
  .setmode           = xspi_setmode,
  .setbits           = xspi_setbits,
#ifdef CONFIG_QSPI_HWFEATURES
  .hwfeatures        = NULL,
#endif
  .command           = xspi_command,
  .memory            = xspi_memory,
  .alloc             = xspi_alloc,
  .free              = xspi_free,
};

static struct stm32_xspidev_s g_xspi0dev =
{
  .xspi              =
  {
    .ops             = &g_xspi0ops,
  },
  .base              = STM32_XSPI2_BASE,
  .lock              = NXMUTEX_INITIALIZER,
#ifdef CONFIG_STM32N6_XSPI_INTERRUPTS
  .handler           = xspi0_interrupt,
  .irq               = STM32_IRQ_XSPI2,
  .op_sem            = SEM_INITIALIZER(0),
#endif
  .intf              = 0,
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: xspi_getreg / xspi_putreg
 ****************************************************************************/

static inline uint32_t xspi_getreg(struct stm32_xspidev_s *priv,
                                   unsigned int offset)
{
  return getreg32(priv->base + offset);
}

static inline void xspi_putreg(struct stm32_xspidev_s *priv,
                               uint32_t value, unsigned int offset)
{
  putreg32(value, priv->base + offset);
}

/****************************************************************************
 * Name: xspi_setupxctnfromcmd
 *
 * Description:
 *   Setup transaction descriptor from a command info structure
 ****************************************************************************/

static int xspi_setupxctnfromcmd(struct xspi_xctnspec_s *xctn,
                                 const struct qspi_cmdinfo_s *cmdinfo)
{
  DEBUGASSERT(xctn != NULL && cmdinfo != NULL);
  DEBUGASSERT(cmdinfo->cmd < 256);

  /* Instruction mode */

  if (QSPICMD_ISIQUAD(cmdinfo->flags))
    {
      xctn->instrmode = CCR_IMODE_QUAD;
    }
  else if (QSPICMD_ISIDUAL(cmdinfo->flags))
    {
      xctn->instrmode = CCR_IMODE_DUAL;
    }
  else
    {
      xctn->instrmode = CCR_IMODE_SINGLE;
    }

  xctn->instr = cmdinfo->cmd;

  /* No alt bytes or dummy cycles for commands */

  xctn->altbytesmode = CCR_ABMODE_NONE;
  xctn->altbytessize = CCR_ABSIZE_8;
  xctn->altbytes = 0;
  xctn->dummycycles = 0;

  /* Address */

  if (QSPICMD_ISADDRESS(cmdinfo->flags))
    {
      xctn->addrmode = CCR_ADMODE_SINGLE;
      if (cmdinfo->addrlen == 1)
        {
          xctn->addrsize = CCR_ADSIZE_8;
        }
      else if (cmdinfo->addrlen == 2)
        {
          xctn->addrsize = CCR_ADSIZE_16;
        }
      else if (cmdinfo->addrlen == 3)
        {
          xctn->addrsize = CCR_ADSIZE_24;
        }
      else if (cmdinfo->addrlen == 4)
        {
          xctn->addrsize = CCR_ADSIZE_32;
        }
      else
        {
          return -EINVAL;
        }

      xctn->addr = cmdinfo->addr;
    }
  else
    {
      xctn->addrmode = CCR_ADMODE_NONE;
      xctn->addrsize = 0;
      xctn->addr = cmdinfo->addr;
    }

  /* Data */

  xctn->buffer = cmdinfo->buffer;
  if (QSPICMD_ISDATA(cmdinfo->flags))
    {
      xctn->datamode = CCR_DMODE_SINGLE;
      xctn->datasize = cmdinfo->buflen;
      xctn->isddr = 0;
    }
  else
    {
      xctn->datamode = CCR_DMODE_NONE;
      xctn->datasize = 0;
      xctn->isddr = 0;
    }

#ifdef CONFIG_STM32N6_XSPI_INTERRUPTS
  xctn->function = QSPICMD_ISWRITE(cmdinfo->flags) ? XSPI_FMODE_INDWR :
                                                     XSPI_FMODE_INDRD;
  xctn->disposition = -EIO;
  xctn->idxnow = 0;
#endif

  return OK;
}

/****************************************************************************
 * Name: xspi_setupxctnfrommem
 *
 * Description:
 *   Setup transaction descriptor from a memory info structure
 ****************************************************************************/

static int xspi_setupxctnfrommem(struct xspi_xctnspec_s *xctn,
                                 const struct qspi_meminfo_s *meminfo)
{
  DEBUGASSERT(xctn != NULL && meminfo != NULL);
  DEBUGASSERT(meminfo->cmd < 256);

  /* Instruction mode */

  if (QSPIMEM_ISIQUAD(meminfo->flags))
    {
      xctn->instrmode = CCR_IMODE_QUAD;
    }
  else if (QSPIMEM_ISIDUAL(meminfo->flags))
    {
      xctn->instrmode = CCR_IMODE_DUAL;
    }
  else
    {
      xctn->instrmode = CCR_IMODE_SINGLE;
    }

  xctn->instr = meminfo->cmd;

  /* No alt bytes */

  xctn->altbytesmode = CCR_ABMODE_NONE;
  xctn->altbytessize = CCR_ABSIZE_8;
  xctn->altbytes = 0;

  xctn->dummycycles = meminfo->dummies;

  /* Address */

  if (QSPIMEM_ISDUALIO(meminfo->flags))
    {
      xctn->addrmode = CCR_ADMODE_DUAL;
    }
  else if (QSPIMEM_ISQUADIO(meminfo->flags))
    {
      xctn->addrmode = CCR_ADMODE_QUAD;
    }
  else
    {
      xctn->addrmode = CCR_ADMODE_SINGLE;
    }

  if (meminfo->addrlen == 1)
    {
      xctn->addrsize = CCR_ADSIZE_8;
    }
  else if (meminfo->addrlen == 2)
    {
      xctn->addrsize = CCR_ADSIZE_16;
    }
  else if (meminfo->addrlen == 3)
    {
      xctn->addrsize = CCR_ADSIZE_24;
    }
  else if (meminfo->addrlen == 4)
    {
      xctn->addrsize = CCR_ADSIZE_32;
    }
  else
    {
      return -EINVAL;
    }

  xctn->addr = meminfo->addr;

  /* Data */

  xctn->buffer = meminfo->buffer;

  if (QSPIMEM_ISDUALIO(meminfo->flags))
    {
      xctn->datamode = CCR_DMODE_DUAL;
    }
  else if (QSPIMEM_ISQUADIO(meminfo->flags))
    {
      xctn->datamode = CCR_DMODE_QUAD;
    }
  else
    {
      xctn->datamode = CCR_DMODE_SINGLE;
    }

  xctn->datasize = meminfo->buflen;
  xctn->isddr = 0;

#ifdef CONFIG_STM32N6_XSPI_INTERRUPTS
  xctn->function = QSPIMEM_ISWRITE(meminfo->flags) ? XSPI_FMODE_INDWR :
                                                     XSPI_FMODE_INDRD;
  xctn->disposition = -EIO;
  xctn->idxnow = 0;
#endif

  return OK;
}

/****************************************************************************
 * Name: xspi_waitstatusflags
 *
 * Description:
 *   Spin wait for specified status flags
 ****************************************************************************/

static void xspi_waitstatusflags(struct stm32_xspidev_s *priv,
                                 uint32_t mask, int polarity)
{
  uint32_t regval;

  if (polarity)
    {
      while (!((regval = xspi_getreg(priv, STM32_XSPI_SR_OFFSET)) & mask));
    }
  else
    {
      while (((regval = xspi_getreg(priv, STM32_XSPI_SR_OFFSET)) & mask));
    }

  UNUSED(regval);
}

/****************************************************************************
 * Name: xspi_abort
 *
 * Description:
 *   Abort any transaction in progress
 ****************************************************************************/

static void xspi_abort(struct stm32_xspidev_s *priv)
{
  uint32_t regval;

  regval  = xspi_getreg(priv, STM32_XSPI_CR_OFFSET);
  regval |= XSPI_CR_ABORT;
  xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);
}

/****************************************************************************
 * Name: xspi_ccrconfig
 *
 * Description:
 *   Configure the CCR, CR, TCR, IR, AR registers for a transaction
 ****************************************************************************/

static void xspi_ccrconfig(struct stm32_xspidev_s *priv,
                           struct xspi_xctnspec_s *xctn,
                           uint8_t fctn)
{
  uint32_t regval;

  /* If we have data, and it's not memory mapped, write the length */

  if (CCR_DMODE_NONE != xctn->datamode && XSPI_FMODE_MEMMAP != fctn)
    {
      xspi_putreg(priv, xctn->datasize - 1, STM32_XSPI_DLR_OFFSET);
    }

  /* If we have alternate bytes, write them */

  if (xctn->altbytesmode != CCR_ABMODE_NONE)
    {
      xspi_putreg(priv, xctn->altbytes, STM32_XSPI_ABR_OFFSET);
    }

  /* Build CCR value */

  regval = XSPI_CCR_IMODE(xctn->instrmode) |
           XSPI_CCR_ADMODE(xctn->addrmode) |
           XSPI_CCR_ADSIZE(xctn->addrsize) |
           XSPI_CCR_ABMODE(xctn->altbytesmode) |
           XSPI_CCR_ABSIZE(xctn->altbytessize) |
           XSPI_CCR_DMODE(xctn->datamode) |
           (xctn->isddr ? XSPI_CCR_DDTR : 0) |
           (xctn->isddr ? XSPI_CCR_ABDTR : 0) |
           (xctn->isddr ? XSPI_CCR_ADDTR : 0);

  /* Write both CCR and WCCR with the same configuration.
   * The STM32N6 XSPI uses WCCR/WTCR/WIR for write operations
   * (FMODE=00 indirect write), while CCR/TCR/IR are used for reads.
   * Both register sets must be configured consistently.
   */

  xspi_putreg(priv, regval, STM32_XSPI_CCR_OFFSET);
  xspi_putreg(priv, regval, STM32_XSPI_WCCR_OFFSET);

  /* Set functional mode */

  regval = xspi_getreg(priv, STM32_XSPI_CR_OFFSET);
  regval = (regval & ~(XSPI_CR_FMODE_MASK)) | XSPI_CR_FMODE(fctn);
  xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);

  /* Set dummy cycles in both TCR and WTCR */

  regval = xspi_getreg(priv, STM32_XSPI_TCR_OFFSET);
  regval = (regval & ~(XSPI_TCR_DCYC_MASK)) |
           XSPI_TCR_DCYC(xctn->dummycycles);
  xspi_putreg(priv, regval, STM32_XSPI_TCR_OFFSET);
  xspi_putreg(priv, regval, STM32_XSPI_WTCR_OFFSET);

  /* Write instruction to both IR and WIR */

  xspi_putreg(priv, xctn->instr, STM32_XSPI_IR_OFFSET);
  xspi_putreg(priv, xctn->instr, STM32_XSPI_WIR_OFFSET);

  /* If we have and need an address, set it.
   *
   * For indirect-read mode, writing AR triggers the transfer.
   * Both xspi_receive_blocking() and xspi_receive_dma() re-write AR
   * to start the transfer after their own setup is complete, so we
   * must NOT write AR here for INDRD — it would double-trigger.
   */

  if (CCR_ADMODE_NONE != xctn->addrmode &&
      XSPI_FMODE_MEMMAP != fctn &&
      XSPI_FMODE_INDRD != fctn)
    {
      xspi_putreg(priv, xctn->addr, STM32_XSPI_AR_OFFSET);
    }
}

#ifdef CONFIG_STM32N6_XSPI_INTERRUPTS
/****************************************************************************
 * Name: xspi0_interrupt
 *
 * Description:
 *   Interrupt handler for XSPI
 ****************************************************************************/

static int xspi0_interrupt(int irq, void *context, void *arg)
{
  uint32_t status;
  uint32_t cr;
  uint32_t regval;

  status = xspi_getreg(&g_xspi0dev, STM32_XSPI_SR_OFFSET);
  cr = xspi_getreg(&g_xspi0dev, STM32_XSPI_CR_OFFSET);

  /* FIFO Threshold */

  if ((status & XSPI_SR_FTF) && (cr & XSPI_CR_FTIE))
    {
      volatile uint32_t *datareg =
        (volatile uint32_t *)(g_xspi0dev.base + STM32_XSPI_DR_OFFSET);

      if (g_xspi0dev.xctn->function == XSPI_FMODE_INDWR)
        {
          while (((regval = xspi_getreg(
                 &g_xspi0dev, STM32_XSPI_SR_OFFSET)) & XSPI_SR_FTF) != 0)
            {
              if (g_xspi0dev.xctn->idxnow < g_xspi0dev.xctn->datasize)
                {
                  *(volatile uint8_t *)datareg =
                    ((uint8_t *)g_xspi0dev.xctn->buffer)
                    [g_xspi0dev.xctn->idxnow];
                  ++g_xspi0dev.xctn->idxnow;
                }
              else
                {
                  break;
                }
            }
        }
      else if (g_xspi0dev.xctn->function == XSPI_FMODE_INDRD)
        {
          while (((regval = xspi_getreg(
                 &g_xspi0dev, STM32_XSPI_SR_OFFSET)) & XSPI_SR_FTF) != 0)
            {
              if (g_xspi0dev.xctn->idxnow < g_xspi0dev.xctn->datasize)
                {
                  ((uint8_t *)g_xspi0dev.xctn->buffer)
                    [g_xspi0dev.xctn->idxnow] =
                    *(volatile uint8_t *)datareg;
                  ++g_xspi0dev.xctn->idxnow;
                }
              else
                {
                  break;
                }
            }
        }
    }

  /* Transfer Complete */

  if ((status & XSPI_SR_TCF) && (cr & XSPI_CR_TCIE))
    {
      xspi_putreg(&g_xspi0dev, XSPI_FCR_CTCF, STM32_XSPI_FCR_OFFSET);

      regval = xspi_getreg(&g_xspi0dev, STM32_XSPI_CR_OFFSET);
      regval &= ~(XSPI_CR_TEIE | XSPI_CR_TCIE | XSPI_CR_FTIE);
      xspi_putreg(&g_xspi0dev, regval, STM32_XSPI_CR_OFFSET);

      /* Drain remaining read data */

      if (g_xspi0dev.xctn->function == XSPI_FMODE_INDRD)
        {
          volatile uint32_t *datareg =
            (volatile uint32_t *)(g_xspi0dev.base +
            STM32_XSPI_DR_OFFSET);

          while (((regval = xspi_getreg(
                 &g_xspi0dev, STM32_XSPI_SR_OFFSET)) &
                 XSPI_SR_FLEVEL_MASK) != 0)
            {
              if (g_xspi0dev.xctn->idxnow < g_xspi0dev.xctn->datasize)
                {
                  ((uint8_t *)g_xspi0dev.xctn->buffer)
                    [g_xspi0dev.xctn->idxnow] =
                    *(volatile uint8_t *)datareg;
                  ++g_xspi0dev.xctn->idxnow;
                }
              else
                {
                  break;
                }
            }
        }

      xspi_abort(&g_xspi0dev);

      g_xspi0dev.xctn->disposition = OK;
      nxsem_post(&g_xspi0dev.op_sem);
    }

  /* Status Match */

  if ((status & XSPI_SR_SMF) && (cr & XSPI_CR_SMIE))
    {
      xspi_putreg(&g_xspi0dev, XSPI_FCR_CSMF, STM32_XSPI_FCR_OFFSET);

      if (cr & XSPI_CR_APMS)
        {
          regval = xspi_getreg(&g_xspi0dev, STM32_XSPI_CR_OFFSET);
          regval &= ~(XSPI_CR_TEIE | XSPI_CR_SMIE);
          xspi_putreg(&g_xspi0dev, regval, STM32_XSPI_CR_OFFSET);

          g_xspi0dev.xctn->disposition = OK;
          nxsem_post(&g_xspi0dev.op_sem);
        }
    }

  /* Transfer Error */

  if ((status & XSPI_SR_TEF) && (cr & XSPI_CR_TEIE))
    {
      xspi_putreg(&g_xspi0dev, XSPI_FCR_CTEF, STM32_XSPI_FCR_OFFSET);

      regval = xspi_getreg(&g_xspi0dev, STM32_XSPI_CR_OFFSET);
      regval &= ~(XSPI_CR_TEIE | XSPI_CR_TCIE | XSPI_CR_FTIE |
                  XSPI_CR_SMIE | XSPI_CR_TOIE);
      xspi_putreg(&g_xspi0dev, regval, STM32_XSPI_CR_OFFSET);

      g_xspi0dev.xctn->disposition = -EIO;
      nxsem_post(&g_xspi0dev.op_sem);
    }

  /* Timeout */

  if ((status & XSPI_SR_TOF) && (cr & XSPI_CR_TOIE))
    {
      xspi_putreg(&g_xspi0dev, XSPI_FCR_CTOF, STM32_XSPI_FCR_OFFSET);
    }

  return OK;
}
#endif /* CONFIG_STM32N6_XSPI_INTERRUPTS */

/****************************************************************************
 * Name: xspi_receive_blocking
 *
 * Description:
 *   Receive data in polling mode
 ****************************************************************************/

#if !defined(CONFIG_STM32N6_XSPI_INTERRUPTS)
static int xspi_receive_blocking(struct stm32_xspidev_s *priv,
                                 struct xspi_xctnspec_s *xctn)
{
  int ret = OK;
  volatile uint32_t *datareg =
    (volatile uint32_t *)(priv->base + STM32_XSPI_DR_OFFSET);
  uint8_t *dest = (uint8_t *)xctn->buffer;

  if (dest != NULL)
    {
      uint32_t remaining = xctn->datasize;

      /* Ensure indirect read mode */

      uint32_t regval = xspi_getreg(priv, STM32_XSPI_CR_OFFSET);
      regval &= ~XSPI_CR_FMODE_MASK;
      regval |= XSPI_CR_FMODE(XSPI_FMODE_INDRD);
      xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);

      /* Start the transfer by writing the address or instruction.
       * For INDRD, writing AR triggers the XSPI to begin clocking
       * data from the flash into the FIFO.
       */

      if (xctn->addrmode != CCR_ADMODE_NONE)
        {
          xspi_putreg(priv, xctn->addr, STM32_XSPI_AR_OFFSET);
        }
      else
        {
          xspi_putreg(priv, xctn->instr, STM32_XSPI_IR_OFFSET);
        }

      if (xspi_getreg(priv, STM32_XSPI_SR_OFFSET) & XSPI_SR_TEF)
        {
          xspi_putreg(priv, XSPI_FCR_CTEF, STM32_XSPI_FCR_OFFSET);
          xspi_abort(priv);
          return -ENOTBLK;
        }

      while (remaining > 0)
        {
          xspi_waitstatusflags(priv, XSPI_SR_FTF | XSPI_SR_TCF, 1);

          *dest = *(volatile uint8_t *)datareg;
          dest++;
          remaining--;
        }

      if (ret == OK)
        {
          xspi_waitstatusflags(priv, XSPI_SR_TCF, 1);
          xspi_putreg(priv, XSPI_FCR_CTCF, STM32_XSPI_FCR_OFFSET);
          xspi_abort(priv);
        }
    }
  else
    {
      ret = -EINVAL;
    }

  return ret;
}

/****************************************************************************
 * Name: xspi_transmit_blocking
 *
 * Description:
 *   Transmit data in polling mode
 ****************************************************************************/

static int xspi_transmit_blocking(struct stm32_xspidev_s *priv,
                                  struct xspi_xctnspec_s *xctn)
{
  int ret = OK;
  volatile uint32_t *datareg =
    (volatile uint32_t *)(priv->base + STM32_XSPI_DR_OFFSET);
  uint8_t *src = (uint8_t *)xctn->buffer;

  if (src != NULL)
    {
      uint32_t remaining = xctn->datasize;

      while (remaining > 0)
        {
          xspi_waitstatusflags(priv, XSPI_SR_FTF, 1);

          *(volatile uint8_t *)datareg = *src++;
          remaining--;
        }

      if (ret == OK)
        {
          xspi_waitstatusflags(priv, XSPI_SR_TCF, 1);
          xspi_putreg(priv, XSPI_FCR_CTCF, STM32_XSPI_FCR_OFFSET);
          xspi_abort(priv);
        }
    }
  else
    {
      ret = -EINVAL;
    }

  return ret;
}
#endif /* !CONFIG_STM32N6_XSPI_INTERRUPTS */

#ifdef CONFIG_STM32N6_XSPI2_DMA
/****************************************************************************
 * Name: xspi_receive_dma
 *
 * Description:
 *   Receive data from XSPI via DMA (peripheral-to-memory).
 *   Uses aligned bounce buffer and polled DMA wait.
 *
 ****************************************************************************/

static int xspi_receive_dma(struct stm32_xspidev_s *priv,
                             struct xspi_xctnspec_s *xctn)
{
  struct stm32_gpdma_cfg_s rxcfg;
  uint32_t regval;
  size_t nbytes = xctn->datasize;
  size_t nbytes_aligned;
  int ret;

  if (xctn->buffer == NULL || nbytes == 0)
    {
      return -EINVAL;
    }

  nbytes_aligned = (nbytes + 31) & ~31u;

  /* Clean+invalidate entire D-cache by set/way before DMA.
   * MVA-based ops (DCIMVAC) fail silently on M55 Secure at certain
   * alignments.  Set/way DCCISW is safe: cleans dirty lines first,
   * then invalidates.  ~2μs for 32KB cache.
   */

  stm32_dcache_clean_invalidate();

  /* Build RX DMA config: XSPI DR (peripheral) → aligned rxdma_buf */

  memset(&rxcfg, 0, sizeof(rxcfg));
  rxcfg.src_addr   = priv->base + STM32_XSPI_DR_OFFSET;
  rxcfg.dest_addr  = (uint32_t)(uintptr_t)priv->rxdma_buf;
  rxcfg.tr1        = GPDMA_CXTR1_SDW_LOG2_BYTE |
                     GPDMA_CXTR1_DDW_LOG2_BYTE |
                     GPDMA_CXTR1_DINC;
  rxcfg.request    = GPDMA_CXTR2_REQSEL(GPDMA_REQ_XSPI2);
  rxcfg.ntransfers = nbytes;

  /* Setup and start DMA before triggering XSPI */

  stm32_dmasetup(priv->rxdma, &rxcfg);
  stm32_dmastart(priv->rxdma, NULL, NULL, false);

  /* Set DMAEN in CR, then configure XSPI for indirect read */

  regval = xspi_getreg(priv, STM32_XSPI_CR_OFFSET);
  regval |= XSPI_CR_DMAEN;
  regval &= ~XSPI_CR_FMODE_MASK;
  regval |= XSPI_CR_FMODE(XSPI_FMODE_INDRD);
  xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);

  /* Write the address to start the transfer */

  if (xctn->addrmode != CCR_ADMODE_NONE)
    {
      xspi_putreg(priv, xctn->addr, STM32_XSPI_AR_OFFSET);
    }
  else
    {
      xspi_putreg(priv, xctn->instr, STM32_XSPI_IR_OFFSET);
    }

  /* Poll for DMA completion */

  ret = stm32_dmapollwait(priv->rxdma, 50000);

  /* Stop DMA and clear DMAEN */

  stm32_dmastop(priv->rxdma);
  regval = xspi_getreg(priv, STM32_XSPI_CR_OFFSET);
  regval &= ~XSPI_CR_DMAEN;
  xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);

  if (ret < 0)
    {
      spierr("XSPI DMA RX %s\n",
             ret == -ETIMEDOUT ? "timeout" : "error");
      xspi_abort(priv);
      return ret;
    }

  /* Wait for XSPI transfer complete */

  xspi_waitstatusflags(priv, XSPI_SR_TCF, 1);
  xspi_putreg(priv, XSPI_FCR_CTCF, STM32_XSPI_FCR_OFFSET);
  xspi_abort(priv);

  /* Clean+invalidate entire D-cache by set/way after DMA */

  stm32_dcache_clean_invalidate();
  memcpy(xctn->buffer, priv->rxdma_buf, nbytes);

  return OK;
}

/****************************************************************************
 * Name: xspi_transmit_dma
 *
 * Description:
 *   Transmit data to XSPI via DMA (memory-to-peripheral).
 *   Uses aligned bounce buffer and polled DMA wait.
 *
 ****************************************************************************/

static int xspi_transmit_dma(struct stm32_xspidev_s *priv,
                              struct xspi_xctnspec_s *xctn)
{
  struct stm32_gpdma_cfg_s txcfg;
  uint32_t regval;
  size_t nbytes = xctn->datasize;
  size_t nbytes_aligned;
  int ret;

  if (xctn->buffer == NULL || nbytes == 0)
    {
      return -EINVAL;
    }

  nbytes_aligned = (nbytes + 31) & ~31u;

  /* Copy TX data to aligned bounce buffer and clean cache */

  memcpy(priv->txdma_buf, xctn->buffer, nbytes);
  up_clean_dcache((uintptr_t)priv->txdma_buf,
                  (uintptr_t)priv->txdma_buf + nbytes_aligned);

  /* Build TX DMA config: aligned txdma_buf → XSPI DR (peripheral) */

  memset(&txcfg, 0, sizeof(txcfg));
  txcfg.src_addr   = (uint32_t)(uintptr_t)priv->txdma_buf;
  txcfg.dest_addr  = priv->base + STM32_XSPI_DR_OFFSET;
  txcfg.tr1        = GPDMA_CXTR1_SDW_LOG2_BYTE |
                     GPDMA_CXTR1_DDW_LOG2_BYTE |
                     GPDMA_CXTR1_SINC;
  txcfg.request    = GPDMA_CXTR2_REQSEL(GPDMA_REQ_XSPI2) |
                     GPDMA_CXTR2_DREQ;
  txcfg.ntransfers = nbytes;

  /* Setup and start DMA before enabling XSPI DMAEN */

  stm32_dmasetup(priv->txdma, &txcfg);
  stm32_dmastart(priv->txdma, NULL, NULL, false);

  /* Set DMAEN in CR (XSPI is already in indirect write mode) */

  regval = xspi_getreg(priv, STM32_XSPI_CR_OFFSET);
  regval |= XSPI_CR_DMAEN;
  xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);

  /* Poll for DMA completion */

  ret = stm32_dmapollwait(priv->txdma, 50000);

  /* Stop DMA and clear DMAEN */

  stm32_dmastop(priv->txdma);
  regval = xspi_getreg(priv, STM32_XSPI_CR_OFFSET);
  regval &= ~XSPI_CR_DMAEN;
  xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);

  if (ret < 0)
    {
      spierr("XSPI DMA TX %s\n",
             ret == -ETIMEDOUT ? "timeout" : "error");
      xspi_abort(priv);
      return ret;
    }

  /* Wait for XSPI transfer complete */

  xspi_waitstatusflags(priv, XSPI_SR_TCF, 1);
  xspi_putreg(priv, XSPI_FCR_CTCF, STM32_XSPI_FCR_OFFSET);
  xspi_abort(priv);

  return OK;
}
#endif /* CONFIG_STM32N6_XSPI2_DMA */

/****************************************************************************
 * Name: xspi_lock
 ****************************************************************************/

static int xspi_lock(struct qspi_dev_s *dev, bool lock)
{
  struct stm32_xspidev_s *priv = (struct stm32_xspidev_s *)dev;
  int ret;

  if (lock)
    {
      ret = nxmutex_lock(&priv->lock);

      /* Suspend memory-mapped mode while indirect ops are in progress.
       * The MTD driver holds the lock for the entire operation sequence
       * (write-enable + program + status-poll), so we only suspend once
       * and resume when the lock is released.
       */

      if (ret == OK && priv->memmap)
        {
          uint32_t regval;

          /* Exit memory-mapped mode: abort + clear FMODE + wait */

          xspi_abort(priv);
          xspi_waitstatusflags(priv, XSPI_SR_BUSY, 0);

          /* Explicitly clear FMODE bits to exit memory-mapped mode.
           * xspi_abort() alone doesn't clear FMODE on STM32N6.
           */

          regval = xspi_getreg(priv, STM32_XSPI_CR_OFFSET);
          regval &= ~XSPI_CR_FMODE_MASK;
          xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);

          xspi_putreg(priv,
                       XSPI_FCR_CTEF | XSPI_FCR_CTCF |
                       XSPI_FCR_CSMF | XSPI_FCR_CTOF,
                       STM32_XSPI_FCR_OFFSET);
          priv->memmap = false;
        }
    }
  else
    {
      /* Resume memory-mapped mode before releasing the lock */

      if (priv->mmap_saved && !priv->memmap)
        {
          struct xspi_xctnspec_s mxctn;
          xspi_abort(priv);
          xspi_waitstatusflags(priv, XSPI_SR_BUSY, 0);
          xspi_putreg(priv,
                       XSPI_FCR_CTEF | XSPI_FCR_CTCF |
                       XSPI_FCR_CSMF | XSPI_FCR_CTOF,
                       STM32_XSPI_FCR_OFFSET);
          xspi_setupxctnfrommem(&mxctn, &priv->mmap_info);
          xspi_ccrconfig(priv, &mxctn, XSPI_FMODE_MEMMAP);
          priv->memmap = true;
        }

      ret = nxmutex_unlock(&priv->lock);
    }

  return ret;
}

/****************************************************************************
 * Name: xspi_setfrequency
 ****************************************************************************/

static uint32_t xspi_setfrequency(struct qspi_dev_s *dev, uint32_t frequency)
{
  struct stm32_xspidev_s *priv = (struct stm32_xspidev_s *)dev;
  uint32_t actual;
  uint32_t prescaler;
  uint32_t regval;

  if (priv->memmap)
    {
      return 0;
    }

  DEBUGASSERT(priv);

  xspi_abort(priv);
  xspi_waitstatusflags(priv, XSPI_SR_BUSY, 0);

  if (priv->frequency == frequency)
    {
      return priv->actual;
    }

  /* prescaler = ceil(XSPI_CLK / frequency) */

  prescaler = (XSPI_CLK_FREQUENCY + frequency - 1) / frequency;

  if (prescaler < 1)
    {
      prescaler = 1;
    }
  else if (prescaler > 256)
    {
      prescaler = 256;
    }

  regval = xspi_getreg(priv, STM32_XSPI_DCR2_OFFSET);
  regval &= ~(XSPI_DCR2_PRESCALER_MASK);
  regval |= (prescaler - 1) << XSPI_DCR2_PRESCALER_SHIFT;
  xspi_putreg(priv, regval, STM32_XSPI_DCR2_OFFSET);

  actual = XSPI_CLK_FREQUENCY / prescaler;

  priv->frequency = frequency;
  priv->actual    = actual;

  spiinfo("Frequency %" PRId32 "->%" PRId32 "\n", frequency, actual);
  return actual;
}

/****************************************************************************
 * Name: xspi_setmode
 ****************************************************************************/

static void xspi_setmode(struct qspi_dev_s *dev, enum qspi_mode_e mode)
{
  struct stm32_xspidev_s *priv = (struct stm32_xspidev_s *)dev;
  uint32_t regval;

  if (priv->memmap)
    {
      return;
    }

  if (mode != priv->mode)
    {
      regval = xspi_getreg(priv, STM32_XSPI_DCR1_OFFSET);
      regval &= ~(XSPI_DCR1_CKMODE);

      switch (mode)
        {
        case QSPIDEV_MODE0:
          break;

        case QSPIDEV_MODE3:
          regval |= XSPI_DCR1_CKMODE;
          break;

        default:
          DEBUGASSERT(FALSE);
          return;
        }

      xspi_putreg(priv, regval, STM32_XSPI_DCR1_OFFSET);
      priv->mode = mode;
    }
}

/****************************************************************************
 * Name: xspi_setbits
 ****************************************************************************/

static void xspi_setbits(struct qspi_dev_s *dev, int nbits)
{
  if (8 != nbits)
    {
      DEBUGASSERT(FALSE);
    }
}

/****************************************************************************
 * Name: xspi_command
 ****************************************************************************/

static int xspi_command(struct qspi_dev_s *dev,
                        struct qspi_cmdinfo_s *cmdinfo)
{
  struct stm32_xspidev_s *priv = (struct stm32_xspidev_s *)dev;
  struct xspi_xctnspec_s xctn;
  int ret;

  /* Memory-mapped mode is suspended by xspi_lock() on entry */

  DEBUGASSERT(!priv->memmap);

  ret = xspi_setupxctnfromcmd(&xctn, cmdinfo);
  if (OK != ret)
    {
      return ret;
    }

  xspi_abort(priv);
  xspi_waitstatusflags(priv, XSPI_SR_BUSY, 0);

  xspi_putreg(priv,
               XSPI_FCR_CTEF | XSPI_FCR_CTCF |
               XSPI_FCR_CSMF | XSPI_FCR_CTOF,
               STM32_XSPI_FCR_OFFSET);

#ifdef CONFIG_STM32N6_XSPI_INTERRUPTS
  priv->xctn = &xctn;

  if (QSPICMD_ISDATA(cmdinfo->flags))
    {
      DEBUGASSERT(cmdinfo->buffer != NULL && cmdinfo->buflen > 0);

      if (QSPICMD_ISWRITE(cmdinfo->flags))
        {
          uint32_t regval;

          xspi_ccrconfig(priv, &xctn, XSPI_FMODE_INDWR);

          regval = xspi_getreg(priv, STM32_XSPI_CR_OFFSET);
          regval |= (XSPI_CR_TEIE | XSPI_CR_FTIE | XSPI_CR_TCIE);
          xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);
        }
      else
        {
          uint32_t regval;
          uint32_t addrval;

          addrval = xspi_getreg(priv, STM32_XSPI_AR_OFFSET);

          xspi_ccrconfig(priv, &xctn, XSPI_FMODE_INDRD);

          xspi_putreg(priv, addrval, STM32_XSPI_AR_OFFSET);

          regval = xspi_getreg(priv, STM32_XSPI_CR_OFFSET);
          regval |= (XSPI_CR_TEIE | XSPI_CR_FTIE | XSPI_CR_TCIE);
          xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);
        }
    }
  else
    {
      uint32_t regval;

      regval = xspi_getreg(priv, STM32_XSPI_CR_OFFSET);
      regval |= (XSPI_CR_TEIE | XSPI_CR_TCIE);
      xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);

      xspi_ccrconfig(priv, &xctn, XSPI_FMODE_INDWR);
    }

  nxsem_wait(&priv->op_sem);
  UP_MB();

  ret = xctn.disposition;

#else
  /* Polling / DMA mode */

  if (QSPICMD_ISDATA(cmdinfo->flags))
    {
      DEBUGASSERT(cmdinfo->buffer != NULL && cmdinfo->buflen > 0);

#ifdef CONFIG_STM32N6_XSPI2_DMA
      if (priv->rxdma != NULL && priv->txdma != NULL &&
          cmdinfo->buflen >= XSPI_DMA_THRESHOLD &&
          cmdinfo->buflen <= XSPI_DMA_BUFSIZE)
        {
          xspi_ccrconfig(priv, &xctn,
                         QSPICMD_ISWRITE(cmdinfo->flags) ?
                         XSPI_FMODE_INDWR : XSPI_FMODE_INDRD);

          if (QSPICMD_ISWRITE(cmdinfo->flags))
            {
              ret = xspi_transmit_dma(priv, &xctn);
            }
          else
            {
              ret = xspi_receive_dma(priv, &xctn);
            }
        }
      else
#endif
        {
          xspi_ccrconfig(priv, &xctn, XSPI_FMODE_INDWR);

          if (QSPICMD_ISWRITE(cmdinfo->flags))
            {
              ret = xspi_transmit_blocking(priv, &xctn);
            }
          else
            {
              ret = xspi_receive_blocking(priv, &xctn);
            }

          xspi_waitstatusflags(priv, XSPI_SR_TCF, 1);
          xspi_waitstatusflags(priv, XSPI_SR_BUSY, 0);
        }

      UP_MB();
    }
  else
    {
      xspi_ccrconfig(priv, &xctn, XSPI_FMODE_INDWR);

      xspi_waitstatusflags(priv, XSPI_SR_TCF, 1);
      xspi_waitstatusflags(priv, XSPI_SR_BUSY, 0);

      ret = OK;
    }

#endif

  return ret;
}

/****************************************************************************
 * Name: xspi_memory
 ****************************************************************************/

static int xspi_memory(struct qspi_dev_s *dev,
                       struct qspi_meminfo_s *meminfo)
{
  struct stm32_xspidev_s *priv = (struct stm32_xspidev_s *)dev;
  struct xspi_xctnspec_s xctn;
  int ret;

  /* Memory-mapped mode is suspended by xspi_lock() on entry */

  DEBUGASSERT(!priv->memmap);

  ret = xspi_setupxctnfrommem(&xctn, meminfo);
  if (OK != ret)
    {
      return ret;
    }

  xspi_abort(priv);
  xspi_waitstatusflags(priv, XSPI_SR_BUSY, 0);

  xspi_putreg(priv,
               XSPI_FCR_CTEF | XSPI_FCR_CTCF |
               XSPI_FCR_CSMF | XSPI_FCR_CTOF,
               STM32_XSPI_FCR_OFFSET);

#ifdef CONFIG_STM32N6_XSPI_INTERRUPTS
  priv->xctn = &xctn;

  DEBUGASSERT(meminfo->buffer != NULL && meminfo->buflen > 0);

  if (QSPIMEM_ISWRITE(meminfo->flags))
    {
      uint32_t regval;

      xspi_ccrconfig(priv, &xctn, XSPI_FMODE_INDWR);

      regval = xspi_getreg(priv, STM32_XSPI_CR_OFFSET);
      regval |= (XSPI_CR_TEIE | XSPI_CR_FTIE | XSPI_CR_TCIE);
      xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);
    }
  else
    {
      uint32_t regval;

      xspi_ccrconfig(priv, &xctn, XSPI_FMODE_INDRD);

      /* Write address to trigger the indirect read transfer */

      xspi_putreg(priv, xctn.addr, STM32_XSPI_AR_OFFSET);

      regval = xspi_getreg(priv, STM32_XSPI_CR_OFFSET);
      regval |= (XSPI_CR_TEIE | XSPI_CR_FTIE | XSPI_CR_TCIE);
      xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);
    }

  nxsem_wait(&priv->op_sem);
  UP_MB();

  ret = xctn.disposition;

#else
  /* Polling / DMA mode */

  DEBUGASSERT(meminfo->buffer != NULL && meminfo->buflen > 0);

#ifdef CONFIG_STM32N6_XSPI2_DMA
  if (priv->rxdma != NULL && priv->txdma != NULL &&
      meminfo->buflen >= XSPI_DMA_THRESHOLD &&
      meminfo->buflen <= XSPI_DMA_BUFSIZE)
    {
      xspi_ccrconfig(priv, &xctn,
                     QSPIMEM_ISWRITE(meminfo->flags) ?
                     XSPI_FMODE_INDWR : XSPI_FMODE_INDRD);

      if (QSPIMEM_ISWRITE(meminfo->flags))
        {
          ret = xspi_transmit_dma(priv, &xctn);
        }
      else
        {
          ret = xspi_receive_dma(priv, &xctn);
        }
    }
  else
#endif
    {
      xspi_ccrconfig(priv, &xctn,
                     QSPIMEM_ISWRITE(meminfo->flags) ?
                     XSPI_FMODE_INDWR : XSPI_FMODE_INDRD);

      if (QSPIMEM_ISWRITE(meminfo->flags))
        {
          ret = xspi_transmit_blocking(priv, &xctn);
        }
      else
        {
          ret = xspi_receive_blocking(priv, &xctn);
        }

      xspi_waitstatusflags(priv, XSPI_SR_TCF, 1);
      xspi_waitstatusflags(priv, XSPI_SR_BUSY, 0);
    }

  UP_MB();
#endif

  return ret;
}

/****************************************************************************
 * Name: xspi_alloc / xspi_free
 ****************************************************************************/

static void *xspi_alloc(struct qspi_dev_s *dev, size_t buflen)
{
  return kmm_malloc(ALIGN_UP(buflen, 4));
}

static void xspi_free(struct qspi_dev_s *dev, void *buffer)
{
  if (buffer)
    {
      kmm_free(buffer);
    }
}

/****************************************************************************
 * Name: xspi_flash_reset
 *
 * Description:
 *   Reset MX25UM51245G from OPI DTR mode to SPI mode.
 *   Boot ROM / FSBL may leave the flash in OPI DTR mode.
 *   Send 0x6600 (Reset Enable) + 0x9900 (Reset Memory) in OPI DTR mode,
 *   then wait for the flash to reset to SPI mode.
 ****************************************************************************/

static void xspi_flash_reset(struct stm32_xspidev_s *priv)
{
  uint32_t regval;

  /* Disable XSPI first */

  regval = xspi_getreg(priv, STM32_XSPI_CR_OFFSET);
  regval &= ~XSPI_CR_EN;
  xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);

  /* Configure for OPI DTR mode to send reset commands.
   * Prescaler = 7 → 200/8 = 25MHz, Macronix memory type.
   */

  xspi_putreg(priv,
               XSPI_DCR1_MTYP_MACRONIX |
               (25 << XSPI_DCR1_DEVSIZE_SHIFT) |
               (1 << XSPI_DCR1_CSHT_SHIFT),
               STM32_XSPI_DCR1_OFFSET);
  xspi_putreg(priv,
               7 << XSPI_DCR2_PRESCALER_SHIFT,
               STM32_XSPI_DCR2_OFFSET);

  /* Enable XSPI */

  regval = xspi_getreg(priv, STM32_XSPI_CR_OFFSET);
  regval |= XSPI_CR_EN;
  xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);

  xspi_waitstatusflags(priv, XSPI_SR_BUSY, 0);

  /* Clear all flags */

  xspi_putreg(priv,
               XSPI_FCR_CTEF | XSPI_FCR_CTCF |
               XSPI_FCR_CSMF | XSPI_FCR_CTOF,
               STM32_XSPI_FCR_OFFSET);

  /* Send Reset Enable (0x6600) in OPI DTR mode.
   * CCR: IMODE=4(octal), IDTR=1, ISIZE=16bit, no addr, no data
   */

  regval = xspi_getreg(priv, STM32_XSPI_CR_OFFSET);
  regval &= ~XSPI_CR_FMODE_MASK;
  regval |= XSPI_CR_FMODE(XSPI_FMODE_INDWR);
  xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);

  xspi_putreg(priv,
               XSPI_CCR_IMODE(CCR_IMODE_OCTAL) | XSPI_CCR_IDTR |
               XSPI_CCR_ISIZE_16b,
               STM32_XSPI_CCR_OFFSET);
  xspi_putreg(priv, 0x0066, STM32_XSPI_IR_OFFSET);

  xspi_waitstatusflags(priv, XSPI_SR_TCF, 1);
  xspi_putreg(priv, XSPI_FCR_CTCF, STM32_XSPI_FCR_OFFSET);

  /* Send Reset Memory (0x9900) in OPI DTR mode */

  xspi_putreg(priv,
               XSPI_CCR_IMODE(CCR_IMODE_OCTAL) | XSPI_CCR_IDTR |
               XSPI_CCR_ISIZE_16b,
               STM32_XSPI_CCR_OFFSET);
  xspi_putreg(priv, 0x0099, STM32_XSPI_IR_OFFSET);

  xspi_waitstatusflags(priv, XSPI_SR_TCF, 1);
  xspi_putreg(priv, XSPI_FCR_CTCF, STM32_XSPI_FCR_OFFSET);

  /* Wait for flash reset to complete (~40us, use 1ms) */

  up_mdelay(1);

  /* Abort and re-disable to reconfigure for SPI mode */

  xspi_abort(priv);
  xspi_waitstatusflags(priv, XSPI_SR_BUSY, 0);

  regval = xspi_getreg(priv, STM32_XSPI_CR_OFFSET);
  regval &= ~XSPI_CR_EN;
  xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);

  spiinfo("Flash reset to SPI mode complete\n");
}

/****************************************************************************
 * Name: xspi_hw_initialize
 *
 * Description:
 *   Initialize the XSPI peripheral from hardware reset
 ****************************************************************************/

static int xspi_hw_initialize(struct stm32_xspidev_s *priv)
{
  uint32_t regval;

  /* Reset flash from OPI DTR to SPI mode (boot ROM/FSBL leaves it in OPI) */

  xspi_flash_reset(priv);

  /* Disable XSPI */

  xspi_abort(priv);

  regval = 0;
  xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);

  xspi_waitstatusflags(priv, XSPI_SR_BUSY, 0);

  /* Disable all interrupts */

  regval = xspi_getreg(priv, STM32_XSPI_CR_OFFSET);
  regval &= ~(XSPI_CR_TEIE | XSPI_CR_TCIE | XSPI_CR_FTIE |
              XSPI_CR_SMIE | XSPI_CR_TOIE | XSPI_CR_MSEL |
              XSPI_CR_DMM);

  /* Configure FIFO threshold */

  regval &= ~(XSPI_CR_FTHRES_MASK);
  regval |= ((CONFIG_STM32N6_XSPI_FIFO_THESHOLD - 1) <<
    XSPI_CR_FTHRES_SHIFT);
  xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);

  xspi_waitstatusflags(priv, XSPI_SR_BUSY, 0);

  /* Configure prescaler: 200MHz / 8 = 25MHz for SPI mode.
   * Prescaler field is (divider - 1), so 7 for /8.
   */

  regval = xspi_getreg(priv, STM32_XSPI_DCR2_OFFSET);
  regval &= ~(XSPI_DCR2_PRESCALER_MASK);
  regval |= (7 << XSPI_DCR2_PRESCALER_SHIFT);
  xspi_putreg(priv, regval, STM32_XSPI_DCR2_OFFSET);

  /* Configure sample shift (delay the data capture by half a cycle) */

  regval = xspi_getreg(priv, STM32_XSPI_TCR_OFFSET);
  regval |= XSPI_TCR_SSHIFT;
  xspi_putreg(priv, regval, STM32_XSPI_TCR_OFFSET);

  /* Configure flash size, CS high time, memory type = Macronix */

  regval = xspi_getreg(priv, STM32_XSPI_DCR1_OFFSET);
  regval &= ~(XSPI_DCR1_CKMODE |
              XSPI_DCR1_CSHT_MASK |
              XSPI_DCR1_DEVSIZE_MASK |
              XSPI_DCR1_MTYP_MASK);

  regval |= ((CONFIG_STM32N6_XSPI_CSHT - 1) << XSPI_DCR1_CSHT_SHIFT);
  regval |= XSPI_DCR1_MTYP_MACRONIX;

  /* Calculate device size: log2(flash_size) - 1 */

  {
    unsigned int nsize = CONFIG_STM32N6_XSPI_FLASH_SIZE;
    int nlog2size = 31;

    while ((nsize & 0x80000000) == 0)
      {
        --nlog2size;
        nsize <<= 1;
      }

    regval |= ((nlog2size - 1) << XSPI_DCR1_DEVSIZE_SHIFT);
  }

  xspi_putreg(priv, regval, STM32_XSPI_DCR1_OFFSET);

  /* Clear DCR3: CSBOUND and MAXTRAN left by FSBL can truncate transfers.
   * CSBOUND causes CS to toggle at 2^CSBOUND byte boundaries — fatal for
   * Page Program which needs CS held low for the entire 256-byte page.
   * MAXTRAN limits the maximum bytes per transaction.
   * Both must be 0 for standard SPI 1-1-1 operation.
   */

  xspi_putreg(priv, 0, STM32_XSPI_DCR3_OFFSET);

  /* Also clear DCR4 (refresh rate) — not needed for standard SPI */

  xspi_putreg(priv, 0, STM32_XSPI_DCR4_OFFSET);

  /* Enable XSPI */

  regval = xspi_getreg(priv, STM32_XSPI_CR_OFFSET);
  regval |= XSPI_CR_EN;
  xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);

  xspi_waitstatusflags(priv, XSPI_SR_BUSY, 0);

  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_xspi_initialize
 *
 * Description:
 *   Initialize the selected XSPI port in master mode
 *
 * Input Parameters:
 *   intf - Interface number (0 = XSPI2 on this board)
 *
 * Returned Value:
 *   Valid QSPI device structure reference on success; a NULL on failure
 *
 ****************************************************************************/

struct qspi_dev_s *stm32_xspi_initialize(int intf)
{
  struct stm32_xspidev_s *priv;
  int ret;

  spiinfo("intf: %d\n", intf);
  DEBUGASSERT(intf == 0);

  if (intf == 0)
    {
      priv = &g_xspi0dev;

      /* Enable AHB5 bus clock first (needed for AHB5 peripherals).
       * This must be done before enabling XSPI2/XSPIM clocks.
       */

      putreg32(RCC_BUSENR_AHB5EN, STM32_RCC_BUSENSR);

      /* Enable GPION clock for XSPI2 pins (may already be on from boot) */

      putreg32(RCC_AHB4ENR_GPIONEN, STM32_RCC_AHB4ENSR);

      /* Enable XSPI2 and XSPIM clocks via ENSR (atomic set) */

      putreg32(RCC_AHB5ENR_XSPI2EN | RCC_AHB5ENR_XSPIMEN,
               STM32_RCC_AHB5ENSR);

      /* Enable LPEN for WFI compatibility */

      putreg32(RCC_AHB5ENR_XSPI2EN | RCC_AHB5ENR_XSPIMEN,
               STM32_RCC_AHB5LPENSR);

      /* Configure XSPI2 kernel clock: IC3 = PLL1/6 = 200MHz.
       * CCIPR6 bits [5:4] = XSPI2SEL: 00=HCLK, 01=PLL1(IC3), etc.
       * We need IC3 configured externally (in RCC clock init).
       * Set XSPI2SEL = 1 (IC3).
       */

      modifyreg32(STM32_RCC_CCIPR6,
                  RCC_CCIPR6_XSPI2SEL_MASK,
                  RCC_CCIPR6_XSPI2SEL_IC3);

      /* DSB to flush write buffer — catch any imprecise bus faults from
       * the RCC writes above BEFORE we try to access GPIO registers.
       */

      __asm volatile ("dsb sy");
      __asm volatile ("isb sy");

      /* Configure GPIO pins for XSPI2 */

      stm32_configgpio(GPIO_XSPI2_CS);
      stm32_configgpio(GPIO_XSPI2_CLK);
      stm32_configgpio(GPIO_XSPI2_D0);
      stm32_configgpio(GPIO_XSPI2_D1);
      stm32_configgpio(GPIO_XSPI2_D2);
      stm32_configgpio(GPIO_XSPI2_D3);

#ifdef GPIO_XSPI2_D4
      stm32_configgpio(GPIO_XSPI2_D4);
      stm32_configgpio(GPIO_XSPI2_D5);
      stm32_configgpio(GPIO_XSPI2_D6);
      stm32_configgpio(GPIO_XSPI2_D7);
#endif

#ifdef GPIO_XSPI2_DQS
      stm32_configgpio(GPIO_XSPI2_DQS);
#endif
    }
  else
    {
      spierr("ERROR: XSPI%d not supported\n", intf);
      return NULL;
    }

  if (!priv->initialized)
    {
#ifdef CONFIG_STM32N6_XSPI_INTERRUPTS
      ret = irq_attach(priv->irq, priv->handler, NULL);
      if (ret < 0)
        {
          spierr("ERROR: Failed to attach irq %d\n", priv->irq);
          return NULL;
        }
#endif

      ret = xspi_hw_initialize(priv);
      if (ret < 0)
        {
          spierr("ERROR: Failed to initialize XSPI hardware\n");
#ifdef CONFIG_STM32N6_XSPI_INTERRUPTS
          irq_detach(priv->irq);
#endif
          return NULL;
        }

      priv->initialized = true;
      priv->memmap = false;

#ifdef CONFIG_STM32N6_XSPI_INTERRUPTS
      up_enable_irq(priv->irq);
#endif

#ifdef CONFIG_STM32N6_XSPI2_DMA
      priv->rxdma = stm32_dmachannel(GPDMA_TTYPE_P2M);
      priv->txdma = stm32_dmachannel(GPDMA_TTYPE_M2P);

      if (priv->rxdma == NULL || priv->txdma == NULL)
        {
          if (priv->rxdma != NULL)
            {
              stm32_dmafree(priv->rxdma);
            }

          if (priv->txdma != NULL)
            {
              stm32_dmafree(priv->txdma);
            }

          priv->rxdma = NULL;
          priv->txdma = NULL;
          spiwarn("XSPI DMA channels unavailable, using polling\n");
        }
#endif
    }

  return &priv->xspi;
}

/****************************************************************************
 * Name: stm32_xspi_enter_memorymapped
 ****************************************************************************/

void stm32_xspi_enter_memorymapped(struct qspi_dev_s *dev,
                                   const struct qspi_meminfo_s *meminfo,
                                   uint32_t lpto)
{
  struct stm32_xspidev_s *priv = (struct stm32_xspidev_s *)dev;
  uint32_t regval;
  struct xspi_xctnspec_s xctn;

  xspi_lock(dev, true);

  if (priv->memmap)
    {
      xspi_lock(dev, false);
      return;
    }

  xspi_abort(priv);
  xspi_waitstatusflags(priv, XSPI_SR_BUSY, 0);

  if (lpto > 0)
    {
      xspi_putreg(priv, lpto, STM32_XSPI_LPTR_OFFSET);

      xspi_putreg(priv, XSPI_FCR_CTOF, STM32_XSPI_FCR_OFFSET);

#ifdef CONFIG_STM32N6_XSPI_INTERRUPTS
      regval = xspi_getreg(priv, STM32_XSPI_CR_OFFSET);
      regval |= (XSPI_CR_TCEN | XSPI_CR_TOIE);
      xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);
#endif
    }
  else
    {
      regval = xspi_getreg(priv, STM32_XSPI_CR_OFFSET);
      regval &= ~XSPI_CR_TCEN;
      xspi_putreg(priv, regval, STM32_XSPI_CR_OFFSET);
    }

  xspi_setupxctnfrommem(&xctn, meminfo);

#ifdef CONFIG_STM32N6_XSPI_INTERRUPTS
  priv->xctn = NULL;
#endif

  xspi_ccrconfig(priv, &xctn, XSPI_FMODE_MEMMAP);
  priv->memmap = true;

  /* Save mmap config so we can re-enter after MTD operations */

  memcpy(&priv->mmap_info, meminfo, sizeof(priv->mmap_info));
  priv->mmap_lpto = lpto;
  priv->mmap_saved = true;

  xspi_lock(dev, false);
}

/****************************************************************************
 * Name: stm32_xspi_exit_memorymapped
 ****************************************************************************/

void stm32_xspi_exit_memorymapped(struct qspi_dev_s *dev)
{
  struct stm32_xspidev_s *priv = (struct stm32_xspidev_s *)dev;

  xspi_lock(dev, true);

  xspi_abort(priv);
  priv->memmap = false;

  xspi_lock(dev, false);
}

int stm32_xspi_mmap_lock(void)
{
  return nxmutex_lock(&g_xspi0dev.lock);
}

void stm32_xspi_mmap_unlock(void)
{
  nxmutex_unlock(&g_xspi0dev.lock);
}

#endif /* CONFIG_STM32N6_XSPI */
