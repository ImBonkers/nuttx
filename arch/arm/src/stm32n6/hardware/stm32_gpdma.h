/****************************************************************************
 * arch/arm/src/stm32n6/hardware/stm32_gpdma.h
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

#ifndef __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32_GPDMA_H
#define __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32_GPDMA_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include "chip.h"
#include "stm32n6xx_dmasigmap.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Register Offsets *********************************************************/

#define STM32_GPDMA_SECCFGR_OFFSET      0x0000
#define STM32_GPDMA_PRIVCFGR_OFFSET     0x0004
#define STM32_GPDMA_RCFGLOCKR_OFFSET    0x0008
#define STM32_GPDMA_MISR_OFFSET         0x000C
#define STM32_GPDMA_SMISR_OFFSET        0x0010

/* Per-channel register offsets (0x80 stride per channel) */

#define STM32_GPDMA_CXLBAR_OFFSET(x)    (0x50+0x80*(x))
#define STM32_GPDMA_CXFCR_OFFSET(x)     (0x5C+0x80*(x))
#define STM32_GPDMA_CXSR_OFFSET(x)      (0x60+0x80*(x))
#define STM32_GPDMA_CXCR_OFFSET(x)      (0x64+0x80*(x))
#define STM32_GPDMA_CXTR1_OFFSET(x)     (0x90+0x80*(x))
#define STM32_GPDMA_CXTR2_OFFSET(x)     (0x94+0x80*(x))
#define STM32_GPDMA_CXBR1_OFFSET(x)     (0x98+0x80*(x))
#define STM32_GPDMA_CXSAR_OFFSET(x)     (0x9C+0x80*(x))
#define STM32_GPDMA_CXDAR_OFFSET(x)     (0xA0+0x80*(x))
#define STM32_GPDMA_CXTR3_OFFSET(x)     (0xA4+0x80*(x))
#define STM32_GPDMA_CXBR2_OFFSET(x)     (0xA8+0x80*(x))
#define STM32_GPDMA_CXLLR_OFFSET(x)     (0xCC+0x80*(x))

/* Named per-channel offsets for channels 0-15 */

#define STM32_GPDMA_C0LBAR_OFFSET   STM32_GPDMA_CXLBAR_OFFSET(0)
#define STM32_GPDMA_C1LBAR_OFFSET   STM32_GPDMA_CXLBAR_OFFSET(1)
#define STM32_GPDMA_C2LBAR_OFFSET   STM32_GPDMA_CXLBAR_OFFSET(2)
#define STM32_GPDMA_C3LBAR_OFFSET   STM32_GPDMA_CXLBAR_OFFSET(3)
#define STM32_GPDMA_C4LBAR_OFFSET   STM32_GPDMA_CXLBAR_OFFSET(4)
#define STM32_GPDMA_C5LBAR_OFFSET   STM32_GPDMA_CXLBAR_OFFSET(5)
#define STM32_GPDMA_C6LBAR_OFFSET   STM32_GPDMA_CXLBAR_OFFSET(6)
#define STM32_GPDMA_C7LBAR_OFFSET   STM32_GPDMA_CXLBAR_OFFSET(7)
#define STM32_GPDMA_C8LBAR_OFFSET   STM32_GPDMA_CXLBAR_OFFSET(8)
#define STM32_GPDMA_C9LBAR_OFFSET   STM32_GPDMA_CXLBAR_OFFSET(9)
#define STM32_GPDMA_C10LBAR_OFFSET  STM32_GPDMA_CXLBAR_OFFSET(10)
#define STM32_GPDMA_C11LBAR_OFFSET  STM32_GPDMA_CXLBAR_OFFSET(11)
#define STM32_GPDMA_C12LBAR_OFFSET  STM32_GPDMA_CXLBAR_OFFSET(12)
#define STM32_GPDMA_C13LBAR_OFFSET  STM32_GPDMA_CXLBAR_OFFSET(13)
#define STM32_GPDMA_C14LBAR_OFFSET  STM32_GPDMA_CXLBAR_OFFSET(14)
#define STM32_GPDMA_C15LBAR_OFFSET  STM32_GPDMA_CXLBAR_OFFSET(15)

