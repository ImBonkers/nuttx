/****************************************************************************
 * boards/arm/stm32n6/nucleo-n657x0-q/src/stm32_bringup.c
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

#include <sys/mount.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <debug.h>

#include <nuttx/board.h>
#include <nuttx/spi/spi_transfer.h>
#include <nuttx/i2c/i2c_master.h>
#ifdef CONFIG_CDCACM
#include <nuttx/usb/cdcacm.h>
#endif
#ifdef CONFIG_RTC_DRIVER
#include <nuttx/timers/rtc.h>
#include "stm32_rtc.h"
#endif

#include <nuttx/cache.h>
#include "arm_internal.h"
#include "stm32_rcc.h"
#include "stm32_gpio.h"
#include "stm32_spi.h"
#include "stm32_i2c.h"
#ifdef CONFIG_STM32N6_ADC
#include "stm32_adc.h"
#endif
#ifdef CONFIG_STM32N6_PWM
#include "stm32_pwm.h"
#endif
#ifdef CONFIG_STM32N6_IWDG
#include "stm32_wdg.h"
#endif
#ifdef CONFIG_STM32N6_XSPI
#include "stm32_xspi.h"
#endif
#include "nucleo-n657x0-q.h"

#include <arch/board/board.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef CONFIG_STM32N6_NPU

/* STRENG register offsets (per engine, base = NPU_BASE + 0x5000 + 0x1000*id) */

#define STRENG_CTRL       0x00
#define STRENG_ADDR       0x08
#define STRENG_FSIZE      0x0c
#define STRENG_STRD       0x14
#define STRENG_FOFFSET    0x18
#define STRENG_FRAME_RPT  0x1c
#define STRENG_FRPTOFF    0x20
#define STRENG_LIMITEN    0x30
#define STRENG_LIMIT      0x34
#define STRENG_IRQ        0x3c
#define STRENG_CID_CACHE  0x48

/* STRSWITCH register offsets (base = NPU_BASE + 0x4000) */

#define STRSWITCH_CTRL    0x00
#define STRSWITCH_DST(n)  (0x08 + 4 * (n))

/* STRENG CTRL bit definitions */

#define STRENG_CTRL_EN      (1 << 0)
#define STRENG_CTRL_CLR     (1 << 1)
#define STRENG_CTRL_SINGLE  (1 << 2)
#define STRENG_CTRL_DIR     (1 << 3)   /* 1 = stream-to-memory (output) */
#define STRENG_CTRL_RAW     (1 << 8)
#define STRENG_CTRL_RUNNING (1u << 31)

/* SIZE0=8, SIZE1/2=0 → 8-bit RAW element (1 lane, simplest mode) */

#define STRENG_CTRL_SIZE_8BIT   (8 << 16)

/* STRSWITCH CTRL bits */

#define STRSWITCH_CTRL_EN      (1 << 0)
#define STRSWITCH_CTRL_CLR     (1 << 1)
#define STRSWITCH_CTRL_CONFCLR (1 << 30)

/* NPU memcopy test buffer size: 96 bytes = 96 elements at 8-bit */

#define NPU_MCPY_BYTES    96
#define NPU_MCPY_ELEMENTS 96

/* CACHEAXI register offsets (base = STM32_CACHEAXI_BASE = 0x580DFC00) */

#define CACHEAXI_CR1          0x00
#define CACHEAXI_SR           0x04
#define CACHEAXI_FCR          0x0c
#define CACHEAXI_CR2          0x100
#define CACHEAXI_CMDRSADDRR   0x104
#define CACHEAXI_CMDREADDRR   0x108

/* SR bits */

#define CACHEAXI_SR_BUSYF     (1 << 0)
#define CACHEAXI_SR_BUSYCMDF  (1 << 3)
#define CACHEAXI_SR_CMDENDF   (1 << 4)

/* CR2 CACHECMD encoding */

#define CACHEAXI_CR2_STARTCMD (1 << 0)
#define CACHEAXI_CR2_CLEAN    (1 << 1)          /* CACHECMD = 01: clean */
#define CACHEAXI_CR2_CLEANINV ((1 << 1) | (1 << 2))  /* CACHECMD = 11: clean+inv */

