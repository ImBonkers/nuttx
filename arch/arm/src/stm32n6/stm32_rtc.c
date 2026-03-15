/****************************************************************************
 * arch/arm/src/stm32n6/stm32_rtc.c
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

#include <stdbool.h>
#include <sched.h>
#include <assert.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/irq.h>
#include <nuttx/clock.h>

#include "arm_internal.h"
#include "stm32_rcc.h"
#include "stm32_pwr.h"
#include "stm32_rtc.h"

#include <arch/board/board.h>

#ifdef CONFIG_STM32N6_RTC

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_RTC_DATETIME
#  error "CONFIG_RTC_DATETIME must be set to use this driver"
#endif

#ifdef CONFIG_RTC_HIRES
#  error "CONFIG_RTC_HIRES must NOT be set with this driver"
#endif

/* Constants ****************************************************************/

#define SYNCHRO_TIMEOUT      (0x00020000)
#define INITMODE_TIMEOUT     (0x00010000)

/* RTC clock source selection for CCIPR7 register.
 *
 * On STM32N6, the RTC clock source is selected via RCC_CCIPR7[1:0]:
 *   00 = No clock
 *   01 = LSE
 *   10 = LSI
 *   11 = HSE/64
 */

#if defined(CONFIG_STM32N6_RTC_HSECLOCK)
#  define RTC_CLKSRC  RCC_CCIPR7_RTCSEL_HSE
#elif defined(CONFIG_STM32N6_RTC_LSICLOCK)
#  define RTC_CLKSRC  RCC_CCIPR7_RTCSEL_LSI
#elif defined(CONFIG_STM32N6_RTC_LSECLOCK)
#  define RTC_CLKSRC  RCC_CCIPR7_RTCSEL_LSE
#else
#  warning "No RTC clock source selected - RTC will not count"
#  define RTC_CLKSRC  RCC_CCIPR7_RTCSEL_NONE
#endif

/* Time conversions */

#define MINUTES_IN_HOUR 60
#define HOURS_IN_DAY 24

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

/* g_rtc_enabled is set true after the RTC has successfully initialized */

volatile bool g_rtc_enabled = false;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: rtc_dumpregs
 ****************************************************************************/

#ifdef CONFIG_DEBUG_RTC_INFO
static void rtc_dumpregs(const char *msg)
{
  rtcinfo("%s:\n", msg);
  rtcinfo("      TR: %08x\n", getreg32(STM32_RTC_TR));
  rtcinfo("      DR: %08x\n", getreg32(STM32_RTC_DR));
  rtcinfo("      CR: %08x\n", getreg32(STM32_RTC_CR));
  rtcinfo("    ICSR: %08x\n", getreg32(STM32_RTC_ICSR));
  rtcinfo("    PRER: %08x\n", getreg32(STM32_RTC_PRER));
  rtcinfo("    WUTR: %08x\n", getreg32(STM32_RTC_WUTR));
  rtcinfo("  ALRMAR: %08x\n", getreg32(STM32_RTC_ALRMAR));
  rtcinfo("  ALRMBR: %08x\n", getreg32(STM32_RTC_ALRMBR));
  rtcinfo("      SR: %08x\n", getreg32(STM32_RTC_SR));
  rtcinfo("MAGICREG: %08x\n", getreg32(RTC_MAGIC_REG));
}
#else
#  define rtc_dumpregs(msg)
#endif

/****************************************************************************
 * Name: rtc_dumptime
 ****************************************************************************/

#ifdef CONFIG_DEBUG_RTC_INFO
static void rtc_dumptime(const struct tm *tp, const char *msg)
{
  rtcinfo("%s:\n", msg);
  rtcinfo("  tm_sec: %08x\n", tp->tm_sec);
  rtcinfo("  tm_min: %08x\n", tp->tm_min);
  rtcinfo(" tm_hour: %08x\n", tp->tm_hour);
  rtcinfo(" tm_mday: %08x\n", tp->tm_mday);
  rtcinfo("  tm_mon: %08x\n", tp->tm_mon);
  rtcinfo(" tm_year: %08x\n", tp->tm_year);
}
#else
#  define rtc_dumptime(tp, msg)
#endif