/* Register Bitfield Definitions ********************************************/

/* Secure configuration register */

#define GPDMA_SECCFGR_SECX(x)       (1 << (x))

/* Privileged configuration register */

#define GPDMA_PRIVCFGR_PRIVX(x)     (1 << (x))

/* Configuration lock register */

#define GPDMA_RCFGLOCKR_LOCKX(x)    (1 << (x))

/* Nonsecure masked interrupt status register */

#define GPDMA_MISR_MISX(x)          (1 << (x))

/* Secure masked interrupt status register */

#define GPDMA_SMISR_MISX(x)         (1 << (x))

/* Channel x linked-list base address register */

#define GPDMA_CXLBAR_LBA_SHIFT        (16)
#define GPDMA_CXLBAR_LBA_MASK         (0xffff << GPDMA_CXLBAR_LBA_SHIFT)

/* Channel x flag clear register */

#define GPDMA_CXFCR_TCF               (1 << 8)
#define GPDMA_CXFCR_HTF               (1 << 9)
#define GPDMA_CXFCR_DTEF              (1 << 10)
#define GPDMA_CXFCR_ULEF              (1 << 11)
#define GPDMA_CXFCR_USEF              (1 << 12)
#define GPDMA_CXFCR_SUSPF             (1 << 13)
#define GPDMA_CXFCR_TOF               (1 << 14)

/* Channel x status register */

#define GPDMA_CXSR_IDLEF              (1 << 0)  /* Idle flag */
#define GPDMA_CXSR_TCF                (1 << 8)  /* Transfer complete flag */
#define GPDMA_CXSR_HTF                (1 << 9)  /* Half transfer flag */
#define GPDMA_CXSR_DTEF               (1 << 10) /* Data transfer error flag */
#define GPDMA_CXSR_ULEF               (1 << 11) /* Update link transfer error */
#define GPDMA_CXSR_USEF               (1 << 12) /* User setting error flag */
#define GPDMA_CXSR_SUSPF              (1 << 13) /* Completed suspension flag */
#define GPDMA_CXSR_TOF                (1 << 14) /* Trigger overrun flag */
#define GPDMA_CXSR_FIFOL_SHIFT        (16)
#define GPDMA_CXSR_FIFOL_MASK         (0xff << GPDMA_CXSR_FIFOL_SHIFT)

/* Channel x control register */

#define GPDMA_CXCR_EN                 (1 << 0)  /* Enable */
#define GPDMA_CXCR_RESET              (1 << 1)  /* Reset */
#define GPDMA_CXCR_SUSP               (1 << 2)  /* Suspend */
#define GPDMA_CXCR_TCIE               (1 << 8)  /* Transfer complete IE */
#define GPDMA_CXCR_HTIE               (1 << 9)  /* Half transfer IE */
#define GPDMA_CXCR_DTEIE              (1 << 10) /* Data transfer error IE */
#define GPDMA_CXCR_ULEIE              (1 << 11) /* Update link error IE */
#define GPDMA_CXCR_USEIE              (1 << 12) /* User setting error IE */
#define GPDMA_CXCR_SUSPEI             (1 << 13) /* Completed suspension IE */
#define GPDMA_CXCR_TOIE               (1 << 14) /* Trigger overrun IE */
#define GPDMA_CXCR_LSM                (1 << 16) /* Link step mode */
#define GPDMA_CXCR_LAP                (1 << 17) /* Linked-list allocated port */
#define GPDMA_CXCR_PRIO_SHIFT         (22)
#define GPDMA_CXCR_PRIO_MASK          (3 << GPDMA_CXCR_PRIO_SHIFT)

#define GPDMA_CXCR_ALLINTS            (GPDMA_CXCR_TOIE  | \
                                        GPDMA_CXCR_SUSPEI | \
                                        GPDMA_CXCR_USEIE  | \
                                        GPDMA_CXCR_ULEIE  | \
                                        GPDMA_CXCR_DTEIE  | \
                                        GPDMA_CXCR_HTIE   | \
                                        GPDMA_CXCR_TCIE)