/* FCR: clear BSYENDF + CMDENDF */

#define CACHEAXI_FCR_CLEARALL 0x12

#endif /* CONFIG_STM32N6_NPU */

/****************************************************************************
 * Private Data
 ****************************************************************************/

#ifdef CONFIG_STM32N6_NPU
/* NPU test buffers will be placed in SRAM2 (0x34100000) at runtime.
 * The NPU CACHEAXI AXI port may only reach SRAM2-6, not SRAM1.
 */

#define NPU_SRC_ADDR  0x34100000  /* SRAM2 src buffer */
#define NPU_DST_ADDR  0x34100100  /* SRAM2 dst buffer */

/****************************************************************************
 * Name: cacheaxi_clean_invalidate_range
 *
 * Description:
 *   Flush CACHEAXI write-back cache for an address range so that CPU can
 *   read data written by the NPU through CACHEAXI.  Without this, NPU
 *   writes with CACHEABLE=1 stay in CACHEAXI and never reach SRAM.
 *
 ****************************************************************************/

static void cacheaxi_clean_invalidate_range(uintptr_t addr, size_t size)
{
  uint32_t base = STM32_CACHEAXI_BASE;
  volatile int timeout;

  /* Wait if a previous command is still in progress */

  timeout = 100000;
  while ((getreg32(base + CACHEAXI_SR) & CACHEAXI_SR_BUSYCMDF)
         && --timeout > 0);

  /* Clear pending flags */

  putreg32(CACHEAXI_FCR_CLEARALL, base + CACHEAXI_FCR);

  /* Set address range: start and end (inclusive) */

  putreg32(addr, base + CACHEAXI_CMDRSADDRR);
  putreg32(addr + size - 1, base + CACHEAXI_CMDREADDRR);

  /* Set command = clean+invalidate, then start */

  modifyreg32(base + CACHEAXI_CR2, 0x07,
              CACHEAXI_CR2_CLEANINV | CACHEAXI_CR2_STARTCMD);

  /* Poll for completion */

  timeout = 100000;
  while (!(getreg32(base + CACHEAXI_SR) & CACHEAXI_SR_CMDENDF)
         && --timeout > 0);

  /* Clear flags for next operation */

  putreg32(CACHEAXI_FCR_CLEARALL, base + CACHEAXI_FCR);
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_bringup
 *
 * Description:
 *   Perform architecture-specific initialization
 *
 *   CONFIG_BOARD_LATE_INITIALIZE=y :
 *     Called from board_late_initialize().
 *
 *   CONFIG_BOARD_LATE_INITIALIZE=n && CONFIG_BOARDCTL=y :
 *     Called from the NSH library
 *
 ****************************************************************************/

int stm32_bringup(void)
{
  int ret;

#ifdef CONFIG_STM32N6_NPU
  /* NPU full init: power on SRAM, configure RIFSC, init ATON fabric
   * (clock gates, bus interfaces, interrupt controller), enable CACHEAXI,
   * enable SRAM interleaving, and scan all IP block VERSION registers.
   * NPU clock enable + reset already done in stm32_start.c.
   */

  {
    /* Table of ATON IP blocks: offset from NPU_BASE, instance count */

    static const struct
    {
      uint32_t off;
      uint8_t  cnt;
    } npu_blocks[] =
    {
      { 0x00000,  1 },  /* CLKCTRL */
      { 0x01000,  1 },  /* INTCTRL */
      { 0x02000,  2 },  /* BUSIF */
      { 0x04000,  1 },  /* STRSWITCH */
      { 0x05000, 10 },  /* STRENG */
      { 0x0f000,  4 },  /* CONVACC */
      { 0x13000,  2 },  /* DECUN */
      { 0x15000,  2 },  /* ACTIV */
      { 0x17000,  4 },  /* ARITH */
      { 0x1b000,  2 },  /* POOL */
      { 0x1d000,  1 },  /* RECBUF */
      { 0x1e000,  1 },  /* EPOCHCTRL */
      { 0x1f000,  1 },  /* DBG_TRACE */
    };

    const int nblocks = sizeof(npu_blocks) / sizeof(npu_blocks[0]);
    int pass = 0;
    int total = 0;
    int i;
    int j;

    /* Enable RAMCFG clock (AHB2) */

    putreg32(RCC_AHB2ENR_RAMCFGEN, STM32_RCC_AHB2ENSR);

    /* Power on SRAM3-6 by clearing SRAMSD (bit 20) in each RAMCFG CR.
     * SRAM2 is the first NPU activation bank but shares NuttX SRAM bank
     * clocks — only SRAM3-6 need explicit RAMCFG power-on.
     */

    modifyreg32(STM32_RAMCFG_BASE + 0x100, (1 << 20), 0); /* SRAM3 */
    modifyreg32(STM32_RAMCFG_BASE + 0x180, (1 << 20), 0); /* SRAM4 */
    modifyreg32(STM32_RAMCFG_BASE + 0x200, (1 << 20), 0); /* SRAM5 */
    modifyreg32(STM32_RAMCFG_BASE + 0x280, (1 << 20), 0); /* SRAM6 */

    /* Configure RIFSC RIMC_ATTR[1] for NPU bus master:
     * MCID=1 (bit4), MSEC=1 (bit8), MPRIV=1 (bit9) → 0x310
     */

    putreg32(0x310, STM32_RIFSC_BASE + 0xc14);  /* RIMC: CID=1, SEC, PRIV */

    /* Set NPU RISC slave attributes: Secure + Privileged.
     * RISC_SECCFGR3 bit 10 = NPU SEC, RISC_PRIVCFGR3 bit 10 = NPU PRIV.
     * Without this, the NPU AXI master port may be inactive.
     */

    modifyreg32(STM32_RIFSC_BASE + 0x1c, 0, (1 << 10));  /* SECCFGR3 */
    modifyreg32(STM32_RIFSC_BASE + 0x3c, 0, (1 << 10));  /* PRIVCFGR3 */

    /* Step 1: ATON internal clock init (follows LL_ATON_Init sequence).
     * Clear the pipeline, then enable CLKCTRL and all clock gates.
     */

    {
      uint32_t t = getreg32(STM32_NPU_BASE + 0x0);
      t |= (1 << 1);  /* CTRL.CLR — clear pipeline */
      putreg32(t, STM32_NPU_BASE + 0x0);
    }

    putreg32(1, STM32_NPU_BASE + 0x00);          /* CTRL.EN = 1 */
    putreg32(0xffffffff, STM32_NPU_BASE + 0x08);  /* AGATES0: all on */
    putreg32(0xffffffff, STM32_NPU_BASE + 0x0c);  /* AGATES1: all on */
    putreg32(0xffffffff, STM32_NPU_BASE + 0x10);  /* BGATES: all on */

    /* Step 2: Enable bus interfaces and interrupt controller */

    putreg32(1, STM32_NPU_BASE + 0x2000);  /* BUSIF0 CTRL.EN */
    putreg32(1, STM32_NPU_BASE + 0x3000);  /* BUSIF1 CTRL.EN */
    putreg32(1, STM32_NPU_BASE + 0x1000);  /* INTCTRL CTRL.EN */

    /* Step 3: Disable SRAM interleaving for NPU (test: might interfere) */

    modifyreg32(STM32_SYSCFG_BASE + 0x78, 1, 0);

    /* Step 4: Enable CACHEAXI (NPU cache).
     * Must enable CACHEAXIRAM clock FIRST, then force-reset, then
     * invalidate + enable (matches x-cube npu_cache_enable_clocks_and_reset).
     */

    putreg32((1 << 10), STM32_RCC_MEMENSR);     /* CACHEAXIRAM clock */
    putreg32((1u << 30), STM32_RCC_AHB5RSTSR);  /* Assert CACHEAXI reset */
    {
      volatile int d;
      for (d = 0; d < 1000; d++);
    }

    putreg32((1u << 30), STM32_RCC_AHB5RSTCR);  /* Release CACHEAXI reset */

    putreg32(0x02, STM32_CACHEAXI_BASE + 0x0);  /* CR1.CACHEINV */
    while (getreg32(STM32_CACHEAXI_BASE + 0x4) & 1)
      {
        /* Wait for BUSYF to clear */
      }

    putreg32(0x01 | 0x3f0f0000, STM32_CACHEAXI_BASE + 0x0);  /* CR1.EN + all monitors */

    /* Step 5: Scan all IP blocks — verify VERSION registers are non-zero */

    for (i = 0; i < nblocks; i++)
      {
        for (j = 0; j < npu_blocks[i].cnt; j++)
          {
            uint32_t addr = STM32_NPU_BASE + npu_blocks[i].off
                            + 0x1000 * j + 0x4;
            total++;
            if (getreg32(addr) != 0)
              {
                pass++;
              }
          }
      }

    syslog(LOG_INFO, "NPU: %d/%d blocks alive\n", pass, total);

    /* SMPS overdrive (PB12 HIGH) is now done early in
     * stm32_configure_pll2_pll3() before PLL2/PLL3 are enabled.
     */

    /* Enable ALL bus + memory clocks */

    putreg32(0xffffffff, STM32_RCC_BUSENSR);
    putreg32(0xffffffff, STM32_RCC_MEMENSR);

    /* STRENG memcopy test — 96 bytes in SRAM2 */

    {
#define STRENG_POS  0x24

      uint32_t se0 = STM32_NPU_BASE + 0x5000;
      uint32_t se1 = STM32_NPU_BASE + 0x5000 + 0x1000;
      uint32_t strswitch = STM32_NPU_BASE + 0x4000;
      uint32_t ctrl;
      int timeout;
      uint8_t *src = (uint8_t *)NPU_SRC_ADDR;
      uint8_t *dst = (uint8_t *)NPU_DST_ADDR;

      /* Fill src with pattern, dst with 0xAA */

      for (i = 0; i < NPU_MCPY_BYTES; i++)
        {
          src[i] = (uint8_t)i;
        }

      memset(dst, 0xaa, NPU_MCPY_BYTES);
      up_clean_dcache((uintptr_t)src,
                      (uintptr_t)src + NPU_MCPY_BYTES);
      up_clean_dcache((uintptr_t)dst,
                      (uintptr_t)dst + NPU_MCPY_BYTES);
      up_invalidate_dcache((uintptr_t)dst,
                           (uintptr_t)dst + NPU_MCPY_BYTES);

      /* Enable AXI interconnect clocks + CACHEAXIRAM */

      /* Reset STRSWITCH */

      putreg32(STRSWITCH_CTRL_CLR, strswitch + STRSWITCH_CTRL);
      timeout = 10000;
      while ((getreg32(strswitch + STRSWITCH_CTRL) & STRSWITCH_CTRL_CLR)
             && --timeout > 0);
      putreg32(STRSWITCH_CTRL_CONFCLR, strswitch + STRSWITCH_CTRL);
      timeout = 10000;
      while ((getreg32(strswitch + STRSWITCH_CTRL)
              & STRSWITCH_CTRL_CONFCLR) && --timeout > 0);

      /* CLR + config SE0 (input) */

      putreg32(STRENG_CTRL_CLR, se0 + STRENG_CTRL);
      timeout = 10000;
      while ((getreg32(se0 + STRENG_CTRL) & STRENG_CTRL_CLR)
             && --timeout > 0);
      putreg32(0xff, se0 + STRENG_IRQ);
      putreg32((uintptr_t)src, se0 + STRENG_ADDR);
      putreg32(NPU_MCPY_ELEMENTS, se0 + STRENG_FSIZE);
      putreg32(0, se0 + STRENG_STRD);
      putreg32(NPU_MCPY_BYTES, se0 + STRENG_FOFFSET);
      putreg32(0, se0 + STRENG_FRAME_RPT);
      putreg32(0, se0 + STRENG_FRPTOFF);
      putreg32(0x00000024, se0 + STRENG_POS);
      putreg32(0x04, se0 + STRENG_LIMITEN);
      putreg32(1, se0 + STRENG_LIMIT);
      putreg32(0x01, se0 + STRENG_CID_CACHE);  /* CID=1, non-cacheable (SRAM) */
      ctrl = STRENG_CTRL_SINGLE | STRENG_CTRL_RAW | STRENG_CTRL_SIZE_8BIT;
      putreg32(ctrl, se0 + STRENG_CTRL);

      /* CLR + config SE1 (output) */

      putreg32(STRENG_CTRL_CLR, se1 + STRENG_CTRL);
      timeout = 10000;
      while ((getreg32(se1 + STRENG_CTRL) & STRENG_CTRL_CLR)
             && --timeout > 0);
      putreg32(0xff, se1 + STRENG_IRQ);
      putreg32((uintptr_t)dst, se1 + STRENG_ADDR);
      putreg32(NPU_MCPY_ELEMENTS, se1 + STRENG_FSIZE);
      putreg32(0, se1 + STRENG_STRD);
      putreg32(NPU_MCPY_BYTES, se1 + STRENG_FOFFSET);
      putreg32(0, se1 + STRENG_FRAME_RPT);
      putreg32(0, se1 + STRENG_FRPTOFF);
      putreg32(0x00000024, se1 + STRENG_POS);
      putreg32(0x04, se1 + STRENG_LIMITEN);
      putreg32(1, se1 + STRENG_LIMIT);
      putreg32(0x01, se1 + STRENG_CID_CACHE);  /* CID=1, non-cacheable (SRAM) */
      ctrl = STRENG_CTRL_SINGLE | STRENG_CTRL_DIR | STRENG_CTRL_RAW
             | STRENG_CTRL_SIZE_8BIT;
      putreg32(ctrl, se1 + STRENG_CTRL);

      /* STRSWITCH: enable + route SE0→SE1 */

      putreg32(STRSWITCH_CTRL_EN, strswitch + STRSWITCH_CTRL);
      putreg32(0x01, strswitch + STRSWITCH_DST(1));

      /* Enable SE1 then SE0 */

      ctrl = getreg32(se1 + STRENG_CTRL);
      putreg32(ctrl | STRENG_CTRL_EN, se1 + STRENG_CTRL);
      ctrl = getreg32(se0 + STRENG_CTRL);
      putreg32(ctrl | STRENG_CTRL_EN, se0 + STRENG_CTRL);

      /* Poll SE1 RUNNING with timeout */

      timeout = 1000000;
      while ((getreg32(se1 + STRENG_CTRL) & STRENG_CTRL_RUNNING)
             && --timeout > 0);

      /* Invalidate CPU D-cache so we read fresh data from SRAM.
       * CACHEAXI flush is not needed here because SRAM transfers use
       * non-cacheable mode (CID_CACHE=0x01).  CACHEAXI caching
       * (CID_CACHE=0x19) is for XSPI flash reads (AI model weights).
       */

      up_invalidate_dcache((uintptr_t)dst,
                           (uintptr_t)dst + NPU_MCPY_BYTES);

      if (timeout <= 0)
        {
          syslog(LOG_ERR, "NPU STRENG memcpy: TIMEOUT "
                 "SE0=0x%lx SE1=0x%lx\n",
                 (unsigned long)getreg32(se0 + STRENG_CTRL),
                 (unsigned long)getreg32(se1 + STRENG_CTRL));
        }
      else if (memcmp(src, dst, NPU_MCPY_BYTES) != 0)
        {
          syslog(LOG_ERR, "NPU STRENG memcpy: %d bytes MISMATCH "
                 "dst[0..3]=%02x %02x %02x %02x\n",
                 NPU_MCPY_BYTES,
                 dst[0], dst[1], dst[2], dst[3]);
        }
      else
        {
          syslog(LOG_INFO, "NPU STRENG memcpy: %d bytes PASS\n",
                 NPU_MCPY_BYTES);
        }

    }
  }
#endif  /* CONFIG_STM32N6_NPU */

  /* Turn on user LEDs: LD6 (green/PG0) and LD7 (blue/PG8) work in DEV mode.
   * LD5 (red/PG10) does not light in DEV mode despite correct GPIO config
   * (confirmed: MODER=output, ODR=LOW, IDR=LOW).  Likely ST-LINK V3EC
   * interference via SWD on PG10 in DEV boot mode.
   */

  stm32_gpiowrite(GPIO_LD6, true);
  stm32_gpiowrite(GPIO_LD7, true);

#ifdef CONFIG_FS_PROCFS
  /* Mount the procfs file system */

  ret = mount(NULL, "/proc", "procfs", 0, NULL);
  if (ret < 0)
    {
      ferr("ERROR: Failed to mount procfs at /proc: %d\n", ret);
    }
#endif

#ifdef CONFIG_RTC_DRIVER
  /* Register /dev/rtc0 */

  {
    struct rtc_lowerhalf_s *lower = stm32_rtc_lowerhalf();
    if (lower != NULL)
      {
        ret = rtc_initialize(0, lower);
        if (ret < 0)
          {
            ferr("ERROR: rtc_initialize failed: %d\n", ret);
          }
      }
  }
#endif

#ifdef CONFIG_STM32N6_ADC1
  /* Initialize ADC1 and register as /dev/adc0 */

  ret = stm32_adc_setup();
  if (ret < 0)
    {
      ferr("ERROR: stm32_adc_setup failed: %d\n", ret);
    }
#endif

#ifdef CONFIG_STM32N6_PWM
  /* Initialize PWM and register as /dev/pwm0 */

  ret = stm32_pwm_setup();
  if (ret < 0)
    {
      ferr("ERROR: stm32_pwm_setup failed: %d\n", ret);
    }
#endif

#ifdef CONFIG_STM32N6_IWDG
  stm32_iwdginitialize("/dev/watchdog0", STM32_LSI_FREQUENCY);
#endif

#ifdef CONFIG_STM32N6_SPI5
  /* Initialize SPI5 and register as /dev/spi5 for the spi tool */

  {
    struct spi_dev_s *spi5 = stm32_spibus_initialize(5);
    if (spi5 != NULL)
      {
        ret = spi_register(spi5, 5);
        if (ret < 0)
          {
            ferr("ERROR: Failed to register /dev/spi5: %d\n", ret);
          }
      }
  }
#endif


#ifdef CONFIG_STM32N6_I2C1
  /* Initialize I2C1 and register as /dev/i2c1 for the i2c tool.
   * I2C1: PH9=SCL, PC1=SDA — morpho CN15 pin3(SCL), pin5(SDA).
   */

  {
    struct i2c_master_s *i2c1 = stm32_i2cbus_initialize(1);
    if (i2c1 != NULL)
      {
        ret = i2c_register(i2c1, 1);
        if (ret < 0)
          {
            ferr("ERROR: Failed to register /dev/i2c1: %d\n", ret);
          }
      }
  }
#endif

#ifdef CONFIG_STM32N6_I2C2
  /* Enable TCPP0203 USB Type-C PD controller on I2C2.
   * The chip has an enable pin on PA7 (push-pull, active HIGH).
   * Must be driven HIGH before I2C communication will succeed.
   */

  stm32_configgpio(GPIO_TCPP03_ENABLE);
  stm32_gpiowrite(GPIO_TCPP03_ENABLE, true);
  up_mdelay(10); /* TCPP03 needs time after enable */

  /* Initialize I2C2 and register as /dev/i2c2 for the i2c tool */

  {
    struct i2c_master_s *i2c2 = stm32_i2cbus_initialize(2);
    if (i2c2 != NULL)
      {
        ret = i2c_register(i2c2, 2);
        if (ret < 0)
          {
            ferr("ERROR: Failed to register /dev/i2c2: %d\n", ret);
          }

      }
  }

#endif

#ifdef CONFIG_STM32N6_XSPI2
  /* Initialize XSPI2 and register external flash */

  ret = stm32_xspi_setup();
  if (ret < 0)
    {
      ferr("ERROR: stm32_xspi_setup failed: %d\n", ret);
    }
#endif

#if defined(CONFIG_CDCACM) && !defined(CONFIG_CDCACM_CONSOLE)
  /* Register CDC/ACM serial device as /dev/ttyACM0 */

  ret = cdcacm_initialize(0, NULL);
  if (ret < 0)
    {
      ferr("ERROR: cdcacm_initialize failed: %d\n", ret);
    }
#endif

  UNUSED(ret);
  return OK;
}
