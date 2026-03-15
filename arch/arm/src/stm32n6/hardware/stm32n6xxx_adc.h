/****************************************************************************
 * arch/arm/src/stm32n6/hardware/stm32n6xxx_adc.h
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

#ifndef __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_ADC_H
#define __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_ADC_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include "chip.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Register Offsets *********************************************************/

#define STM32_ADC_ISR_OFFSET       0x0000  /* Interrupt and status register */
#define STM32_ADC_IER_OFFSET       0x0004  /* Interrupt enable register */
#define STM32_ADC_CR_OFFSET        0x0008  /* Control register */
#define STM32_ADC_CFGR1_OFFSET     0x000c  /* Configuration register 1 */
#define STM32_ADC_CFGR2_OFFSET     0x0010  /* Configuration register 2 */
#define STM32_ADC_SMPR1_OFFSET     0x0014  /* Sample time register 1 */
#define STM32_ADC_SMPR2_OFFSET     0x0018  /* Sample time register 2 */
#define STM32_ADC_PCSEL_OFFSET     0x001c  /* Channel preselection register */
#define STM32_ADC_SQR1_OFFSET      0x0030  /* Regular sequence register 1 */
#define STM32_ADC_SQR2_OFFSET      0x0034  /* Regular sequence register 2 */
#define STM32_ADC_SQR3_OFFSET      0x0038  /* Regular sequence register 3 */
#define STM32_ADC_SQR4_OFFSET      0x003c  /* Regular sequence register 4 */
#define STM32_ADC_DR_OFFSET        0x0040  /* Regular data register */
#define STM32_ADC_JSQR_OFFSET      0x004c  /* Injected sequence register */
#define STM32_ADC_OFCFGR1_OFFSET   0x0050  /* Offset configuration register 1 */
#define STM32_ADC_OFCFGR2_OFFSET   0x0054  /* Offset configuration register 2 */
#define STM32_ADC_OFCFGR3_OFFSET   0x0058  /* Offset configuration register 3 */
#define STM32_ADC_OFCFGR4_OFFSET   0x005c  /* Offset configuration register 4 */
#define STM32_ADC_OFR1_OFFSET      0x0060  /* Offset register 1 */
#define STM32_ADC_OFR2_OFFSET      0x0064  /* Offset register 2 */
#define STM32_ADC_OFR3_OFFSET      0x0068  /* Offset register 3 */
#define STM32_ADC_OFR4_OFFSET      0x006c  /* Offset register 4 */
#define STM32_ADC_GCOMP_OFFSET     0x0070  /* Gain compensation register */
#define STM32_ADC_JDR1_OFFSET      0x0080  /* Injected data register 1 */
#define STM32_ADC_JDR2_OFFSET      0x0084  /* Injected data register 2 */
#define STM32_ADC_JDR3_OFFSET      0x0088  /* Injected data register 3 */
#define STM32_ADC_JDR4_OFFSET      0x008c  /* Injected data register 4 */
#define STM32_ADC_AWD2CR_OFFSET    0x00a0  /* Analog watchdog 2 config register */
#define STM32_ADC_AWD3CR_OFFSET    0x00a4  /* Analog watchdog 3 config register */
#define STM32_ADC_AWD1LTR_OFFSET   0x00a8  /* AWD1 low threshold register */
#define STM32_ADC_AWD1HTR_OFFSET   0x00ac  /* AWD1 high threshold register */
#define STM32_ADC_AWD2LTR_OFFSET   0x00b0  /* AWD2 low threshold register */
#define STM32_ADC_AWD2HTR_OFFSET   0x00b4  /* AWD2 high threshold register */
#define STM32_ADC_AWD3LTR_OFFSET   0x00b8  /* AWD3 low threshold register */
#define STM32_ADC_AWD3HTR_OFFSET   0x00bc  /* AWD3 high threshold register */
#define STM32_ADC_DIFSEL_OFFSET    0x00c0  /* Differential mode selection register */
#define STM32_ADC_CALFACT_OFFSET   0x00c4  /* Calibration factors register */
#define STM32_ADC_OR_OFFSET        0x00d0  /* Option register */

