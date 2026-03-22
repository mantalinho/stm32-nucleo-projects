#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

// RCC
#define RCC_AHB1ENR  (*((volatile uint32_t*)0x40023830))
#define RCC_APB1ENR  (*((volatile uint32_t*)0x40023840))

// GPIOA
#define GPIOA_MODER  (*((volatile uint32_t*)0x40020000))
#define GPIOA_ODR    (*((volatile uint32_t*)0x40020014))
#define GPIOA_AFRL   (*((volatile uint32_t*)0x40020020))

// GPIOC - Button PC13
#define GPIOC_MODER  (*((volatile uint32_t*)0x40020800))
#define GPIOC_IDR    (*((volatile uint32_t*)0x40020810))

// USART2
#define USART2_SR    (*((volatile uint32_t*)0x40004400))
#define USART2_DR    (*((volatile uint32_t*)0x40004404))
#define USART2_BRR   (*((volatile uint32_t*)0x40004408))
#define USART2_CR1   (*((volatile uint32_t*)0x4000440C))

// ECU States
typedef enum {
    STATE_IDLE,
    STATE_STARTING,
    STATE_RUNNING,
    STATE_FAULT,
    STATE_SHUTDOWN
} ECU_State;

volatile ECU_State currentState = STATE_IDLE;
volatile uint32_t uptime = 0;
volatile uint32_t rpm = 0;
volatile uint32_t temp = 20;

// Watchdog task counters
volatile uint32_t wdg_ecu = 0;
volatile uint32_t wdg_dashboard = 0;

// UART functions
void uart_send_char(char c) {
    while (!(USART2_SR & (1 << 7)));
    USART2_DR = c;
}

void uart_send_string(const char* s) {
    while (*s) uart_send_char(*s++);
}

void uart_send_int(uint32_t n) {
    char buf[10];
    int i = 0;
    if (n == 0) { uart_send_char('0'); return; }
    while (n > 0) { buf[i++] = '0' + (n % 10); n /= 10; }
    for (int j = i-1; j >= 0; j--) uart_send_char(buf[j]);
}

// Task 1: ECU state machine + button input
void Task_ECU(void *pvParameters) {
    while(1) {
        wdg_ecu++;  // Kick watchdog

        // Button press advances ECU state
        if (!(GPIOC_IDR & (1 << 13))) {
            vTaskDelay(pdMS_TO_TICKS(50));  // Debounce
            if (!(GPIOC_IDR & (1 << 13))) {
                switch(currentState) {
                    case STATE_IDLE:     currentState = STATE_STARTING; break;
                    case STATE_STARTING: currentState = STATE_RUNNING;  break;
                    case STATE_RUNNING:  currentState = STATE_FAULT;    break;
                    case STATE_FAULT:    currentState = STATE_SHUTDOWN; break;
                    case STATE_SHUTDOWN: currentState = STATE_IDLE;     break;
                }
                vTaskDelay(pdMS_TO_TICKS(300));  // Wait for button release
            }
        }

        // Simulate RPM and temperature based on state
        if (currentState == STATE_RUNNING) {
            rpm = 2000 + (uptime * 7 % 2000);
            temp = 80 + (uptime % 20);
            GPIOA_ODR |= (1 << 5);   // LED ON = running
        } else if (currentState == STATE_FAULT) {
            GPIOA_ODR ^= (1 << 5);   // LED blink = fault
        } else {
            GPIOA_ODR &= ~(1 << 5);  // LED OFF
        }

        uptime++;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Task 2: UART dashboard output
void Task_Dashboard(void *pvParameters) {
    while(1) {
        wdg_dashboard++;  // Kick watchdog

        uart_send_string("╔══════════════════════════════╗\r\n");
        uart_send_string("║    ECU DIAGNOSTIC DASHBOARD  ║\r\n");
        uart_send_string("╠══════════════════════════════╣\r\n");

        uart_send_string("║ STATE:  ");
        switch(currentState) {
            case STATE_IDLE:     uart_send_string("IDLE          "); break;
            case STATE_STARTING: uart_send_string("STARTING      "); break;
            case STATE_RUNNING:  uart_send_string("RUNNING       "); break;
            case STATE_FAULT:    uart_send_string("FAULT !!!     "); break;
            case STATE_SHUTDOWN: uart_send_string("SHUTDOWN      "); break;
        }
        uart_send_string("║\r\n");

        uart_send_string("║ RPM:    ");
        uart_send_int(rpm);
        uart_send_string("              ║\r\n");

        uart_send_string("║ TEMP:   ");
        uart_send_int(temp);
        uart_send_string(" C            ║\r\n");

        uart_send_string("║ UPTIME: ");
        uart_send_int(uptime / 10);
        uart_send_string(" s            ║\r\n");

        uart_send_string("╚══════════════════════════════╝\r\n");
        uart_send_string("\r\n[BUTTON] = Change State\r\n");

        if (temp > 90)
            uart_send_string("[WARN] High temperature!\r\n");
        if (currentState == STATE_FAULT)
            uart_send_string("[FAULT] Check engine!\r\n");

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// Task 3: Software watchdog monitor
void Task_Watchdog(void *pvParameters) {
    uint32_t last_ecu = 0;
    uint32_t last_dashboard = 0;

    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));

        if (wdg_ecu == last_ecu)
            uart_send_string("[WDG] WARN: Task_ECU not responding!\r\n");
        else
            uart_send_string("[WDG] Task_ECU .......... OK\r\n");

        if (wdg_dashboard == last_dashboard)
            uart_send_string("[WDG] WARN: Task_Dashboard not responding!\r\n");
        else
            uart_send_string("[WDG] Task_Dashboard .... OK\r\n");

        last_ecu = wdg_ecu;
        last_dashboard = wdg_dashboard;
    }
}

int main(void) {
    // Enable clocks
    RCC_AHB1ENR |= (1 << 0);   // GPIOA
    RCC_AHB1ENR |= (1 << 2);   // GPIOC
    RCC_APB1ENR |= (1 << 17);  // USART2

    // PA5 = output (onboard LED LD2)
    GPIOA_MODER |=  (1 << 10);
    GPIOA_MODER &= ~(1 << 11);

    // PC13 = input (onboard button B1)
    GPIOC_MODER &= ~(3 << 26);

    // PA2 = AF7 (USART2 TX)
    GPIOA_MODER &= ~(3 << 4);
    GPIOA_MODER |=  (2 << 4);
    GPIOA_AFRL  &= ~(0xF << 8);
    GPIOA_AFRL  |=  (7 << 8);

    // USART2: 115200 baud @ 16MHz
    USART2_BRR = 0x008B;
    USART2_CR1 |= (1 << 3);
    USART2_CR1 |= (1 << 13);

    uart_send_string("BOOT OK\r\n");

    // Create FreeRTOS tasks
    xTaskCreate(Task_Watchdog,  "Watchdog",  256, NULL, 3, NULL);
    xTaskCreate(Task_ECU,       "ECU",       256, NULL, 2, NULL);
    xTaskCreate(Task_Dashboard, "Dashboard", 256, NULL, 1, NULL);

    // Start scheduler
    vTaskStartScheduler();

    while(1);
}
