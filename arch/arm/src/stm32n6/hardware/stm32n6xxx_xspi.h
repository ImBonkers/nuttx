/****************************************************************************
 * arch/arm/src/stm32n6/hardware/stm32n6xxx_xspi.h
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

#ifndef __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_XSPI_H
#define __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_XSPI_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include "chip.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* General Characteristics **************************************************/

#define STM32N6_XSPI_MINBITS          8         /* Minimum word width */
#define STM32N6_XSPI_MAXBITS          32        /* Maximum word width */

/* XSPI register offsets ****************************************************/

#define STM32_XSPI_CR_OFFSET         0x0000    /* Control Register */
#define STM32_XSPI_DCR1_OFFSET       0x0008    /* Device Configuration Register 1 */
#define STM32_XSPI_DCR2_OFFSET       0x000c    /* Device Configuration Register 2 */
#define STM32_XSPI_DCR3_OFFSET       0x0010    /* Device Configuration Register 3 */
#define STM32_XSPI_DCR4_OFFSET       0x0014    /* Device Configuration Register 4 */
#define STM32_XSPI_SR_OFFSET         0x0020    /* Status Register */
#define STM32_XSPI_FCR_OFFSET        0x0024    /* Flag Clear Register */
#define STM32_XSPI_DLR_OFFSET        0x0040    /* Data Length Register */
#define STM32_XSPI_AR_OFFSET         0x0048    /* Address Register */
#define STM32_XSPI_DR_OFFSET         0x0050    /* Data Register */
#define STM32_XSPI_PSMKR_OFFSET      0x0080    /* Polling Status Mask Register */
#define STM32_XSPI_PSMAR_OFFSET      0x0088    /* Polling Status Match Register */
#define STM32_XSPI_PIR_OFFSET        0x0090    /* Polling Interval Register */
#define STM32_XSPI_CCR_OFFSET        0x0100    /* Communication Configuration Register */
#define STM32_XSPI_TCR_OFFSET        0x0108    /* Timing Configuration Register */
#define STM32_XSPI_IR_OFFSET         0x0110    /* Instruction Register */
#define STM32_XSPI_ABR_OFFSET        0x0120    /* Alternate Bytes Register */
#define STM32_XSPI_LPTR_OFFSET       0x0130    /* Low-Power Timeout Register */
#define STM32_XSPI_WPCCR_OFFSET      0x0140    /* Wrap Communication Config Register */
#define STM32_XSPI_WPTCR_OFFSET      0x0148    /* Wrap Timing Config Register */
#define STM32_XSPI_WPIR_OFFSET       0x0150    /* Wrap Instruction Register */
#define STM32_XSPI_WPABR_OFFSET      0x0160    /* Wrap Alternate Bytes Register */
#define STM32_XSPI_WCCR_OFFSET       0x0180    /* Write Communication Config Register */
#define STM32_XSPI_WTCR_OFFSET       0x0188    /* Write Timing Configuration Register */
#define STM32_XSPI_WIR_OFFSET        0x0190    /* Write Instruction Register */
#define STM32_XSPI_WABR_OFFSET       0x01a0    /* Write Alternate Bytes Register */
#define STM32_XSPI_HLCR_OFFSET       0x0200    /* HyperBus Latency Config Register */

/* XSPI register addresses (using XSPI2 base) ******************************/