/* Common registers (offset from ADC12_COMMON base) */

#define STM32_ADC_CSR_OFFSET       0x0000  /* Common status register */
#define STM32_ADC_CCR_OFFSET       0x0008  /* Common control register */
#define STM32_ADC_CDR_OFFSET       0x000c  /* Common regular data register */
#define STM32_ADC_CDR2_OFFSET      0x0010  /* Common regular data register 2 */

/* Register Addresses (ADC1) ************************************************/

#define STM32_ADC1_ISR        (STM32_ADC1_BASE + STM32_ADC_ISR_OFFSET)
#define STM32_ADC1_IER        (STM32_ADC1_BASE + STM32_ADC_IER_OFFSET)
#define STM32_ADC1_CR         (STM32_ADC1_BASE + STM32_ADC_CR_OFFSET)
#define STM32_ADC1_CFGR1      (STM32_ADC1_BASE + STM32_ADC_CFGR1_OFFSET)
#define STM32_ADC1_CFGR2      (STM32_ADC1_BASE + STM32_ADC_CFGR2_OFFSET)
#define STM32_ADC1_SMPR1      (STM32_ADC1_BASE + STM32_ADC_SMPR1_OFFSET)
#define STM32_ADC1_SMPR2      (STM32_ADC1_BASE + STM32_ADC_SMPR2_OFFSET)
#define STM32_ADC1_PCSEL      (STM32_ADC1_BASE + STM32_ADC_PCSEL_OFFSET)
#define STM32_ADC1_SQR1       (STM32_ADC1_BASE + STM32_ADC_SQR1_OFFSET)
#define STM32_ADC1_SQR2       (STM32_ADC1_BASE + STM32_ADC_SQR2_OFFSET)
#define STM32_ADC1_SQR3       (STM32_ADC1_BASE + STM32_ADC_SQR3_OFFSET)
#define STM32_ADC1_SQR4       (STM32_ADC1_BASE + STM32_ADC_SQR4_OFFSET)
#define STM32_ADC1_DR         (STM32_ADC1_BASE + STM32_ADC_DR_OFFSET)
#define STM32_ADC1_DIFSEL     (STM32_ADC1_BASE + STM32_ADC_DIFSEL_OFFSET)
#define STM32_ADC1_CALFACT    (STM32_ADC1_BASE + STM32_ADC_CALFACT_OFFSET)

/* Common Register Addresses ************************************************/

#define STM32_ADC12_CSR       (STM32_ADC12_COMMON_BASE + STM32_ADC_CSR_OFFSET)
#define STM32_ADC12_CCR       (STM32_ADC12_COMMON_BASE + STM32_ADC_CCR_OFFSET)
#define STM32_ADC12_CDR       (STM32_ADC12_COMMON_BASE + STM32_ADC_CDR_OFFSET)

/* Register Bitfield Definitions ********************************************/

/* ADC Interrupt and Status Register (ISR) */

#define ADC_ISR_ADRDY          (1 << 0)   /* Bit 0:  ADC ready */
#define ADC_ISR_EOSMP          (1 << 1)   /* Bit 1:  End of sampling */
#define ADC_ISR_EOC            (1 << 2)   /* Bit 2:  End of conversion */
#define ADC_ISR_EOS            (1 << 3)   /* Bit 3:  End of regular sequence */
#define ADC_ISR_OVR            (1 << 4)   /* Bit 4:  Overrun */
#define ADC_ISR_JEOC           (1 << 5)   /* Bit 5:  End of injected conversion */
#define ADC_ISR_JEOS           (1 << 6)   /* Bit 6:  End of injected sequence */
#define ADC_ISR_AWD1           (1 << 7)   /* Bit 7:  Analog watchdog 1 */
#define ADC_ISR_AWD2           (1 << 8)   /* Bit 8:  Analog watchdog 2 */
#define ADC_ISR_AWD3           (1 << 9)   /* Bit 9:  Analog watchdog 3 */

