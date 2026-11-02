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

#define OLED_SCLK_HIGH()  (GPIOA->BSRR = GPIO_BSRR_BS5)
#define OLED_SCLK_LOW()   (GPIOA->BSRR = GPIO_BSRR_BR5)

#define OLED_SDIN_HIGH()  (GPIOA->BSRR = GPIO_BSRR_BS7)
#define OLED_SDIN_LOW()   (GPIOA->BSRR = GPIO_BSRR_BR7)

#define OLED_RES_HIGH()   (GPIOB->BSRR = GPIO_BSRR_BS0)
#define OLED_RES_LOW()    (GPIOB->BSRR = GPIO_BSRR_BR0)

#define OLED_DC_HIGH()    (GPIOB->BSRR = GPIO_BSRR_BS1)
#define OLED_DC_LOW()     (GPIOB->BSRR = GPIO_BSRR_BR1)

#define OLED_CS_HIGH()    (GPIOB->BSRR = GPIO_BSRR_BS10)
#define OLED_CS_LOW()     (GPIOB->BSRR = GPIO_BSRR_BR10)

void SPI1_Init(void);
void SPI1_Write(uint8_t data);
uint8_t SPI1_Read(void);
void OLED_Write(uint8_t data, uint8_t isData);
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawChessboard(void);

void SPI1_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;

    GPIOA->CRL &= ~(GPIO_CRL_MODE5 | GPIO_CRL_CNF5);
    GPIOA->CRL |= (GPIO_CRL_MODE5_0 | GPIO_CRL_MODE5_1);

    GPIOA->CRL &= ~(GPIO_CRL_MODE7 | GPIO_CRL_CNF7);
    GPIOA->CRL |= (GPIO_CRL_MODE7_0 | GPIO_CRL_MODE7_1);

    GPIOB->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);
    GPIOB->CRL |= (GPIO_CRL_MODE0_0 | GPIO_CRL_MODE0_1); 

    GPIOB->CRL &= ~(GPIO_CRL_MODE1 | GPIO_CRL_CNF1);
    GPIOB->CRL |= (GPIO_CRL_MODE1_0 | GPIO_CRL_MODE1_1);

    GPIOB->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10);
    GPIOB->CRH |= (GPIO_CRH_MODE10_0 | GPIO_CRH_MODE10_1);

    OLED_CS_HIGH();
    OLED_SCLK_LOW();
    OLED_DC_LOW();
    
    OLED_RES_LOW();
    delay_ms(20);
    OLED_RES_HIGH();
    delay_ms(20);
}

void SPI1_Write(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++) {
        OLED_SCLK_LOW();
        if (data & 0x80) {
            OLED_SDIN_HIGH();
        } else {
            OLED_SDIN_LOW();
        }
        OLED_SCLK_HIGH();
        data <<= 1;
    }
    OLED_SCLK_LOW();
}

uint8_t SPI1_Read(void) { return 0; }

void OLED_Write(uint8_t data, uint8_t isData)
{
    OLED_CS_LOW();
    if (isData) OLED_DC_HIGH();
    else        OLED_DC_LOW();
    
    SPI1_Write(data);
    
    OLED_CS_HIGH();
}

void OLED_Init(void)
{
    SPI1_Init();

    OLED_Write(0xAE, 0);
    OLED_Write(0xD5, 0); OLED_Write(0x80, 0);
    OLED_Write(0xA8, 0); OLED_Write(0x3F, 0);
    OLED_Write(0xD3, 0); OLED_Write(0x00, 0);
    OLED_Write(0x40, 0);
    OLED_Write(0x8D, 0); OLED_Write(0x14, 0);
    OLED_Write(0x20, 0); OLED_Write(0x02, 0);
    OLED_Write(0xA1, 0);
    OLED_Write(0xC8, 0);
    OLED_Write(0xDA, 0); OLED_Write(0x12, 0);
    OLED_Write(0x81, 0); OLED_Write(0x7F, 0);
    OLED_Write(0xD9, 0); OLED_Write(0xF1, 0);
    OLED_Write(0xDB, 0); OLED_Write(0x40, 0);
    OLED_Write(0xA4, 0);
    OLED_Write(0xA6, 0);
    OLED_Write(0xAF, 0);
}

void OLED_Clear(void)
{
    for (uint8_t page = 0; page < 8; page++) {
        OLED_Write(0xB0 + page, 0);
        OLED_Write(0x00, 0);
        OLED_Write(0x10, 0);
        for (uint8_t col = 0; col < 128; col++) {
            OLED_Write(0x00, 1);
        }
    }
}

void OLED_DrawChessboard(void)
{
    for (uint8_t page = 0; page < 8; page++) {
        OLED_Write(0xB0 + page, 0);
        OLED_Write(0x00, 0);
        OLED_Write(0x10, 0);
        
        for (uint8_t col = 0; col < 128; col++) {
            uint8_t cellX = col / 8;
            uint8_t cellY = page;
            uint8_t pattern = ((cellX + cellY) % 2 == 0) ? 0xFF : 0x00;
            OLED_Write(pattern, 1);
        }
    }
}

int __attribute((noreturn)) main(void)
{
    SysTick->LOAD = 8000 - 1; 
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;

    OLED_Init();
    OLED_Clear();
    OLED_DrawChessboard();

    while (1) {
    }
}