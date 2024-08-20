#include "gpio.h"
#include "stm32f4xx.h"

void GPIO_init(void) {
   /* Enable GPIOE clock */
  SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOEEN);

  /* Init user buttons on board */
  /* Init SW1-SW3 to input, no pull-up/pull-down */
  GPIOE->MODER &= ~(GPIO_MODER_MODE10|GPIO_MODER_MODE11|GPIO_MODER_MODE12);
  
  /* Init user LED on board */
  /* Init LED1-LED3 to output */
  GPIOE->MODER &= ~(GPIO_MODER_MODE13|GPIO_MODER_MODE14|GPIO_MODER_MODE15);
  GPIOE->MODER |= GPIO_MODER_MODE13_0|GPIO_MODER_MODE14_0|GPIO_MODER_MODE15_0;
  /* LED1-LED3 off */
  GPIOE->BSRR |= GPIO_BSRR_BS13|GPIO_BSRR_BS14|GPIO_BSRR_BS15;
  
}