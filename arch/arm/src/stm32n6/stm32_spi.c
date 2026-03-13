/****************************************************************************
 * arch/arm/src/stm32n6/stm32_spi.c
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
 * The external functions, stm32_spi1/2/3/4/5/6select and
 * stm32_spi1/2/3/4/5/6status must be provided by board-specific logic.  They
 * are implementations of the select and status methods of the SPI interface
 * defined by struct spi_ops_s (see include/nuttx/spi/spi.h).  All other
 * methods (including stm32_spibus_initialize()) are provided by common STM32
 * logic.  To use this common SPI logic on your board:
 *
 *   1. Provide logic in stm32_boardinitialize() to configure SPI chip select
 *      pins.
 *   2. Provide stm32_spi1/2/3/4/5/6select() and stm32_spi1/2/3/4/5/6status()
 *      functions in your board-specific logic.  These functions will perform
 *      chip selection and status operations using GPIOs in the way your
 *      board is configured.
 *   3. Add a calls to stm32_spibus_initialize() in your low level
 *      application initialization logic
 *   4. The handle returned by stm32_spibus_initialize() may then be used to
 *      bind the SPI driver to higher level logic (e.g., calling
 *      mmcsd_spislotinitialize(), for example, will bind the SPI driver to
 *      the SPI MMC/SD driver).
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <nuttx/compiler.h>
#include <nuttx/mutex.h>
#include <nuttx/spi/spi.h>
#include <nuttx/power/pm.h>

#include <arch/board/board.h>

#include "arm_internal.h"
#include "chip.h"
#include "stm32_rcc.h"
#include "stm32_gpio.h"
#include "stm32_spi.h"

#if defined(CONFIG_STM32N6_SPI1) || defined(CONFIG_STM32N6_SPI2) || \
    defined(CONFIG_STM32N6_SPI3) || defined(CONFIG_STM32N6_SPI4) || \
    defined(CONFIG_STM32N6_SPI5) || defined(CONFIG_STM32N6_SPI6)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

/* SPI interrupts */

#ifdef CONFIG_STM32N6_SPI_INTERRUPTS
#  error "Interrupt driven SPI not yet supported"
#endif

/* Kernel clock configuration
 * Default to PCLK frequency for each SPI.  Board may override in board.h.
 */

#ifdef CONFIG_STM32N6_SPI1
#  ifndef STM32_SPI1_FREQUENCY
#    define STM32_SPI1_FREQUENCY STM32_PCLK2_FREQUENCY
#  endif
#endif

#ifdef CONFIG_STM32N6_SPI2
#  ifndef STM32_SPI2_FREQUENCY
#    define STM32_SPI2_FREQUENCY STM32_PCLK1_FREQUENCY
#  endif
#endif

#ifdef CONFIG_STM32N6_SPI3
#  ifndef STM32_SPI3_FREQUENCY
#    define STM32_SPI3_FREQUENCY STM32_PCLK1_FREQUENCY
#  endif
#endif

#ifdef CONFIG_STM32N6_SPI4
#  ifndef STM32_SPI4_FREQUENCY
#    define STM32_SPI4_FREQUENCY STM32_PCLK2_FREQUENCY
#  endif
#endif

#ifdef CONFIG_STM32N6_SPI5
#  ifndef STM32_SPI5_FREQUENCY
#    define STM32_SPI5_FREQUENCY STM32_PCLK2_FREQUENCY
#  endif
#endif

#ifdef CONFIG_STM32N6_SPI6
#  ifndef STM32_SPI6_FREQUENCY
#    define STM32_SPI6_FREQUENCY STM32_PCLK2_FREQUENCY
#  endif
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

enum spi_config_e
{
  FULL_DUPLEX = 0,
  SIMPLEX_TX,
  SIMPLEX_RX,
  HALF_DUPLEX
};

