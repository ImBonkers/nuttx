/****************************************************************************
 * boards/arm/stm32n6/nucleo-n657x0-q/include/board.h
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

#ifndef __BOARDS_ARM_STM32N6_NUCLEO_N657X0_Q_INCLUDE_BOARD_H
#define __BOARDS_ARM_STM32N6_NUCLEO_N657X0_Q_INCLUDE_BOARD_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#ifndef __ASSEMBLY__
#  include <stdint.h>
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Clocking *****************************************************************/

/* PLL1 is configured for 600 MHz CPU clock:
 *
 *   HSI 64 MHz -> PLL1 (M=4, N=75, P1=1, P2=1) -> 1200 MHz
 *     IC1  /2  = 600 MHz  -> CPU (CPUSW = IC1)
 *     IC2  /3  = 400 MHz  \
 *     IC6  /4  = 300 MHz   > System bus (SYSSW)
 *     IC11 /3  = 400 MHz  /
 *     HPRE /2  = 200 MHz  -> HCLK
 *     PPRE1 /1 = 200 MHz  -> APB1 (PCLK1)
 *     PPRE2 /1 = 200 MHz  -> APB2 (PCLK2)
 *
 * With CONFIG_STM32N6_NPU, PLL2/PLL3 are also configured:
 *   HSI 64 MHz -> PLL2 (M=8, N=125) -> 1000 MHz
 *     IC6  /1  = 1000 MHz -> NPU core (replaces PLL1/4)
 *   HSI 64 MHz -> PLL3 (M=8, N=225, P2=2) -> 900 MHz
 *     IC11 /1  = 900 MHz  -> NPU SRAM (replaces PLL1/3)
 */

#define STM32_HSI_FREQUENCY     64000000ul
#define STM32_CPUCLK_FREQUENCY  600000000ul  /* IC1 = PLL1/2 = 600 MHz */
#define STM32_SYSCLK_FREQUENCY  400000000ul  /* IC2 = PLL1/3 = 400 MHz */
#define STM32_HCLK_FREQUENCY    200000000ul  /* SYSCLK / HPRE(2) = 200 MHz */
#define STM32_PCLK1_FREQUENCY   200000000ul  /* HCLK / PPRE1(1) = 200 MHz */
#define STM32_PCLK2_FREQUENCY   200000000ul  /* HCLK / PPRE2(1) = 200 MHz */

/* PLL2/PLL3 frequencies for NPU support */

#define STM32_PLL2_FREQUENCY      1000000000ul /* PLL2 VCO = 1000 MHz */
#define STM32_PLL3_FREQUENCY       900000000ul /* PLL3 output = 900 MHz */
#define STM32_NPU_CLK_FREQUENCY   1000000000ul /* IC6 = PLL2/1 = 1000 MHz */
#define STM32_NPUSRAM_FREQUENCY    900000000ul /* IC11 = PLL3/1 = 900 MHz */

/* Timer input clock.  On STM32N6, RCC_CFGR2.TIMPRE[25:24] selects the
 * timer group prescaler.  With TIMPRE=0 (default), timer clock = SYSCLK
 * (not HCLK or PCLK).  TIMPRE=1 would give SYSCLK/2, etc.
 */

#define STM32_APB1_TIM_FREQUENCY STM32_SYSCLK_FREQUENCY  /* 400 MHz */
#define STM32_APB2_TIM_FREQUENCY STM32_SYSCLK_FREQUENCY  /* 400 MHz */

/* ADC kernel clock = HSI (64 MHz) via RCC_CCIPR1 ADC12SEL */

#define STM32_ADC_CLK_FREQUENCY 64000000ul

/* LSI oscillator frequency (nominal 32 kHz) */

#define STM32_LSI_FREQUENCY 32000

#define BOARD_LOOPSPERMSEC      75000

/* LED definitions **********************************************************/

/* The Nucleo-N657X0-Q has three user-controllable LEDs (active-LOW):
 *
 *   LED  I/O   Color
 *   LD5  PG10  Red
 *   LD6  PG0   Green
 *   LD7  PG8   Blue
 *
 * - When the I/O is HIGH, the LED is OFF.
 * - When the I/O is LOW, the LED is ON.
 */