#define ADC_ISR_ALLINTS        (ADC_ISR_ADRDY | ADC_ISR_EOSMP | \
                                ADC_ISR_EOC | ADC_ISR_EOS | ADC_ISR_OVR | \
                                ADC_ISR_JEOC | ADC_ISR_JEOS | \
                                ADC_ISR_AWD1 | ADC_ISR_AWD2 | ADC_ISR_AWD3)

/* ADC Interrupt Enable Register (IER) */

#define ADC_IER_ADRDYIE        (1 << 0)   /* Bit 0:  ADC ready interrupt */
#define ADC_IER_EOSMPIE        (1 << 1)   /* Bit 1:  End of sampling interrupt */
#define ADC_IER_EOCIE          (1 << 2)   /* Bit 2:  End of conversion interrupt */
#define ADC_IER_EOSIE          (1 << 3)   /* Bit 3:  End of regular sequence interrupt */
#define ADC_IER_OVRIE          (1 << 4)   /* Bit 4:  Overrun interrupt */
#define ADC_IER_JEOCIE         (1 << 5)   /* Bit 5:  End of injected conversion int */
#define ADC_IER_JEOSIE         (1 << 6)   /* Bit 6:  End of injected sequence int */
#define ADC_IER_AWD1IE         (1 << 7)   /* Bit 7:  Analog watchdog 1 interrupt */
#define ADC_IER_AWD2IE         (1 << 8)   /* Bit 8:  Analog watchdog 2 interrupt */
#define ADC_IER_AWD3IE         (1 << 9)   /* Bit 9:  Analog watchdog 3 interrupt */

/* ADC Control Register (CR) */

#define ADC_CR_ADEN            (1 << 0)   /* Bit 0:  ADC enable */
#define ADC_CR_ADDIS           (1 << 1)   /* Bit 1:  ADC disable */
#define ADC_CR_ADSTART         (1 << 2)   /* Bit 2:  ADC regular start */
#define ADC_CR_JADSTART        (1 << 3)   /* Bit 3:  ADC injected start */
#define ADC_CR_ADSTP           (1 << 4)   /* Bit 4:  ADC regular stop */
#define ADC_CR_JADSTP          (1 << 5)   /* Bit 5:  ADC injected stop */
                                           /* Bits 6-28: Reserved */
#define ADC_CR_DEEPPWD         (1 << 29)  /* Bit 29: Deep power down */
#define ADC_CR_ADCALDIF        (1 << 30)  /* Bit 30: Differential calibration */
#define ADC_CR_ADCAL           (1 << 31)  /* Bit 31: ADC calibration */

/* NOTE: STM32N6 has NO ADVREGEN bit in CR (unlike H5).
 * To exit deep power down, just clear DEEPPWD and wait 20us.
 */

/* ADC Configuration Register 1 (CFGR1) — different layout from H5 CFGR! */

#define ADC_CFGR1_DMNGT_SHIFT  (0)        /* Bits 1:0: Data management config */
#define ADC_CFGR1_DMNGT_MASK   (3 << ADC_CFGR1_DMNGT_SHIFT)
#  define ADC_CFGR1_DMNGT_DR   (0 << ADC_CFGR1_DMNGT_SHIFT) /* Data stored in DR */
#  define ADC_CFGR1_DMNGT_DMA1 (1 << ADC_CFGR1_DMNGT_SHIFT) /* DMA one-shot */
#  define ADC_CFGR1_DMNGT_DFSDM (2 << ADC_CFGR1_DMNGT_SHIFT) /* DFSDM mode */
#  define ADC_CFGR1_DMNGT_DMA3 (3 << ADC_CFGR1_DMNGT_SHIFT) /* DMA circular */