struct stm32_spidev_s
{
  struct spi_dev_s spidev;       /* Externally visible part of the SPI interface */
  uint32_t         spibase;      /* SPIn base address */
  uint32_t         spiclock;     /* Clocking for the SPI module */
  uint8_t          spiirq;       /* SPI IRQ number */
#ifdef CONFIG_SPI_TRIGGER
  bool             defertrig;    /* Flag indicating that trigger should be deferred */
  bool             trigarmed;    /* Flag indicating that the trigger is armed */
#endif
  bool             initialized;  /* Has SPI interface been initialized */
  mutex_t          lock;         /* Held while chip is selected for mutual exclusion */
  uint32_t         frequency;    /* Requested clock frequency */
  uint32_t         actual;       /* Actual clock frequency */
  int8_t           nbits;        /* Width of word in bits */
  uint8_t          mode;         /* Mode 0,1,2,3 */
#ifdef CONFIG_PM
  struct pm_callback_s pm_cb;    /* PM callbacks */
#endif
  enum spi_config_e config;      /* full/half duplex, simplex transmit/read only */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/* Helpers */

static inline uint32_t spi_getreg(struct stm32_spidev_s *priv,
                                  uint32_t offset);
static inline void spi_putreg(struct stm32_spidev_s *priv,
                              uint32_t offset, uint32_t value);
static inline uint32_t spi_readword(struct stm32_spidev_s *priv);
static inline void spi_writeword(struct stm32_spidev_s *priv,
                                 uint32_t byte);
#ifdef CONFIG_DEBUG_SPI_INFO
static inline void spi_dumpregs(struct stm32_spidev_s *priv);
#endif

/* SPI methods */

static int         spi_lock(struct spi_dev_s *dev, bool lock);
static uint32_t    spi_setfrequency(struct spi_dev_s *dev,
                                    uint32_t frequency);
#ifdef CONFIG_SPI_DELAY_CONTROL
static int         spi_setdelay(struct spi_dev_s *dev, uint32_t startdelay,
                                uint32_t stopdelay, uint32_t csdelay,
                                uint32_t ifdelay);
#endif
static void        spi_setmode(struct spi_dev_s *dev,
                               enum spi_mode_e mode);
static void        spi_setbits(struct spi_dev_s *dev, int nbits);
#ifdef CONFIG_SPI_HWFEATURES
static int         spi_hwfeatures(struct spi_dev_s *dev,
                                  spi_hwfeatures_t features);
#endif
static uint32_t    spi_send(struct spi_dev_s *dev, uint32_t wd);
static void        spi_exchange(struct spi_dev_s *dev,
                                const void *txbuffer, void *rxbuffer,
                                size_t nwords);
#ifdef CONFIG_SPI_TRIGGER
static int         spi_trigger(struct spi_dev_s *dev);
#endif
#ifndef CONFIG_SPI_EXCHANGE
static void        spi_sndblock(struct spi_dev_s *dev,
                                const void *txbuffer, size_t nwords);
static void        spi_recvblock(struct spi_dev_s *dev,
                                 void *rxbuffer, size_t nwords);
#endif

/* Initialization */

static void        spi_bus_initialize(struct stm32_spidev_s *priv);

/* PM interface */

#ifdef CONFIG_PM
static int         spi_pm_prepare(struct pm_callback_s *cb, int domain,
                                  enum pm_state_e pmstate);
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/

#ifdef CONFIG_STM32N6_SPI1
static const struct spi_ops_s g_sp1iops =
{
  .lock              = spi_lock,
  .select            = stm32_spi1select,
  .setfrequency      = spi_setfrequency,
#ifdef CONFIG_SPI_DELAY_CONTROL
  .setdelay          = spi_setdelay,
#endif
  .setmode           = spi_setmode,
  .setbits           = spi_setbits,
#ifdef CONFIG_SPI_HWFEATURES
  .hwfeatures        = spi_hwfeatures,
#endif
  .status            = stm32_spi1status,
#ifdef CONFIG_SPI_CMDDATA
  .cmddata           = stm32_spi1cmddata,
#endif
  .send              = spi_send,
#ifdef CONFIG_SPI_EXCHANGE
  .exchange          = spi_exchange,
#else
  .sndblock          = spi_sndblock,
  .recvblock         = spi_recvblock,
#endif
#ifdef CONFIG_SPI_TRIGGER
  .trigger           = spi_trigger,
#endif
#ifdef CONFIG_SPI_CALLBACK
  .registercallback  = stm32_spi1register,  /* Provided externally */
#else
  .registercallback  = 0,                   /* Not implemented */
#endif
};

static struct stm32_spidev_s g_spi1dev =
{
  .spidev   =
  {
    .ops    = &g_sp1iops,
  },
  .spibase  = STM32_SPI1_BASE,
  .spiclock = STM32_SPI1_FREQUENCY,
  .spiirq   = STM32_IRQ_SPI1,
  .lock     = NXMUTEX_INITIALIZER,
#ifdef CONFIG_PM
  .pm_cb.prepare = spi_pm_prepare,
#endif
#ifdef CONFIG_STM32N6_SPI1_COMMTYPE
  .config   = CONFIG_STM32N6_SPI1_COMMTYPE,
#else
  .config   = FULL_DUPLEX,
#endif
};
#endif /* CONFIG_STM32N6_SPI1 */

#ifdef CONFIG_STM32N6_SPI2
static const struct spi_ops_s g_sp2iops =
{
  .lock              = spi_lock,
  .select            = stm32_spi2select,
  .setfrequency      = spi_setfrequency,
#ifdef CONFIG_SPI_DELAY_CONTROL
  .setdelay          = spi_setdelay,
#endif
  .setmode           = spi_setmode,
  .setbits           = spi_setbits,
#ifdef CONFIG_SPI_HWFEATURES
  .hwfeatures        = spi_hwfeatures,
#endif
  .status            = stm32_spi2status,
#ifdef CONFIG_SPI_CMDDATA
  .cmddata           = stm32_spi2cmddata,
#endif
  .send              = spi_send,
#ifdef CONFIG_SPI_EXCHANGE
  .exchange          = spi_exchange,
#else
  .sndblock          = spi_sndblock,
  .recvblock         = spi_recvblock,
#endif
#ifdef CONFIG_SPI_TRIGGER
  .trigger           = spi_trigger,
#endif
#ifdef CONFIG_SPI_CALLBACK
  .registercallback  = stm32_spi2register,  /* provided externally */
#else
  .registercallback  = 0,  /* not implemented */
#endif
};

static struct stm32_spidev_s g_spi2dev =
{
  .spidev   =
  {
    .ops    = &g_sp2iops,
  },
  .spibase  = STM32_SPI2_BASE,
  .spiclock = STM32_SPI2_FREQUENCY,
  .spiirq   = STM32_IRQ_SPI2,
  .lock     = NXMUTEX_INITIALIZER,
#ifdef CONFIG_PM
  .pm_cb.prepare = spi_pm_prepare,
#endif
#ifdef CONFIG_STM32N6_SPI2_COMMTYPE
  .config   = CONFIG_STM32N6_SPI2_COMMTYPE,
#else
  .config   = FULL_DUPLEX,
#endif
};
#endif /* CONFIG_STM32N6_SPI2 */

#ifdef CONFIG_STM32N6_SPI3
static const struct spi_ops_s g_sp3iops =
{
  .lock              = spi_lock,
  .select            = stm32_spi3select,
  .setfrequency      = spi_setfrequency,
#ifdef CONFIG_SPI_DELAY_CONTROL
  .setdelay          = spi_setdelay,
#endif
  .setmode           = spi_setmode,
  .setbits           = spi_setbits,
#ifdef CONFIG_SPI_HWFEATURES
  .hwfeatures        = spi_hwfeatures,
#endif
  .status            = stm32_spi3status,
#ifdef CONFIG_SPI_CMDDATA
  .cmddata           = stm32_spi3cmddata,
#endif
  .send              = spi_send,
#ifdef CONFIG_SPI_EXCHANGE
  .exchange          = spi_exchange,
#else
  .sndblock          = spi_sndblock,
  .recvblock         = spi_recvblock,
#endif
#ifdef CONFIG_SPI_TRIGGER
  .trigger           = spi_trigger,
#endif
#ifdef CONFIG_SPI_CALLBACK
  .registercallback  = stm32_spi3register,  /* provided externally */
#else
  .registercallback  = 0,                   /* not implemented */
#endif
};

static struct stm32_spidev_s g_spi3dev =
{
  .spidev   =
  {
    .ops    = &g_sp3iops,
  },
  .spibase  = STM32_SPI3_BASE,
  .spiclock = STM32_SPI3_FREQUENCY,
  .spiirq   = STM32_IRQ_SPI3,
  .lock     = NXMUTEX_INITIALIZER,
#ifdef CONFIG_PM
  .pm_cb.prepare = spi_pm_prepare,
#endif
#ifdef CONFIG_STM32N6_SPI3_COMMTYPE
  .config   = CONFIG_STM32N6_SPI3_COMMTYPE,
#else
  .config   = FULL_DUPLEX,
#endif
};
#endif /* CONFIG_STM32N6_SPI3 */

#ifdef CONFIG_STM32N6_SPI4
static const struct spi_ops_s g_sp4iops =
{
  .lock              = spi_lock,
  .select            = stm32_spi4select,
  .setfrequency      = spi_setfrequency,
#ifdef CONFIG_SPI_DELAY_CONTROL
  .setdelay          = spi_setdelay,
#endif
  .setmode           = spi_setmode,
  .setbits           = spi_setbits,
#ifdef CONFIG_SPI_HWFEATURES
  .hwfeatures        = spi_hwfeatures,
#endif
  .status            = stm32_spi4status,
#ifdef CONFIG_SPI_CMDDATA
  .cmddata           = stm32_spi4cmddata,
#endif
  .send              = spi_send,
#ifdef CONFIG_SPI_EXCHANGE
  .exchange          = spi_exchange,
#else
  .sndblock          = spi_sndblock,
  .recvblock         = spi_recvblock,
#endif
#ifdef CONFIG_SPI_TRIGGER
  .trigger           = spi_trigger,
#endif
#ifdef CONFIG_SPI_CALLBACK
  .registercallback  = stm32_spi4register,  /* provided externally */
#else
  .registercallback  = 0,                   /* not implemented */
#endif
};

static struct stm32_spidev_s g_spi4dev =
{
  .spidev   =
  {
    .ops    = &g_sp4iops,
  },
  .spibase  = STM32_SPI4_BASE,
  .spiclock = STM32_SPI4_FREQUENCY,
  .spiirq   = STM32_IRQ_SPI4,
  .lock     = NXMUTEX_INITIALIZER,
#ifdef CONFIG_PM
  .pm_cb.prepare = spi_pm_prepare,
#endif
#ifdef CONFIG_STM32N6_SPI4_COMMTYPE
  .config   = CONFIG_STM32N6_SPI4_COMMTYPE,
#else
  .config   = FULL_DUPLEX,
#endif
};
#endif /* CONFIG_STM32N6_SPI4 */

#ifdef CONFIG_STM32N6_SPI5
static const struct spi_ops_s g_sp5iops =
{
  .lock              = spi_lock,
  .select            = stm32_spi5select,
  .setfrequency      = spi_setfrequency,
#ifdef CONFIG_SPI_DELAY_CONTROL
  .setdelay          = spi_setdelay,
#endif
  .setmode           = spi_setmode,
  .setbits           = spi_setbits,
#ifdef CONFIG_SPI_HWFEATURES
  .hwfeatures        = spi_hwfeatures,
#endif
  .status            = stm32_spi5status,
#ifdef CONFIG_SPI_CMDDATA
  .cmddata           = stm32_spi5cmddata,
#endif
  .send              = spi_send,
#ifdef CONFIG_SPI_EXCHANGE
  .exchange          = spi_exchange,
#else
  .sndblock          = spi_sndblock,
  .recvblock         = spi_recvblock,
#endif
#ifdef CONFIG_SPI_TRIGGER
  .trigger           = spi_trigger,
#endif
#ifdef CONFIG_SPI_CALLBACK
  .registercallback  = stm32_spi5register,  /* provided externally */
#else
  .registercallback  = 0,                   /* not implemented */
#endif
};

static struct stm32_spidev_s g_spi5dev =
{
  .spidev   =
  {
    .ops    = &g_sp5iops,
  },
  .spibase  = STM32_SPI5_BASE,
  .spiclock = STM32_SPI5_FREQUENCY,
  .spiirq   = STM32_IRQ_SPI5,
  .lock     = NXMUTEX_INITIALIZER,
#ifdef CONFIG_PM
  .pm_cb.prepare = spi_pm_prepare,
#endif
#ifdef CONFIG_STM32N6_SPI5_COMMTYPE
  .config   = CONFIG_STM32N6_SPI5_COMMTYPE,
#else
  .config   = FULL_DUPLEX,
#endif
};
#endif /* CONFIG_STM32N6_SPI5 */

#ifdef CONFIG_STM32N6_SPI6
static const struct spi_ops_s g_sp6iops =
{
  .lock              = spi_lock,
  .select            = stm32_spi6select,
  .setfrequency      = spi_setfrequency,
#ifdef CONFIG_SPI_DELAY_CONTROL
  .setdelay          = spi_setdelay,
#endif
  .setmode           = spi_setmode,
  .setbits           = spi_setbits,
#ifdef CONFIG_SPI_HWFEATURES
  .hwfeatures        = spi_hwfeatures,
#endif
  .status            = stm32_spi6status,
#ifdef CONFIG_SPI_CMDDATA
  .cmddata           = stm32_spi6cmddata,
#endif
  .send              = spi_send,
#ifdef CONFIG_SPI_EXCHANGE
  .exchange          = spi_exchange,
#else
  .sndblock          = spi_sndblock,
  .recvblock         = spi_recvblock,
#endif
#ifdef CONFIG_SPI_TRIGGER
  .trigger           = spi_trigger,
#endif
#ifdef CONFIG_SPI_CALLBACK
  .registercallback  = stm32_spi6register,  /* provided externally */
#else
  .registercallback  = 0,                   /* not implemented */
#endif
};

static struct stm32_spidev_s g_spi6dev =
{
  .spidev   =
  {
    .ops    = &g_sp6iops,
  },
  .spibase  = STM32_SPI6_BASE,
  .spiclock = STM32_SPI6_FREQUENCY,
  .spiirq   = STM32_IRQ_SPI6,
  .lock     = NXMUTEX_INITIALIZER,
#ifdef CONFIG_PM
  .pm_cb.prepare = spi_pm_prepare,
#endif
#ifdef CONFIG_STM32N6_SPI6_COMMTYPE
  .config   = CONFIG_STM32N6_SPI6_COMMTYPE,
#else
  .config   = FULL_DUPLEX,
#endif
};
#endif /* CONFIG_STM32N6_SPI6 */

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: spi_getreg8
 *
 * Description:
 *   Get the contents of the SPI register at offset
 *
 * Input Parameters:
 *   priv   - private SPI device structure
 *   offset - offset to the register of interest
 *
 * Returned Value:
 *   The contents of the 8-bit register
 *
 ****************************************************************************/

static inline uint8_t spi_getreg8(struct stm32_spidev_s *priv,
                                  uint32_t offset)
{
  return getreg8(priv->spibase + offset);
}

/****************************************************************************
 * Name: spi_putreg8
 *
 * Description:
 *   Write a 8-bit value to the SPI register at offset
 *
 * Input Parameters:
 *   priv   - private SPI device structure
 *   offset - offset to the register of interest
 *   value  - the 8-bit value to be written
 *
 ****************************************************************************/

static inline void spi_putreg8(struct stm32_spidev_s *priv,
                               uint32_t offset, uint8_t value)
{
  putreg8(value, priv->spibase + offset);
}

/****************************************************************************
 * Name: spi_getreg16
 *
 * Description:
 *   Get the contents of the SPI register at offset
 *
 * Input Parameters:
 *   priv   - private SPI device structure
 *   offset - offset to the register of interest
 *
 * Returned Value:
 *   The contents of the 16-bit register
 *
 ****************************************************************************/

static inline uint16_t spi_getreg16(struct stm32_spidev_s *priv,
                                    uint32_t offset)
{
  return getreg16(priv->spibase + offset);
}

/****************************************************************************
 * Name: spi_putreg16
 *
 * Description:
 *   Write a 16-bit value to the SPI register at offset
 *
 * Input Parameters:
 *   priv   - private SPI device structure
 *   offset - offset to the register of interest
 *   value  - the 16-bit value to be written
 *
 ****************************************************************************/

static inline void spi_putreg16(struct stm32_spidev_s *priv,
                               uint32_t offset, uint16_t value)
{
  putreg16(value, priv->spibase + offset);
}

/****************************************************************************
 * Name: spi_getreg
 *
 * Description:
 *   Get the contents of the SPI register at offset
 *
 * Input Parameters:
 *   priv   - private SPI device structure
 *   offset - offset to the register of interest
 *
 * Returned Value:
 *   The contents of the 32-bit register
 *
 ****************************************************************************/

static inline uint32_t spi_getreg(struct stm32_spidev_s *priv,
                                  uint32_t offset)
{
  return getreg32(priv->spibase + offset);
}

/****************************************************************************
 * Name: spi_putreg
 *
 * Description:
 *   Write a 32-bit value to the SPI register at offset
 *
 * Input Parameters:
 *   priv   - private SPI device structure
 *   offset - offset to the register of interest
 *   value  - the 32-bit value to be written
 *
 ****************************************************************************/

static inline void spi_putreg(struct stm32_spidev_s *priv,
                              uint32_t offset, uint32_t value)
{
  putreg32(value, priv->spibase + offset);
}

/****************************************************************************
 * Name: spi_modifyreg
 *
 * Description:
 *   Modify an SPI register
 *
 ****************************************************************************/

static void spi_modifyreg(struct stm32_spidev_s *priv, uint32_t offset,
                          uint32_t clrbits, uint32_t setbits)
{
  /* Only 32-bit registers */

  modifyreg32(priv->spibase + offset, clrbits, setbits);
}

/****************************************************************************
 * Name: spi_readword
 *
 * Description:
 *   Read one word from SPI
 *
 * Input Parameters:
 *   priv - Device-specific state data
 *
 * Returned Value:
 *   Word as read
 *
 ****************************************************************************/

static inline uint32_t spi_readword(struct stm32_spidev_s *priv)
{
  /* Can't receive in tx only mode */

  if (priv->config == SIMPLEX_TX)
    {
      return 0;
    }

  /* Wait until the receive buffer is not empty */

  while ((spi_getreg(priv, STM32_SPI_SR_OFFSET) & SPI_SR_RXP) == 0);

  /* Then return the received 16 bit word */

  return spi_getreg16(priv, STM32_SPI_RXDR_OFFSET);
}

/****************************************************************************
 * Name: spi_writeword
 *
 * Description:
 *   Write one word to SPI
 *
 * Input Parameters:
 *   priv - Device-specific state data
 *   word - Word to send
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

static inline void spi_writeword(struct stm32_spidev_s *priv,
                                 uint32_t word)
{
  /* Can't transmit in rx only mode */