/* LED index values for use with board_userled() */

#define BOARD_LED1        0
#define BOARD_LED2        1
#define BOARD_LED3        2
#define BOARD_NLEDS       3

#define BOARD_LED_RED     BOARD_LED1
#define BOARD_LED_GREEN   BOARD_LED2
#define BOARD_LED_BLUE    BOARD_LED3

/* LED bits for use with board_userled_all() */

#define BOARD_LED1_BIT    (1 << BOARD_LED1)
#define BOARD_LED2_BIT    (1 << BOARD_LED2)
#define BOARD_LED3_BIT    (1 << BOARD_LED3)

/* If CONFIG_ARCH_LEDS is defined, the usage by the board port is defined
 * in include/board.h and src/stm32_autoleds.c.  The LEDs are used to
 * encode OS-related events as follows:
 *
 *                                     RED   GREEN BLUE
 *   LED_STARTED            0          OFF   OFF   OFF
 *   LED_HEAPALLOCATE       1          OFF   OFF   ON
 *   LED_IRQSENABLED        2          OFF   ON    OFF
 *   LED_STACKCREATED       3          OFF   ON    ON
 *   LED_INIRQ              4          GLOW  N/C   N/C
 *   LED_SIGNAL             5          N/C   GLOW  N/C
 *   LED_ASSERTION          6          GLOW  N/C   GLOW
 *   LED_PANIC              7          BLINK OFF   OFF
 *   LED_IDLE               8          OFF   OFF   OFF
 */

#define LED_STARTED        0
#define LED_HEAPALLOCATE   1
#define LED_IRQSENABLED    2
#define LED_STACKCREATED   3
#define LED_INIRQ          4
#define LED_SIGNAL         5
#define LED_ASSERTION      6
#define LED_PANIC          7
#define LED_IDLE           8

/* Button definitions *******************************************************/

/* The Nucleo-N657X0-Q supports one button: Pushbutton B1, labeled "User",
 * is connected to GPIO PC13.
 */

#define BUTTON_USER        0
#define NUM_BUTTONS        1
#define BUTTON_USER_BIT    (1 << BUTTON_USER)

/* Alternate function pin selections ****************************************/

/* USART1 GPIOs *************************************************************/

/* USART1 (Nucleo Virtual Console): PE5=TX (AF7), PE6=RX (AF7)
 * Connected to the on-board ST-Link to provide a Virtual COM Port.
 */

#define GPIO_USART1_TX   GPIO_USART1_TX_1
#define GPIO_USART1_RX   GPIO_USART1_RX_1

/* SPI GPIOs ****************************************************************/

/* SPI5: PE15=SCK (AF5), PG1=MISO (AF5), PG2=MOSI (AF5)
 * Arduino connector: D13=SCK, D12=MISO, D11=MOSI
 * Also on morpho CN15: pin11=SCK, pin13=MISO, pin15=MOSI
 */

#define GPIO_SPI5_SCK    GPIO_SPI5_SCK_2
#define GPIO_SPI5_MISO   GPIO_SPI5_MISO_2
#define GPIO_SPI5_MOSI   GPIO_SPI5_MOSI_2

/* I2C GPIOs ****************************************************************/

/* I2C1: PH9=SCL (AF4), PC1=SDA (AF4) — morpho CN15 pin3/pin5
 * Also Arduino D15/D14 with solder bridge mod (SB2+SB4 ON, SB3+SB5 OFF).
 */

#define GPIO_I2C1_SCL    GPIO_I2C1_SCL_2
#define GPIO_I2C1_SDA    GPIO_I2C1_SDA_2

/* I2C2: PB10=SCL (AF4), PB11=SDA (AF4) — TCPP03 + Arduino/morpho header */

#define GPIO_I2C2_SCL    GPIO_I2C2_SCL_1
#define GPIO_I2C2_SDA    GPIO_I2C2_SDA_1

