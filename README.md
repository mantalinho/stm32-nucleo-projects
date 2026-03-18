# STM32F446RE Bare-Metal Projects

Embedded systems projects for the STM32 Nucleo-F446RE board.
Written in C using direct register access (no HAL).

## Hardware
- Board: STM32 Nucleo-F446RE
- MCU: STM32F446RE (ARM Cortex-M4, 180MHz)

## Projects

### 01 - Blink
Toggle onboard LED (PA5) using direct GPIO register access.
Concepts: RCC clock enable, GPIO MODER, ODR register.

### 02 - Button
Control LED (PA5) with onboard button (PC13).
Concepts: GPIO input, IDR register, polling.

### 03 - UART
Transmit string to PC via USART2 at 9600 baud.
Concepts: USART configuration, TX, Alternate Function GPIO.
Tools: PuTTY for serial monitor.

## Setup
- IDE: STM32CubeIDE 2.1.0
- No external libraries — pure register-level C
- Flash via ST-LINK (onboard)

## Coming Soon
- FreeRTOS task scheduling
- State machine (automotive ECU simulation)
- Watchdog timer (ISO 26262 concept)
