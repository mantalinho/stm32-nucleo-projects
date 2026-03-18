#include <stdint.h>

// Register addresses - STM32F446RE datasheet
#define RCC_AHB1ENR   (*((volatile uint32_t*)0x40023830))
#define GPIOA_MODER   (*((volatile uint32_t*)0x40020000))
#define GPIOA_ODR     (*((volatile uint32_t*)0x40020014))

void delay(volatile uint32_t d) {
    while(d--);
}

int main(void) {
    // Enable GPIOA clock
    RCC_AHB1ENR |= (1 << 0);

    // PA5 = output mode
    GPIOA_MODER |= (1 << 10);
    GPIOA_MODER &= ~(1 << 11);

    while(1) {
        GPIOA_ODR ^= (1 << 5);  // Toggle onboard LED LD2
        delay(500000);
    }
}