#define ADC_CFGR1_RES_SHIFT    (2)        /* Bits 3:2: Data resolution */
#define ADC_CFGR1_RES_MASK     (3 << ADC_CFGR1_RES_SHIFT)
#  define ADC_CFGR1_RES_12BIT  (0 << ADC_CFGR1_RES_SHIFT) /* 12-bit */
#  define ADC_CFGR1_RES_10BIT  (1 << ADC_CFGR1_RES_SHIFT) /* 10-bit */
#  define ADC_CFGR1_RES_8BIT   (2 << ADC_CFGR1_RES_SHIFT) /* 8-bit */
#  define ADC_CFGR1_RES_6BIT   (3 << ADC_CFGR1_RES_SHIFT) /* 6-bit */

#define ADC_CFGR1_EXTSEL_SHIFT (5)        /* Bits 9:5: External trigger selection */
#define ADC_CFGR1_EXTSEL_MASK  (0x1f << ADC_CFGR1_EXTSEL_SHIFT)

#define ADC_CFGR1_EXTEN_SHIFT  (10)       /* Bits 11:10: External trigger enable */
#define ADC_CFGR1_EXTEN_MASK   (3 << ADC_CFGR1_EXTEN_SHIFT)
#  define ADC_CFGR1_EXTEN_NONE (0 << ADC_CFGR1_EXTEN_SHIFT) /* Software trigger */
#  define ADC_CFGR1_EXTEN_RISE (1 << ADC_CFGR1_EXTEN_SHIFT) /* Rising edge */
#  define ADC_CFGR1_EXTEN_FALL (2 << ADC_CFGR1_EXTEN_SHIFT) /* Falling edge */
#  define ADC_CFGR1_EXTEN_BOTH (3 << ADC_CFGR1_EXTEN_SHIFT) /* Both edges */

#define ADC_CFGR1_OVRMOD       (1 << 12)  /* Bit 12: Overrun mode */
#define ADC_CFGR1_CONT         (1 << 13)  /* Bit 13: Continuous conversion mode */
#define ADC_CFGR1_AUTDLY       (1 << 14)  /* Bit 14: Auto delayed conversion */

#define ADC_CFGR1_DISCEN       (1 << 16)  /* Bit 16: Discontinuous mode */
#define ADC_CFGR1_DISCNUM_SHIFT (17)      /* Bits 19:17: Discontinuous count */
#define ADC_CFGR1_DISCNUM_MASK (7 << ADC_CFGR1_DISCNUM_SHIFT)

#define ADC_CFGR1_JDISCEN      (1 << 20)  /* Bit 20: Injected discontinuous mode */

#define ADC_CFGR1_AWD1SGL      (1 << 22)  /* Bit 22: AWD1 single channel */
#define ADC_CFGR1_AWD1EN       (1 << 23)  /* Bit 23: AWD1 enable on regular */
#define ADC_CFGR1_JAWD1EN      (1 << 24)  /* Bit 24: AWD1 enable on injected */
#define ADC_CFGR1_JAUTO        (1 << 25)  /* Bit 25: Auto injected group */
#define ADC_CFGR1_AWD1CH_SHIFT (26)       /* Bits 30:26: AWD1 channel selection */
#define ADC_CFGR1_AWD1CH_MASK  (0x1f << ADC_CFGR1_AWD1CH_SHIFT)

/* ADC Configuration Register 2 (CFGR2) */

#define ADC_CFGR2_ROVSE        (1 << 0)   /* Bit 0:  Regular oversampling enable */
#define ADC_CFGR2_JOVSE        (1 << 1)   /* Bit 1:  Injected oversampling enable */

#define ADC_CFGR2_OVSS_SHIFT   (5)        /* Bits 8:5: Oversampling right shift */
#define ADC_CFGR2_OVSS_MASK    (0xf << ADC_CFGR2_OVSS_SHIFT)

#define ADC_CFGR2_TROVS        (1 << 9)   /* Bit 9:  Triggered oversampling */
#define ADC_CFGR2_ROVSM        (1 << 10)  /* Bit 10: Oversampling mode */