/****************************************************************************
 * Name: rtc_wprunlock
 *
 * Description:
 *    Disable RTC write protection
 *
 ****************************************************************************/

static void rtc_wprunlock(void)
{
  /* Enable write access to the backup domain (RTC registers, RTC backup
   * data registers and backup SRAM).
   */

  stm32_pwr_enablebkp(true);

  /* The following steps are required to unlock the write protection on all
   * the RTC registers (except for RTC_ICSR[13:8], RTC_TAFCR, and
   * RTC_BKPxR).
   *
   * 1. Write 0xCA into the RTC_WPR register.
   * 2. Write 0x53 into the RTC_WPR register.
   *
   * Writing a wrong key re-activates the write protection.
   */

  putreg32(0xca, STM32_RTC_WPR);
  putreg32(0x53, STM32_RTC_WPR);
}

/****************************************************************************
 * Name: rtc_wprlock
 *
 * Description:
 *    Enable RTC write protection
 *
 ****************************************************************************/

static inline void rtc_wprlock(void)
{
  /* Writing any wrong key re-activates the write protection. */

  putreg32(0xff, STM32_RTC_WPR);

  /* Disable write access to the backup domain (RTC registers, RTC backup
   * data registers and backup SRAM).
   */

  stm32_pwr_enablebkp(false);
}

/****************************************************************************
 * Name: rtc_synchwait
 *
 * Description:
 *   Waits until the RTC Time and Date registers (RTC_TR and RTC_DR) are
 *   synchronized with RTC APB clock.
 *
 ****************************************************************************/

static int rtc_synchwait(void)
{
  volatile uint32_t timeout;
  uint32_t regval;
  int ret;

  /* Disable the write protection for RTC registers */

  rtc_wprunlock();

  /* Clear Registers synchronization flag (RSF) in ICSR */

  regval  = getreg32(STM32_RTC_ICSR);
  regval &= ~RTC_ICSR_RSF;
  putreg32(regval, STM32_RTC_ICSR);

  /* Now wait the registers to become synchronised */

  ret = -ETIMEDOUT;
  for (timeout = 0; timeout < SYNCHRO_TIMEOUT; timeout++)
    {
      regval = getreg32(STM32_RTC_ICSR);
      if ((regval & RTC_ICSR_RSF) != 0)
        {
          /* Synchronized */

          ret = OK;
          break;
        }
    }

  /* Re-enable the write protection for RTC registers */

  rtc_wprlock();
  return ret;
}

/****************************************************************************
 * Name: rtc_enterinit
 *
 * Description:
 *   Enter RTC initialization mode.
 *
 ****************************************************************************/

static int rtc_enterinit(void)
{
  volatile uint32_t timeout;
  uint32_t regval;
  int ret;

  /* Check if the Initialization mode is already set */

  regval = getreg32(STM32_RTC_ICSR);

  ret = OK;
  if ((regval & RTC_ICSR_INITF) == 0)
    {
      /* Set the Initialization mode */

      putreg32(RTC_ICSR_INIT, STM32_RTC_ICSR);

      /* Wait until the RTC is in the INIT state (or a timeout occurs) */

      ret = -ETIMEDOUT;
      for (timeout = 0; timeout < INITMODE_TIMEOUT; timeout++)
        {
          regval = getreg32(STM32_RTC_ICSR);
          if ((regval & RTC_ICSR_INITF) != 0)
            {
              ret = OK;
              break;
            }
        }
    }

  return ret;
}

/****************************************************************************
 * Name: rtc_exitinit
 *
 * Description:
 *   Exit RTC initialization mode.
 *
 ****************************************************************************/

static void rtc_exitinit(void)
{
  uint32_t regval;

  regval = getreg32(STM32_RTC_ICSR);
  regval &= ~(RTC_ICSR_INIT);
  putreg32(regval, STM32_RTC_ICSR);
}

