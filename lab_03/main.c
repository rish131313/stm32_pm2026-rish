#include <stdint.h>
#include <stm32f10x.h>

volatile uint32_t ms_ticks = 0;

void SysTick_Handler(void) {
    ms_ticks++;
}

void delay_ms(uint32_t ms) {
    uint32_t start = ms_ticks;
    while ((ms_ticks - start) < ms) {
        __NOP();
    }
}

int __attribute((noreturn)) main(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    GPIOC->CRH &= ~(GPIO_CRH_CNF12 | GPIO_CRH_MODE12 | GPIO_CRH_CNF13 | GPIO_CRH_MODE13);
    GPIOC->CRH |= (GPIO_CRH_MODE12_0 | GPIO_CRH_MODE13_0); 

    GPIOC->CRH &= ~(GPIO_CRH_CNF14 | GPIO_CRH_MODE14 | GPIO_CRH_CNF15 | GPIO_CRH_MODE15);
    GPIOC->CRH |= (GPIO_CRH_CNF14_1 | GPIO_CRH_CNF15_1); 
    GPIOC->ODR |= (GPIO_ODR_ODR14 | GPIO_ODR_ODR15);     

    SysTick->LOAD = 8000 - 1; 
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;

    uint32_t half_period_ms = 500; 
    
    const uint32_t MIN_PERIOD_MS = 8;    
    const uint32_t MAX_PERIOD_MS = 32000; 

    uint8_t btn_a_was_pressed = 0;
    uint8_t btn_c_was_pressed = 0;

    while (1) {
        if (!(GPIOC->IDR & GPIO_IDR_IDR14)) { 
            if (!btn_a_was_pressed) {
                delay_ms(20);
                if (!(GPIOC->IDR & GPIO_IDR_IDR14)) {
                    if (half_period_ms > MIN_PERIOD_MS) {
                        half_period_ms /= 2; 
                    }
                    btn_a_was_pressed = 1; 
                }
            }
        } else {
            btn_a_was_pressed = 0; 
        }

        if (!(GPIOC->IDR & GPIO_IDR_IDR15)) {
            if (!btn_c_was_pressed) {
                delay_ms(20);
                if (!(GPIOC->IDR & GPIO_IDR_IDR15)) {
                    if (half_period_ms < MAX_PERIOD_MS) {
                        half_period_ms *= 2; 
                    }
                    btn_c_was_pressed = 1;
                }
            }
        } else {
            btn_c_was_pressed = 0;
        }

        GPIOC->ODR |= (GPIO_ODR_ODR12 | GPIO_ODR_ODR13);  
        delay_ms(half_period_ms);
        
        GPIOC->ODR &= ~(GPIO_ODR_ODR12 | GPIO_ODR_ODR13); 
        delay_ms(half_period_ms);
    }
}