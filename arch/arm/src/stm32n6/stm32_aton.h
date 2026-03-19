/****************************************************************************
 * arch/arm/src/stm32n6/stm32_aton.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * ATON Neural-ART NPU register definitions for STM32N6.
 * Only registers/fields actually used by the generated model code.
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_STM32N6_STM32_ATON_H
#define __ARCH_ARM_SRC_STM32N6_STM32_ATON_H

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* ATON base address (Secure alias) */

#define STM32_ATON_BASE           0x580E0000

/* Unit base offsets and instance counts */

#define ATON_CLKCTRL_OFFSET       0x0000
#define ATON_INTCTRL_OFFSET       0x1000
#define ATON_BUSIF_OFFSET         0x2000    /* +0x1000*unit */
#define ATON_STRSWITCH_OFFSET     0x4000
#define ATON_STRENG_OFFSET        0x5000    /* +0x1000*unit */
#define ATON_CONVACC_OFFSET       0xF000    /* +0x1000*unit */
#define ATON_DECUN_OFFSET         0x13000   /* +0x1000*unit */
#define ATON_ACTIV_OFFSET         0x15000   /* +0x1000*unit */
#define ATON_ARITH_OFFSET         0x17000   /* +0x1000*unit */
#define ATON_POOL_OFFSET          0x1B000   /* +0x1000*unit */
#define ATON_RECBUF_OFFSET        0x1D000
#define ATON_EPOCHCTRL_OFFSET     0x1E000
#define ATON_UNIT_STRIDE          0x1000

#define ATON_STRENG_NUM           10
#define ATON_CONVACC_NUM          4
#define ATON_ARITH_NUM            4
#define ATON_POOL_NUM             2
#define ATON_DECUN_NUM            2
#define ATON_ACTIV_NUM            2
#define ATON_RECBUF_NUM           1

/* Unit base address macros */

#define ATON_STRENG_BASE(n)    (STM32_ATON_BASE + ATON_STRENG_OFFSET + \
                                (n) * ATON_UNIT_STRIDE)
#define ATON_CONVACC_BASE(n)   (STM32_ATON_BASE + ATON_CONVACC_OFFSET + \
                                (n) * ATON_UNIT_STRIDE)
#define ATON_ARITH_BASE(n)     (STM32_ATON_BASE + ATON_ARITH_OFFSET + \
                                (n) * ATON_UNIT_STRIDE)
#define ATON_POOL_BASE(n)      (STM32_ATON_BASE + ATON_POOL_OFFSET + \
                                (n) * ATON_UNIT_STRIDE)
#define ATON_DECUN_BASE(n)     (STM32_ATON_BASE + ATON_DECUN_OFFSET + \
                                (n) * ATON_UNIT_STRIDE)
#define ATON_ACTIV_BASE(n)     (STM32_ATON_BASE + ATON_ACTIV_OFFSET + \
                                (n) * ATON_UNIT_STRIDE)
#define ATON_RECBUF_BASE(n)    (STM32_ATON_BASE + ATON_RECBUF_OFFSET + \
                                (n) * ATON_UNIT_STRIDE)
#define ATON_STRSWITCH_BASE    (STM32_ATON_BASE + ATON_STRSWITCH_OFFSET)
#define ATON_CLKCTRL_BASE      (STM32_ATON_BASE + ATON_CLKCTRL_OFFSET)
#define ATON_INTCTRL_BASE      (STM32_ATON_BASE + ATON_INTCTRL_OFFSET)

/****************************************************************************
 * CLKCTRL Registers (ATON_BASE + 0x0000)
 ****************************************************************************/

#define ATON_CLKCTRL_CTRL         0x00
#define ATON_CLKCTRL_VERSION      0x04
#define ATON_CLKCTRL_AGATES0      0x08
#define ATON_CLKCTRL_AGATES1      0x0C
#define ATON_CLKCTRL_BGATES       0x10

/* CLKCTRL_CTRL bits */

