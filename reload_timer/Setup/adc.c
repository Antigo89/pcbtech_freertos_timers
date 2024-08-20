#include "adc.h"
#include "stm32f4xx.h"

void ADC1_Init(void){
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
  //PA
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  GPIOA->MODER |= GPIO_MODER_MODE5; //PA5 - Analog
  //ADC1
  ADC1->SMPR2 |= ADC_SMPR2_SMP5_0;
  ADC1->JSQR &= ~(ADC_JSQR_JL); //1 measurement
  ADC1->JSQR |= (5 << ADC_JSQR_JSQ4_Pos); //CH5
  ADC1->CR1 &= ~(ADC_CR1_RES); //12bit
  ADC1->CR1 |= ADC_CR1_SCAN; //SCAN J-channel
  ADC1->CR2 |= ADC_CR2_ADON; //ADC enable
}