  if (priv->config == SIMPLEX_RX)
    {
      return;
    }

  /* Wait until the transmit buffer is empty */

  while ((spi_getreg(priv, STM32_SPI_SR_OFFSET) & SPI_SR_TXP) == 0);

  /* Then send the 16 bit word */

  spi_putreg16(priv, STM32_SPI_TXDR_OFFSET, word);
}

/****************************************************************************
 * Name: spi_readbyte
 *
 * Description:
 *   Read one byte from SPI
 *
 * Input Parameters:
 *   priv - Device-specific state data
 *
 * Returned Value:
 *   Byte as read
 *
 ****************************************************************************/

static inline uint8_t spi_readbyte(struct stm32_spidev_s *priv)
{
  /* Can't receive in tx only mode */

  if (priv->config == SIMPLEX_TX)
    {
      return 0;
    }

  /* Wait until the receive buffer is not empty */

  while ((spi_getreg(priv, STM32_SPI_SR_OFFSET) & SPI_SR_RXP) == 0);

  /* Then return the received byte */

  return spi_getreg8(priv, STM32_SPI_RXDR_OFFSET);
}

/****************************************************************************
 * Name: spi_writebyte
 *
 * Description:
 *   Write one 8-bit frame to the SPI FIFO
 *
 * Input Parameters:
 *   priv - Device-specific state data
 *   byte - Byte to send
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

static inline void spi_writebyte(struct stm32_spidev_s *priv,
                                 uint8_t byte)
{
  /* Can't transmit in rx only mode */

  if (priv->config == SIMPLEX_RX)
    {
      return;
    }

  /* Wait until the transmit buffer is empty */

  while ((spi_getreg(priv, STM32_SPI_SR_OFFSET) & SPI_SR_TXP) == 0);

  /* Then send the byte */

  spi_putreg8(priv, STM32_SPI_TXDR_OFFSET, byte);
}

