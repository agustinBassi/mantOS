/*==================[inclusions]=============================================*/

#include "uart_ios.h"
//#include "ciaaUART.h"
#include "os_config.h"
#include "os.h"
#include "board.h"
#include "queue.h"

/*==================[macros and definitions]=================================*/

/** tamaño de pila para los threads */
#define STACK_SIZE 2048

#define SEMAPHORE_QUEUE	0
#define SEMAPHORE_2	1
#define SEMAPHORE_3	2
#define SEMAPHORE_4	3
#define SEMAPHORE_5	4

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/** Funciones de inicializacion*/
static void InterruptsInit 	(void);
static void HardwareInit	(void);

/** Tareas comunes de usuario. */
static void * Task1			(void * param);
static void * Task2			(void * param);

/** Tareas que trabajan con interrupciones. */
void *		Isr_CiaaTec1 		(void);
void *		Isr_CiaaTec2 		(void);
void *		Isr_CiaaTec3 		(void);
void *		Isr_CiaaTec4 		(void);

void *		Isr_CiaaUartUsb		(void);

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/* cola para el ejercicio */
Queue_t Queue;

/* pilas de cada tarea */
uint8_t StackFrame1[STACK_SIZE];
uint8_t StackFrame2[STACK_SIZE];

const TaskDefinition_t Os_TaskList[TASK_COUNT] = {
		{StackFrame1, STACK_SIZE, Task1, (void *)0xAAAAAAAA, TASK_PRIORITY_LOW},
		{StackFrame2, STACK_SIZE, Task2, (void *)0xBBBBBBBB, TASK_PRIORITY_MEDIUM}
};

/*==================[internal functions definition]==========================*/

/**
 * Inicializa el hardware donde corre el sistema operativo.
 */
static void HardwareInit (){
	/* Inicialización del MCU */
	Board_Init();

    ciaaIOInit();

//    uartIosInit();
}

/** Por cada interrupción que quiera utilizar la aplicacion de usuario se debe:
 *
 * 		Realizar las llamadas a CMSIS o LPCOpen en caso de la EDU CIAA NXP
 * 		para configurar la interrupcion que quiera utilizar.
 *
 * 		Cada interrupcion que se utilice debera estar vinculada
 * 		a una funcion del usuario declarada en este archivo. La vinculacion se realiza
 * 		con la funcion IsrHandlers_AttachIrq (IRQn_Type irqName, UserFunctionPointer_t Isr_xxx).
 * 		Con el motivo de facilitar los nombres de las funciones que atienden interrupciones se les debera poner
 * 		el prefijo Isr_xxx.
 *
 * 		Los nombres de las interrupciones (en el caso de la CIAA) estan en cmsis_43xx.h .
 *
 * 		Ejemplo de attach de interrupcion:
 * 				IsrHandlers_AttachIrq		(PIN_INT0_IRQn, Isr_GPIO0);
 * */