#define STM32_XSPI2_CR        (STM32_XSPI2_BASE + STM32_XSPI_CR_OFFSET)
#define STM32_XSPI2_DCR1      (STM32_XSPI2_BASE + STM32_XSPI_DCR1_OFFSET)
#define STM32_XSPI2_DCR2      (STM32_XSPI2_BASE + STM32_XSPI_DCR2_OFFSET)
#define STM32_XSPI2_DCR3      (STM32_XSPI2_BASE + STM32_XSPI_DCR3_OFFSET)
#define STM32_XSPI2_DCR4      (STM32_XSPI2_BASE + STM32_XSPI_DCR4_OFFSET)
#define STM32_XSPI2_SR        (STM32_XSPI2_BASE + STM32_XSPI_SR_OFFSET)
#define STM32_XSPI2_FCR       (STM32_XSPI2_BASE + STM32_XSPI_FCR_OFFSET)
#define STM32_XSPI2_DLR       (STM32_XSPI2_BASE + STM32_XSPI_DLR_OFFSET)
#define STM32_XSPI2_AR        (STM32_XSPI2_BASE + STM32_XSPI_AR_OFFSET)
#define STM32_XSPI2_DR        (STM32_XSPI2_BASE + STM32_XSPI_DR_OFFSET)
#define STM32_XSPI2_PSMKR     (STM32_XSPI2_BASE + STM32_XSPI_PSMKR_OFFSET)
#define STM32_XSPI2_PSMAR     (STM32_XSPI2_BASE + STM32_XSPI_PSMAR_OFFSET)
#define STM32_XSPI2_PIR       (STM32_XSPI2_BASE + STM32_XSPI_PIR_OFFSET)
#define STM32_XSPI2_CCR       (STM32_XSPI2_BASE + STM32_XSPI_CCR_OFFSET)
#define STM32_XSPI2_TCR       (STM32_XSPI2_BASE + STM32_XSPI_TCR_OFFSET)
#define STM32_XSPI2_IR        (STM32_XSPI2_BASE + STM32_XSPI_IR_OFFSET)
#define STM32_XSPI2_ABR       (STM32_XSPI2_BASE + STM32_XSPI_ABR_OFFSET)
#define STM32_XSPI2_LPTR      (STM32_XSPI2_BASE + STM32_XSPI_LPTR_OFFSET)
#define STM32_XSPI2_WPCCR     (STM32_XSPI2_BASE + STM32_XSPI_WPCCR_OFFSET)
#define STM32_XSPI2_WPTCR     (STM32_XSPI2_BASE + STM32_XSPI_WPTCR_OFFSET)
#define STM32_XSPI2_WPIR      (STM32_XSPI2_BASE + STM32_XSPI_WPIR_OFFSET)
#define STM32_XSPI2_WPABR     (STM32_XSPI2_BASE + STM32_XSPI_WPABR_OFFSET)
#define STM32_XSPI2_WCCR      (STM32_XSPI2_BASE + STM32_XSPI_WCCR_OFFSET)
#define STM32_XSPI2_WTCR      (STM32_XSPI2_BASE + STM32_XSPI_WTCR_OFFSET)
#define STM32_XSPI2_WIR       (STM32_XSPI2_BASE + STM32_XSPI_WIR_OFFSET)
#define STM32_XSPI2_WABR      (STM32_XSPI2_BASE + STM32_XSPI_WABR_OFFSET)
#define STM32_XSPI2_HLCR      (STM32_XSPI2_BASE + STM32_XSPI_HLCR_OFFSET)

/* XSPI register bit definitions *******************************************/

/* Control Register */

#define XSPI_CR_EN                 (1 << 0)   /* Bit 0:  XSPI Enable */
#define XSPI_CR_ABORT              (1 << 1)   /* Bit 1:  Abort request */
#define XSPI_CR_DMAEN              (1 << 2)   /* Bit 2:  DMA enable */
#define XSPI_CR_TCEN               (1 << 3)   /* Bit 3:  Timeout counter enable */
#define XSPI_CR_DMM                (1 << 6)   /* Bit 6:  Dual-memory mode */
#define XSPI_CR_MSEL               (1 << 7)   /* Bit 7:  Memory selection */
#define XSPI_CR_FTHRES_SHIFT       (8)        /* Bits 8-12: FIFO threshold level */
#define XSPI_CR_FTHRES_MASK        (0x1f << XSPI_CR_FTHRES_SHIFT)
#define XSPI_CR_TEIE               (1 << 16)  /* Bit 16: Transfer error interrupt enable */
#define XSPI_CR_TCIE               (1 << 17)  /* Bit 17: Transfer complete interrupt enable */
#define XSPI_CR_FTIE               (1 << 18)  /* Bit 18: FIFO threshold interrupt enable */
#define XSPI_CR_SMIE               (1 << 19)  /* Bit 19: Status match interrupt enable */
#define XSPI_CR_TOIE               (1 << 20)  /* Bit 20: Timeout interrupt enable */
#define XSPI_CR_APMS               (1 << 22)  /* Bit 22: Automatic poll mode stop */
#define XSPI_CR_PMM                (1 << 23)  /* Bit 23: Polling match mode */