#define ADC_CFGR2_OVSR_SHIFT   (16)       /* Bits 25:16: Oversampling ratio */
#define ADC_CFGR2_OVSR_MASK    (0x3ff << ADC_CFGR2_OVSR_SHIFT)

#define ADC_CFGR2_LSHIFT_SHIFT (28)       /* Bits 31:28: Left shift factor */
#define ADC_CFGR2_LSHIFT_MASK  (0xf << ADC_CFGR2_LSHIFT_SHIFT)

/* ADC Sample Time Register 1 (SMPR1) — channels 0-9, 3 bits each */

#define ADC_SMPR1_SMP_SHIFT(ch)  ((ch) * 3)
#define ADC_SMPR1_SMP_MASK(ch)   (7 << ADC_SMPR1_SMP_SHIFT(ch))

#define ADC_SMPR_1p5           0   /* 1.5 ADC clock cycles */
#define ADC_SMPR_2p5           1   /* 2.5 ADC clock cycles */
#define ADC_SMPR_6p5           2   /* 6.5 ADC clock cycles */
#define ADC_SMPR_11p5          3   /* 11.5 ADC clock cycles */
#define ADC_SMPR_23p5          4   /* 23.5 ADC clock cycles */
#define ADC_SMPR_46p5          5   /* 46.5 ADC clock cycles */
#define ADC_SMPR_246p5         6   /* 246.5 ADC clock cycles */
#define ADC_SMPR_1499p5        7   /* 1499.5 ADC clock cycles */

/* ADC Sample Time Register 2 (SMPR2) — channels 10-19, 3 bits each */

#define ADC_SMPR2_SMP_SHIFT(ch)  (((ch) - 10) * 3)
#define ADC_SMPR2_SMP_MASK(ch)   (7 << ADC_SMPR2_SMP_SHIFT(ch))

/* ADC Channel Preselection Register (PCSEL) — one bit per channel */

#define ADC_PCSEL_PCSEL(ch)    (1 << (ch))

/* ADC Regular Sequence Register 1 (SQR1) */

#define ADC_SQR1_L_SHIFT       (0)        /* Bits 3:0: Sequence length */
#define ADC_SQR1_L_MASK        (0xf << ADC_SQR1_L_SHIFT)

#define ADC_SQR1_SQ1_SHIFT     (6)        /* Bits 10:6: 1st conversion */
#define ADC_SQR1_SQ1_MASK      (0x1f << ADC_SQR1_SQ1_SHIFT)
#define ADC_SQR1_SQ2_SHIFT     (12)       /* Bits 16:12: 2nd conversion */
#define ADC_SQR1_SQ2_MASK      (0x1f << ADC_SQR1_SQ2_SHIFT)
#define ADC_SQR1_SQ3_SHIFT     (18)       /* Bits 22:18: 3rd conversion */
#define ADC_SQR1_SQ3_MASK      (0x1f << ADC_SQR1_SQ3_SHIFT)
#define ADC_SQR1_SQ4_SHIFT     (24)       /* Bits 28:24: 4th conversion */
#define ADC_SQR1_SQ4_MASK      (0x1f << ADC_SQR1_SQ4_SHIFT)

/* ADC Regular Sequence Register 2 (SQR2) */

#define ADC_SQR2_SQ5_SHIFT     (0)        /* Bits 4:0: 5th conversion */
#define ADC_SQR2_SQ5_MASK      (0x1f << ADC_SQR2_SQ5_SHIFT)
#define ADC_SQR2_SQ6_SHIFT     (6)        /* Bits 10:6: 6th conversion */
#define ADC_SQR2_SQ6_MASK      (0x1f << ADC_SQR2_SQ6_SHIFT)
#define ADC_SQR2_SQ7_SHIFT     (12)       /* Bits 16:12: 7th conversion */
#define ADC_SQR2_SQ7_MASK      (0x1f << ADC_SQR2_SQ7_SHIFT)
#define ADC_SQR2_SQ8_SHIFT     (18)       /* Bits 22:18: 8th conversion */
#define ADC_SQR2_SQ8_MASK      (0x1f << ADC_SQR2_SQ8_SHIFT)
#define ADC_SQR2_SQ9_SHIFT     (24)       /* Bits 28:24: 9th conversion */
#define ADC_SQR2_SQ9_MASK      (0x1f << ADC_SQR2_SQ9_SHIFT)

