/****************************************************************************
 * arch/arm/src/stm32n6/hardware/stm32n6xxx_rtc.h
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

#ifndef __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_RTC_H
#define __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_RTC_H

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Register Offsets *********************************************************/

#define STM32_RTC_TR_OFFSET       0x0000 /* RTC time register */
#define STM32_RTC_DR_OFFSET       0x0004 /* RTC date register */
#define STM32_RTC_SSR_OFFSET      0x0008 /* RTC sub second register */
#define STM32_RTC_ICSR_OFFSET     0x000c /* RTC initialization control and status register */
#define STM32_RTC_PRER_OFFSET     0x0010 /* RTC prescaler register */
#define STM32_RTC_WUTR_OFFSET     0x0014 /* RTC wakeup timer register */
#define STM32_RTC_CR_OFFSET       0x0018 /* RTC control register */
#define STM32_RTC_WPR_OFFSET      0x0024 /* RTC write protection register */
#define STM32_RTC_CALR_OFFSET     0x0028 /* RTC calibration register */
#define STM32_RTC_SHIFTR_OFFSET   0x002c /* RTC shift control register */
#define STM32_RTC_TSTR_OFFSET     0x0030 /* RTC time stamp time register */
#define STM32_RTC_TSDR_OFFSET     0x0034 /* RTC time stamp date register */
#define STM32_RTC_TSSSR_OFFSET    0x0038 /* RTC timestamp sub second register */
#define STM32_RTC_ALRMAR_OFFSET   0x0040 /* RTC alarm A register */
#define STM32_RTC_ALRMASSR_OFFSET 0x0044 /* RTC alarm A sub second register */
#define STM32_RTC_ALRMBR_OFFSET   0x0048 /* RTC alarm B register */
#define STM32_RTC_ALRMBSSR_OFFSET 0x004c /* RTC alarm B sub second register */
#define STM32_RTC_SR_OFFSET       0x0050 /* RTC status register */
#define STM32_RTC_MISR_OFFSET     0x0054 /* RTC masked interrupt status register */
#define STM32_RTC_SCR_OFFSET      0x005c /* RTC status clear register */

#define STM32_RTC_BKR_OFFSET(n)   (0x0100+((n)<<2))
#define STM32_RTC_BK0R_OFFSET     0x0100 /* RTC backup register 0 */
#define STM32_RTC_BK1R_OFFSET     0x0104 /* RTC backup register 1 */
#define STM32_RTC_BK2R_OFFSET     0x0108 /* RTC backup register 2 */
#define STM32_RTC_BK3R_OFFSET     0x010c /* RTC backup register 3 */
#define STM32_RTC_BK4R_OFFSET     0x0110 /* RTC backup register 4 */
#define STM32_RTC_BK5R_OFFSET     0x0114 /* RTC backup register 5 */
#define STM32_RTC_BK6R_OFFSET     0x0118 /* RTC backup register 6 */
#define STM32_RTC_BK7R_OFFSET     0x011c /* RTC backup register 7 */
#define STM32_RTC_BK8R_OFFSET     0x0120 /* RTC backup register 8 */
#define STM32_RTC_BK9R_OFFSET     0x0124 /* RTC backup register 9 */
#define STM32_RTC_BK10R_OFFSET    0x0128 /* RTC backup register 10 */
#define STM32_RTC_BK11R_OFFSET    0x012c /* RTC backup register 11 */
#define STM32_RTC_BK12R_OFFSET    0x0130 /* RTC backup register 12 */
#define STM32_RTC_BK13R_OFFSET    0x0134 /* RTC backup register 13 */
#define STM32_RTC_BK14R_OFFSET    0x0138 /* RTC backup register 14 */
#define STM32_RTC_BK15R_OFFSET    0x013c /* RTC backup register 15 */
#define STM32_RTC_BK16R_OFFSET    0x0140 /* RTC backup register 16 */
#define STM32_RTC_BK17R_OFFSET    0x0144 /* RTC backup register 17 */
#define STM32_RTC_BK18R_OFFSET    0x0148 /* RTC backup register 18 */
#define STM32_RTC_BK19R_OFFSET    0x014c /* RTC backup register 19 */
#define STM32_RTC_BK20R_OFFSET    0x0150 /* RTC backup register 20 */
#define STM32_RTC_BK21R_OFFSET    0x0154 /* RTC backup register 21 */
#define STM32_RTC_BK22R_OFFSET    0x0158 /* RTC backup register 22 */
#define STM32_RTC_BK23R_OFFSET    0x015c /* RTC backup register 23 */
#define STM32_RTC_BK24R_OFFSET    0x0160 /* RTC backup register 24 */
#define STM32_RTC_BK25R_OFFSET    0x0164 /* RTC backup register 25 */
#define STM32_RTC_BK26R_OFFSET    0x0168 /* RTC backup register 26 */
#define STM32_RTC_BK27R_OFFSET    0x016c /* RTC backup register 27 */
#define STM32_RTC_BK28R_OFFSET    0x0170 /* RTC backup register 28 */
#define STM32_RTC_BK29R_OFFSET    0x0174 /* RTC backup register 29 */
#define STM32_RTC_BK30R_OFFSET    0x0178 /* RTC backup register 30 */
#define STM32_RTC_BK31R_OFFSET    0x017c /* RTC backup register 31 */

