/****************************************************************************
 * arch/arm/src/stm32n6/hardware/stm32n6xxx_pinmap.h
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

#ifndef __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_PINMAP_H
#define __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_PINMAP_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Alternate Pin Functions.
 *
 * Alternative pin selections are provided with a numeric suffix like _1, _2,
 * etc.  Drivers, however, will use the pin selection without the numeric
 * suffix.  Additional definitions are required in the board.h file.  For
 * example, if USART1_TX connects via PE5 on some board, then the following
 * definition should appear in the board.h header file for that board:
 *
 * #define GPIO_USART1_TX GPIO_USART1_TX_1
 *
 * The driver will then automatically configure PE5 as the USART1 TX pin.
 */

/* USART1: PE5=TX (AF7), PE6=RX (AF7) - ST-Link Virtual COM Port */

#define GPIO_USART1_TX_1   (GPIO_ALT | GPIO_AF7 | GPIO_SPEED_50MHZ | GPIO_PUSHPULL | GPIO_PORTE | GPIO_PIN5)
#define GPIO_USART1_RX_1   (GPIO_ALT | GPIO_AF7 | GPIO_SPEED_50MHZ | GPIO_PORTE | GPIO_PIN6)

/* USART2: PD5=TX (AF7), PD6=RX (AF7) */

#define GPIO_USART2_TX_1   (GPIO_ALT | GPIO_AF7 | GPIO_SPEED_50MHZ | GPIO_PUSHPULL | GPIO_PORTD | GPIO_PIN5)
#define GPIO_USART2_RX_1   (GPIO_ALT | GPIO_AF7 | GPIO_SPEED_50MHZ | GPIO_PORTD | GPIO_PIN6)

/* USART3: PD8=TX (AF7), PD9=RX (AF7) */

#define GPIO_USART3_TX_1   (GPIO_ALT | GPIO_AF7 | GPIO_SPEED_50MHZ | GPIO_PUSHPULL | GPIO_PORTD | GPIO_PIN8)
#define GPIO_USART3_RX_1   (GPIO_ALT | GPIO_AF7 | GPIO_SPEED_50MHZ | GPIO_PORTD | GPIO_PIN9)

/* SPI1 (AF5): PA4=NSS, PA5=SCK, PA6=MISO, PA7=MOSI
 *             PB3=SCK, PB4=MISO, PB5=MOSI (alternate set)
 */

#define GPIO_SPI1_NSS_1    (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTA | GPIO_PIN4)
#define GPIO_SPI1_SCK_1    (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTA | GPIO_PIN5)
#define GPIO_SPI1_MISO_1   (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PORTA | GPIO_PIN6)
#define GPIO_SPI1_MOSI_1   (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTA | GPIO_PIN7)
#define GPIO_SPI1_SCK_2    (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTB | GPIO_PIN3)
#define GPIO_SPI1_MISO_2   (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PORTB | GPIO_PIN4)
#define GPIO_SPI1_MOSI_2   (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTB | GPIO_PIN5)

/* SPI2 (AF5): PB10=SCK, PB14=MISO, PB15=MOSI, PB12=NSS
 *             PB9=NSS, PC2=MISO, PC3=MOSI (alternate)
 */

#define GPIO_SPI2_NSS_1    (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTB | GPIO_PIN12)
#define GPIO_SPI2_SCK_1    (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTB | GPIO_PIN10)
#define GPIO_SPI2_MISO_1   (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PORTB | GPIO_PIN14)
#define GPIO_SPI2_MOSI_1   (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTB | GPIO_PIN15)
#define GPIO_SPI2_NSS_2    (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTB | GPIO_PIN9)
#define GPIO_SPI2_MISO_2   (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PORTC | GPIO_PIN2)
#define GPIO_SPI2_MOSI_2   (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTC | GPIO_PIN3)

/* SPI3 (AF6): PB3=SCK, PB4=MISO, PB5=MOSI, PA15=NSS
 *             PC10=SCK, PC11=MISO, PC12=MOSI (alternate)
 */

#define GPIO_SPI3_NSS_1    (GPIO_ALT | GPIO_AF6 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTA | GPIO_PIN15)
#define GPIO_SPI3_SCK_1    (GPIO_ALT | GPIO_AF6 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTB | GPIO_PIN3)
#define GPIO_SPI3_MISO_1   (GPIO_ALT | GPIO_AF6 | GPIO_SPEED_100MHZ | GPIO_PORTB | GPIO_PIN4)
#define GPIO_SPI3_MOSI_1   (GPIO_ALT | GPIO_AF6 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTB | GPIO_PIN5)
#define GPIO_SPI3_SCK_2    (GPIO_ALT | GPIO_AF6 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTC | GPIO_PIN10)
#define GPIO_SPI3_MISO_2   (GPIO_ALT | GPIO_AF6 | GPIO_SPEED_100MHZ | GPIO_PORTC | GPIO_PIN11)
#define GPIO_SPI3_MOSI_2   (GPIO_ALT | GPIO_AF6 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTC | GPIO_PIN12)

/* SPI4 (AF5): PE2=SCK, PE5=MISO, PE6=MOSI, PE4=NSS */

#define GPIO_SPI4_NSS_1    (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTE | GPIO_PIN4)
#define GPIO_SPI4_SCK_1    (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTE | GPIO_PIN2)
#define GPIO_SPI4_MISO_1   (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PORTE | GPIO_PIN5)
#define GPIO_SPI4_MOSI_1   (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTE | GPIO_PIN6)

/* SPI5 (AF5): PF7=SCK, PF8=MISO, PF9=MOSI, PF6=NSS */

#define GPIO_SPI5_SCK_1    (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTF | GPIO_PIN15)
#define GPIO_SPI5_MISO_1   (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PORTF | GPIO_PIN12)
#define GPIO_SPI5_MOSI_1   (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTF | GPIO_PIN11)

/* SPI5 (AF5): PE15=SCK, PG1=MISO, PG2=MOSI — Nucleo Arduino/morpho header */

#define GPIO_SPI5_SCK_2    (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTE | GPIO_PIN15)
#define GPIO_SPI5_MISO_2   (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PORTG | GPIO_PIN1)
#define GPIO_SPI5_MOSI_2   (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTG | GPIO_PIN2)

/* SPI6 (AF5): PA5=SCK, PA6=MISO, PA7=MOSI, PA4=NSS (shared with SPI1) */

#define GPIO_SPI6_NSS_1    (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTA | GPIO_PIN4)
#define GPIO_SPI6_SCK_1    (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTA | GPIO_PIN5)
#define GPIO_SPI6_MISO_1   (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PORTA | GPIO_PIN6)
#define GPIO_SPI6_MOSI_1   (GPIO_ALT | GPIO_AF5 | GPIO_SPEED_100MHZ | GPIO_PUSHPULL | GPIO_PORTA | GPIO_PIN7)

#endif /* __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32N6XXX_PINMAP_H */