/****************************************************************************
 * Name: rtc_bin2bcd
 *
 * Description:
 *   Converts a 2 digit binary to BCD format
 *
 ****************************************************************************/

static uint32_t rtc_bin2bcd(int value)
{
  uint32_t msbcd = 0;

  while (value >= 10)
    {
      msbcd++;
      value -= 10;
    }

  return (msbcd << 4) | value;
}

/****************************************************************************
 * Name: rtc_bcd2bin
 *
 * Description:
 *   Convert from 2 digit BCD to binary.
 *
 ****************************************************************************/

static int rtc_bcd2bin(uint32_t value)
{
  uint32_t tens = (value >> 4) * 10;
  return (int)(tens + (value & 0x0f));
}

/****************************************************************************
 * Name: rtc_setup
 *
 * Description:
 *   Performs first time configuration of the RTC.  A special value written
 *   into back-up register 0 will prevent this function from being called on
 *   sub-sequent resets or power up.
 *
 ****************************************************************************/

static int rtc_setup(void)
{
  uint32_t regval;
  int ret;

  /* Disable the write protection for RTC registers */

  rtc_wprunlock();

  /* Set Initialization mode */

  ret = rtc_enterinit();
  if (ret == OK)
    {
      /* Set the 24 hour format by clearing the FMT bit in the RTC
       * control register
       */

      regval = getreg32(STM32_RTC_CR);
      regval &= ~RTC_CR_FMT;
      putreg32(regval, STM32_RTC_CR);

      /* Configure RTC pre-scaler with the required values */

#ifdef CONFIG_STM32N6_RTC_HSECLOCK
      /* For HSE/64 at 48MHz: HSE/64 = 750kHz
       * PREDIV_A=124, PREDIV_S=5999 -> 1 Hz
       * N6 RTC requires two writes: PREDIV_S first, then PREDIV_A.
       */

      putreg32((uint32_t)5999 << RTC_PRER_PREDIV_S_SHIFT,
              STM32_RTC_PRER);
      putreg32(((uint32_t)5999 << RTC_PRER_PREDIV_S_SHIFT) |
              ((uint32_t)124 << RTC_PRER_PREDIV_A_SHIFT),
              STM32_RTC_PRER);
#else
      /* Correct values for 32.768 KHz LSE clock and inaccurate LSI clock.
       * N6 RTC requires two writes: PREDIV_S first, then PREDIV_A.
       */

      putreg32((uint32_t)0xff << RTC_PRER_PREDIV_S_SHIFT,
              STM32_RTC_PRER);
      putreg32(((uint32_t)0xff << RTC_PRER_PREDIV_S_SHIFT) |
              ((uint32_t)0x7f << RTC_PRER_PREDIV_A_SHIFT),
              STM32_RTC_PRER);
#endif

      /* Exit RTC initialization mode */

      rtc_exitinit();
    }

  /* Re-enable the write protection for RTC registers */

  rtc_wprlock();

  return ret;
}

/****************************************************************************
 * Name: rtc_resume
 *
 * Description:
 *   Called when the RTC was already initialized on a previous power cycle.
 *   This just brings the RTC back into full operation.
 *
 ****************************************************************************/

static void rtc_resume(void)
{
  /* On N6, flags are cleared via SCR (write-1-to-clear), not by
   * read-modify-write on ISR like H7.
   */

  putreg32(RTC_SCR_CALRAF | RTC_SCR_CALRBF, STM32_RTC_SCR);
}

/****************************************************************************
 * Name: stm32_rtc_enablelse
 *
 * Description:
 *   Enable the LSE oscillator via RCC_CR.LSEON and wait for LSERDY.
 *   On STM32N6, LSE is controlled via RCC_CR (not RCC_BDCR like H7).
 *
 ****************************************************************************/