#define XSPI_CR_FMODE_SHIFT        (28)       /* Bits 28-29: Functional mode */
#define XSPI_CR_FMODE_MASK         (0x3 << XSPI_CR_FMODE_SHIFT)
#  define XSPI_CR_FMODE(n)         ((uint32_t)(n) << XSPI_CR_FMODE_SHIFT)

#define XSPI_FMODE_INDWR           0   /* Indirect write mode */
#define XSPI_FMODE_INDRD           1   /* Indirect read mode */
#define XSPI_FMODE_AUTOPOLL        2   /* Automatic polling mode */
#define XSPI_FMODE_MEMMAP          3   /* Memory-mapped mode */

/* Device Configuration Register 1 */

#define XSPI_DCR1_CKMODE            (1 << 0)   /* Bit 0:  Mode 0 / mode 3 */
#define XSPI_DCR1_FRCK              (1 << 1)   /* Bit 1:  Free Running Clock */
#define XSPI_DCR1_DLYBYP            (1 << 2)   /* Bit 2:  Delay block bypass */

#define XSPI_DCR1_CSHT_SHIFT        (8)        /* Bits 8-13: Chip select high time */
#define XSPI_DCR1_CSHT_MASK         (0x3f << XSPI_DCR1_CSHT_SHIFT)
#define XSPI_DCR1_DEVSIZE_SHIFT     (16)       /* Bits 16-20: Device size */
#define XSPI_DCR1_DEVSIZE_MASK      (0x1f << XSPI_DCR1_DEVSIZE_SHIFT)
#define XSPI_DCR1_MTYP_SHIFT        (24)       /* Bits 24-26: Memory Type */
#define XSPI_DCR1_MTYP_MASK         (0x7 << XSPI_DCR1_MTYP_SHIFT)
#define XSPI_DCR1_MTYP_MICRON       (0x0 << XSPI_DCR1_MTYP_SHIFT)
#define XSPI_DCR1_MTYP_MACRONIX     (0x1 << XSPI_DCR1_MTYP_SHIFT)
#define XSPI_DCR1_MTYP_STANDARD     (0x2 << XSPI_DCR1_MTYP_SHIFT)
#define XSPI_DCR1_MTYP_MACRNX_RAM   (0x3 << XSPI_DCR1_MTYP_SHIFT)
#define XSPI_DCR1_MTYP_HYPBUS_MEM   (0x4 << XSPI_DCR1_MTYP_SHIFT)
#define XSPI_DCR1_MTYP_HYPBUS_REG   (0x5 << XSPI_DCR1_MTYP_SHIFT)

/* Device Configuration Register 2 */

#define XSPI_DCR2_PRESCALER_SHIFT   (0)        /* Bits 0-7: Clock prescaler */
#define XSPI_DCR2_PRESCALER_MASK    (0xff << XSPI_DCR2_PRESCALER_SHIFT)

#define XSPI_DCR2_WRAPSIZE_SHIFT    (16)       /* Bits 16-18: Wrap Size */
#define XSPI_DCR2_WRAPSIZE_MASK     (0x7 << XSPI_DCR2_WRAPSIZE_SHIFT)

/* Device Configuration Register 3 */

#define XSPI_DCR3_CSBOUND_SHIFT     (16)       /* Bits 16-20: CS boundary */
#define XSPI_DCR3_CSBOUND_MASK      (0x1f << XSPI_DCR3_CSBOUND_SHIFT)

/* Status Register */

#define XSPI_SR_TEF                 (1 << 0)   /* Bit 0:  Transfer error flag */
#define XSPI_SR_TCF                 (1 << 1)   /* Bit 1:  Transfer complete flag */
#define XSPI_SR_FTF                 (1 << 2)   /* Bit 2:  FIFO threshold flag */
#define XSPI_SR_SMF                 (1 << 3)   /* Bit 3:  Status match flag */
#define XSPI_SR_TOF                 (1 << 4)   /* Bit 4:  Timeout flag */
#define XSPI_SR_BUSY                (1 << 5)   /* Bit 5:  Busy */
#define XSPI_SR_FLEVEL_SHIFT        (8)        /* Bits 8-13: FIFO level */
#define XSPI_SR_FLEVEL_MASK         (0x3f << XSPI_SR_FLEVEL_SHIFT)

/* Flag Clear Register */