/* PWM GPIOs ****************************************************************/

/* TIM1: PE9=CH1 (AF1) — Arduino D3 (CN13 pin 4)
 * This is the primary PWM output for testing.
 */

#define GPIO_TIM1_CH1OUT  GPIO_TIM1_CH1OUT_2
#define GPIO_TIM1_CH2OUT  GPIO_TIM1_CH2OUT_2
#define GPIO_TIM1_CH3OUT  GPIO_TIM1_CH3OUT_2
#define GPIO_TIM1_CH4OUT  GPIO_TIM1_CH4OUT_2

/* TIM3: PA6=CH1 (AF2) */

#define GPIO_TIM3_CH1OUT  GPIO_TIM3_CH1OUT_1
#define GPIO_TIM3_CH2OUT  GPIO_TIM3_CH2OUT_1
#define GPIO_TIM3_CH3OUT  GPIO_TIM3_CH3OUT_1
#define GPIO_TIM3_CH4OUT  GPIO_TIM3_CH4OUT_1

/* TIM4: PB6=CH1 (AF2), PB7=CH2, PB8=CH3, PB9=CH4 — morpho header */

#define GPIO_TIM4_CH1OUT  GPIO_TIM4_CH1OUT_1
#define GPIO_TIM4_CH2OUT  GPIO_TIM4_CH2OUT_1
#define GPIO_TIM4_CH3OUT  GPIO_TIM4_CH3OUT_1
#define GPIO_TIM4_CH4OUT  GPIO_TIM4_CH4OUT_1

/* XSPI2 GPIOs **************************************************************/

/* XSPI2: MX25UM51245G 64MB Octal NOR flash on GPION, all AF9.
 *   PN1=CS, PN6=CLK, PN2=D0(SI), PN3=D1(SO), PN4=D2, PN5=D3,
 *   PN8=D4, PN9=D5, PN10=D6, PN11=D7, PN0=DQS
 * Very high speed (100MHz) for all data/clock lines.
 */

#define GPIO_XSPI2_CS   (GPIO_ALT | GPIO_AF9 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTN | GPIO_PIN1)
#define GPIO_XSPI2_CLK  (GPIO_ALT | GPIO_AF9 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTN | GPIO_PIN6)
#define GPIO_XSPI2_D0   (GPIO_ALT | GPIO_AF9 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTN | GPIO_PIN2)
#define GPIO_XSPI2_D1   (GPIO_ALT | GPIO_AF9 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTN | GPIO_PIN3)
#define GPIO_XSPI2_D2   (GPIO_ALT | GPIO_AF9 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTN | GPIO_PIN4)
#define GPIO_XSPI2_D3   (GPIO_ALT | GPIO_AF9 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTN | GPIO_PIN5)
#define GPIO_XSPI2_D4   (GPIO_ALT | GPIO_AF9 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTN | GPIO_PIN8)
#define GPIO_XSPI2_D5   (GPIO_ALT | GPIO_AF9 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTN | GPIO_PIN9)
#define GPIO_XSPI2_D6   (GPIO_ALT | GPIO_AF9 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTN | GPIO_PIN10)
#define GPIO_XSPI2_D7   (GPIO_ALT | GPIO_AF9 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTN | GPIO_PIN11)
#define GPIO_XSPI2_DQS  (GPIO_ALT | GPIO_AF9 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTN | GPIO_PIN0)

/* XSPI2 kernel clock frequency: IC3 = PLL1/6 = 200MHz */

#define STM32_XSPI_FREQUENCY  200000000ul

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifndef __ASSEMBLY__

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_board_initialize
 *
 * Description:
 *   All STM32N6 architectures must provide the following entry point.
 *   This entry point is called early in the initialization -- after all
 *   memory has been configured and mapped but before any devices
 *   have been initialized.
 *
 ****************************************************************************/

void stm32_board_initialize(void);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __ASSEMBLY__ */
#endif /* __BOARDS_ARM_STM32N6_NUCLEO_N657X0_Q_INCLUDE_BOARD_H */
