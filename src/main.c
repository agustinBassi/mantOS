/*==================[inclusions]=============================================*/

#include "os.h"
#include "board.h"

/*==================[macros and definitions]=================================*/

/** tamaño de pila para los threads */
#define STACK_SIZE 512

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

static void * Task1(void * param);
static void * Task2(void * param);
static void * Task3(void * param);
static void * Task4(void * param);
static void * Task5(void * param);

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/* pilas de cada tarea */
uint8_t stackFrame1[STACK_SIZE];
uint8_t StackFrame2[STACK_SIZE];
uint8_t StackFrame3[STACK_SIZE];
uint8_t StackFrame4[STACK_SIZE];
uint8_t StackFrame5[STACK_SIZE];

const TaskDefinition_t Os_TaskList[TASK_COUNT] = {
		{stackFrame1, STACK_SIZE, Task1, (void *)0xAAAAAAAA, TASK_PRIORITY_LOW},
		{StackFrame2, STACK_SIZE, Task2, (void *)0xBBBBBBBB, TASK_PRIORITY_MEDIUM},
		{StackFrame3, STACK_SIZE, Task3, (void *)0xBBBBBBBB, TASK_PRIORITY_HIGH},
		{StackFrame4, STACK_SIZE, Task4, (void *)0xBBBBBBBB, TASK_PRIORITY_HIGH},
		{StackFrame5, STACK_SIZE, Task5, (void *)0xBBBBBBBB, TASK_PRIORITY_HIGH}
};

/*==================[internal functions definition]==========================*/

static void * Task1(void * param){
	int j=4;
	while (1) {
		Board_LED_Toggle(1);
		Os_Delay(500);
//		for(j=0; j<0x8FFFFF; j++);
	}
	return (void *)0; /* a dónde va? */
}

static void * Task2(void * param){
	int j=4;
	while (j) {
		Board_LED_Toggle(2);
//		for(j=0; j<0xFFFFFF; j++);
		Os_Delay(650);
	}
	return (void *)4; /* a dónde va? */
}

static void * Task3(void * param){
	int j=4;
	while (j) {
		Board_LED_Toggle(3);
//		for(j=0; j<0x5FFFFF; j++);
		Os_Delay(800);
	}
	return (void *)4; /* a dónde va? */
}

static void * Task4(void * param){
	int j=4;
	while (j) {
		Board_LED_Toggle(4);
//		for(j=0; j<0xAFFFFF; j++);
		Os_Delay(900);
	}
	return (void *)4; /* a dónde va? */
}

static void * Task5(void * param){
	int j=4;
	while (j) {
		Board_LED_Toggle(5);
//		for(j=0; j<0xFFFFFF; j++);
		Os_Delay(250);
	}
	return (void *)4; /* a dónde va? */
}

/*==================[external functions definition]==========================*/

int main(void)
{
	/* Inicialización del MCU */
	Board_Init();

	/* Inicio OS */
	Os_Start();

	/* no deberíamos volver acá */
	while(1);
}

/*==================[end of file]============================================*/
