#include "stm32f4xx.h"

void usart1_init(void) {
  /* Using USART1 on PA9, PA10, tx only in polling mode (no interrupts) */

  /* Enable PORT E clock */
  SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);
  // TX - PD8, AF7
  SET_BIT(GPIOA->MODER, GPIO_MODER_MODE9_1);
  SET_BIT(GPIOA->AFR[1], GPIO_AFRH_AFSEL9_0 | GPIO_AFRH_AFSEL9_1 | GPIO_AFRH_AFSEL9_2);
  // RX - PD9, AF7
  SET_BIT(GPIOA->MODER, GPIO_MODER_MODE10_1);
  SET_BIT(GPIOA->AFR[1], GPIO_AFRH_AFSEL10_0 | GPIO_AFRH_AFSEL10_1 | GPIO_AFRH_AFSEL10_2);

  // Init USART1 module
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN);
  // For Fpclk = 16 MHz, oversampling by 16
  // Baud = 9600, USARTDIV = 104.1875, M = 0x68, F = 0x3
  WRITE_REG(USART1->BRR, 0x683);
  // Tx enable
  SET_BIT(USART1->CR1, USART_CR1_TE);
  // 8 bit, no parity, 1 stop
  CLEAR_BIT(USART1->CR1, USART_CR1_M | USART_CR1_PCE);
  CLEAR_BIT(USART1->CR2, USART_CR2_STOP_0 | USART_CR2_STOP_0);
  // enable USART
  SET_BIT(USART1->CR1, USART_CR1_UE);
}

/* retarget the C library printf function to the USART */
int __SEGGER_RTL_X_file_write(__SEGGER_RTL_FILE *__stream, const char *__s, unsigned __len) {
  
  /* Send string over USART1 in pending mode */
  for (; __len != 0; --__len) {
    USART1->DR = * __s++;
    while (RESET == READ_BIT(USART1->SR, USART_SR_TXE));
  } 

  return 0;
}