/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

Задание 1
Реализовать систему управления подсветкой: условный диод подсветки должен включаться во время
активных действий пользователя и, если пользователь не нажимает на кнопки устройства по истечении
определённого интервала времени, диод подсветки должен выключаться. Основные условия:
1. В рамках использования демо-платы предлагается использовать одну пользовательскую кнопку и
один пользовательский светодиод, со следующим поведением:
1.1. Исходное состояние системы – диод выключен.
1.2. При нажатии на кнопку происходит включение диода.
1.3. По истечении 4х секунд диод гаснет.
1.4. Если в течение 4х секунд пользователь повторно нажимает кнопку, то период выключения диода
обновляется. Это может происходить неоднократно.
2. Выполнить сборку, запуск и тестирование проекта на отладочной плате.
Критерии готовности (Definition of Done)
1. Проект собирается и загружается в отладочную плату без ошибок и предупреждений.
2. Начальные условия при включении системы соблюдаются.
3. Нажатия на пользовательскую кнопку обрабатываются корректно и однозначно определяются,
многократные срабатывания не происходят.
4. Система работает согласно описанному сценарию работы подсветки.
5. В решении используется программный таймер FreeRTOS в интервальном режиме (one-shot timer).



USING STM32F407VET6 board:

SWD interface
PE10 input key S1
PE13 output LED1

*/
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "gpio.h"

#include <stdio.h>
#include <stdlib.h>

/* Typedef for ID field in new struct */
typedef enum {
  MSG_ID_KEYBOARD = 0,
  MSG_ID_OTHER
} msgID;
/* Typedef for special struct - element of queue */
typedef struct {
  msgID id;
} queueMsg;

/************************global values********************************/
#define QUEUE_REC_NUM (100) //record number in queue
#define QUEUE_TIMEOUT_SEND (500) //timeout write in queue
#define QUEUE_TIMEOUT_REC (1000) //timeout read in queue

/* Queue handler */
QueueHandle_t myQueue;
/* Timer Handler */
xTimerHandle pulseLedOff;
/****************************func************************************/

void vLedOff(xTimerHandle xTimer){
  GPIOE->BSRR |= GPIO_BSRR_BS13;
}

/* read user button */
void vTaskButton(void * pvParameters){
  portTickType lastTickCountTaskButton = xTaskGetTickCount();
  queueMsg senderMsg;
  while(1){
    if(!(GPIOE->IDR & GPIO_IDR_ID10)){
      vTaskDelayUntil(&lastTickCountTaskButton, 100);
      if(!(GPIOE->IDR & GPIO_IDR_ID10)){
        senderMsg.id = MSG_ID_KEYBOARD;
        xQueueSend(myQueue, &senderMsg, QUEUE_TIMEOUT_SEND); 
        while(!(GPIOE->IDR & GPIO_IDR_ID10)){}
      }    
    }
  }
  //vTaskDelete(NULL);
}
/* Read queue */

void vTaskControl(void * pvParameters){
  queueMsg receiveMsg;
  while(1){
    if(xQueueReceive(myQueue, &receiveMsg, QUEUE_TIMEOUT_REC) == pdTRUE){
      switch (receiveMsg.id) {
        case MSG_ID_KEYBOARD:
          if(GPIOE->ODR & GPIO_ODR_OD13){
            GPIOE->BSRR |= GPIO_BSRR_BR13;
            xTimerStart(pulseLedOff, 0);
          }else{
            xTimerReset(pulseLedOff, 0);
          }
          break;
        case MSG_ID_OTHER:
          break;
        default:
          break;
      }
    }
  }
  //vTaskDelete(NULL);
}

/****************************main************************************/

 int main(void) {
  GPIO_init();
  
  myQueue = xQueueCreate(QUEUE_REC_NUM, sizeof(queueMsg));
  pulseLedOff = xTimerCreate("TimerLedOff", 4000, pdFALSE, (void*) 0, vLedOff);

  xTaskCreate(vTaskButton, "vTaskButton", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  xTaskCreate(vTaskControl, "vTaskControl", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
  vTaskStartScheduler();
  while(1){}
}

/*************************** End of file ****************************/
