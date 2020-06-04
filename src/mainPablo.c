///*==================[inclusions]=============================================*/
//
//#include "os.h"
//#include "queue.h"
//#include "board.h"
//#include "ciaaUART.h"
//#include "ciaaIO.h"
//
//#include <stdlib.h>
//#include <string.h>
//
///*==================[macros and definitions]=================================*/
//
///** tamaño de pila para los threads */
//#define STACK_SIZE 2048
//
///*==================[internal data declaration]==============================*/
//
///*==================[internal functions declaration]=========================*/
//
//static void * tarea1(void * param);
//static void * tarea2(void * param);
//
///*==================[internal data definition]===============================*/
//
///*==================[external data definition]===============================*/
//
///* cola para el ejercicio */
//queue_t q;
//
///* pilas de cada tarea */
//uint8_t stack1[STACK_SIZE];
//uint8_t stack2[STACK_SIZE];
//
//const taskDefinition task_list[TASK_COUNT] = {
//		{stack1, STACK_SIZE, tarea1, (void *)0xAAAAAAAA, TASK_PRIORITY_LOW},
//		{stack2, STACK_SIZE, tarea2, (void *)0xBBBBBBBB, TASK_PRIORITY_MEDIUM},
//};
//
///*==================[internal functions definition]==========================*/
//
//static void * tarea1(void * param)
//{
//	queueItem_t item;
//	char str[100];
//
//	strcpy(str, "[t1] esperando datos...\r\n");
//	uartSend(str, strlen(str));
//
//	while (1) {
//		item = queueGet(&q);
//		/* la siguiente es una forma alternativa al %f para formatear un float
//		 * si uso directamente %f se me va a hard fault :(
//		 * si alguno lo soluciona y usa %f, lo tendré en cuenta ;)
//		 */
//		sprintf(str, "[t1] recibido: %d.%d\r\n", (int)item,(int)(item*10000)-((int)item)*10000);
//		uartSend(str, strlen(str));
//	}
//	return NULL;
//}
//
//static void * tarea2(void * param)
//{
//	char str[50];
//
//	strcpy(str, "[t2] enviando pi cada 5 segundos...\r\n");
//	uartSend(str, strlen(str));
//
//	while (1) {
//		delay(5000);
//		strcpy(str, "[t2] enviando datos...\r\n");
//		uartSend(str, strlen(str));
//		queuePut(&q, 3.1416);
//	}
//	return NULL;
//}
//
///*==================[external functions definition]==========================*/
//
//int main(void)
//{
//	/* Inicialización del MCU */
//	Board_Init();
//
//	uartInit();
//
//	ioInit();
//
//	if (queueInit(&q) != 0) {
//		/* error al crear cola */
//		while (1) { }
//	}
//
//	/* Inicio OS */
//	start_os();
//
//	/* no deberíamos volver acá */
//	while(1);
//}
//
///*==================[end of file]============================================*/
