#include <stdint.h>

// RCC
#define RCC_AHB1ENR  (*((volatile uint32_t*)0x40023830))
#define RCC_APB1ENR  (*((volatile uint32_t*)0x40023840))

// GPIOA
#define GPIOA_MODER  (*((volatile uint32_t*)0x40020000))
#define GPIOA_AFRL   (*((volatile uint32_t*)0x40020020))

// USART2
#define USART2_SR    (*((volatile uint32_t*)0x40004400))
#define USART2_DR    (*((volatile uint32_t*)0x40004404))
#define USART2_BRR   (*((volatile uint32_t*)0x40004408))
#define USART2_CR1   (*((volatile uint32_t*)0x4000440C))

void uart_send_char(char c) {
    while (!(USART2_SR & (1 << 7)));  // Wait until TX buffer is ready
    USART2_DR = c;
}

void uart_send_string(const char* s) {
    while (*s) uart_send_char(*s++);
}

void delay(volatile uint32_t d) {
    while(d--);
}

int main(void) {
    // Enable clocks
    RCC_AHB1ENR |= (1 << 0);   // GPIOA
    RCC_APB1ENR |= (1 << 17);  // USART2

    // PA2 = Alternate Function mode (USART2 TX)
    GPIOA_MODER &= ~(3 << 4);
    GPIOA_MODER |=  (2 << 4);

    // Set AF7 on PA2 (USART2)
    GPIOA_AFRL &= ~(0xF << 8);
    GPIOA_AFRL |=  (7   << 8);

    // USART2: 9600 baud @ 16MHz HSI
    USART2_BRR = 0x0683;

    // Enable TX and USART
    USART2_CR1 |= (1 << 3);   // TE - Transmitter enable
    USART2_CR1 |= (1 << 13);  // UE - USART enable

    while(1) {
        uart_send_string("Hello from STM32!\r\n");
        delay(1000000);
    }
}
