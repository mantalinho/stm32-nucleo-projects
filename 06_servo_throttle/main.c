#include <stdint.h>

// RCC
#define RCC_AHB1ENR  (*((volatile uint32_t*)0x40023830))
#define RCC_APB1ENR  (*((volatile uint32_t*)0x40023840))

// GPIOA
#define GPIOA_MODER  (*((volatile uint32_t*)0x40020000))
#define GPIOA_AFRL   (*((volatile uint32_t*)0x40020020))

// GPIOC - Button
#define GPIOC_MODER  (*((volatile uint32_t*)0x40020800))
#define GPIOC_IDR    (*((volatile uint32_t*)0x40020810))

// TIM2 - PA0 = AF1 (PWM output)
#define TIM2_CR1   (*((volatile uint32_t*)0x40000000))
#define TIM2_PSC   (*((volatile uint32_t*)0x40000028))
#define TIM2_ARR   (*((volatile uint32_t*)0x4000002C))
#define TIM2_CCR1  (*((volatile uint32_t*)0x40000034))
#define TIM2_CCMR1 (*((volatile uint32_t*)0x40000018))
#define TIM2_CCER  (*((volatile uint32_t*)0x40000020))

void delay(volatile uint32_t d) { while(d--); }

void servo_init(void) {
    // Enable clocks
    RCC_AHB1ENR |= (1<<0);  // GPIOA
    RCC_APB1ENR |= (1<<0);  // TIM2

    // PA0 = Alternate Function AF1 (TIM2_CH1)
    GPIOA_MODER &= ~(3<<0);
    GPIOA_MODER |=  (2<<0);
    GPIOA_AFRL  &= ~(0xF<<0);
    GPIOA_AFRL  |=  (1<<0);

    // TIM2: 50Hz PWM (20ms period)
    // 16MHz / 160 = 100kHz, ARR=2000 -> 50Hz
    TIM2_PSC   = 159;   // Prescaler
    TIM2_ARR   = 1999;  // Period = 20ms

    // PWM Mode 1 on Channel 1
    TIM2_CCMR1 |= (6<<4);  // OC1M = PWM mode 1
    TIM2_CCMR1 |= (1<<3);  // OC1PE preload enable
    TIM2_CCER  |= (1<<0);  // CC1E output enable
    TIM2_CR1   |= (1<<0);  // Enable timer
}

// Set servo position via pulse width
// 1000us = 0deg, 1500us = 90deg, 2000us = 180deg
void servo_set(uint16_t pulse_us) {
    TIM2_CCR1 = pulse_us / 10;
}

int main(void) {
    // Enable GPIOC clock for button
    RCC_AHB1ENR |= (1<<2);

    // PC13 = input (onboard button B1)
    GPIOC_MODER &= ~(3<<26);

    servo_init();

    uint8_t state = 0;

    while(1) {
        // Button press cycles through throttle states
        if(!(GPIOC_IDR & (1<<13))) {
            delay(50000);  // Debounce
            if(!(GPIOC_IDR & (1<<13))) {
                state = (state + 1) % 3;
                while(!(GPIOC_IDR & (1<<13)));  // Wait for release
            }
        }

        // Set servo position based on throttle state
        switch(state) {
            case 0: servo_set(1000); break;  // IDLE    - 0 deg
            case 1: servo_set(1500); break;  // PARTIAL - 90 deg
            case 2: servo_set(2000); break;  // FULL    - 180 deg
        }
    }
}
