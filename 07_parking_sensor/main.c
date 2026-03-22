#include <stdint.h>

// RCC
#define RCC_AHB1ENR  (*((volatile uint32_t*)0x40023830))
#define RCC_APB1ENR  (*((volatile uint32_t*)0x40023840))

// GPIOA
#define GPIOA_MODER  (*((volatile uint32_t*)0x40020000))
#define GPIOA_ODR    (*((volatile uint32_t*)0x40020014))
#define GPIOA_IDR    (*((volatile uint32_t*)0x40020010))
#define GPIOA_AFRL   (*((volatile uint32_t*)0x40020020))

// USART2
#define USART2_SR  (*((volatile uint32_t*)0x40004400))
#define USART2_DR  (*((volatile uint32_t*)0x40004404))
#define USART2_BRR (*((volatile uint32_t*)0x40004408))
#define USART2_CR1 (*((volatile uint32_t*)0x4000440C))

// TIM2 - Servo PWM output (PA0)
#define TIM2_CR1   (*((volatile uint32_t*)0x40000000))
#define TIM2_PSC   (*((volatile uint32_t*)0x40000028))
#define TIM2_ARR   (*((volatile uint32_t*)0x4000002C))
#define TIM2_CCR1  (*((volatile uint32_t*)0x40000034))
#define TIM2_CCMR1 (*((volatile uint32_t*)0x40000018))
#define TIM2_CCER  (*((volatile uint32_t*)0x40000020))

// TIM5 - Ultrasonic pulse timing
#define TIM5_CR1   (*((volatile uint32_t*)0x40000C00))
#define TIM5_PSC   (*((volatile uint32_t*)0x40000C28))
#define TIM5_ARR   (*((volatile uint32_t*)0x40000C2C))
#define TIM5_CNT   (*((volatile uint32_t*)0x40000C24))

void delay_us(volatile uint32_t us) { us *= 3; while(us--); }
void delay_ms(volatile uint32_t ms) { while(ms--) delay_us(1000); }

// UART init - PA2 = USART2 TX, 115200 baud @ 16MHz
void uart_init(void) {
    RCC_APB1ENR |= (1<<17);
    GPIOA_MODER &= ~(3<<4); GPIOA_MODER |= (2<<4);
    GPIOA_AFRL  &= ~(0xF<<8); GPIOA_AFRL |= (7<<8);
    USART2_BRR = 0x008B;
    USART2_CR1 |= (1<<3)|(1<<13);
}

void uart_char(char c) { while(!(USART2_SR & (1<<7))); USART2_DR = c; }
void uart_string(const char* s) { while(*s) uart_char(*s++); }
void uart_int(uint32_t n) {
    char buf[10]; int i = 0;
    if(n == 0) { uart_char('0'); return; }
    while(n > 0) { buf[i++] = '0' + (n%10); n /= 10; }
    for(int j = i-1; j >= 0; j--) uart_char(buf[j]);
}

// Servo init - TIM2 CH1 on PA0, 50Hz PWM
void servo_init(void) {
    RCC_APB1ENR |= (1<<0);
    GPIOA_MODER &= ~(3<<0); GPIOA_MODER |= (2<<0);
    GPIOA_AFRL  &= ~(0xF<<0); GPIOA_AFRL |= (1<<0);
    TIM2_PSC = 159;   // 16MHz / 160 = 100kHz
    TIM2_ARR = 1999;  // 100kHz / 2000 = 50Hz
    TIM2_CCMR1 |= (6<<4)|(1<<3);  // PWM mode 1, preload enable
    TIM2_CCER  |= (1<<0);          // Channel 1 output enable
    TIM2_CR1   |= (1<<0);          // Enable timer
}

// Set servo pulse width in microseconds
// 1000us = 0deg, 1500us = 90deg, 2000us = 180deg
void servo_set(uint16_t pulse_us) { TIM2_CCR1 = pulse_us / 10; }

// HC-SR04 init - PA1 = TRIG output, PA4 = ECHO input
void hcsr04_init(void) {
    GPIOA_MODER &= ~(3<<2); GPIOA_MODER |= (1<<2);  // PA1 output
    GPIOA_MODER &= ~(3<<8);                           // PA4 input
    RCC_APB1ENR |= (1<<3);   // TIM5 clock
    TIM5_PSC = 15;            // 16MHz / 16 = 1MHz = 1us per tick
    TIM5_ARR = 0xFFFFFFFF;
    TIM5_CR1 |= (1<<0);
}