/* Channel x transfer register 1 */

#define GPDMA_CXTR1_SDW_LOG2_SHIFT    (0)
#define GPDMA_CXTR1_SDW_LOG2_MASK     (3 << GPDMA_CXTR1_SDW_LOG2_SHIFT)
#  define GPDMA_CXTR1_SDW_LOG2_BYTE   (0 << GPDMA_CXTR1_SDW_LOG2_SHIFT)
#  define GPDMA_CXTR1_SDW_LOG2_HW     (1 << GPDMA_CXTR1_SDW_LOG2_SHIFT)
#  define GPDMA_CXTR1_SDW_LOG2_WORD   (2 << GPDMA_CXTR1_SDW_LOG2_SHIFT)

#define GPDMA_CXTR1_SINC              (1 << 3)  /* Source incrementing burst */
#define GPDMA_CXTR1_SBL_1_SHIFT       (4)
#define GPDMA_CXTR1_SBL_1_MASK        (0x3f << GPDMA_CXTR1_SBL_1_SHIFT)
#define GPDMA_CXTR1_SBL_1(n)          (((n)-1) << GPDMA_CXTR1_SBL_1_SHIFT)
#define GPDMA_CXTR1_PAM_SHIFT         (11)
#define GPDMA_CXTR1_PAM_MASK          (3 << GPDMA_CXTR1_PAM_SHIFT)
#define GPDMA_CXTR1_SBX               (1 << 13) /* Source byte exchange */
#define GPDMA_CXTR1_SAP               (1 << 14) /* Source allocated port */
#define GPDMA_CXTR1_SSEC              (1 << 15) /* Source security attribute */

#define GPDMA_CXTR1_DDW_LOG2_SHIFT    (16)
#define GPDMA_CXTR1_DDW_LOG2_MASK     (3 << GPDMA_CXTR1_DDW_LOG2_SHIFT)
#  define GPDMA_CXTR1_DDW_LOG2_BYTE   (0 << GPDMA_CXTR1_DDW_LOG2_SHIFT)
#  define GPDMA_CXTR1_DDW_LOG2_HW     (1 << GPDMA_CXTR1_DDW_LOG2_SHIFT)
#  define GPDMA_CXTR1_DDW_LOG2_WORD   (2 << GPDMA_CXTR1_DDW_LOG2_SHIFT)

#define GPDMA_CXTR1_DINC              (1 << 19) /* Destination incrementing */
#define GPDMA_CXTR1_DBL_1_SHIFT       (20)
#define GPDMA_CXTR1_DBL_1_MASK        (0x3f << GPDMA_CXTR1_DBL_1_SHIFT)
#define GPDMA_CXTR1_DBL_1(n)          (((n)-1) << GPDMA_CXTR1_DBL_1_SHIFT)
#define GPDMA_CXTR1_DBX               (1 << 26) /* Destination byte exchange */
#define GPDMA_CXTR1_DHX               (1 << 27) /* Destination half-word exchange */
#define GPDMA_CXTR1_DAP               (1 << 30) /* Destination allocated port */
#define GPDMA_CXTR1_DSEC              (1 << 31) /* Destination security attr */

/* Channel x transfer register 2 */

#define GPDMA_CXTR2_REQSEL_SHIFT      (0)
#define GPDMA_CXTR2_REQSEL_MASK       (0xff << GPDMA_CXTR2_REQSEL_SHIFT)
#define GPDMA_CXTR2_REQSEL(r)         ((r) << GPDMA_CXTR2_REQSEL_SHIFT)

#define GPDMA_CXTR2_SWREQ             (1 << 9)  /* Software request */
#define GPDMA_CXTR2_DREQ              (1 << 10) /* Destination hardware req */
#define GPDMA_CXTR2_BREQ              (1 << 11) /* Block hardware request */
#define GPDMA_CXTR2_PFREQ             (1 << 12) /* Peripheral flow control */