/****************************************************************************
 * Name: spi_dumpregs
 ****************************************************************************/

#ifdef CONFIG_DEBUG_SPI_INFO
static void spi_dumpregs(struct stm32_spidev_s *priv)
{
  spiinfo("CR1: 0x%08" PRIx32 " CFG1: 0x%08" PRIx32
          " CFG2: 0x%08" PRIx32 "\n",
          spi_getreg(priv, STM32_SPI_CR1_OFFSET),
          spi_getreg(priv, STM32_SPI_CFG1_OFFSET),
          spi_getreg(priv, STM32_SPI_CFG2_OFFSET));
  spiinfo("IER: 0x%08" PRIx32 " SR: 0x%08" PRIx32
          " I2SCFGR: 0x%08" PRIx32 "\n",
          spi_getreg(priv, STM32_SPI_IER_OFFSET),
          spi_getreg(priv, STM32_SPI_SR_OFFSET),
          spi_getreg(priv, STM32_SPI_I2SCFGR_OFFSET));
}
#endif

/****************************************************************************
 * Name: spi_interrupt
 *
 * Description:
 *   Handle the SPI EOT/TXC interrupt for transfer completion notification
 *
 ****************************************************************************/

static int spi_interrupt(int irq, void *context, void *arg)
{
  struct stm32_spidev_s *priv = (struct stm32_spidev_s *)arg;
  uint32_t sr = spi_getreg(priv, STM32_SPI_SR_OFFSET);

  if (sr & SPI_SR_TXC)
    {
      /* Disable interrupt. This needs to be done
       * before disabling SPI to avoid spurious TXC interrupt
       * (ERRATA: Q2.14.4 TXP interrupt occurring while SPI/I2S disabled")
       * There is no need to clear the interrupt since it is disabled and
       * SPI will be disabled after transmit as well.
       */

      spi_modifyreg(priv, STM32_SPI_IER_OFFSET, SPI_IER_EOTIE, 0);
    }

  return 0;
}

/****************************************************************************
 * Name: spi_lock
 *
 * Description:
 *   On SPI buses where there are multiple devices, it will be necessary to
 *   lock SPI to have exclusive access to the buses for a sequence of
 *   transfers.  The bus should be locked before the chip is selected. After
 *   locking the SPI bus, the caller should then also call the setfrequency,
 *   setbits, and setmode methods to make sure that the SPI is properly
 *   configured for the device.  If the SPI bus is being shared, then it
 *   may have been left in an incompatible state.
 *
 * Input Parameters:
 *   dev  - Device-specific state data
 *   lock - true: Lock spi bus, false: unlock SPI bus
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

static int spi_lock(struct spi_dev_s *dev, bool lock)
{
  struct stm32_spidev_s *priv = (struct stm32_spidev_s *)dev;
  int ret;

  if (lock)
    {
      ret = nxmutex_lock(&priv->lock);
    }
  else
    {
      ret = nxmutex_unlock(&priv->lock);
    }

  return ret;
}

/****************************************************************************
 * Name: spi_enable
 ****************************************************************************/

static inline int spi_enable(struct stm32_spidev_s *priv, bool state)
{
  int ret = OK;

  if (state == true)
    {
      /* Enable SPI */

      spi_modifyreg(priv, STM32_SPI_CR1_OFFSET, 0, SPI_CR1_SPE);
    }
  else
    {
      /* Disable SPI */

      spi_modifyreg(priv, STM32_SPI_CR1_OFFSET, SPI_CR1_SPE, 0);
    }

  return ret;
}

/****************************************************************************
 * Name: spi_setfrequency
 *
 * Description:
 *   Set the SPI frequency.
 *
 * Input Parameters:
 *   dev -       Device-specific state data
 *   frequency - The SPI frequency requested
 *
 * Returned Value:
 *   Returns the actual frequency selected
 *
 ****************************************************************************/

static uint32_t spi_setfrequency(struct spi_dev_s *dev,
                                 uint32_t frequency)
{
  struct stm32_spidev_s *priv = (struct stm32_spidev_s *)dev;
  uint32_t setbits = 0;
  uint32_t actual  = 0;
  bool bypass  = false;

  /* Limit to max possible */

  if (frequency > STM32_SPI_CLK_MAX)
    {
      frequency = STM32_SPI_CLK_MAX;
    }

  /* Has the frequency changed? */

  if (frequency != priv->frequency)
    {
      /* Choices are limited by PCLK frequency
       * Check bypass to see if we use the prescaler
       */

      if (frequency >= priv->spiclock)
        {
          actual = priv->spiclock;
          bypass = true;
        }
      else if (frequency >= priv->spiclock >> 1)
        {
          /* More than fPCLK/2.  This is as fast as we can go */

          setbits = SPI_CFG1_MBR_FPCLKd2; /* 000: fPCLK/2 */
          actual = priv->spiclock >> 1;
        }
      else if (frequency >= priv->spiclock >> 2)
        {
          /* Between fPCLCK/2 and fPCLCK/4, pick the slower */

          setbits = SPI_CFG1_MBR_FPCLKd4; /* 001: fPCLK/4 */
          actual = priv->spiclock >> 2;
        }
      else if (frequency >= priv->spiclock >> 3)
        {
          /* Between fPCLCK/4 and fPCLCK/8, pick the slower */

          setbits = SPI_CFG1_MBR_FPCLKd8; /* 010: fPCLK/8 */
          actual = priv->spiclock >> 3;
        }
      else if (frequency >= priv->spiclock >> 4)
        {
          /* Between fPCLCK/8 and fPCLCK/16, pick the slower */

          setbits = SPI_CFG1_MBR_FPCLKd16; /* 011: fPCLK/16 */
          actual = priv->spiclock >> 4;
        }
      else if (frequency >= priv->spiclock >> 5)
        {
          /* Between fPCLCK/16 and fPCLCK/32, pick the slower */

          setbits = SPI_CFG1_MBR_FPCLKd32; /* 100: fPCLK/32 */
          actual = priv->spiclock >> 5;
        }
      else if (frequency >= priv->spiclock >> 6)
        {
          /* Between fPCLCK/32 and fPCLCK/64, pick the slower */

          setbits = SPI_CFG1_MBR_FPCLKd64; /*  101: fPCLK/64 */
          actual = priv->spiclock >> 6;
        }
      else if (frequency >= priv->spiclock >> 7)
        {
          /* Between fPCLCK/64 and fPCLCK/128, pick the slower */

          setbits = SPI_CFG1_MBR_FPCLKd128; /* 110: fPCLK/128 */
          actual = priv->spiclock >> 7;
        }
      else
        {
          /* Less than fPCLK/128.  This is as slow as we can go */

          setbits = SPI_CFG1_MBR_FPCLKd256; /* 111: fPCLK/256 */
          actual = priv->spiclock >> 8;
        }

      spi_enable(priv, false);
      if (bypass)
        {
          spi_modifyreg(priv,
                        STM32_SPI_CFG1_OFFSET,
                        SPI_CFG1_MBR_MASK,
                        SPI_CFG1_BPASS);
        }
      else
        {
          spi_modifyreg(priv,
                        STM32_SPI_CFG1_OFFSET,
                        SPI_CFG1_MBR_MASK | SPI_CFG1_BPASS,
                        setbits);
        }

      spi_enable(priv, true);

      /* Save the frequency selection so that subsequent reconfigurations
       * will be faster.
       */

      spiinfo("Frequency %" PRId32 "->%" PRId32 "\n", frequency, actual);

      priv->frequency = frequency;
      priv->actual    = actual;
    }

  return priv->actual;
}