/* Register Addresses *******************************************************/

#define STM32_RTC_TR              (STM32_RTC_BASE+STM32_RTC_TR_OFFSET)
#define STM32_RTC_DR              (STM32_RTC_BASE+STM32_RTC_DR_OFFSET)
#define STM32_RTC_SSR             (STM32_RTC_BASE+STM32_RTC_SSR_OFFSET)
#define STM32_RTC_ICSR            (STM32_RTC_BASE+STM32_RTC_ICSR_OFFSET)
#define STM32_RTC_PRER            (STM32_RTC_BASE+STM32_RTC_PRER_OFFSET)
#define STM32_RTC_WUTR            (STM32_RTC_BASE+STM32_RTC_WUTR_OFFSET)
#define STM32_RTC_CR              (STM32_RTC_BASE+STM32_RTC_CR_OFFSET)
#define STM32_RTC_WPR             (STM32_RTC_BASE+STM32_RTC_WPR_OFFSET)
#define STM32_RTC_CALR            (STM32_RTC_BASE+STM32_RTC_CALR_OFFSET)
#define STM32_RTC_SHIFTR          (STM32_RTC_BASE+STM32_RTC_SHIFTR_OFFSET)
#define STM32_RTC_TSTR            (STM32_RTC_BASE+STM32_RTC_TSTR_OFFSET)
#define STM32_RTC_TSDR            (STM32_RTC_BASE+STM32_RTC_TSDR_OFFSET)
#define STM32_RTC_TSSSR           (STM32_RTC_BASE+STM32_RTC_TSSSR_OFFSET)
#define STM32_RTC_ALRMAR          (STM32_RTC_BASE+STM32_RTC_ALRMAR_OFFSET)
#define STM32_RTC_ALRMASSR        (STM32_RTC_BASE+STM32_RTC_ALRMASSR_OFFSET)
#define STM32_RTC_ALRMBR          (STM32_RTC_BASE+STM32_RTC_ALRMBR_OFFSET)
#define STM32_RTC_ALRMBSSR        (STM32_RTC_BASE+STM32_RTC_ALRMBSSR_OFFSET)
#define STM32_RTC_SR              (STM32_RTC_BASE+STM32_RTC_SR_OFFSET)
#define STM32_RTC_MISR            (STM32_RTC_BASE+STM32_RTC_MISR_OFFSET)
#define STM32_RTC_SCR             (STM32_RTC_BASE+STM32_RTC_SCR_OFFSET)

