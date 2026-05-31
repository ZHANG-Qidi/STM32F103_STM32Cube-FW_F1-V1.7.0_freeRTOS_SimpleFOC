#include "Arduino_HAL.h"

#include "Arduino.h"
#include "Arduino_interface.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "main.h"
#include "task.h"

// CPU / Global settings
volatile uint8_t SREG;

// USART registers (dummy AVR compatibility)
volatile uint8_t UBRRH;
volatile uint8_t UBRRL;
volatile uint8_t UCSRA;
volatile uint8_t UCSRB;
volatile uint8_t UCSRC;
volatile uint8_t UDR;

volatile uint8_t UBRR0H;
volatile uint8_t UBRR0L;
volatile uint8_t UCSR0A;
volatile uint8_t UCSR0B;
volatile uint8_t UCSR0C;
volatile uint8_t UDR0;

// SPI registers (dummy AVR compatibility)
volatile uint8_t SPCR;
volatile uint8_t SPSR;
volatile uint8_t SPDR;

// delayMicroseconds using DWT cycle counter
void delayMicroseconds(unsigned int us) {
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = (SystemCoreClock / 1000000) * us;
    while ((DWT->CYCCNT - start) < cycles) {
    }
}

// delay with FreeRTOS awareness
void delay(unsigned long ms) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        HAL_Delay(ms);
    } else {
        osDelay(ms);
    }
}

// micros() using DWT
unsigned long micros(void) { return (unsigned long)(DWT->CYCCNT / (SystemCoreClock / 1000000)); }

// digitalWrite using pin number encoding (port = pin/16, bit = pin%16)
void digitalWrite(uint8_t pin, uint8_t val) {
    GPIO_PinState pin_val = (val == HIGH) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    GPIO_TypeDef *pin_port;
    uint16_t pin_pin = 1 << (pin % 16);

    switch (pin / 16) {
        case 0:
            pin_port = GPIOA;
            break;
        case 1:
            pin_port = GPIOB;
            break;
        case 2:
            pin_port = GPIOC;
            break;
        case 3:
            pin_port = GPIOD;
            break;
        default:
            pin_port = GPIOA;
            break;
    }

    HAL_GPIO_WritePin(pin_port, pin_pin, pin_val);
}

// pinMode dummy (not implemented)
void pinMode(uint8_t pin, uint8_t mode) {
    (void)pin;
    (void)mode;
}

// analogWrite using TIM PWM
void analogWrite(uint8_t pin, int val) {
    if (val < 0) val = 0;
    if (val > 255) val = 255;

    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(g_htim);
    uint32_t duty = (uint32_t)val * (arr + 1) / 255.0f;

    switch (pin) {
        case 0:
            __HAL_TIM_SET_COMPARE(g_htim, TIM_CHANNEL_1, duty);
            break;
        case 1:
            __HAL_TIM_SET_COMPARE(g_htim, TIM_CHANNEL_2, duty);
            break;
        case 2:
            __HAL_TIM_SET_COMPARE(g_htim, TIM_CHANNEL_3, duty);
            break;
        default:
            break;
    }
}