/****************************************************************************
 * Name: spi_setdelay
 *
 * Description:
 *   Set the SPI Delays in nanoseconds. Optional.
 *
 * Input Parameters:
 *   dev        - Device-specific state data
 *   startdelay - The delay between CS active and first CLK
 *   stopdelay  - The delay between last CLK and CS inactive
 *   csdelay    - The delay between CS inactive and CS active again
 *   ifdelay    - The delay between frames
 *
 * Returned Value:
 *   Returns zero (OK) on success; a negated errno value is return on any
 *   failure.
 *
 ****************************************************************************/

#ifdef CONFIG_SPI_DELAY_CONTROL
static int spi_setdelay(struct spi_dev_s *dev, uint32_t startdelay,
                        uint32_t stopdelay, uint32_t csdelay,
                        uint32_t ifdelay)
{
  struct stm32_spidev_s *priv = (struct stm32_spidev_s *)dev;
  uint32_t setbits  = 0;
  uint32_t clrbits  = SPI_CFG2_MSSI_MASK | SPI_CFG2_MIDI_MASK;
  uint32_t nsperclk = NSEC_PER_SEC / priv->actual;

  startdelay /= nsperclk;
  ifdelay    /= nsperclk;

  setbits = ((ifdelay << SPI_CFG2_MIDI_SHIFT) & SPI_CFG2_MIDI_MASK) |
            ((startdelay << SPI_CFG2_MSSI_SHIFT) & SPI_CFG2_MSSI_MASK);

  spi_enable(priv, false);

  /* Change SPI mode */

  spi_modifyreg(priv, STM32_SPI_CFG2_OFFSET, clrbits, setbits);

  /* Re-enable SPI */

  spi_enable(priv, true);
  return OK;
}
#endif

/****************************************************************************
 * Name: spi_setmode
 *
 * Description:
 *   Set the SPI mode.  see enum spi_mode_e for mode definitions
 *
 * Input Parameters:
 *   dev  - Device-specific state data
 *   mode - The SPI mode requested
 *
 * Returned Value:
 *   Returns the actual frequency selected
 *
 ****************************************************************************/

static void spi_setmode(struct spi_dev_s *dev, enum spi_mode_e mode)
{
  struct stm32_spidev_s *priv = (struct stm32_spidev_s *)dev;
  uint32_t setbits = 0;
  uint32_t clrbits = 0;

  spiinfo("mode=%" PRIx32 "\n", (uint32_t) mode);

  /* Has the mode changed? */

  if (mode != priv->mode)
    {
      /* Yes... Set CR1 appropriately */

      switch (mode)
        {
        case SPIDEV_MODE0: /* CPOL=0; CPHA=0 */
          setbits = 0;
          clrbits = SPI_CFG2_CPOL | SPI_CFG2_CPHA;
          break;

        case SPIDEV_MODE1: /* CPOL=0; CPHA=1 */
          setbits = SPI_CFG2_CPHA;
          clrbits = SPI_CFG2_CPOL;
          break;

        case SPIDEV_MODE2: /* CPOL=1; CPHA=0 */
          setbits = SPI_CFG2_CPOL;
          clrbits = SPI_CFG2_CPHA;
          break;

        case SPIDEV_MODE3: /* CPOL=1; CPHA=1 */
          setbits = SPI_CFG2_CPOL | SPI_CFG2_CPHA;
          clrbits = 0;
          break;

        default:
          return;
        }

      spi_enable(priv, false);

      /* Change SPI mode */

      spi_modifyreg(priv, STM32_SPI_CFG2_OFFSET, clrbits, setbits);

      /* Re-enable SPI */

      spi_enable(priv, true);
      while ((spi_getreg(priv, STM32_SPI_SR_OFFSET) &
             SPI_SR_RXPLVL_MASK) != 0)
        {
          /* Flush SPI read FIFO */

          spi_getreg(priv, STM32_SPI_RXDR_OFFSET);
        }

      /* Save the mode so that subsequent re-configurations will be faster */

      priv->mode = mode;
    }
}

/****************************************************************************
 * Name: spi_setbits
 *
 * Description:
 *   Set the number of bits per word.
 *
 * Input Parameters:
 *   dev   - Device-specific state data
 *   nbits - The number of bits requested
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

static void spi_setbits(struct spi_dev_s *dev, int nbits)
{
  struct stm32_spidev_s *priv = (struct stm32_spidev_s *)dev;
  uint32_t setbits = 0;
  uint32_t clrbits = 0;

  spiinfo("nbits=%d\n", nbits);

  /* Has the number of bits changed? */

  if (nbits != priv->nbits)
    {
      /* Yes... Set CFG1 appropriately */

      /* Set the number of bits (valid range 4-32) */

      if (nbits < 4 || nbits > 32)
        {
          return;
        }

      clrbits = SPI_CFG1_DSIZE_MASK;
      setbits = SPI_CFG1_DSIZE_VAL(nbits);

      /* RX FIFO Threshold 1 Frame either 8 or 16 bits */

      setbits |= SPI_CFG1_FTHLV_1DATA;

      spi_enable(priv, false);
      spi_modifyreg(priv, STM32_SPI_CFG1_OFFSET, clrbits, setbits);
      spi_enable(priv, true);

      /* Save the selection so that subsequent re-configurations will be
       * faster.
       */

      priv->nbits = nbits;
    }
}

/****************************************************************************
 * Name: spi_hwfeatures
 *
 * Description:
 *   Set hardware-specific feature flags.
 *
 * Input Parameters:
 *   dev      - Device-specific state data
 *   features - H/W feature flags
 *
 * Returned Value:
 *   Zero (OK) if the selected H/W features are enabled; A negated errno
 *   value if any H/W feature is not supportable.
 *
 ****************************************************************************/