#define STM32_RTC_BKR(n)          (STM32_RTC_BASE+STM32_RTC_BKR_OFFSET(n))
#define STM32_RTC_BK0R            (STM32_RTC_BASE+STM32_RTC_BK0R_OFFSET)
#define STM32_RTC_BK1R            (STM32_RTC_BASE+STM32_RTC_BK1R_OFFSET)
#define STM32_RTC_BK2R            (STM32_RTC_BASE+STM32_RTC_BK2R_OFFSET)
#define STM32_RTC_BK3R            (STM32_RTC_BASE+STM32_RTC_BK3R_OFFSET)
#define STM32_RTC_BK4R            (STM32_RTC_BASE+STM32_RTC_BK4R_OFFSET)
#define STM32_RTC_BK5R            (STM32_RTC_BASE+STM32_RTC_BK5R_OFFSET)
#define STM32_RTC_BK6R            (STM32_RTC_BASE+STM32_RTC_BK6R_OFFSET)
#define STM32_RTC_BK7R            (STM32_RTC_BASE+STM32_RTC_BK7R_OFFSET)
#define STM32_RTC_BK8R            (STM32_RTC_BASE+STM32_RTC_BK8R_OFFSET)
#define STM32_RTC_BK9R            (STM32_RTC_BASE+STM32_RTC_BK9R_OFFSET)
#define STM32_RTC_BK10R           (STM32_RTC_BASE+STM32_RTC_BK10R_OFFSET)
#define STM32_RTC_BK11R           (STM32_RTC_BASE+STM32_RTC_BK11R_OFFSET)
#define STM32_RTC_BK12R           (STM32_RTC_BASE+STM32_RTC_BK12R_OFFSET)
#define STM32_RTC_BK13R           (STM32_RTC_BASE+STM32_RTC_BK13R_OFFSET)
#define STM32_RTC_BK14R           (STM32_RTC_BASE+STM32_RTC_BK14R_OFFSET)
#define STM32_RTC_BK15R           (STM32_RTC_BASE+STM32_RTC_BK15R_OFFSET)
#define STM32_RTC_BK16R           (STM32_RTC_BASE+STM32_RTC_BK16R_OFFSET)
#define STM32_RTC_BK17R           (STM32_RTC_BASE+STM32_RTC_BK17R_OFFSET)
#define STM32_RTC_BK18R           (STM32_RTC_BASE+STM32_RTC_BK18R_OFFSET)
#define STM32_RTC_BK19R           (STM32_RTC_BASE+STM32_RTC_BK19R_OFFSET)
#define STM32_RTC_BK20R           (STM32_RTC_BASE+STM32_RTC_BK20R_OFFSET)
#define STM32_RTC_BK21R           (STM32_RTC_BASE+STM32_RTC_BK21R_OFFSET)
#define STM32_RTC_BK22R           (STM32_RTC_BASE+STM32_RTC_BK22R_OFFSET)
#define STM32_RTC_BK23R           (STM32_RTC_BASE+STM32_RTC_BK23R_OFFSET)
#define STM32_RTC_BK24R           (STM32_RTC_BASE+STM32_RTC_BK24R_OFFSET)
#define STM32_RTC_BK25R           (STM32_RTC_BASE+STM32_RTC_BK25R_OFFSET)
#define STM32_RTC_BK26R           (STM32_RTC_BASE+STM32_RTC_BK26R_OFFSET)
#define STM32_RTC_BK27R           (STM32_RTC_BASE+STM32_RTC_BK27R_OFFSET)
#define STM32_RTC_BK28R           (STM32_RTC_BASE+STM32_RTC_BK28R_OFFSET)
#define STM32_RTC_BK29R           (STM32_RTC_BASE+STM32_RTC_BK29R_OFFSET)
#define STM32_RTC_BK30R           (STM32_RTC_BASE+STM32_RTC_BK30R_OFFSET)
#define STM32_RTC_BK31R           (STM32_RTC_BASE+STM32_RTC_BK31R_OFFSET)

#define STM32_RTC_BKCOUNT         32

/* Register Bitfield Definitions ********************************************/

/* RTC time register */

#define RTC_TR_SU_SHIFT           (0)       /* Bits 0-3: Second units in BCD format */
#define RTC_TR_SU_MASK            (15 << RTC_TR_SU_SHIFT)
#define RTC_TR_ST_SHIFT           (4)       /* Bits 4-6: Second tens in BCD format */
#define RTC_TR_ST_MASK            (7 << RTC_TR_ST_SHIFT)
#define RTC_TR_MNU_SHIFT          (8)       /* Bit 8-11: Minute units in BCD format */
#define RTC_TR_MNU_MASK           (15 << RTC_TR_MNU_SHIFT)
#define RTC_TR_MNT_SHIFT          (12)      /* Bits 12-14: Minute tens in BCD format */
#define RTC_TR_MNT_MASK           (7 << RTC_TR_MNT_SHIFT)
#define RTC_TR_HU_SHIFT           (16)      /* Bit 16-19: Hour units in BCD format */
#define RTC_TR_HU_MASK            (15 << RTC_TR_HU_SHIFT)
#define RTC_TR_HT_SHIFT           (20)      /* Bits 20-21: Hour tens in BCD format */
#define RTC_TR_HT_MASK            (3 << RTC_TR_HT_SHIFT)
#define RTC_TR_PM                 (1 << 22) /* Bit 22: AM/PM notation */
#define RTC_TR_RESERVED_BITS      (0xff808080)