#define ATON_CLKCTRL_CTRL_EN      (1 << 0)
#define ATON_CLKCTRL_CTRL_CLR     (1 << 1)
#define ATON_CLKCTRL_CTRL_CONFCLR (1 << 30)

/* Clock gate bit indices for BGATES register.
 * Used by LL_ATON_EnableClock/DisableClock.
 */

#define ATON_CLKB_STRENG(n)    (n)            /* bits 0-9 */
#define ATON_CLKB_CONVACC(n)   ((n) + 10)     /* bits 10-13 */
#define ATON_CLKB_DECUN(n)     ((n) + 14)     /* bits 14-15 */
#define ATON_CLKB_ACTIV(n)     ((n) + 16)     /* bits 16-17 */
#define ATON_CLKB_ARITH(n)     ((n) + 18)     /* bits 18-21 */
#define ATON_CLKB_POOL(n)      ((n) + 22)     /* bits 22-23 */
#define ATON_CLKB_RECBUF       24             /* bit 24 */

/****************************************************************************
 * STRSWITCH Registers (ATON_BASE + 0x4000)
 ****************************************************************************/

#define ATON_STRSWITCH_CTRL       0x00
#define ATON_STRSWITCH_VERSION    0x04
#define ATON_STRSWITCH_DST(idx)   (0x08 + 4 * (idx))

/* STRSWITCH_CTRL bits */

#define ATON_STRSWITCH_CTRL_EN      (1 << 0)
#define ATON_STRSWITCH_CTRL_CLR     (1 << 1)
#define ATON_STRSWITCH_CTRL_CONFCLR (1 << 30)

/* DST register bit positions */

#define ATON_DST_EN0_SHIFT     0
#define ATON_DST_LINK0_SHIFT   1
#define ATON_DST_LINK0_MASK    (0x3F << 1)
#define ATON_DST_FNR0_SHIFT    8
#define ATON_DST_FNR0_MASK     (0xFF << 8)
#define ATON_DST_EN1_SHIFT     16
#define ATON_DST_LINK1_SHIFT   17
#define ATON_DST_LINK1_MASK    (0x3F << 17)
#define ATON_DST_FNR1_SHIFT    24
#define ATON_DST_FNR1_MASK     (0xFF << 24)

/* Source port IDs (LINK values) */

#define ATON_SRCPORT_STRENG(n)    (n)          /* 0-9 */
#define ATON_SRCPORT_CONVACC(n)   ((n) + 10)   /* 10-13 */
#define ATON_SRCPORT_DECUN(n)     ((n) + 14)   /* 14-15 */
#define ATON_SRCPORT_ACTIV(n)     ((n) + 16)   /* 16-17 */
#define ATON_SRCPORT_ARITH(n)     ((n) + 18)   /* 18-21 */
#define ATON_SRCPORT_POOL(n)      ((n) + 22)   /* 22-23 */
#define ATON_SRCPORT_RECBUF(n, p) (24 + (p))   /* 24-26 */

/* Destination port register index (IDX into DST array) */

#define ATON_DSTIDX_STRENG(n, p)    (n)                      /* 0-9 */
#define ATON_DSTIDX_CONVACC(n, p)   (10 + 3 * (n) + (p))    /* 10-21 */
#define ATON_DSTIDX_DECUN(n, p)     (22 + 2 * (n) + (p))    /* 22-25 */
#define ATON_DSTIDX_ACTIV(n, p)     (26 + (n))               /* 26-27 */
#define ATON_DSTIDX_ARITH(n, p)     (28 + 2 * (n) + (p))    /* 28-35 */
#define ATON_DSTIDX_POOL(n, p)      (36 + (n))               /* 36-37 */
#define ATON_DSTIDX_RECBUF(n, p)    (38 + (p))               /* 38-40 */

/* Destination port byte offset from STRSWITCH base */