#if defined(CONFIG_STM32N6_RTC_LSECLOCK)
static void stm32_rtc_enablelse(void)
{
  volatile uint32_t timeout;
  uint32_t regval;

  /* Enable LSE via atomic SET register */

  putreg32(RCC_CR_LSEON, STM32_RCC_CSR);

  /* Wait for LSE to become ready */

  for (timeout = 0; timeout < 0x00100000; timeout++)
    {
      regval = getreg32(STM32_RCC_SR);
      if ((regval & RCC_SR_LSERDY) != 0)
        {
          break;
        }
    }
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_rtc_initialize
 *
 * Description:
 *   Initialize the hardware RTC per the selected configuration.  This
 *   function is called once during the OS initialization sequence
 *
 ****************************************************************************/

int up_rtc_initialize(void)
{
  uint32_t regval;
  int ret;
  int maxretry = 10;
  int nretry = 0;

  /* Enable the RTC APB interface clock.  On STM32N6 the RTC APB clock
   * enable is RTCAPBEN (bit 17) in APB4ENR1, not in BDCR like STM32H7.
   */

  putreg32(RCC_APB4ENR1_RTCAPBEN, STM32_RCC_APB4ENSR);

  /* Save the token before losing it when resetting */

  regval = getreg32(RTC_MAGIC_REG);

  /* Enable write access to the backup domain (RTC registers, RTC
   * backup data registers and backup SRAM).
   */

  stm32_pwr_enablebkp(true);

  if (regval != RTC_MAGIC && regval != RTC_MAGIC_TIME_SET)
    {
      /* First time initialization.  On STM32N6, backup domain reset is
       * via RCC_BDCR.VSWRST (bit 31, offset 0x002C).
       */

      modifyreg32(STM32_RCC_BDCR, 0, RCC_BDCR_VSWRST);
      modifyreg32(STM32_RCC_BDCR, RCC_BDCR_VSWRST, 0);

#if defined(CONFIG_STM32N6_RTC_LSECLOCK)
      /* Re-enable LSE after backup domain reset */

      stm32_rtc_enablelse();
#endif

      rtc_dumpregs("On reset");

      /* Select the RTC clock source via CCIPR7 */

      modifyreg32(STM32_RCC_CCIPR7, RCC_CCIPR7_RTCSEL_MASK, RTC_CLKSRC);
    }
  else
    {
      uint32_t clksrc = getreg32(STM32_RCC_CCIPR7);

      rtc_dumpregs("On reset warm");

      /* The RTC is already in use: check if the clock source has changed */

      if ((clksrc & RCC_CCIPR7_RTCSEL_MASK) != RTC_CLKSRC)
        {
          /* Backup domain reset to switch clock source */

          modifyreg32(STM32_RCC_BDCR, 0, RCC_BDCR_VSWRST);
          modifyreg32(STM32_RCC_BDCR, RCC_BDCR_VSWRST, 0);

#if defined(CONFIG_STM32N6_RTC_LSECLOCK)
          stm32_rtc_enablelse();
#endif

          /* Change to the new clock source */

          modifyreg32(STM32_RCC_CCIPR7, RCC_CCIPR7_RTCSEL_MASK,
                      RTC_CLKSRC);

          /* Keep the fact that the RTC is initialized */

          putreg32(RTC_MAGIC, RTC_MAGIC_REG);
        }
    }

  /* Loop, attempting to initialize/resume the RTC.  This loop is necessary
   * because it seems that occasionally it takes longer to initialize the RTC
   * (the actual failure is in rtc_synchwait()).
   */

  do
    {
      /* Wait for the RTC Time and Date registers to be synchronized with
       * RTC APB clock.
       */

      ret = rtc_synchwait();

      /* Check that rtc_syncwait() returned successfully */

      switch (ret)
        {
          case OK:
            {
              rtcinfo("rtc_syncwait() okay\n");
              break;
            }

          default:
            {
              rtcerr("ERROR: rtc_syncwait() failed (%d)\n", ret);
              break;
            }
        }
    }
  while (ret != OK && ++nretry < maxretry);

  /* Check if the one-time initialization of the RTC has already been
   * performed. We can determine this by checking if the magic number
   * has been written to back-up date register DR0.
   */

  if (regval != RTC_MAGIC && regval != RTC_MAGIC_TIME_SET)
    {
      rtcinfo("Do setup\n");

      /* Perform the one-time setup of the LSE clocking to the RTC */

      ret = rtc_setup();

      /* Remember that the RTC is initialized */

      putreg32(RTC_MAGIC, RTC_MAGIC_REG);
    }
  else
    {
      rtcinfo("Do resume\n");

      /* RTC already set-up, just resume normal operation */

      rtc_resume();
      rtc_dumpregs("Did resume");
    }

  /* Disable write access to the backup domain (RTC registers, RTC backup
   * data registers and backup SRAM).
   */

  stm32_pwr_enablebkp(false);

  if (ret != OK && nretry > 0)
    {
      rtcinfo("setup/resume ran %d times and failed with %d\n",
              nretry, ret);
      return -ETIMEDOUT;
    }

  g_rtc_enabled = true;
  rtc_dumpregs("After Initialization");
  return OK;
}

/****************************************************************************
 * Name: up_rtc_getdatetime
 *
 * Description:
 *   Get the current date and time from the date/time RTC.  This interface
 *   is only supported by the date/time RTC hardware implementation.
 *   It is used to replace the system timer.
 *
 ****************************************************************************/

int up_rtc_getdatetime(struct tm *tp)
{
  uint32_t dr;
  uint32_t tr;
  uint32_t tmp;

  /* Sample the data time registers.  There is a race condition here... If
   * we sample the time just before midnight on December 31, the date could
   * be wrong because the day rolled over while we were sampling.
   */

  do
    {
      dr  = getreg32(STM32_RTC_DR);
      tr  = getreg32(STM32_RTC_TR);
      tmp = getreg32(STM32_RTC_DR);
      if (tmp == dr)
        {
          break;
        }
    }
  while (1);

  rtc_dumpregs("Reading Time");

  /* Convert the RTC time to fields in struct tm format. All of the STM32
   * ranges of values correspond between struct tm and the time register.
   */

  tmp = (tr & (RTC_TR_SU_MASK | RTC_TR_ST_MASK)) >> RTC_TR_SU_SHIFT;
  tp->tm_sec = rtc_bcd2bin(tmp);

  tmp = (tr & (RTC_TR_MNU_MASK | RTC_TR_MNT_MASK)) >> RTC_TR_MNU_SHIFT;
  tp->tm_min = rtc_bcd2bin(tmp);

  tmp = (tr & (RTC_TR_HU_MASK | RTC_TR_HT_MASK)) >> RTC_TR_HU_SHIFT;
  tp->tm_hour = rtc_bcd2bin(tmp);

  /* Now convert the RTC date to fields in struct tm format:
   * Days: 1-31 match in both cases.
   * Month: STM32 is 1-12, struct tm is 0-11.
   * Years: STM32 is 00-99, struct tm is years since 1900.
   * WeekDay: STM32 is 1 = Mon - 7 = Sun
   */

  tmp = (dr & (RTC_DR_DU_MASK | RTC_DR_DT_MASK)) >> RTC_DR_DU_SHIFT;
  tp->tm_mday = rtc_bcd2bin(tmp);

  tmp = (dr & (RTC_DR_MU_MASK | RTC_DR_MT)) >> RTC_DR_MU_SHIFT;
  tp->tm_mon = rtc_bcd2bin(tmp) - 1;

  tmp = (dr & (RTC_DR_YU_MASK | RTC_DR_YT_MASK)) >> RTC_DR_YU_SHIFT;
  tp->tm_year = rtc_bcd2bin(tmp) + 100;

  tmp = (dr & RTC_DR_WDU_MASK) >> RTC_DR_WDU_SHIFT;
  tp->tm_wday = tmp % 7;
  tp->tm_yday = tp->tm_mday - 1 +
    clock_daysbeforemonth(tp->tm_mon, clock_isleapyear(tp->tm_year + 1900));
  tp->tm_isdst = 0;

  rtc_dumptime((const struct tm *)tp, "Returning");

  return OK;
}

/****************************************************************************
 * Name: stm32_rtc_setdatetime
 *
 * Description:
 *   Set the RTC to the provided time. RTC implementations which provide
 *   up_rtc_getdatetime() (CONFIG_RTC_DATETIME is selected) should provide
 *   this function.
 *
 ****************************************************************************/

int stm32_rtc_setdatetime(const struct tm *tp)
{
  uint32_t tr;
  uint32_t dr;
  int ret;

  rtc_dumptime(tp, "Setting time");

  /* Convert the struct tm format to RTC time register fields. */

  tr = (rtc_bin2bcd(tp->tm_sec)  << RTC_TR_SU_SHIFT) |
       (rtc_bin2bcd(tp->tm_min)  << RTC_TR_MNU_SHIFT) |
       (rtc_bin2bcd(tp->tm_hour) << RTC_TR_HU_SHIFT);
  tr &= ~RTC_TR_RESERVED_BITS;

  /* Now convert the fields in struct tm format to the RTC date register
   * fields:
   *
   *   Days: 1-31 match in both cases.
   *   Month: STM32 is 1-12, struct tm is 0-11.
   *   Years: STM32 is 00-99, struct tm is years since 1900.
   *   WeekDay: STM32 is 1 = Mon - 7 = Sun
   */

  dr = (rtc_bin2bcd(tp->tm_mday) << RTC_DR_DU_SHIFT) |
       ((rtc_bin2bcd(tp->tm_mon + 1))  << RTC_DR_MU_SHIFT) |
       ((tp->tm_wday == 0 ? 7 : (tp->tm_wday & 7))  << RTC_DR_WDU_SHIFT) |
       ((rtc_bin2bcd(tp->tm_year - 100)) << RTC_DR_YU_SHIFT);

  dr &= ~RTC_DR_RESERVED_BITS;

  /* Disable the write protection for RTC registers */

  rtc_wprunlock();

  /* Set Initialization mode */

  ret = rtc_enterinit();
  if (ret == OK)
    {
      /* Set the RTC TR and DR registers */

      putreg32(tr, STM32_RTC_TR);
      putreg32(dr, STM32_RTC_DR);

      /* Exit Initialization mode and wait for the RTC Time and Date
       * registers to be synchronized with RTC APB clock.
       */

      rtc_exitinit();
      ret = rtc_synchwait();
    }

  /* Remember that the RTC is initialized and had its time set. */

  if (getreg32(RTC_MAGIC_REG) != RTC_MAGIC_TIME_SET)
    {
      stm32_pwr_enablebkp(true);
      putreg32(RTC_MAGIC_TIME_SET, RTC_MAGIC_REG);
      stm32_pwr_enablebkp(false);
    }

  /* Re-enable the write protection for RTC registers */

  rtc_wprlock();
  rtc_dumpregs("New time setting");
  return ret;
}

/****************************************************************************
 * Name: stm32_rtc_havesettime
 *
 * Description:
 *   Check if RTC time has been set.
 *
 ****************************************************************************/

bool stm32_rtc_havesettime(void)
{
  return getreg32(RTC_MAGIC_REG) == RTC_MAGIC_TIME_SET;
}

/****************************************************************************
 * Name: up_rtc_settime
 *
 * Description:
 *   Set the RTC to the provided time.  All RTC implementations must be able
 *   to set their time based on a standard timespec.
 *
 ****************************************************************************/

int up_rtc_settime(const struct timespec *tp)
{
  struct tm newtime;

  /* Break out the time values (note that the time is set only to units of
   * seconds)
   */

  gmtime_r(&tp->tv_sec, &newtime);
  return stm32_rtc_setdatetime(&newtime);
}

#endif /* CONFIG_STM32N6_RTC */