/* RTC date register */

#define RTC_DR_DU_SHIFT           (0)       /* Bits 0-3: Date units in BCD format */
#define RTC_DR_DU_MASK            (15 << RTC_DR_DU_SHIFT)
#define RTC_DR_DT_SHIFT           (4)       /* Bits 4-5: Date tens in BCD format */
#define RTC_DR_DT_MASK            (3 << RTC_DR_DT_SHIFT)
#define RTC_DR_MU_SHIFT           (8)      /* Bits 8-11: Month units in BCD format */
#define RTC_DR_MU_MASK            (15 << RTC_DR_MU_SHIFT)
#define RTC_DR_MT                 (1 << 12) /* Bit 12: Month tens in BCD format */
#define RTC_DR_WDU_SHIFT          (13)      /* Bits 13-15: Week day units */
#define RTC_DR_WDU_MASK           (7 << RTC_DR_WDU_SHIFT)
#  define RTC_DR_WDU_MONDAY       (1 << RTC_DR_WDU_SHIFT)
#  define RTC_DR_WDU_TUESDAY      (2 << RTC_DR_WDU_SHIFT)
#  define RTC_DR_WDU_WEDNESDAY    (3 << RTC_DR_WDU_SHIFT)
#  define RTC_DR_WDU_THURSDAY     (4 << RTC_DR_WDU_SHIFT)
#  define RTC_DR_WDU_FRIDAY       (5 << RTC_DR_WDU_SHIFT)
#  define RTC_DR_WDU_SATURDAY     (6 << RTC_DR_WDU_SHIFT)
#  define RTC_DR_WDU_SUNDAY       (7 << RTC_DR_WDU_SHIFT)
#define RTC_DR_YU_SHIFT           (16)     /* Bits 16-19: Year units in BCD format */
#define RTC_DR_YU_MASK            (15 << RTC_DR_YU_SHIFT)
#define RTC_DR_YT_SHIFT           (20)     /* Bits 20-23: Year tens in BCD format */
#define RTC_DR_YT_MASK            (15 << RTC_DR_YT_SHIFT)
#define RTC_DR_RESERVED_BITS      (0xff0000c0)

/* RTC control register (at offset 0x18 on N6) */

#define RTC_CR_WUCKSEL_SHIFT      (0)      /* Bits 0-2: Wakeup clock selection */
#define RTC_CR_WUCKSEL_MASK       (7 << RTC_CR_WUCKSEL_SHIFT)
#  define RTC_CR_WUCKSEL_RTCDIV16  (0 << RTC_CR_WUCKSEL_SHIFT)
#  define RTC_CR_WUCKSEL_RTCDIV8   (1 << RTC_CR_WUCKSEL_SHIFT)
#  define RTC_CR_WUCKSEL_RTCDIV4   (2 << RTC_CR_WUCKSEL_SHIFT)
#  define RTC_CR_WUCKSEL_RTCDIV2   (3 << RTC_CR_WUCKSEL_SHIFT)
#  define RTC_CR_WUCKSEL_CKSPRE    (4 << RTC_CR_WUCKSEL_SHIFT)
#  define RTC_CR_WUCKSEL_CKSPREADD (6 << RTC_CR_WUCKSEL_SHIFT)