#ifdef CONFIG_SPI_HWFEATURES
static int spi_hwfeatures(struct spi_dev_s *dev,
                          spi_hwfeatures_t features)
{
#if defined(CONFIG_SPI_BITORDER) || defined(CONFIG_SPI_TRIGGER)
  struct stm32_spidev_s *priv = (struct stm32_spidev_s *)dev;
#endif

#ifdef CONFIG_SPI_BITORDER
  uint32_t setbits = 0;
  uint32_t clrbits = 0;

  spiinfo("features=%08" PRIx8 "\n", features);

  /* Transfer data LSB first? */

  if ((features & HWFEAT_LSBFIRST) != 0)
    {
      setbits = SPI_CFG2_LSBFRST;
      clrbits = 0;
    }
  else
    {
      setbits = 0;
      clrbits = SPI_CFG2_LSBFRST;
    }

  spi_enable(priv, false);
  spi_modifyreg(priv, STM32_SPI_CFG2_OFFSET, clrbits, setbits);
  spi_enable(priv, true);

  features &= ~HWFEAT_LSBFIRST;
#endif

#ifdef CONFIG_SPI_TRIGGER
/* Turn deferred trigger mode on or off.  Only applicable for DMA mode. If a
 * transfer is deferred then the DMA will not actually be triggered until a
 * subsequent call to SPI_TRIGGER to set it off. The thread will be waiting
 * on the transfer completing as normal.
 */

  priv->defertrig = ((features & HWFEAT_TRIGGER) != 0);
  features &= ~HWFEAT_TRIGGER;
#endif

  /* Other H/W features are not supported */

  return (features == 0) ? OK : -ENOSYS;
}
#endif

/****************************************************************************
 * Name: spi_send
 *
 * Description:
 *   Exchange one word on SPI
 *
 * Input Parameters:
 *   dev - Device-specific state data
 *   wd  - The word to send.  the size of the data is determined by the
 *         number of bits selected for the SPI interface.
 *
 * Returned Value:
 *   response
 *
 ****************************************************************************/

static uint32_t spi_send(struct spi_dev_s *dev, uint32_t wd)
{
  struct stm32_spidev_s *priv = (struct stm32_spidev_s *)dev;
  uint32_t ret = 0;

  DEBUGASSERT(priv && priv->spibase);

  /* Disable SPI to configure TSIZE */

  spi_enable(priv, false);

  /* Set TSIZE = 1 frame */

  spi_putreg(priv, STM32_SPI_CR2_OFFSET, 1);

  /* Clear EOT/TXTF/SUSP flags from any prior transfer */

  spi_putreg(priv, STM32_SPI_IFCR_OFFSET,
             SPI_IFCR_EOTC | SPI_IFCR_TXTFC | SPI_IFCR_SUSPC);

  /* Enable SPI */

  spi_enable(priv, true);

  /* Master transfer start */

  spi_modifyreg(priv, STM32_SPI_CR1_OFFSET, 0, SPI_CR1_CSTART);

  /* Write and read one frame */

  if (priv->nbits > 8)
    {
      spi_writeword(priv, (uint16_t)(wd & 0xffff));
      ret = spi_readword(priv);
    }
  else
    {
      spi_writebyte(priv, (uint8_t)(wd & 0xff));
      ret = (uint32_t)spi_readbyte(priv);
    }

  /* Wait for end of transfer */

  while ((spi_getreg(priv, STM32_SPI_SR_OFFSET) & SPI_SR_EOT) == 0);

  /* Clear flags and disable SPI */

  spi_putreg(priv, STM32_SPI_IFCR_OFFSET,
             SPI_IFCR_EOTC | SPI_IFCR_TXTFC);
  spi_enable(priv, false);

  spiinfo("Sent: %02" PRIx32 " Return: %02" PRIx32 "\n", wd, ret);

  return ret;
}

/****************************************************************************
 * Name: spi_exchange (polling, no DMA)
 *
 * Description:
 *   Exchange a block of data on SPI without using DMA
 *
 * Input Parameters:
 *   dev      - Device-specific state data
 *   txbuffer - A pointer to the buffer of data to be sent
 *   rxbuffer - A pointer to a buffer in which to receive data
 *   nwords   - the length of data to be exchanged in units of words.
 *              The wordsize is determined by the number of bits-per-word
 *              selected for the SPI interface.  If nbits <= 8, the data is
 *              packed into uint8_t's; if nbits >8, the data is packed into
 *              uint16_t's
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

static void spi_exchange(struct spi_dev_s *dev, const void *txbuffer,
                         void *rxbuffer, size_t nwords)
{
  struct stm32_spidev_s *priv = (struct stm32_spidev_s *)dev;
  size_t txcount = nwords;
  size_t rxcount = nwords;

  DEBUGASSERT(priv && priv->spibase);

  spiinfo("txbuffer=%p rxbuffer=%p nwords=%d\n", txbuffer, rxbuffer,
          (int)nwords);

  if (nwords == 0)
    {
      return;
    }

  /* Disable SPI to configure TSIZE */

  spi_enable(priv, false);

  /* Set TSIZE = number of frames to transfer */

  spi_putreg(priv, STM32_SPI_CR2_OFFSET,
             nwords & SPI_CR2_TSIZE_MASK);

  /* Clear EOT/TXTF/SUSP flags from any prior transfer */

  spi_putreg(priv, STM32_SPI_IFCR_OFFSET,
             SPI_IFCR_EOTC | SPI_IFCR_TXTFC | SPI_IFCR_SUSPC);

  /* Enable SPI and start master transfer */

  spi_enable(priv, true);
  spi_modifyreg(priv, STM32_SPI_CR1_OFFSET, 0, SPI_CR1_CSTART);

  /* 8- or 16-bit mode? */

  if (priv->nbits > 8)
    {
      /* 16-bit mode */

      const uint16_t *src  = (const uint16_t *)txbuffer;
            uint16_t *dest = (uint16_t *)rxbuffer;

      while (txcount > 0 || rxcount > 0)
        {
          uint32_t sr = spi_getreg(priv, STM32_SPI_SR_OFFSET);

          /* Transmit if TXP and we have data to send */

          if ((sr & SPI_SR_TXP) != 0 && txcount > 0)
            {
              spi_putreg16(priv, STM32_SPI_TXDR_OFFSET,
                           src ? *src++ : 0xffff);
              txcount--;
            }

          /* Receive if RXP and we have data to read */

          if ((sr & SPI_SR_RXP) != 0 && rxcount > 0)
            {
              uint16_t word = spi_getreg16(priv, STM32_SPI_RXDR_OFFSET);
              if (dest)
                {
                  *dest++ = word;
                }

              rxcount--;
            }
        }
    }
  else
    {
      /* 8-bit mode */

      const uint8_t *src  = (const uint8_t *)txbuffer;
            uint8_t *dest = (uint8_t *)rxbuffer;

      while (txcount > 0 || rxcount > 0)
        {
          uint32_t sr = spi_getreg(priv, STM32_SPI_SR_OFFSET);

          /* Transmit if TXP and we have data to send */

          if ((sr & SPI_SR_TXP) != 0 && txcount > 0)
            {
              spi_putreg8(priv, STM32_SPI_TXDR_OFFSET,
                          src ? *src++ : 0xff);
              txcount--;
            }

          /* Receive if RXP and we have data to read */

          if ((sr & SPI_SR_RXP) != 0 && rxcount > 0)
            {
              uint8_t word = spi_getreg8(priv, STM32_SPI_RXDR_OFFSET);
              if (dest)
                {
                  *dest++ = word;
                }

              rxcount--;
            }
        }
    }

  /* Wait for end of transfer */

  while ((spi_getreg(priv, STM32_SPI_SR_OFFSET) & SPI_SR_EOT) == 0);

  /* Clear flags and disable SPI */

  spi_putreg(priv, STM32_SPI_IFCR_OFFSET,
             SPI_IFCR_EOTC | SPI_IFCR_TXTFC);
  spi_enable(priv, false);
}