#define XSPI_FCR_CTEF               (1 << 0)   /* Bit 0:  Clear transfer error flag */
#define XSPI_FCR_CTCF               (1 << 1)   /* Bit 1:  Clear transfer complete flag */
#define XSPI_FCR_CSMF               (1 << 3)   /* Bit 3:  Clear status match flag */
#define XSPI_FCR_CTOF               (1 << 4)   /* Bit 4:  Clear timeout flag */

/* Communication Configuration Register (CCR) */

#define XSPI_CCR_IMODE_SHIFT        (0)        /* Bits 0-2: Instruction mode */
#define XSPI_CCR_IMODE_MASK         (0x7 << XSPI_CCR_IMODE_SHIFT)
#  define XSPI_CCR_IMODE(n)         ((uint32_t)(n) << XSPI_CCR_IMODE_SHIFT)

#define XSPI_CCR_IDTR               (1 << 3)   /* Bit 3:  Instruction DTR */

#define XSPI_CCR_ISIZE_SHIFT        (4)        /* Bits 4-5: Instruction size */
#define XSPI_CCR_ISIZE_MASK         (0x3 << XSPI_CCR_ISIZE_SHIFT)
#  define XSPI_CCR_ISIZE_8b         (0 << XSPI_CCR_ISIZE_SHIFT)
#  define XSPI_CCR_ISIZE_16b        (1 << XSPI_CCR_ISIZE_SHIFT)
#  define XSPI_CCR_ISIZE_24b        (2 << XSPI_CCR_ISIZE_SHIFT)
#  define XSPI_CCR_ISIZE_32b        (3 << XSPI_CCR_ISIZE_SHIFT)

#define XSPI_CCR_ADMODE_SHIFT       (8)        /* Bits 8-10: Address mode */
#define XSPI_CCR_ADMODE_MASK        (0x7 << XSPI_CCR_ADMODE_SHIFT)
#  define XSPI_CCR_ADMODE(n)        ((uint32_t)(n) << XSPI_CCR_ADMODE_SHIFT)

#define XSPI_CCR_ADDTR              (1 << 11)  /* Bit 11: Address DTR */

#define XSPI_CCR_ADSIZE_SHIFT       (12)       /* Bits 12-13: Address size */
#define XSPI_CCR_ADSIZE_MASK        (0x3 << XSPI_CCR_ADSIZE_SHIFT)
#  define XSPI_CCR_ADSIZE(n)        ((uint32_t)(n) << XSPI_CCR_ADSIZE_SHIFT)
#  define XSPI_CCR_ADSIZE_8b        (0 << XSPI_CCR_ADSIZE_SHIFT)
#  define XSPI_CCR_ADSIZE_16b       (1 << XSPI_CCR_ADSIZE_SHIFT)
#  define XSPI_CCR_ADSIZE_24b       (2 << XSPI_CCR_ADSIZE_SHIFT)
#  define XSPI_CCR_ADSIZE_32b       (3 << XSPI_CCR_ADSIZE_SHIFT)

#define XSPI_CCR_ABMODE_SHIFT       (16)       /* Bits 16-18: Alternate bytes mode */
#define XSPI_CCR_ABMODE_MASK        (0x7 << XSPI_CCR_ABMODE_SHIFT)
#  define XSPI_CCR_ABMODE(n)        ((uint32_t)(n) << XSPI_CCR_ABMODE_SHIFT)

#define XSPI_CCR_ABDTR              (1 << 19)  /* Bit 19: Alternate-byte DTR */

#define XSPI_CCR_ABSIZE_SHIFT       (20)       /* Bits 20-21: Alternate bytes size */
#define XSPI_CCR_ABSIZE_MASK        (0x3 << XSPI_CCR_ABSIZE_SHIFT)
#  define XSPI_CCR_ABSIZE(n)        ((uint32_t)(n) << XSPI_CCR_ABSIZE_SHIFT)

#define XSPI_CCR_DMODE_SHIFT        (24)       /* Bits 24-26: Data mode */
#define XSPI_CCR_DMODE_MASK         (0x7 << XSPI_CCR_DMODE_SHIFT)
#  define XSPI_CCR_DMODE(n)         ((uint32_t)(n) << XSPI_CCR_DMODE_SHIFT)