#define ATON_DSTPORT_STRENG(n, p)    ATON_STRSWITCH_DST(ATON_DSTIDX_STRENG(n, p))
#define ATON_DSTPORT_CONVACC(n, p)   ATON_STRSWITCH_DST(ATON_DSTIDX_CONVACC(n, p))
#define ATON_DSTPORT_DECUN(n, p)     ATON_STRSWITCH_DST(ATON_DSTIDX_DECUN(n, p))
#define ATON_DSTPORT_ACTIV(n, p)     ATON_STRSWITCH_DST(ATON_DSTIDX_ACTIV(n, p))
#define ATON_DSTPORT_ARITH(n, p)     ATON_STRSWITCH_DST(ATON_DSTIDX_ARITH(n, p))
#define ATON_DSTPORT_POOL(n, p)      ATON_STRSWITCH_DST(ATON_DSTIDX_POOL(n, p))
#define ATON_DSTPORT_RECBUF(n, p)    ATON_STRSWITCH_DST(ATON_DSTIDX_RECBUF(n, p))

/****************************************************************************
 * STRENG Registers (per instance, ATON_BASE + 0x5000 + 0x1000*n)
 ****************************************************************************/

#define ATON_STRENG_CTRL          0x00
#define ATON_STRENG_VERSION       0x04
#define ATON_STRENG_ADDR          0x08
#define ATON_STRENG_FSIZE         0x0C
#define ATON_STRENG_DEPTH         0x10
#define ATON_STRENG_STRD          0x14
#define ATON_STRENG_FOFFSET       0x18
#define ATON_STRENG_FRAME_RPT     0x1C
#define ATON_STRENG_FRPTOFF       0x20
#define ATON_STRENG_POS           0x24
#define ATON_STRENG_EVENT         0x28
#define ATON_STRENG_STOPTAG       0x2C
#define ATON_STRENG_LIMITEN       0x30
#define ATON_STRENG_LIMIT         0x34
#define ATON_STRENG_LIMITADDR     0x38
#define ATON_STRENG_IRQ           0x3C
#define ATON_STRENG_ENCR_LSB      0x40
#define ATON_STRENG_ENCR_MSB      0x44
#define ATON_STRENG_CID_CACHE     0x48
#define ATON_STRENG_EXTSYNC       0x4C

/* STRENG_CTRL bitfields */

#define STRENG_CTRL_EN            (1 << 0)
#define STRENG_CTRL_CLR           (1 << 1)
#define STRENG_CTRL_SINGLE        (1 << 2)
#define STRENG_CTRL_DIR           (1 << 3)    /* 0=read, 1=write */
#define STRENG_CTRL_NOINC         (1 << 4)
#define STRENG_CTRL_ISTART        (1 << 5)
#define STRENG_CTRL_SUBSAMPLE     (1 << 6)
#define STRENG_CTRL_CONT          (1 << 7)
#define STRENG_CTRL_RAW           (1 << 8)
#define STRENG_CTRL_RAW_OUT       (1 << 9)
#define STRENG_CTRL_DESCR         (1 << 10)
#define STRENG_CTRL_NOBLK         (1 << 11)
#define STRENG_CTRL_ROUND         (1 << 12)
#define STRENG_CTRL_BEFORCE       (1 << 13)
#define STRENG_CTRL_LSBMODE       (1 << 14)
#define STRENG_CTRL_SIGNEXT       (1 << 15)
#define STRENG_CTRL_SIZE0_SHIFT   16
#define STRENG_CTRL_SIZE0_MASK    (0xF << 16)
#define STRENG_CTRL_SIZE1_SHIFT   20
#define STRENG_CTRL_SIZE1_MASK    (0xF << 20)
#define STRENG_CTRL_SIZE2_SHIFT   24
#define STRENG_CTRL_SIZE2_MASK    (0xF << 24)
#define STRENG_CTRL_SERDES_SHIFT  28
#define STRENG_CTRL_SERDES_MASK   (3 << 28)
#define STRENG_CTRL_CONFCLR       (1 << 30)
#define STRENG_CTRL_RUNNING       (1 << 31)

/* STRENG_FSIZE bitfields */

#define STRENG_FSIZE_WIDTH_SHIFT  0
#define STRENG_FSIZE_WIDTH_MASK   0x0000FFFF
#define STRENG_FSIZE_HEIGHT_SHIFT 16
#define STRENG_FSIZE_HEIGHT_MASK  0xFFFF0000