// Read distance in cm from HC-SR04
uint32_t hcsr04_read(void) {
    GPIOA_ODR |=  (1<<1); delay_us(10);   // 10us trigger pulse
    GPIOA_ODR &= ~(1<<1);

    uint32_t t = 100000;
    while(!(GPIOA_IDR & (1<<4)) && t--);  // Wait for echo high
    if(!t) return 999;                     // Timeout = no object

    TIM5_CNT = 0;
    t = 100000;
    while((GPIOA_IDR & (1<<4)) && t--);   // Wait for echo low

    return TIM5_CNT / 58;  // Convert pulse width to cm
}

int main(void) {
    RCC_AHB1ENR |= (1<<0);  // GPIOA clock

    // PA5 = output (onboard LED LD2)
    GPIOA_MODER |=  (1<<10);
    GPIOA_MODER &= ~(1<<11);

    uart_init();
    servo_init();
    hcsr04_init();

    uart_string("PARKING SENSOR READY\r\n");

    static uint32_t min_dist = 9999;
    static uint32_t max_dist = 0;

    while(1) {
        uint32_t dist = hcsr04_read();

        // Update min/max distance records
        if(dist > 0 && dist < 999) {
            if(dist < min_dist) min_dist = dist;
            if(dist > max_dist) max_dist = dist;
        }

        // Clear terminal and print dashboard
        uart_string("\033[2J\033[H");
        uart_string("╔══════════════════════════════╗\r\n");
        uart_string("║   AUTOMOTIVE PARKING SENSOR  ║\r\n");
        uart_string("╠══════════════════════════════╣\r\n");
        uart_string("║ DISTANCE: ");
        uart_int(dist);
        uart_string(" cm              ║\r\n");
        uart_string("║ STATUS:   ");

        uint32_t speed_delay;
        if(dist > 500) {
            uart_string("✓ CLEAR          ║\r\n");
            speed_delay = 0;
        } else if(dist > 300) {
            uart_string("⚠ APPROACHING    ║\r\n");
            speed_delay = 800;
        } else if(dist > 200) {
            uart_string("⚠ CAUTION        ║\r\n");
            speed_delay = 500;
        } else if(dist > 100) {
            uart_string("✗ DANGER         ║\r\n");
            speed_delay = 300;
        } else {
            uart_string("✗ STOP!!!        ║\r\n");
            speed_delay = 150;
        }

        // Colored proximity bar (green=far, yellow=mid, red=close)
        uart_string("╠══════════════════════════════╣\r\n");
        uart_string("║ ");
        uint32_t bars = dist > 500 ? 0 : (500 - dist) * 20 / 500;
        for(uint32_t i = 0; i < 20; i++) {
            if(i < bars) {
                if(bars > 13)     uart_string("\033[31m█\033[0m");  // Red
                else if(bars > 7) uart_string("\033[33m█\033[0m");  // Yellow
                else              uart_string("\033[32m█\033[0m");  // Green
            } else {
                uart_string("░");
            }
        }
        uart_string(" ║\r\n");

        // Min/Max distance log
        uart_string("╠══════════════════════════════╣\r\n");
        uart_string("║ MIN: "); uart_int(min_dist);
        uart_string("cm  MAX: "); uart_int(max_dist);
        uart_string("cm          ║\r\n");
        uart_string("╚══════════════════════════════╝\r\n");

        // Servo oscillation speed + LED beep rate based on distance
        if(speed_delay == 0) {
            servo_set(1000);           // Servo at rest position
            GPIOA_ODR &= ~(1<<5);     // LED OFF = clear
            delay_ms(500);
        } else {
            servo_set(1000);           // Swing to 0 deg
            GPIOA_ODR |= (1<<5);      // LED ON
            delay_ms(speed_delay);

            servo_set(2000);           // Swing to 180 deg
            GPIOA_ODR &= ~(1<<5);     // LED OFF
            delay_ms(speed_delay);
        }
    }
}