/****************************************************************************
 * Name: spi_trigger
 *
 * Description:
 *   Trigger a previously configured DMA transfer.
 *
 * Input Parameters:
 *   dev      - Device-specific state data
 *
 * Returned Value:
 *   OK       - Trigger was fired
 *   ENOTSUP  - Trigger not fired due to lack of DMA support
 *   EIO      - Trigger not fired because not previously primed
 *
 ****************************************************************************/

#ifdef CONFIG_SPI_TRIGGER
static int spi_trigger(struct spi_dev_s *dev)
{
  /* No DMA support in this driver, return -ENOSYS */

  return -ENOSYS;
}
#endif

/****************************************************************************
 * Name: spi_sndblock
 *
 * Description:
 *   Send a block of data on SPI
 *
 * Input Parameters:
 *   dev      - Device-specific state data
 *   txbuffer - A pointer to the buffer of data to be sent
 *   nwords   - the length of data to send from the buffer in number of
 *              words. The wordsize is determined by the number of
 *              bits-per-word selected for the SPI interface.  If nbits <= 8,
 *              the data is packed into uint8_t's; if nbits >8, the data is
 *              packed into uint16_t's
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

#ifndef CONFIG_SPI_EXCHANGE
static void spi_sndblock(struct spi_dev_s *dev,
                         const void *txbuffer,
                         size_t nwords)
{
  spiinfo("txbuffer=%p nwords=%d\n", txbuffer, nwords);
  return spi_exchange(dev, txbuffer, NULL, nwords);
}
#endif

/****************************************************************************
 * Name: spi_recvblock
 *
 * Description:
 *   Receive a block of data from SPI
 *
 * Input Parameters:
 *   dev      - Device-specific state data
 *   rxbuffer - A pointer to the buffer in which to receive data
 *   nwords   - the length of data that can be received in the buffer in
 *              number of words.  The wordsize is determined by the number of
 *              bits-per-word selected for the SPI interface.  If nbits <= 8,
 *              the data is packed into uint8_t's; if nbits >8, the data is
 *              packed into uint16_t's
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

#ifndef CONFIG_SPI_EXCHANGE
static void spi_recvblock(struct spi_dev_s *dev,
                          void *rxbuffer,
                          size_t nwords)
{
  spiinfo("rxbuffer=%p nwords=%d\n", rxbuffer, nwords);
  return spi_exchange(dev, NULL, rxbuffer, nwords);
}
#endif

/****************************************************************************
 * Name: spi_pm_prepare
 *
 * Description:
 *   Request the driver to prepare for a new power state. This is a
 *   warning that the system is about to enter into a new power state.  The
 *   driver should begin whatever operations that may be required to enter
 *   power state.  The driver may abort the state change mode by returning
 *   a non-zero value from the callback function.
 *
 * Input Parameters:
 *   cb      - Returned to the driver.  The driver version of the callback
 *             structure may include additional, driver-specific state
 *             data at the end of the structure.
 *   domain  - Identifies the activity domain of the state change
 *   pmstate - Identifies the new PM state
 *
 * Returned Value:
 *   0 (OK) means the event was successfully processed and that the driver
 *   is prepared for the PM state change.  Non-zero means that the driver
 *   is not prepared to perform the tasks needed achieve this power setting
 *   and will cause the state change to be aborted.  NOTE:  The prepare
 *   method will also be recalled when reverting from lower back to higher
 *   power consumption modes (say because another driver refused a lower
 *   power state change).  Drivers are not permitted to return non-zero
 *   values when reverting back to higher power consumption modes!
 *
 ****************************************************************************/

#ifdef CONFIG_PM
static int spi_pm_prepare(struct pm_callback_s *cb, int domain,
                          enum pm_state_e pmstate)
{
  struct stm32_spidev_s *priv =
      (struct stm32_spidev_s *)((char *)cb -
                                    offsetof(struct stm32_spidev_s, pm_cb));

  /* Logic to prepare for a reduced power state goes here. */

  switch (pmstate)
    {
    case PM_NORMAL:
    case PM_IDLE:
      break;

    case PM_STANDBY:
    case PM_SLEEP:

      /* Check if exclusive lock for SPI bus is held. */

      if (nxmutex_is_locked(&priv->lock))
        {
          /* Exclusive lock is held, do not allow entry to deeper PM
           * states.
           */

          return -EBUSY;
        }

      break;

    default:

      /* Should not get here */

      break;
    }

  return OK;
}
#endif

/****************************************************************************
 * Name: spi_enableclk
 *
 * Description:
 *   Enable the SPI peripheral clock using the atomic SET register (ENSR).
 *   STM32N6 requires ENSR writes instead of direct ENR read-modify-write.
 *
 * Input Parameters:
 *   priv - private SPI device structure
 *
 ****************************************************************************/

static void spi_enableclk(struct stm32_spidev_s *priv)
{
#ifdef CONFIG_STM32N6_SPI1
  if (priv->spibase == STM32_SPI1_BASE)
    {
      putreg32(RCC_APB2ENR_SPI1EN, STM32_RCC_APB2ENSR);
      return;
    }
#endif

#ifdef CONFIG_STM32N6_SPI2
  if (priv->spibase == STM32_SPI2_BASE)
    {
      putreg32(RCC_APB1ENR1_SPI2EN, STM32_RCC_APB1ENSR1);
      return;
    }
#endif

#ifdef CONFIG_STM32N6_SPI3
  if (priv->spibase == STM32_SPI3_BASE)
    {
      putreg32(RCC_APB1ENR1_SPI3EN, STM32_RCC_APB1ENSR1);
      return;
    }
#endif

#ifdef CONFIG_STM32N6_SPI4
  if (priv->spibase == STM32_SPI4_BASE)
    {
      putreg32(RCC_APB2ENR_SPI4EN, STM32_RCC_APB2ENSR);
      return;
    }
#endif

#ifdef CONFIG_STM32N6_SPI5
  if (priv->spibase == STM32_SPI5_BASE)
    {
      putreg32(RCC_APB2ENR_SPI5EN, STM32_RCC_APB2ENSR);
      return;
    }
#endif

#ifdef CONFIG_STM32N6_SPI6
  if (priv->spibase == STM32_SPI6_BASE)
    {
      putreg32(RCC_APB4ENR_SPI6EN, STM32_RCC_APB4ENSR);
      return;
    }
#endif
}