/* STRENG_DEPTH bitfields */

#define STRENG_DEPTH_SIZE_SHIFT   0
#define STRENG_DEPTH_SIZE_MASK    0x0000FFFF
#define STRENG_DEPTH_OFFSET_SHIFT 16
#define STRENG_DEPTH_OFFSET_MASK  0xFFFF0000

/* STRENG_STRD bitfields */

#define STRENG_STRD_LOFF_SHIFT    0
#define STRENG_STRD_LOFF_MASK     0x0000FFFF
#define STRENG_STRD_FGAP_SHIFT    16
#define STRENG_STRD_FGAP_MASK     (0x3F << 16)
#define STRENG_STRD_BGAP_SHIFT    24
#define STRENG_STRD_BGAP_MASK     (0x3F << 24)

/* STRENG_EVENT bitfields */

#define STRENG_EVENT_EN_BUFBL      (1 << 16)
#define STRENG_EVENT_EN_OFLOW_FRM  (1 << 19)
#define STRENG_EVENT_EN_ILLCFG     (1 << 20)
#define STRENG_EVENT_FRMTRG_EN     (1 << 23)
#define STRENG_EVENT_FRMTRG_SRC_SHIFT 24
#define STRENG_EVENT_FRMTRG_SRC_MASK  (0x1F << 24)

/* STRENG_POS bitfields */

#define STRENG_POS_GAPCYCLES_SHIFT 16
#define STRENG_POS_GAPCYCLES_MASK  (0xFFFF << 16)

/* STRENG_LIMITEN bitfields (from ATON.h — verified) */

#define STRENG_LIMITEN_ADDRLIMIT   (1 << 0)
#define STRENG_LIMITEN_STOPPREFTC  (1 << 1)
#define STRENG_LIMITEN_FRAMELIMIT  (1 << 2)

/* STRENG_CID_CACHE bitfields */

#define STRENG_CID_CID_SHIFT      0
#define STRENG_CID_CID_MASK       (7 << 0)
#define STRENG_CID_CACHEABLE      (1 << 3)
#define STRENG_CID_ALLOC          (1 << 4)
#define STRENG_CID_PFETCH         (1 << 5)
#define STRENG_CID_LINESIZE_SHIFT 6
#define STRENG_CID_LINESIZE_MASK  (3 << 6)
#define STRENG_CID_LOFF_MSB_SHIFT 16
#define STRENG_CID_LOFF_MSB_MASK  (0xFFFF << 16)

/****************************************************************************
 * CONVACC Registers (per instance, ATON_BASE + 0xF000 + 0x1000*n)
 ****************************************************************************/

#define ATON_CONVACC_CTRL         0x00
#define ATON_CONVACC_VERSION      0x04
#define ATON_CONVACC_KFORMAT      0x08
#define ATON_CONVACC_SAMPLE       0x0C
#define ATON_CONVACC_DFORMAT      0x10
#define ATON_CONVACC_FFORMAT      0x14
#define ATON_CONVACC_FHCROP       0x18
#define ATON_CONVACC_FVCROP       0x1C
#define ATON_CONVACC_KFILT        0x20
#define ATON_CONVACC_AFILT        0x24
#define ATON_CONVACC_ZFRAME       0x28
#define ATON_CONVACC_ITER         0x2C
#define ATON_CONVACC_FSUB         0x30
#define ATON_CONVACC_ZFBIAS       0x34

/* CONVACC_CTRL bitfields */