static void InterruptsInit (){

	Chip_PININT_Init(LPC_GPIO_PIN_INT);

	/** Llamadas a LPC Open para configurar interrupcion PIN_INT0 en la TEC 1 de la EDU CIAA NXP*/
	Chip_SCU_GPIOIntPinSel		(0, 0, 4);
	Chip_PININT_SetPinModeEdge	(LPC_GPIO_PIN_INT,	PININTCH0);
	Chip_PININT_EnableIntLow	(LPC_GPIO_PIN_INT,	PININTCH0);
//	Chip_PININT_EnableIntHigh	(LPC_GPIO_PIN_INT,	PININTCH0);
   /** Vincula las interrupcion con la funcion de usuario. */
	IsrHandlers_AttachIrq		(PIN_INT0_IRQn, 	Isr_CiaaTec1);
//
	/** Llamadas a LPC Open para configurar interrupcion PIN_INT1 en la TEC 2 de la EDU CIAA NXP*/
	Chip_SCU_GPIOIntPinSel		(1, 0, 8);
	Chip_PININT_SetPinModeEdge	(LPC_GPIO_PIN_INT,	PININTCH1);
	Chip_PININT_EnableIntLow	(LPC_GPIO_PIN_INT,	PININTCH1);
//	Chip_PININT_EnableIntHigh	(LPC_GPIO_PIN_INT,	PININTCH1);
   /** Vincula las interrupcion con la funcion de usuario. */
	IsrHandlers_AttachIrq		(PIN_INT1_IRQn, 	Isr_CiaaTec2);

	/** Llamadas a LPC Open para configurar interrupcion PIN_INT2 en la TEC 3 de la EDU CIAA NXP*/
	Chip_SCU_GPIOIntPinSel		(2, 0, 9);
	Chip_PININT_SetPinModeEdge	(LPC_GPIO_PIN_INT,	PININTCH2);
	Chip_PININT_EnableIntLow	(LPC_GPIO_PIN_INT,	PININTCH2);
//	Chip_PININT_EnableIntHigh	(LPC_GPIO_PIN_INT,	PININTCH2);
   /** Vincula las interrupcion con la funcion de usuario. */
	IsrHandlers_AttachIrq		(PIN_INT2_IRQn, 	Isr_CiaaTec3);

	/** Llamadas a LPC Open para configurar interrupcion PIN_INT3 en la TEC 4 de la EDU CIAA NXP*/
	Chip_SCU_GPIOIntPinSel		(3, 1, 9);
	Chip_PININT_SetPinModeEdge	(LPC_GPIO_PIN_INT,	PININTCH3);
	Chip_PININT_EnableIntLow	(LPC_GPIO_PIN_INT,	PININTCH3);
//	Chip_PININT_EnableIntHigh	(LPC_GPIO_PIN_INT,	PININTCH3);
   /** Vincula las interrupcion con la funcion de usuario. */
	IsrHandlers_AttachIrq		(PIN_INT3_IRQn, 	Isr_CiaaTec4);

//	IsrHandlers_AttachIrq(USART2_IRQn, Isr_CiaaUartUsb);


	NVIC_EnableIRQ(PIN_INT0_IRQn);
	NVIC_EnableIRQ(PIN_INT1_IRQn);
	NVIC_EnableIRQ(PIN_INT2_IRQn);
	NVIC_EnableIRQ(PIN_INT3_IRQn);
//	NVIC_EnableIRQ(USART2_IRQn);


}

static void * Task1(void * param){

	QueueItem_t item;
	while (1) {
		item = Queue_GetItem(&Queue);
		// PUNTO DE DEBUG ACA PORQUE NO ANDA LA UART.
		//ACA PODES APRETAR CADA BOTON Y VER QUE EL DATO LE LLEGA
		//
		item = item;

	}
	return (void *)0;
}

static void * Task2(void * param){
	while (1) {
		Os_Delay(5000);
		Queue_PutItem(&Queue, 3.1416);
	}
	return (void *)0;
}

/*==================[Interrupts functions definition]==========================*/

void *		Isr_CiaaTec1 (void){

	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH0);
	Queue_PutItem(&Queue, 1.1);
}


void *		Isr_CiaaTec2 (void){

	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH1);
	Queue_PutItem(&Queue, 2.2);
}


void *		Isr_CiaaTec3 (void){

	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH2);
	Queue_PutItem(&Queue, 3.3);
}


void *		Isr_CiaaTec4 (void){

	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH3);
	Queue_PutItem(&Queue, 4.4);
}

void *		Isr_CiaaUartUsb		(void){
//	char dataReceived;
//	char dataSend [] = "Recibido:  .\n\r";
//	uartInterrupt();
//	uartRecv(CIAA_UART_USB, &dataReceived, sizeof(char));
//	dataSend[10] = dataReceived;
//	uartSend(CIAA_UART_USB, dataSend, strlen (dataSend));
}

/*==================[external functions definition]==========================*/

int main(void){

	/** Inicializa el hardware donde corre el sistema operativo. */
	HardwareInit();

	/** Inicializa las interrupciones de los perifericos y se les asocia un handler.*/
	InterruptsInit();

	if (Queue_Init(&Queue, SEMAPHORE_QUEUE) != 0) {
		/* error al crear cola */
//		while (1) { }
	}

	/** Inicia OS. */
	Os_Start();

	/** No debería volver acá. */
	while(1);
}

/*==================[end of file]============================================*/
