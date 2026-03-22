# STM32F446RE Bare-Metal Embedded Projects

Bare-metal embedded systems projects for the STM32 Nucleo-F446RE board.
Written in C using direct register access — no HAL, no Arduino.
Focused on automotive-relevant concepts: state machines, RTOS, actuator control, sensor fusion.

## Hardware
- Board: STM32 Nucleo-F446RE
- MCU: STM32F446RE (ARM Cortex-M4, 180MHz)
- IDE: STM32CubeIDE 2.1.0
- No external libraries — pure register-level C

## Components
| Component | Purpose | Connection |
|---|---|---|
| STM32 Nucleo-F446RE | Main microcontroller board | — |
| SG90 Servo Motor | Throttle/actuator simulation | PA0 (PWM), 5V, GND |
| HC-SR04 Ultrasonic Sensor | Distance measurement | PA1 (TRIG), PA4 (ECHO), 5V, GND |
| SSD1306 OLED 128x64 | Display dashboard (coming soon) | PB8 (SCL), PB9 (SDA), 3.3V, GND |
| Breadboard + Jumper Wires | Prototyping connections | — |
## Projects

### 01 - Blink
Toggle onboard LED (PA5) using direct GPIO register access.
Concepts: RCC clock enable, GPIO MODER, ODR register.

### 02 - Button
Control LED (PA5) with onboard button (PC13).
Concepts: GPIO input, IDR register, polling.

### 03 - UART
Transmit string to PC via USART2 at 115200 baud.
Concepts: USART configuration, TX, Alternate Function GPIO.
Tools: PuTTY serial monitor.

### 04 - FreeRTOS Tasks
Two concurrent tasks running on FreeRTOS scheduler.
Concepts: RTOS, task creation, vTaskDelay, priority scheduling.

### 05 - ECU Dashboard
Automotive ECU simulator with state machine, UART dashboard and software watchdog.
States: IDLE → STARTING → RUNNING → FAULT → SHUTDOWN
Concepts: FreeRTOS, ECU state machine, software watchdog (ISO 26262 concept), UART monitoring.
Tools: PuTTY for live dashboard.

### 06 - Servo Throttle Control
PWM-based servo control simulating automotive throttle actuator.
Button cycles through IDLE / PARTIAL / FULL throttle positions.
Concepts: TIM2 PWM generation, actuator control, duty cycle.

### 07 - Automotive Parking Sensor
HC-SR04 ultrasonic distance sensor + SG90 servo actuator + UART live dashboard.
Servo oscillates faster as object approaches. LED beeps with distance.
Concepts: Ultrasonic sensing, TIM5 pulse measurement, servo PWM, sensor-actuator loop.
Tools: PuTTY for live colored dashboard with ANSI terminal output.

## Concepts Covered
- Bare-metal register programming (no HAL)
- GPIO input/output
- UART serial communication
- PWM generation with hardware timers
- Ultrasonic distance measurement
- FreeRTOS task scheduling
- Automotive ECU state machine
- Software watchdog timer (ISO 26262)
- Sensor-actuator control loop

## Coming Soon
- CAN Bus communication (MCP2515)
- OLED SSD1306 display dashboard
- Hardware watchdog timer (IWDG)
- AUTOSAR-inspired task architecture