#define CONVACC_CTRL_EN           (1 << 0)
#define CONVACC_CTRL_CLR          (1 << 1)
#define CONVACC_CTRL_NOSUM        (1 << 2)
#define CONVACC_CTRL_KT1          (1 << 3)
#define CONVACC_CTRL_NO1SUM       (1 << 4)
#define CONVACC_CTRL_SIMD_SHIFT   5
#define CONVACC_CTRL_SIMD_MASK    (3 << 5)
#define CONVACC_CTRL_AFILTMODE_SHIFT 8
#define CONVACC_CTRL_AFILTMODE_MASK  (3 << 8)
#define CONVACC_CTRL_GEN1SUM      (1 << 10)
#define CONVACC_CTRL_FC           (1 << 11)
#define CONVACC_CTRL_FUNSIGNED    (1 << 20)
#define CONVACC_CTRL_KUNSIGNED    (1 << 21)
#define CONVACC_CTRL_KSETEN_SHIFT 22
#define CONVACC_CTRL_KSETEN_MASK  (3 << 22)
#define CONVACC_CTRL_FSTAT        (1 << 24)
#define CONVACC_CTRL_DEEPMODE     (1 << 28)
#define CONVACC_CTRL_DSS2MODE     (1 << 29)
#define CONVACC_CTRL_CONFCLR      (1 << 30)

/* CONVACC_DFORMAT bitfields (from ATON.h — verified exact positions) */

#define CONVACC_DFORMAT_INSHIFT_SHIFT  0
#define CONVACC_DFORMAT_INSHIFT_MASK   (0x3F << 0)
#define CONVACC_DFORMAT_FRNDMODE_SHIFT 6
#define CONVACC_DFORMAT_FRNDMODE_MASK  (3 << 6)
#define CONVACC_DFORMAT_OUTSHIFT_SHIFT 8
#define CONVACC_DFORMAT_OUTSHIFT_MASK  (0x3F << 8)
#define CONVACC_DFORMAT_ORNDMODE_SHIFT 14
#define CONVACC_DFORMAT_ORNDMODE_MASK  (3 << 14)
#define CONVACC_DFORMAT_ROUND_SHIFT    16
#define CONVACC_DFORMAT_SAT_SHIFT      17
#define CONVACC_DFORMAT_RAW_SHIFT      18
#define CONVACC_DFORMAT_OBYTES_SHIFT   20
#define CONVACC_DFORMAT_OBYTES_MASK    (3 << 20)
#define CONVACC_DFORMAT_FBYTES_SHIFT   22
#define CONVACC_DFORMAT_FBYTES_MASK    (3 << 22)
#define CONVACC_DFORMAT_FSHIFT_SHIFT   24
#define CONVACC_DFORMAT_FSHIFT_MASK    (0x3F << 24)
#define CONVACC_DFORMAT_FROUND_SHIFT   30
#define CONVACC_DFORMAT_FSAT_SHIFT     31

/* CONVACC_FFORMAT bitfields */

#define CONVACC_FFORMAT_WIDTH_SHIFT  0
#define CONVACC_FFORMAT_WIDTH_MASK   0x0000FFFF
#define CONVACC_FFORMAT_HEIGHT_SHIFT 16
#define CONVACC_FFORMAT_HEIGHT_MASK  0xFFFF0000

/* CONVACC_KFORMAT bitfields (from ATON.h — 8-bit fields) */

#define CONVACC_KFORMAT_WIDTH_SHIFT    0
#define CONVACC_KFORMAT_WIDTH_MASK     (0xFF << 0)
#define CONVACC_KFORMAT_HEIGHT_SHIFT   8
#define CONVACC_KFORMAT_HEIGHT_MASK    (0xFF << 8)
#define CONVACC_KFORMAT_BTCDEPTH_SHIFT 16
#define CONVACC_KFORMAT_BTCDEPTH_MASK  (0xFF << 16)
#define CONVACC_KFORMAT_NR_SHIFT       24
#define CONVACC_KFORMAT_NR_MASK        (0xFF << 24)

/* CONVACC_SAMPLE bitfields (from ATON.h — LPAD/RPAD first) */