#define XSPI_CCR_DDTR               (1 << 27)  /* Bit 27: Data DTR */
#define XSPI_CCR_DQSE               (1 << 29)  /* Bit 29: DQS enable */

/* Mode field values (shared by CCR/WPCCR/WCCR) */

#define CCR_IMODE_NONE               0   /* No instruction */
#define CCR_IMODE_SINGLE             1   /* Instruction on a single line */
#define CCR_IMODE_DUAL               2   /* Instruction on two lines */
#define CCR_IMODE_QUAD               3   /* Instruction on four lines */
#define CCR_IMODE_OCTAL              4   /* Instruction on eight lines */

#define CCR_ADMODE_NONE              0   /* No address */
#define CCR_ADMODE_SINGLE            1   /* Address on a single line */
#define CCR_ADMODE_DUAL              2   /* Address on two lines */
#define CCR_ADMODE_QUAD              3   /* Address on four lines */
#define CCR_ADMODE_OCTAL             4   /* Address on eight lines */

#define CCR_ADSIZE_8                 0   /* 8-bit address */
#define CCR_ADSIZE_16                1   /* 16-bit address */
#define CCR_ADSIZE_24                2   /* 24-bit address */
#define CCR_ADSIZE_32                3   /* 32-bit address */

#define CCR_ABMODE_NONE              0   /* No alternate bytes */
#define CCR_ABMODE_SINGLE            1   /* Alternate bytes on a single line */
#define CCR_ABMODE_DUAL              2   /* Alternate bytes on two lines */
#define CCR_ABMODE_QUAD              3   /* Alternate bytes on four lines */
#define CCR_ABMODE_OCTAL             4   /* Alternate bytes on eight lines */

#define CCR_ABSIZE_8                 0   /* 8-bit alternate byte */
#define CCR_ABSIZE_16                1   /* 16-bit alternate bytes */
#define CCR_ABSIZE_24                2   /* 24-bit alternate bytes */
#define CCR_ABSIZE_32                3   /* 32-bit alternate bytes */

#define CCR_DMODE_NONE               0   /* No data */
#define CCR_DMODE_SINGLE             1   /* Data on a single line */
#define CCR_DMODE_DUAL               2   /* Data on two lines */
#define CCR_DMODE_QUAD               3   /* Data on four lines */
#define CCR_DMODE_OCTAL              4   /* Data on eight lines */

/* Timing Configuration Register */

#define XSPI_TCR_DCYC_SHIFT         (0)        /* Bits 0-4: Number of dummy cycles */
#define XSPI_TCR_DCYC_MASK          (0x1f << XSPI_TCR_DCYC_SHIFT)
#  define XSPI_TCR_DCYC(n)          ((uint32_t)(n) << XSPI_TCR_DCYC_SHIFT)

#define XSPI_TCR_DHQC               (1 << 28)  /* Bit 28: Delay hold quarter cycle */
#define XSPI_TCR_SSHIFT             (1 << 30)  /* Bit 30: Sample shift */

/* Polling Interval Register */

#define XSPI_PIR_INTERVAL_SHIFT     (0)        /* Bits 0-15: Polling interval */
#define XSPI_PIR_INTERVAL_MASK      (0xffff << XSPI_PIR_INTERVAL_SHIFT)

/* Low-Power Timeout Register */

#define XSPI_LPTR_TIMEOUT_SHIFT     (0)        /* Bits 0-15: Timeout period */
#define XSPI_LPTR_TIMEOUT_MASK      (0xffff << XSPI_LPTR_TIMEOUT_SHIFT)

/* HyperBus Latency Configuration Register */

#define XSPI_HLCR_LM                (1 << 0)   /* Bit 0: Latency Mode */
#define XSPI_HLCR_WZL               (1 << 1)   /* Bit 1: Write Zero Latency */
#define XSPI_HLCR_TACC_SHIFT        (8)        /* Bits 8-15: Access Timing */
#define XSPI_HLCR_TACC_MASK         (0xff << XSPI_HLCR_TACC_SHIFT)
#define XSPI_HLCR_TRWR_SHIFT        (16)       /* Bits 16-23: Read-write recovery time */
#define XSPI_HLCR_TRWR_MASK         (0xff << XSPI_HLCR_TRWR_SHIFT)

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Public Functions Prototypes
 ****************************************************************************/

#endif /* __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_XSPI_H */
