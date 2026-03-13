/****************************************************************************
 * arch/arm/src/stm32n6/stm32_spi.h
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

#ifndef __ARCH_ARM_SRC_STM32N6_STM32_SPI_H
#define __ARCH_ARM_SRC_STM32N6_STM32_SPI_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdbool.h>

#include "chip.h"
#include "hardware/stm32n6xxx_spi.h"

#ifndef __ASSEMBLY__

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

struct spi_dev_s; /* Forward reference */

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_spibus_initialize
 *
 * Description:
 *   Initialize the selected SPI bus
 *
 * Input Parameters:
 *   bus number (for hardware that has multiple SPI interfaces)
 *
 * Returned Value:
 *   Valid SPI device structure reference on success; a NULL on failure
 *
 ****************************************************************************/

struct spi_dev_s *stm32_spibus_initialize(int bus);

/****************************************************************************
 * Name:  stm32_spi1/2/...select and stm32_spi1/2/...status
 *
 * Description:
 *   The external functions, stm32_spi1/2/...select, stm32_spi1/2/...status,
 *   and stm32_spi1/2/...cmddata must be provided by board-specific logic.
 *   These are implementations of the select, status, and cmddata methods of
 *   the SPI interface defined by struct spi_ops_s (see
 *   include/nuttx/spi/spi.h).
 *
 ****************************************************************************/

#ifdef CONFIG_STM32N6_SPI1
void stm32_spi1select(struct spi_dev_s *dev, uint32_t devid,
                      bool selected);
uint8_t stm32_spi1status(struct spi_dev_s *dev, uint32_t devid);
int stm32_spi1cmddata(struct spi_dev_s *dev, uint32_t devid, bool cmd);
#endif

#ifdef CONFIG_STM32N6_SPI2
void stm32_spi2select(struct spi_dev_s *dev, uint32_t devid,
                      bool selected);
uint8_t stm32_spi2status(struct spi_dev_s *dev, uint32_t devid);
int stm32_spi2cmddata(struct spi_dev_s *dev, uint32_t devid, bool cmd);
#endif

#ifdef CONFIG_STM32N6_SPI3
void stm32_spi3select(struct spi_dev_s *dev, uint32_t devid,
                      bool selected);
uint8_t stm32_spi3status(struct spi_dev_s *dev, uint32_t devid);
int stm32_spi3cmddata(struct spi_dev_s *dev, uint32_t devid, bool cmd);
#endif

#ifdef CONFIG_STM32N6_SPI4
void stm32_spi4select(struct spi_dev_s *dev, uint32_t devid,
                      bool selected);
uint8_t stm32_spi4status(struct spi_dev_s *dev, uint32_t devid);
int stm32_spi4cmddata(struct spi_dev_s *dev, uint32_t devid, bool cmd);
#endif

#ifdef CONFIG_STM32N6_SPI5
void stm32_spi5select(struct spi_dev_s *dev, uint32_t devid,
                      bool selected);
uint8_t stm32_spi5status(struct spi_dev_s *dev, uint32_t devid);
int stm32_spi5cmddata(struct spi_dev_s *dev, uint32_t devid, bool cmd);
#endif

#ifdef CONFIG_STM32N6_SPI6
void stm32_spi6select(struct spi_dev_s *dev, uint32_t devid,
                      bool selected);
uint8_t stm32_spi6status(struct spi_dev_s *dev, uint32_t devid);
int stm32_spi6cmddata(struct spi_dev_s *dev, uint32_t devid, bool cmd);
#endif

/****************************************************************************
 * Name: stm32_spi1/2/...register
 *
 * Description:
 *   If the board supports a card detect callback to inform the SPI-based
 *   MMC/SD driver when an SD card is inserted or removed, then
 *   CONFIG_SPI_CALLBACK should be defined and the following function(s) must
 *   be implemented.
 *
 ****************************************************************************/

#ifdef CONFIG_SPI_CALLBACK
#ifdef CONFIG_STM32N6_SPI1
int stm32_spi1register(struct spi_dev_s *dev, spi_mediachange_t callback,
                       void *arg);
#endif
#ifdef CONFIG_STM32N6_SPI2
int stm32_spi2register(struct spi_dev_s *dev, spi_mediachange_t callback,
                       void *arg);
#endif
#ifdef CONFIG_STM32N6_SPI3
int stm32_spi3register(struct spi_dev_s *dev, spi_mediachange_t callback,
                       void *arg);
#endif
#ifdef CONFIG_STM32N6_SPI4
int stm32_spi4register(struct spi_dev_s *dev, spi_mediachange_t callback,
                       void *arg);
#endif
#ifdef CONFIG_STM32N6_SPI5
int stm32_spi5register(struct spi_dev_s *dev, spi_mediachange_t callback,
                       void *arg);
#endif
#ifdef CONFIG_STM32N6_SPI6
int stm32_spi6register(struct spi_dev_s *dev, spi_mediachange_t callback,
                       void *arg);
#endif
#endif

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __ASSEMBLY__ */
#endif /* __ARCH_ARM_SRC_STM32N6_STM32_SPI_H */
