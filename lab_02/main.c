#include <stdint.h>
#include <stm32f10x.h>

void delay(uint32_t ticks) {
	for (int i=0; i<ticks; i++) {
		__NOP();
	}
}

int __attribute((noreturn)) main(void) {
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    GPIOC->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_CNF12); 
    GPIOC->CRH |= (GPIO_CRH_MODE13_0 | GPIO_CRH_MODE12_0); 

    while (1) {
        GPIOC->ODR |= (1U << 13U) | (1U << 12U); 
        delay(10000);

        GPIOC->ODR &= ~((1U << 13U) | (1U << 12U));
        delay(10000);
    }
}