/* ADC Regular Sequence Register 3 (SQR3) */

#define ADC_SQR3_SQ10_SHIFT    (0)
#define ADC_SQR3_SQ10_MASK     (0x1f << ADC_SQR3_SQ10_SHIFT)
#define ADC_SQR3_SQ11_SHIFT    (6)
#define ADC_SQR3_SQ11_MASK     (0x1f << ADC_SQR3_SQ11_SHIFT)
#define ADC_SQR3_SQ12_SHIFT    (12)
#define ADC_SQR3_SQ12_MASK     (0x1f << ADC_SQR3_SQ12_SHIFT)
#define ADC_SQR3_SQ13_SHIFT    (18)
#define ADC_SQR3_SQ13_MASK     (0x1f << ADC_SQR3_SQ13_SHIFT)
#define ADC_SQR3_SQ14_SHIFT    (24)
#define ADC_SQR3_SQ14_MASK     (0x1f << ADC_SQR3_SQ14_SHIFT)

/* ADC Regular Sequence Register 4 (SQR4) */

#define ADC_SQR4_SQ15_SHIFT    (0)
#define ADC_SQR4_SQ15_MASK     (0x1f << ADC_SQR4_SQ15_SHIFT)
#define ADC_SQR4_SQ16_SHIFT    (6)
#define ADC_SQR4_SQ16_MASK     (0x1f << ADC_SQR4_SQ16_SHIFT)

/* ADC Common Control Register (CCR) */

#define ADC_CCR_DUAL_SHIFT     (0)        /* Bits 4:0: Dual ADC mode */
#define ADC_CCR_DUAL_MASK      (0x1f << ADC_CCR_DUAL_SHIFT)
#  define ADC_CCR_DUAL_INDEP   (0 << ADC_CCR_DUAL_SHIFT) /* Independent mode */

#define ADC_CCR_DELAY_SHIFT    (8)        /* Bits 11:8: Delay between 2 phases */
#define ADC_CCR_DELAY_MASK     (0xf << ADC_CCR_DELAY_SHIFT)

#define ADC_CCR_DAMDF_SHIFT    (14)       /* Bits 15:14: Dual ADC data format */
#define ADC_CCR_DAMDF_MASK     (3 << ADC_CCR_DAMDF_SHIFT)

#define ADC_CCR_VREFEN         (1 << 22)  /* Bit 22: VrefInt channel enable */
#define ADC_CCR_VBATEN         (1 << 24)  /* Bit 24: VBAT channel enable */

/* NOTE: STM32N6 ADC CCR has NO PRESC or CKMODE fields.
 * ADC clock is selected entirely via RCC_CCIPR1 ADC12SEL.
 */

/* ADC Common Status Register (CSR) — master (ADC1) in low 16 bits */

#define ADC_CSR_ADRDY_MST      (1 << 0)
#define ADC_CSR_EOC_MST        (1 << 2)
#define ADC_CSR_EOS_MST        (1 << 3)
#define ADC_CSR_OVR_MST        (1 << 4)

/* Maximum ADC channel number */

#define ADC_MAX_CHANNELS       20

/* Sequence register helpers */

#define ADC_SQR_MAX_FIRST      4    /* SQR1 holds SQ1-SQ4 */
#define ADC_SQR_MAX_SECOND     5    /* SQR2 holds SQ5-SQ9 */
#define ADC_SQR_MAX_THIRD      5    /* SQR3 holds SQ10-SQ14 */
#define ADC_SQR_MAX_FOURTH     2    /* SQR4 holds SQ15-SQ16 */

#endif /* __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_ADC_H */