#define CONVACC_SAMPLE_LPAD_SHIFT  0
#define CONVACC_SAMPLE_LPAD_MASK   (3 << 0)
#define CONVACC_SAMPLE_RPAD_SHIFT  2
#define CONVACC_SAMPLE_RPAD_MASK   (3 << 2)
#define CONVACC_SAMPLE_TPAD_SHIFT  4
#define CONVACC_SAMPLE_TPAD_MASK   (3 << 4)
#define CONVACC_SAMPLE_BPAD_SHIFT  6
#define CONVACC_SAMPLE_BPAD_MASK   (3 << 6)
#define CONVACC_SAMPLE_HSTRD_SHIFT 8
#define CONVACC_SAMPLE_HSTRD_MASK  (7 << 8)
#define CONVACC_SAMPLE_VSTRD_SHIFT 12
#define CONVACC_SAMPLE_VSTRD_MASK  (7 << 12)
#define CONVACC_SAMPLE_FSTATCNT_SHIFT 16
#define CONVACC_SAMPLE_FSTATCNT_MASK  (0xFFFF << 16)

/* CONVACC_ZFRAME bitfields */

#define CONVACC_ZFRAME_TOP_SHIFT    0
#define CONVACC_ZFRAME_TOP_MASK     (0xFF << 0)
#define CONVACC_ZFRAME_BOTTOM_SHIFT 8
#define CONVACC_ZFRAME_BOTTOM_MASK  (0xFF << 8)
#define CONVACC_ZFRAME_LEFT_SHIFT   16
#define CONVACC_ZFRAME_LEFT_MASK    (0xFF << 16)
#define CONVACC_ZFRAME_LEFT_W       8
#define CONVACC_ZFRAME_RIGHT_SHIFT  24
#define CONVACC_ZFRAME_RIGHT_MASK   (0xFF << 24)
#define CONVACC_ZFRAME_RIGHT_W      8

/* CONVACC_FHCROP bitfields */

#define CONVACC_FHCROP_LEFT_SHIFT   0
#define CONVACC_FHCROP_LEFT_MASK    0x0000FFFF
#define CONVACC_FHCROP_RIGHT_SHIFT  16
#define CONVACC_FHCROP_RIGHT_MASK   0xFFFF0000

/* CONVACC_FVCROP bitfields */

#define CONVACC_FVCROP_TOP_SHIFT    0
#define CONVACC_FVCROP_TOP_MASK     0x0000FFFF
#define CONVACC_FVCROP_BOTTOM_SHIFT 16
#define CONVACC_FVCROP_BOTTOM_MASK  0xFFFF0000

/* CONVACC_KFILT bitfields */

#define CONVACC_KFILT_TOT_SHIFT     0
#define CONVACC_KFILT_TOT_MASK      (0xFF << 0)
#define CONVACC_KFILT_FIRST_SHIFT   8
#define CONVACC_KFILT_FIRST_MASK    (0xFF << 8)
#define CONVACC_KFILT_LAST_SHIFT    16
#define CONVACC_KFILT_LAST_MASK     (0xFF << 16)

/* CONVACC_AFILT bitfields */

#define CONVACC_AFILT_TOT_SHIFT     0
#define CONVACC_AFILT_TOT_MASK      (0xFF << 0)
#define CONVACC_AFILT_FIRST_SHIFT   8
#define CONVACC_AFILT_FIRST_MASK    (0xFF << 8)
#define CONVACC_AFILT_LAST_SHIFT    16
#define CONVACC_AFILT_LAST_MASK     (0xFF << 16)

/* CONVACC_FSUB bitfields */

#define CONVACC_FSUB_FSUB_SHIFT     0
#define CONVACC_FSUB_FSUB_MASK      (0xFF << 0)
#define CONVACC_FSUB_VSHIFT_SHIFT   8
#define CONVACC_FSUB_VSHIFT_MASK    (3 << 8)

/* CONVACC_ZFBIAS bitfields */

#define CONVACC_ZFBIAS_ZFBIAS_SHIFT 0
#define CONVACC_ZFBIAS_ZFBIAS_MASK  (0xFFFF << 0)

/****************************************************************************
 * ARITH Registers (per instance, ATON_BASE + 0x17000 + 0x1000*n)
 ****************************************************************************/

