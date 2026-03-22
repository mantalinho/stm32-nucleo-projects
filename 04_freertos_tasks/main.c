#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

// Register addresses
#define RCC_AHB1ENR  (*((volatile uint32_t*)0x40023830))
#define GPIOA_MODER  (*((volatile uint32_t*)0x40020000))
#define GPIOA_ODR    (*((volatile uint32_t*)0x40020014))

// Task 1: Blink LED every 200ms - simulates sensor monitor task
void Task_Sensor(void *pvParameters) {
    while(1) {
        GPIOA_ODR ^= (1 << 5);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// Task 2: Logger task - runs every 1 second
void Task_Logger(void *pvParameters) {
    while(1) {
        // Placeholder - UART logging to be added
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main(void) {
    // Enable GPIOA clock
    RCC_AHB1ENR |= (1 << 0);

    // PA5 = output mode (onboard LED LD2)
    GPIOA_MODER |=  (1 << 10);
    GPIOA_MODER &= ~(1 << 11);

    // Create tasks
    xTaskCreate(Task_Sensor, "Sensor", 128, NULL, 2, NULL);
    xTaskCreate(Task_Logger, "Logger", 128, NULL, 1, NULL);

    // Start FreeRTOS scheduler
    vTaskStartScheduler();

    while(1);
}