/****************************************************************************
 * Name: spi_bus_initialize
 *
 * Description:
 *   Initialize the selected SPI bus in its default state (Master, 8-bit,
 *   mode 0, etc.)
 *
 * Input Parameters:
 *   priv   - private SPI device structure
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

static void spi_bus_initialize(struct stm32_spidev_s *priv)
{
  uint32_t setbits = 0;
  uint32_t clrbits = 0;
#ifdef CONFIG_PM
  int ret;
#endif

  /* Configure CR1, CFG1 and CFG2. Default configuration:
   *   Mode 0:                        CFG2.CPHA=0 and CFG2.CPOL=0
   *   Master:                        CFG2.MSTR=1
   *   8-bit:                         CFG1.DSIZE=7
   *   MSB transmitted first:         CFG2.LSBFRST=0
   *   Replace NSS with SSI & SSI=1:  CR1.SSI=1 CFG2.SSM=1 (prevents MODF
   *                                  error)
   *   Two lines full duplex:         CFG2.COMM=0
   */

  /* CR1 */

  clrbits = SPI_CR1_SPE;
  setbits = SPI_CR1_SSI;

  spi_modifyreg(priv, STM32_SPI_CR1_OFFSET, clrbits, setbits);

  /* CFG1 */

  clrbits = SPI_CFG1_DSIZE_MASK;
  setbits = SPI_CFG1_DSIZE_8BIT | SPI_CFG1_FTHLV_1DATA; /* REVISIT: FTHLV */
  spi_modifyreg(priv, STM32_SPI_CFG1_OFFSET, clrbits, setbits);

  /* CFG2 */

  clrbits = SPI_CFG2_CPHA | SPI_CFG2_CPOL | SPI_CFG2_LSBFRST |
            SPI_CFG2_COMM_MASK;
  setbits = SPI_CFG2_AFCNTR | SPI_CFG2_MASTER | SPI_CFG2_SSM;

  switch (priv->config)
    {
    default:
    case FULL_DUPLEX:
      setbits |= SPI_CFG2_COMM_FULL;
      break;
    case SIMPLEX_TX:
      setbits |= SPI_CFG2_COMM_STX;
      break;
    case SIMPLEX_RX:
      setbits |= SPI_CFG2_COMM_SRX;
      break;
    case HALF_DUPLEX:
      setbits |= SPI_CFG2_COMM_HALF;
      break;
    }

  spi_modifyreg(priv, STM32_SPI_CFG2_OFFSET, clrbits, setbits);

  priv->frequency = 0;
  priv->nbits     = 8;
  priv->mode      = SPIDEV_MODE0;

  /* Select a default frequency of approx. 400KHz */

  spi_setfrequency((struct spi_dev_s *)priv, 400000);

  /* CRCPOLY configuration */

  spi_putreg(priv, STM32_SPI_CRCPOLY_OFFSET, 7);

  /* Attach the IRQ to the driver */

  if (irq_attach(priv->spiirq, spi_interrupt, priv))
    {
      /* We could not attach the ISR to the interrupt */

      spierr("ERROR: Can't initialize spi irq\n");
      return;
    }

  /* Enable SPI interrupts */

  up_enable_irq(priv->spiirq);

  /* Note: SPE is NOT enabled here.  Each transfer (spi_send/spi_exchange)
   * manages SPE: disable → set TSIZE → enable → CSTART → transfer → EOT →
   * disable.  This matches the ST HAL approach for STM32N6.
   */

#ifdef CONFIG_PM
  /* Register to receive power management callbacks */

  ret = pm_register(&priv->pm_cb);
  DEBUGASSERT(ret == OK);
  UNUSED(ret);
#endif

#ifdef CONFIG_DEBUG_SPI_INFO
  /* Dump registers after initialization */

  spi_dumpregs(priv);
#endif
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_spibus_initialize
 *
 * Description:
 *   Initialize the selected SPI bus
 *
 * Input Parameters:
 *   Port number (for hardware that has multiple SPI interfaces)
 *
 * Returned Value:
 *   Valid SPI device structure reference on success; a NULL on failure
 *
 ****************************************************************************/

struct spi_dev_s *stm32_spibus_initialize(int bus)
{
  struct stm32_spidev_s *priv = NULL;

  irqstate_t flags = enter_critical_section();
#ifdef CONFIG_STM32N6_SPI1
  if (bus == 1)
    {
      /* Select SPI1 */

      priv = &g_spi1dev;

      /* Only configure if the bus is not already configured */

      if (!priv->initialized)
        {
          /* Enable the SPI1 clock */

          spi_enableclk(priv);

          /* Configure SPI1 pins: SCK, MISO, and MOSI */

          stm32_configgpio(GPIO_SPI1_SCK);
          stm32_configgpio(GPIO_SPI1_MISO);
          stm32_configgpio(GPIO_SPI1_MOSI);

          /* Set up default configuration: Master, 8-bit, etc. */

          spi_bus_initialize(priv);
          priv->initialized = true;
        }
    }
  else
#endif
#ifdef CONFIG_STM32N6_SPI2
  if (bus == 2)
    {
      /* Select SPI2 */

      priv = &g_spi2dev;

      /* Only configure if the bus is not already configured */

      if (!priv->initialized)
        {
          /* Enable the SPI2 clock */

          spi_enableclk(priv);

          /* Configure SPI2 pins: SCK, MISO, and MOSI */

          stm32_configgpio(GPIO_SPI2_SCK);
          stm32_configgpio(GPIO_SPI2_MISO);
          stm32_configgpio(GPIO_SPI2_MOSI);

          /* Set up default configuration: Master, 8-bit, etc. */

          spi_bus_initialize(priv);
          priv->initialized = true;
        }
    }
  else
#endif
#ifdef CONFIG_STM32N6_SPI3
  if (bus == 3)
    {
      /* Select SPI3 */

      priv = &g_spi3dev;

      /* Only configure if the bus is not already configured */

      if (!priv->initialized)
        {
          /* Enable the SPI3 clock */

          spi_enableclk(priv);

          /* Configure SPI3 pins: SCK, MISO, and MOSI */

          stm32_configgpio(GPIO_SPI3_SCK);
          stm32_configgpio(GPIO_SPI3_MISO);
          stm32_configgpio(GPIO_SPI3_MOSI);

          /* Set up default configuration: Master, 8-bit, etc. */

          spi_bus_initialize(priv);
          priv->initialized = true;
        }
    }
  else
#endif
#ifdef CONFIG_STM32N6_SPI4
  if (bus == 4)
    {
      /* Select SPI4 */

      priv = &g_spi4dev;

      /* Only configure if the bus is not already configured */

      if (!priv->initialized)
        {
          /* Enable the SPI4 clock */

          spi_enableclk(priv);

          /* Configure SPI4 pins: SCK, MISO, and MOSI */

          stm32_configgpio(GPIO_SPI4_SCK);
          stm32_configgpio(GPIO_SPI4_MISO);
          stm32_configgpio(GPIO_SPI4_MOSI);

          /* Set up default configuration: Master, 8-bit, etc. */

          spi_bus_initialize(priv);
          priv->initialized = true;
        }
    }
  else
#endif
#ifdef CONFIG_STM32N6_SPI5
  if (bus == 5)
    {
      /* Select SPI5 */

      priv = &g_spi5dev;

      /* Only configure if the bus is not already configured */

      if (!priv->initialized)
        {
          /* Enable the SPI5 clock */

          spi_enableclk(priv);

          /* Configure SPI5 pins: SCK, MISO, and MOSI */

          stm32_configgpio(GPIO_SPI5_SCK);
          stm32_configgpio(GPIO_SPI5_MISO);
          stm32_configgpio(GPIO_SPI5_MOSI);

          /* Set up default configuration: Master, 8-bit, etc. */

          spi_bus_initialize(priv);
          priv->initialized = true;
        }
    }
  else
#endif
#ifdef CONFIG_STM32N6_SPI6
  if (bus == 6)
    {
      /* Select SPI6 */

      priv = &g_spi6dev;

      /* Only configure if the bus is not already configured */

      if (!priv->initialized)
        {
          /* Enable the SPI6 clock */

          spi_enableclk(priv);

          /* Configure SPI6 pins: SCK, MISO, and MOSI */

          stm32_configgpio(GPIO_SPI6_SCK);
          stm32_configgpio(GPIO_SPI6_MISO);
          stm32_configgpio(GPIO_SPI6_MOSI);

          /* Set up default configuration: Master, 8-bit, etc. */

          spi_bus_initialize(priv);
          priv->initialized = true;
        }
    }
  else
#endif
    {
      spierr("ERROR: Unsupported SPI bus: %d\n", bus);
    }

  leave_critical_section(flags);
  return (struct spi_dev_s *)priv;
}

#endif /* CONFIG_STM32N6_SPI1 || CONFIG_STM32N6_SPI2 || CONFIG_STM32N6_SPI3 ||
        * CONFIG_STM32N6_SPI4 || CONFIG_STM32N6_SPI5 || CONFIG_STM32N6_SPI6
        */
