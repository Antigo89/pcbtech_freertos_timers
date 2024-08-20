/* Use USART3 (stm32f407) for debug output.
 Configured as 9600 baud, 8 bit, no parity, 1 stop.
 Call init function first, then use printf.

 This implementation only supports segger studio.
 */

void usart1_init(void);