#define RTC_CR_TSEDGE             (1 << 3)  /* Bit 3:  Timestamp event active edge */
#define RTC_CR_REFCKON            (1 << 4)  /* Bit 4:  Reference clock detection enable */
#define RTC_CR_BYPSHAD            (1 << 5)  /* Bit 5:  Bypass the shadow registers */
#define RTC_CR_FMT                (1 << 6)  /* Bit 6:  Hour format */
#define RTC_CR_ALRAE              (1 << 8)  /* Bit 8:  Alarm A enable */
#define RTC_CR_ALRBE              (1 << 9)  /* Bit 9:  Alarm B enable */
#define RTC_CR_WUTE               (1 << 10) /* Bit 10: Wakeup timer enable */
#define RTC_CR_TSE                (1 << 11) /* Bit 11: Time stamp enable */
#define RTC_CR_ALRAIE             (1 << 12) /* Bit 12: Alarm A interrupt enable */
#define RTC_CR_ALRBIE             (1 << 13) /* Bit 13: Alarm B interrupt enable */
#define RTC_CR_WUTIE              (1 << 14) /* Bit 14: Wakeup timer interrupt enable */
#define RTC_CR_TSIE               (1 << 15) /* Bit 15: Timestamp interrupt enable */
#define RTC_CR_ADD1H              (1 << 16) /* Bit 16: Add 1 hour */
#define RTC_CR_SUB1H              (1 << 17) /* Bit 17: Subtract 1 hour */
#define RTC_CR_BKP                (1 << 18) /* Bit 18: Backup */
#define RTC_CR_COSEL              (1 << 19) /* Bit 19: Calibration output selection */
#define RTC_CR_POL                (1 << 20) /* Bit 20: Output polarity */
#define RTC_CR_OSEL_SHIFT         (21)      /* Bits 21-22: Output selection */
#define RTC_CR_OSEL_MASK          (3 << RTC_CR_OSEL_SHIFT)
#  define RTC_CR_OSEL_DISABLED    (0 << RTC_CR_OSEL_SHIFT)
#  define RTC_CR_OSEL_ALRMA       (1 << RTC_CR_OSEL_SHIFT)
#  define RTC_CR_OSEL_ALRMB       (2 << RTC_CR_OSEL_SHIFT)
#  define RTC_CR_OSEL_WUT         (3 << RTC_CR_OSEL_SHIFT)

#define RTC_CR_COE                (1 << 23) /* Bit 23: Calibration output enable */
#define RTC_CR_ITSE               (1 << 24) /* Bit 24: Timestamp on internal event enable */
#define RTC_CR_TAMPTS             (1 << 25) /* Bit 25: Tamper timestamp enable */
#define RTC_CR_TAMPOE             (1 << 26) /* Bit 26: Tamper detection output enable */
#define RTC_CR_TAMPALRM_PU        (1 << 29) /* Bit 29: TAMPALRM pull-up enable */
#define RTC_CR_TAMPALRM_TYPE      (1 << 30) /* Bit 30: TAMPALRM output type */
#define RTC_CR_OUT2EN             (1 << 31) /* Bit 31: RTC_OUT2 output enable */

/* RTC initialization control and status register (ICSR) */

#define RTC_ICSR_ALRAWF           (1 << 0)  /* Bit 0:  Alarm A write flag */
#define RTC_ICSR_ALRBWF           (1 << 1)  /* Bit 1:  Alarm B write flag */
#define RTC_ICSR_WUTWF            (1 << 2)  /* Bit 2:  Wakeup timer write flag */
#define RTC_ICSR_SHPF             (1 << 3)  /* Bit 3:  Shift operation pending */
#define RTC_ICSR_INITS            (1 << 4)  /* Bit 4:  Initialization status flag */
#define RTC_ICSR_RSF              (1 << 5)  /* Bit 5:  Registers synchronization flag */
#define RTC_ICSR_INITF            (1 << 6)  /* Bit 6:  Initialization flag */
#define RTC_ICSR_INIT             (1 << 7)  /* Bit 7:  Initialization mode */
#define RTC_ICSR_RECALPF          (1 << 16) /* Bit 16: Recalibration pending flag */

/* RTC status register (SR) — read-only flags */

#define RTC_SR_ALRAF              (1 << 0)  /* Bit 0:  Alarm A flag */
#define RTC_SR_ALRBF              (1 << 1)  /* Bit 1:  Alarm B flag */
#define RTC_SR_WUTF               (1 << 2)  /* Bit 2:  Wakeup timer flag */
#define RTC_SR_TSF                (1 << 3)  /* Bit 3:  Timestamp flag */
#define RTC_SR_TSOVF              (1 << 4)  /* Bit 4:  Timestamp overflow flag */
#define RTC_SR_ITSF               (1 << 5)  /* Bit 5:  Internal timestamp flag */