#define ATON_ARITH_CTRL           0x00
#define ATON_ARITH_VERSION        0x04
#define ATON_ARITH_SHIFT          0x08
#define ATON_ARITH_INCCNT         0x0C
#define ATON_ARITH_RSTCNT1        0x10
#define ATON_ARITH_RSTCNT2        0x14
#define ATON_ARITH_RSTCNT3        0x18
#define ATON_ARITH_COEFFAC        0x1C
#define ATON_ARITH_COEFFB         0x20
#define ATON_ARITH_ADDROFFSET     0x24
#define ATON_ARITH_INCOFFSET      0x28
#define ATON_ARITH_TRANSLATEADDR  0x2C
#define ATON_ARITH_COEFFADDR      0x30
#define ATON_ARITH_INSHIFTER      0x34
#define ATON_ARITH_CLIPRANGE      0x38

/* ARITH_CTRL bitfields */

#define ARITH_CTRL_EN             (1 << 0)
#define ARITH_CTRL_CLR            (1 << 1)
#define ARITH_CTRL_CNT1           (1 << 2)
#define ARITH_CTRL_CNT2           (1 << 3)
#define ARITH_CTRL_CNT3           (1 << 4)
#define ARITH_CTRL_ROUND          (1 << 5)
#define ARITH_CTRL_SAT            (1 << 6)
#define ARITH_CTRL_COEFFA         (1 << 7)
#define ARITH_CTRL_DUALIN         (1 << 8)
#define ARITH_CTRL_OP_SHIFT       9
#define ARITH_CTRL_OP_MASK        (0x3F << 9)
#define ARITH_CTRL_COEFFB         (1 << 21)
#define ARITH_CTRL_COEFFC         (1 << 22)
#define ARITH_CTRL_LOGICALOP      (1 << 23)
#define ARITH_CTRL_ORNDMODE_SHIFT 24
#define ARITH_CTRL_ORNDMODE_MASK  (3 << 24)
#define ARITH_CTRL_OBYTES_SHIFT   26
#define ARITH_CTRL_OBYTES_MASK    (3 << 26)
#define ARITH_CTRL_COMBINEBC      (1 << 28)
#define ARITH_CTRL_CLIPOUT        (1 << 29)
#define ARITH_CTRL_CONFCLR        (1 << 30)

/* ARITH_INSHIFTER bitfields (offset 0x34) */

#define ARITH_INSHIFTER_FBYTESX_SHIFT  0
#define ARITH_INSHIFTER_FBYTESX_MASK   (3 << 0)
#define ARITH_INSHIFTER_FSHIFTX_SHIFT  2
#define ARITH_INSHIFTER_FSHIFTX_MASK   (0x3F << 2)
#define ARITH_INSHIFTER_FROUNDX_SHIFT  8
#define ARITH_INSHIFTER_FSATX_SHIFT    9
#define ARITH_INSHIFTER_FRNDMODEX_SHIFT 10
#define ARITH_INSHIFTER_FRNDMODEX_MASK (3 << 10)
#define ARITH_INSHIFTER_FOBYTESX_SHIFT 12
#define ARITH_INSHIFTER_FOBYTESX_MASK  (3 << 12)
#define ARITH_INSHIFTER_FBYTESY_SHIFT  16
#define ARITH_INSHIFTER_FBYTESY_MASK   (3 << 16)
#define ARITH_INSHIFTER_FSHIFTY_SHIFT  18
#define ARITH_INSHIFTER_FSHIFTY_MASK   (0x3F << 18)
#define ARITH_INSHIFTER_FROUNDY_SHIFT  24
#define ARITH_INSHIFTER_FSATY_SHIFT    25
#define ARITH_INSHIFTER_FRNDMODEY_SHIFT 26
#define ARITH_INSHIFTER_FRNDMODEY_MASK (3 << 26)
#define ARITH_INSHIFTER_FOBYTESY_SHIFT 28
#define ARITH_INSHIFTER_FOBYTESY_MASK  (3 << 28)

/* ARITH_SHIFT bitfields (offset 0x08 — tightly packed, from ATON.h) */

