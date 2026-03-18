#include <stdint.h>

// RCC
#define RCC_AHB1ENR  (*((volatile uint32_t*)0x40023830))

// GPIOA - LED (PA5)
#define GPIOA_MODER  (*((volatile uint32_t*)0x40020000))
#define GPIOA_ODR    (*((volatile uint32_t*)0x40020014))

// GPIOC - Button (PC13)
#define GPIOC_MODER  (*((volatile uint32_t*)0x40020800))
#define GPIOC_IDR    (*((volatile uint32_t*)0x40020810))

void delay(volatile uint32_t d) {
    while(d--);
}

int main(void) {
    // Enable clocks for GPIOA and GPIOC
    RCC_AHB1ENR |= (1 << 0);  // GPIOA
    RCC_AHB1ENR |= (1 << 2);  // GPIOC

    // PA5 = output mode
    GPIOA_MODER |= (1 << 10);
    GPIOA_MODER &= ~(1 << 11);

    // PC13 = input mode (explicit)
    GPIOC_MODER &= ~(3 << 26);

    while(1) {
        // Button pressed = PC13 reads 0 (active low)
        if (!(GPIOC_IDR & (1 << 13))) {
            GPIOA_ODR |= (1 << 5);   // LED ON
        } else {
            GPIOA_ODR &= ~(1 << 5);  // LED OFF
        }
    }
}