/* RTC status clear register (SCR) — write-1-to-clear */

#define RTC_SCR_CALRAF            (1 << 0)  /* Bit 0:  Clear alarm A flag */
#define RTC_SCR_CALRBF            (1 << 1)  /* Bit 1:  Clear alarm B flag */
#define RTC_SCR_CWUTF             (1 << 2)  /* Bit 2:  Clear wakeup timer flag */
#define RTC_SCR_CTSF              (1 << 3)  /* Bit 3:  Clear timestamp flag */
#define RTC_SCR_CTSOVF            (1 << 4)  /* Bit 4:  Clear timestamp overflow flag */
#define RTC_SCR_CITSF             (1 << 5)  /* Bit 5:  Clear internal timestamp flag */

/* RTC prescaler register */

#define RTC_PRER_PREDIV_S_SHIFT   (0)       /* Bits 0-14: Synchronous prescaler factor */
#define RTC_PRER_PREDIV_S_MASK    (0x7fff << RTC_PRER_PREDIV_S_SHIFT)
#define RTC_PRER_PREDIV_A_SHIFT   (16)      /* Bits 16-22: Asynchronous prescaler factor */
#define RTC_PRER_PREDIV_A_MASK    (0x7f << RTC_PRER_PREDIV_A_SHIFT)

/* RTC wakeup timer register */

#define RTC_WUTR_MASK             (0xffff)  /* Bits 15:0  Wakeup auto-reload value bits */

/* RTC alarm A/B registers */

#define RTC_ALRMR_SU_SHIFT        (0)       /* Bits 0-3: Second units in BCD format. */
#define RTC_ALRMR_SU_MASK         (15 << RTC_ALRMR_SU_SHIFT)
#define RTC_ALRMR_ST_SHIFT        (4)       /* Bits 4-6: Second tens in BCD format. */
#define RTC_ALRMR_ST_MASK         (7 << RTC_ALRMR_ST_SHIFT)
#define RTC_ALRMR_MSK1            (1 << 7)  /* Bit 7 : Alarm seconds mask */
#define RTC_ALRMR_MNU_SHIFT       (8)       /* Bits 8-11: Minute units in BCD format. */
#define RTC_ALRMR_MNU_MASK        (15 << RTC_ALRMR_MNU_SHIFT)
#define RTC_ALRMR_MNT_SHIFT       (12)      /* Bits 12-14: Minute tens in BCD format. */
#define RTC_ALRMR_MNT_MASK        (7 << RTC_ALRMR_MNT_SHIFT)
#define RTC_ALRMR_MSK2            (1 << 15) /* Bit 15 : Alarm minutes mask */
#define RTC_ALRMR_HU_SHIFT        (16)      /* Bits 16-19: Hour units in BCD format. */
#define RTC_ALRMR_HU_MASK         (15 << RTC_ALRMR_HU_SHIFT)
#define RTC_ALRMR_HT_SHIFT        (20)      /* Bits 20-21: Hour tens in BCD format. */
#define RTC_ALRMR_HT_MASK         (3 << RTC_ALRMR_HT_SHIFT)
#define RTC_ALRMR_PM              (1 << 22) /* Bit 22 : AM/PM notation */
#define RTC_ALRMR_MSK3            (1 << 23) /* Bit 23 : Alarm hours mask */
#define RTC_ALRMR_DU_SHIFT        (24)      /* Bits 24-27: Date units or day in BCD format. */
#define RTC_ALRMR_DU_MASK         (15 << RTC_ALRMR_DU_SHIFT)
#define RTC_ALRMR_DT_SHIFT        (28)      /* Bits 28-29: Date tens in BCD format. */
#define RTC_ALRMR_DT_MASK         (3 << RTC_ALRMR_DT_SHIFT)
#define RTC_ALRMR_WDSEL           (1 << 30) /* Bit 30: Week day selection */
#define RTC_ALRMR_MSK4            (1 << 31) /* Bit 31: Alarm date mask */

/* RTC write protection register */

#define RTC_WPR_MASK              (0xff)    /* Bits 0-7: Write protection key */

/* RTC sub second register */