#define GPDMA_CXTR2_TRIGM_SHIFT       (14)
#define GPDMA_CXTR2_TRIGM_MASK        (3 << GPDMA_CXTR2_TRIGM_SHIFT)
#define GPDMA_CXTR2_TRIGSEL_SHIFT     (16)
#define GPDMA_CXTR2_TRIGSEL_MASK      (0x3f << GPDMA_CXTR2_TRIGSEL_SHIFT)

#define GPDMA_CXTR2_TRIGPOL_SHIFT     (24)
#define GPDMA_CXTR2_TRIGPOL_MASK      (3 << GPDMA_CXTR2_TRIGPOL_SHIFT)
#  define GPDMA_CXTR2_TRIGPOL_NONE    (0 << GPDMA_CXTR2_TRIGPOL_SHIFT)
#  define GPDMA_CXTR2_TRIGPOL_RISING  (1 << GPDMA_CXTR2_TRIGPOL_SHIFT)
#  define GPDMA_CXTR2_TRIGPOL_FALLING (2 << GPDMA_CXTR2_TRIGPOL_SHIFT)

#define GPDMA_CXTR2_TCEM_SHIFT        (30)
#define GPDMA_CXTR2_TCEM_MASK         (3 << GPDMA_CXTR2_TCEM_SHIFT)

/* Channel x block register 1 */

#define GPDMA_CXBR1_BNDT_SHIFT        (0)
#define GPDMA_CXBR1_BNDT_MASK         (0xffff << GPDMA_CXBR1_BNDT_SHIFT)
#define GPDMA_CXBR1_BRC_SHIFT         (16)
#define GPDMA_CXBR1_BRC_MASK          (0x7ff << GPDMA_CXBR1_BRC_SHIFT)
#define GPDMA_CXBR1_SDEC              (1 << 28) /* Source address decrement */
#define GPDMA_CXBR1_DDEC              (1 << 29) /* Dest address decrement */
#define GPDMA_CXBR1_BRSDEC            (1 << 30) /* Block repeat src decr */
#define GPDMA_CXBR1_BRDDEC            (1 << 31) /* Block repeat dest decr */

/* Channel x transfer register 3 */

#define GPDMA_CXTR3_SAO_SHIFT         (0)
#define GPDMA_CXTR3_SAO_MASK          (0x1fff << GPDMA_CXTR3_SAO_SHIFT)
#define GPDMA_CXTR3_DAO_SHIFT         (16)
#define GPDMA_CXTR3_DAO_MASK          (0x1fff << GPDMA_CXTR3_DAO_SHIFT)

/* Channel x block register 2 */

#define GPDMA_CXBR2_BRSAO_SHIFT       (0)
#define GPDMA_CXBR2_BRSAO_MASK        (0xffff << GPDMA_CXBR2_BRSAO_SHIFT)
#define GPDMA_CXBR2_BRDAO_SHIFT       (16)
#define GPDMA_CXBR2_BRDAO_MASK        (0xffff << GPDMA_CXBR2_BRDAO_SHIFT)

/* Channel x linked-list address register */

#define GPDMA_CXLLR_LA_SHIFT          (2)
#define GPDMA_CXLLR_LA_MASK           (0x3fff << GPDMA_CXLLR_LA_SHIFT)
#define GPDMA_CXLLR_ULL               (1 << 16) /* Update CxLLR from memory */
#define GPDMA_CXLLR_UDA               (1 << 27) /* Update CxDAR from memory */
#define GPDMA_CXLLR_USA               (1 << 28) /* Update CxSAR from memory */
#define GPDMA_CXLLR_UB1               (1 << 29) /* Update CxBR1 from memory */
#define GPDMA_CXLLR_UT2               (1 << 30) /* Update CxTR2 from memory */
#define GPDMA_CXLLR_UT1               (1 << 31) /* Update CxTR1 from memory */

#endif /* __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32_GPDMA_H */