#define ARITH_SHIFT_AX_SHIFT      0
#define ARITH_SHIFT_AX_MASK       (0x1F << 0)    /* 5 bits [4:0] */
#define ARITH_SHIFT_BY_SHIFT      5
#define ARITH_SHIFT_BY_MASK       (0x1F << 5)    /* 5 bits [9:5] */
#define ARITH_SHIFT_C_SHIFT       10
#define ARITH_SHIFT_C_MASK        (0x1F << 10)   /* 5 bits [14:10] */
#define ARITH_SHIFT_RES_SHIFT     15
#define ARITH_SHIFT_RES_MASK      (0x3F << 15)   /* 6 bits [20:15] */

/* ARITH_COEFFAC bitfields */

#define ARITH_COEFFAC_A_SHIFT     0
#define ARITH_COEFFAC_A_MASK      0x0000FFFF
#define ARITH_COEFFAC_C_SHIFT     16
#define ARITH_COEFFAC_C_MASK      0xFFFF0000

/* ARITH_COEFFB bitfields */

#define ARITH_COEFFB_B_SHIFT      0
#define ARITH_COEFFB_B_MASK       0x0000FFFF

/* ARITH_CLIPRANGE bitfields */

#define ARITH_CLIPRANGE_CLIPMIN_SHIFT 0
#define ARITH_CLIPRANGE_CLIPMIN_MASK  0x0000FFFF
#define ARITH_CLIPRANGE_CLIPMAX_SHIFT 16
#define ARITH_CLIPRANGE_CLIPMAX_MASK  0xFFFF0000

/* ARITH operation codes (hardware encoding) */

#define ARITH_HW_OP_AFFINE        0x00
#define ARITH_HW_OP_MIN           0x01
#define ARITH_HW_OP_MAX           0x02
#define ARITH_HW_OP_MUL           0x03
#define ARITH_HW_OP_AND           0x04
#define ARITH_HW_OP_OR            0x05
#define ARITH_HW_OP_NOT           0x06
#define ARITH_HW_OP_XOR           0x07
#define ARITH_HW_OP_EQ            0x08
#define ARITH_HW_OP_LT            0x09
#define ARITH_HW_OP_LE            0x0A
#define ARITH_HW_OP_GT            0x0B
#define ARITH_HW_OP_GE            0x0C
#define ARITH_HW_OP_ABS           0x0D
#define ARITH_HW_OP_SIGN          0x0E
#define ARITH_HW_OP_CLIP          0x0F

/* Shift encoding: signed shift → HW encoding (range 0-39, no shift = 16) */

#define ATON_SHIFT_ENCODE(x)      ((x) + 16)

/****************************************************************************
 * INTCTRL Registers (ATON_BASE + 0x1000 — interrupt routing)
 ****************************************************************************/

#define ATON_INTCTRL_CTRL         0x00
#define ATON_INTCTRL_INTREG       0x08   /* RO: active internal signals */
#define ATON_INTCTRL_INTSET       0x0C
#define ATON_INTCTRL_INTCLR       0x10   /* Write-1-to-clear signals */
#define ATON_INTCTRL_INTORMSK(n)  (0x14 + (n) * 4)  /* OR-mask → NPUn_IRQ */
#define ATON_INTCTRL_INTANDMSK(n) (0x24 + (n) * 4)  /* AND-mask */

/****************************************************************************
 * CACHEAXI Registers (0x580DFC00 — NPU AXI cache controller)
 ****************************************************************************/

#define ATON_CACHEAXI_CR1         0x00   /* Enable (bit 0), full invalidate (bit 1) */
#define ATON_CACHEAXI_SR          0x04   /* BUSYF (0), BUSYCMDF (3), CMDENDF (4) */
#define ATON_CACHEAXI_FCR         0x0C   /* Flag clear (write 0x12) */
#define ATON_CACHEAXI_CR2         0x100  /* CACHECMD (bits 1-2) + STARTCMD (bit 0) */
#define ATON_CACHEAXI_RSADDR      0x104  /* Range start address */
#define ATON_CACHEAXI_READDR      0x108  /* Range end address (inclusive) */

#endif /* __ARCH_ARM_SRC_STM32N6_STM32_ATON_H */