#define RTC_SSR_MASK              (0xffff)  /* Bits 0-15: Sub second value */

/* RTC shift control register */

#define RTC_SHIFTR_SUBFS_SHIFT    (0)       /* Bits 0-14: Subtract a fraction of a second */
#define RTC_SHIFTR_SUBFS_MASK     (0x7fff << RTC_SHIFTR_SUBFS_SHIFT)
#define RTC_SHIFTR_ADD1S          (1 << 31) /* Bit 31: Add one second */

/* RTC time stamp time register */

#define RTC_TSTR_SU_SHIFT         (0)       /* Bits 0-3: Second units in BCD format. */
#define RTC_TSTR_SU_MASK          (15 << RTC_TSTR_SU_SHIFT)
#define RTC_TSTR_ST_SHIFT         (4)       /* Bits 4-6: Second tens in BCD format. */
#define RTC_TSTR_ST_MASK          (7 << RTC_TSTR_ST_SHIFT)
#define RTC_TSTR_MNU_SHIFT        (8)       /* Bits 8-11: Minute units in BCD format. */
#define RTC_TSTR_MNU_MASK         (15 << RTC_TSTR_MNU_SHIFT)
#define RTC_TSTR_MNT_SHIFT        (12)      /* Bits 12-14: Minute tens in BCD format. */
#define RTC_TSTR_MNT_MASK         (7 << RTC_TSTR_MNT_SHIFT)
#define RTC_TSTR_HU_SHIFT         (16)      /* Bits 16-19: Hour units in BCD format. */
#define RTC_TSTR_HU_MASK          (15 << RTC_TSTR_HU_SHIFT)
#define RTC_TSTR_HT_SHIFT         (20)      /* Bits 20-21: Hour tens in BCD format. */
#define RTC_TSTR_HT_MASK          (3 << RTC_TSTR_HT_SHIFT)
#define RTC_TSTR_PM               (1 << 22) /* Bit 22: AM/PM notation */

/* RTC time stamp date register */

#define RTC_TSDR_DU_SHIFT         (0)       /* Bit 0-3: Date units in BCD format */
#define RTC_TSDR_DU_MASK          (15 << RTC_TSDR_DU_SHIFT)
#define RTC_TSDR_DT_SHIFT         (4)       /* Bits 4-5: Date tens in BCD format */
#define RTC_TSDR_DT_MASK          (3 << RTC_TSDR_DT_SHIFT)
#define RTC_TSDR_MU_SHIFT         (8)       /* Bits 8-11: Month units in BCD format */
#define RTC_TSDR_MU_MASK          (15 << RTC_TSDR_MU_SHIFT)
#define RTC_TSDR_MT               (1 << 12) /* Bit 12: Month tens in BCD format */
#define RTC_TSDR_WDU_SHIFT        (13)      /* Bits 13-15: Week day units */
#define RTC_TSDR_WDU_MASK         (7 << RTC_TSDR_WDU_SHIFT)

/* RTC timestamp sub second register */

#define RTC_TSSSR_MASK            (0xffff)  /* Bits 0-15: Sub second value */

/* RTC calibration register */

#define RTC_CALR_CALM_SHIFT       (0)       /* Bits 0-8: Calibration minus */
#define RTC_CALR_CALM_MASK        (0x1ff << RTC_CALR_CALM_SHIFT)
#define RTC_CALR_CALW16           (1 << 13) /* Bit 13: Use a 16-second calibration cycle period */
#define RTC_CALR_CALW8            (1 << 14) /* Bit 14: Use an 8-second calibration cycle period */
#define RTC_CALR_CALP             (1 << 15) /* Bit 15: Increase frequency of RTC by 488.5 ppm */

/* RTC alarm A/B sub second register */

#define RTC_ALRMSSR_SS_SHIFT      (0)   /* Bits 0-14: Sub second value */
#define RTC_ALRMSSR_SS_MASK       (0x7fff << RTC_ALRMSSR_SS_SHIFT)
#define RTC_ALRMSSR_MASKSS_SHIFT  (24)  /* Bits 24-27:  Mask the most-significant bits */
#define RTC_ALRMSSR_MASKSS_MASK   (0xf << RTC_ALRMSSR_MASKSS_SHIFT)

#endif /* __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_RTC_H */
