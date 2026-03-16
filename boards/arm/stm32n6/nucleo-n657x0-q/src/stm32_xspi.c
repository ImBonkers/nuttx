/****************************************************************************
 * boards/arm/stm32n6/nucleo-n657x0-q/src/stm32_xspi.c
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

#include <stdio.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <debug.h>

#include <nuttx/mtd/mtd.h>
#include <nuttx/spi/qspi.h>

#include "stm32_xspi.h"

#ifdef CONFIG_STM32N6_XSPI

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_xspi_setup
 *
 * Description:
 *   Initialize XSPI2 and register the MX25UM51245G as an MTD device.
 *   The flash is a 64MB Octal NOR. We use SPI 1-1-1 mode initially.
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno value on failure.
 *
 ****************************************************************************/

int stm32_xspi_setup(void)
{
  struct qspi_dev_s *qspi;
  struct mtd_dev_s *mtd;
  int ret;

  /* Initialize the XSPI2 driver */

  qspi = stm32_xspi_initialize(0);
  if (qspi == NULL)
    {
      ferr("ERROR: stm32_xspi_initialize failed\n");
      return -ENODEV;
    }

  /* Initialize the MX25RXX MTD driver.
   *
   * NOTE: The mx25rxx driver supports MX25R/MX25L parts (JEDEC type
   * 0x28/0x20). The MX25UM51245G has JEDEC type 0x80 (Octal). In SPI
   * 1-1-1 mode, it responds to standard SPI commands but with a different
   * JEDEC ID. If mx25rxx_initialize fails, the flash is still accessible
   * via direct QSPI command/memory ops.
   */

  mtd = mx25rxx_initialize(qspi, true);
  if (mtd == NULL)
    {
      fwarn("WARNING: mx25rxx_initialize failed (expected for MX25UM)\n");

      /* For now, the QSPI device is still available for direct use.
       * A custom MTD driver for MX25UM51245G can be added later.
       */

      return OK;
    }

  /* Register the MTD device as /dev/mtdblock0 */

  ret = register_mtddriver("/dev/mtdblock0", mtd, 0755, NULL);
  if (ret < 0)
    {
      ferr("ERROR: register_mtddriver failed: %d\n", ret);
      return ret;
    }

  finfo("XSPI2 flash registered as /dev/mtdblock0\n");

#ifdef CONFIG_FS_LITTLEFS
  /* Mount littlefs on the flash. Try normal mount first, then format. */

  mkdir("/mnt", 0777);
  ret = nx_mount("/dev/mtdblock0", "/mnt", "littlefs", 0, NULL);
  if (ret < 0)
    {
      fwarn("littlefs mount failed (%d), formatting...\n", ret);
      ret = nx_mount("/dev/mtdblock0", "/mnt", "littlefs", 0, "forceformat");
      if (ret < 0)
        {
          ferr("ERROR: littlefs format+mount failed: %d\n", ret);
        }
      else
        {
          finfo("littlefs formatted and mounted at /mnt\n");
        }
    }
  else
    {
      finfo("littlefs mounted at /mnt\n");
    }
#endif

  return OK;
}

#endif /* CONFIG_STM32N6_XSPI */
