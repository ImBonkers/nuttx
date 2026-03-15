/****************************************************************************
 * arch/arm/src/stm32n6/stm32_rtc_lowerhalf.c
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

#include <sys/types.h>
#include <stdbool.h>
#include <errno.h>

#include <nuttx/arch.h>
#include <nuttx/mutex.h>
#include <nuttx/timers/rtc.h>

#include "arm_internal.h"
#include "chip.h"
#include "stm32_rtc.h"

#ifdef CONFIG_RTC_DRIVER

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct stm32_lowerhalf_s
{
  const struct rtc_ops_s *ops;
  mutex_t devlock;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int stm32_rdtime(struct rtc_lowerhalf_s *lower,
                        struct rtc_time *rtctime);
static int stm32_settime(struct rtc_lowerhalf_s *lower,
                         const struct rtc_time *rtctime);
static bool stm32_havesettime(struct rtc_lowerhalf_s *lower);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct rtc_ops_s g_rtc_ops =
{
  .rdtime      = stm32_rdtime,
  .settime     = stm32_settime,
  .havesettime = stm32_havesettime,
};

static struct stm32_lowerhalf_s g_rtc_lowerhalf =
{
  .ops     = &g_rtc_ops,
  .devlock = NXMUTEX_INITIALIZER,
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_rdtime
 ****************************************************************************/

static int stm32_rdtime(struct rtc_lowerhalf_s *lower,
                        struct rtc_time *rtctime)
{
  struct stm32_lowerhalf_s *priv;
  int ret;

  priv = (struct stm32_lowerhalf_s *)lower;

  ret = nxmutex_lock(&priv->devlock);
  if (ret < 0)
    {
      return ret;
    }

  ret = up_rtc_getdatetime((struct tm *)rtctime);

  nxmutex_unlock(&priv->devlock);
  return ret;
}

/****************************************************************************
 * Name: stm32_settime
 ****************************************************************************/

static int stm32_settime(struct rtc_lowerhalf_s *lower,
                         const struct rtc_time *rtctime)
{
  struct stm32_lowerhalf_s *priv;
  int ret;

  priv = (struct stm32_lowerhalf_s *)lower;

  ret = nxmutex_lock(&priv->devlock);
  if (ret < 0)
    {
      return ret;
    }

  ret = stm32_rtc_setdatetime((const struct tm *)rtctime);

  nxmutex_unlock(&priv->devlock);
  return ret;
}

/****************************************************************************
 * Name: stm32_havesettime
 ****************************************************************************/

static bool stm32_havesettime(struct rtc_lowerhalf_s *lower)
{
  return stm32_rtc_havesettime();
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_rtc_lowerhalf
 *
 * Description:
 *   Instantiate the RTC lower half driver for the STM32.
 *
 ****************************************************************************/

struct rtc_lowerhalf_s *stm32_rtc_lowerhalf(void)
{
  return (struct rtc_lowerhalf_s *)&g_rtc_lowerhalf;
}

#endif /* CONFIG_RTC_DRIVER */
