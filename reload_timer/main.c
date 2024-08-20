/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

Задание 2
Реализовать систему управления гирляндой. Основные условия:
1. В рамках использования демо-платы предлагается использование двух или трёх пользовательских
светодиодов (по выбору разработчика), и одна пользовательская кнопка со следующим поведением:
1.1. Исходное состояние системы – диоды выключены.
1.2. При нажатии на кнопку происходит включение первого режима работы «гирлянды» – постоянное
свечение всех диодов.
1.3. При повторном нажатии на кнопку происходит включение второго режима – включение и
выключение всех диодов с периодом 1 секунда.
1.4. При повторном нажатии на кнопку происходит включение третьего режима – включение и
выключение всех диодов с периодом 0,5 секунды.
1.5. При повторном нажатии на кнопку происходит остановка работы «гирлянды», все диоды гаснут.
1.6. Повторное нажатие на кнопку снова включает «гирлянду» в первый режим и повторяется
поведение п.1.3 – 1.5.
2. Выполнить сборку, запуск и тестирование проекта на отладочной плате.

Критерии готовности (Definition of Done)
1. Проект собирается и загружается в отладочную плату без ошибок и предупреждений.
2. Начальные условия при включении системы соблюдаются.
3. Нажатия на пользовательскую кнопку обрабатываются корректно и однозначно определяются,
многократные срабатывания не происходят.
4. Система работает согласно описанному сценарию работы гирлянды.
5. В решении используется один или несколько программных таймеров FreeRTOS в периодическом
режиме (auto-reload timer).


USING STM32F407VET6 board:

SWD interface
PE10 input key S1
PE13 output LED1
PE14 output LED2
PE15 output LED3

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
  uint8_t mode;
} queueMsg;

/************************global values********************************/
#define QUEUE_REC_NUM (10) //record number in queue
#define QUEUE_TIMEOUT_SEND (200) //timeout write in queue
#define QUEUE_TIMEOUT_REC (400) //timeout read in queue

#define MAX_MODE  (4) //max number mode

#define TIMER_PERION_LOW    (500)
#define TIMER_PERION_HIGHT  (1000)

/* Queue handler */
QueueHandle_t myQueue;
/* Timer Handler */
xTimerHandle generate;
/****************************func************************************/
void vLedToggle(xTimerHandle xTimer){
  if(GPIOE->ODR & GPIO_ODR_OD13){
    GPIOE->BSRR |= GPIO_BSRR_BR13|GPIO_BSRR_BR14|GPIO_BSRR_BR15;
  }else{
    GPIOE->BSRR |= GPIO_BSRR_BS13|GPIO_BSRR_BS14|GPIO_BSRR_BS15;
  }
}

/* read user button */
void vTaskButton(void * pvParameters){
  uint8_t ucMode = 0;
  queueMsg senderMsg;
  while(1){
    if(!(GPIOE->IDR & GPIO_IDR_ID10)){
      vTaskDelay(50);
      if(!(GPIOE->IDR & GPIO_IDR_ID10)){
        ucMode = ucMode == (MAX_MODE-1) ? 0 : ucMode+1;
        senderMsg.id = MSG_ID_KEYBOARD;
        senderMsg.mode = ucMode;
        xQueueSend(myQueue, &senderMsg, QUEUE_TIMEOUT_SEND); 
        while(!(GPIOE->IDR & GPIO_IDR_ID10)){};
      }    
    }
    vTaskDelay(100);
  }
  //vTaskDelete(NULL);
}
/* Read queue */

void vTaskControl(void * pvParameters){
  queueMsg receiveMsg;
  while(1){
    if(xQueueReceive(myQueue, &receiveMsg, QUEUE_TIMEOUT_REC) == pdTRUE){
      if(receiveMsg.id == MSG_ID_KEYBOARD){
        switch (receiveMsg.mode) {
          case 0:
            xTimerStop(generate, 100);
            GPIOE->BSRR |= GPIO_BSRR_BS13|GPIO_BSRR_BS14|GPIO_BSRR_BS15;
            break;
          case 1:
            //xTimerStop(generate, 100);
            GPIOE->BSRR |= GPIO_BSRR_BR13|GPIO_BSRR_BR14|GPIO_BSRR_BR15;
            break;
          case 2:
            //xTimerStop(generate, 100);
            xTimerChangePeriod(generate, TIMER_PERION_LOW, 100);
            xTimerStart(generate, 100);
            break;
          case 3:
            //xTimerStop(generate, 100);
            xTimerChangePeriod(generate, TIMER_PERION_HIGHT, 100);
            xTimerStart(generate, 100);
            break;
          default:
            break;
        }
      }
    }
  }
  //vTaskDelete(NULL);
}
/****************************main************************************/

 int main(void) {
  GPIO_init();
  
  myQueue = xQueueCreate(QUEUE_REC_NUM, sizeof(queueMsg));
  generate = xTimerCreate("TimLedOff", TIMER_PERION_LOW, pdTRUE, (void*) 0, vLedToggle);

  xTaskCreate(vTaskButton, "vTaskButton", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  xTaskCreate(vTaskControl, "vTaskControl", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  vTaskStartScheduler();
  while(1){}
}

/*************************** End of file ****************************/
