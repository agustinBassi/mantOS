/*==================[inclusions]=============================================*/

#include "board.h"
#include "os.h"

#include <string.h>

/*==================[macros and definitions]=================================*/

/** TASK COUNT number used by OS y task user count + 1*/
#define TASK_COUNT_OS (TASK_COUNT+1)

/** valor de retorno de excepción a cargar en el LR */
#define EXC_RETURN 0xFFFFFFF9

/** id de tarea idle */
#define TASK_IDLE (TASK_COUNT_OS - 1)

/** id de tarea inválida */
#define TASK_INVALID (TASK_IDLE + 1)

/** tamaño de stack de tarea idle */
#define STACK_IDLE_SIZE 256

/** Posibles eventos que le pueden ocurrir a las tareas. */
typedef enum TaskEvent{
	TASK_EVENT_INVALID,
	TASK_EVENT_PREEMT,
	TASK_EVENT_SCHED,
	TASK_EVENT_RETURN,
	TASK_EVENT_RETURN_DEATACHED,
	TASK_EVENT_JOIN,
	TASK_EVENT_BLOCK,
	TASK_EVENT_UNBLOCK,
	TASK_EVENT_START,
} TaskEvent_t;

/** posibles estados de una tarea */
typedef enum TaskState {
	TASK_STATE_INVALID,
	TASK_STATE_READY,
	TASK_STATE_RUNNING,
	TASK_STATE_WAITING,
	TASK_STATE_ZOMBIE,
	TASK_STATE_TERMINATED
} TaskState_t;

/** estructura interna de control de tareas */
typedef struct TaskControlBlock {
	uint32_t stackPointer;
	const TaskDefinition_t * taskDefinition;
	TaskState_t taskState;
	uint32_t waitingTime;
} TaskControlBlock_t;

/*==================[internal data declaration]==============================*/

/** Lista de tareas en estado ready que se mirara para seleccionar la siguiente tarea a ejecutar*/
uint8_t ReadyTasksList [TASK_PRIORITY_COUNT][TASK_COUNT_OS];

/** estructura interna de control de tareas */
static TaskControlBlock_t TaskControlList[TASK_COUNT_OS];

/** contexto de la tarea idle */
uint8_t StackFrameIdle[STACK_IDLE_SIZE];

/*==================[internal functions declaration]=========================*/

__attribute__ ((weak)) void * IdleHook(void * p);

static uint32_t AddTaskToReadyList(uint32_t taskIndex, uint8_t priorityLevel);
static uint32_t RemoveTaskFromReadyList(uint32_t taskIndex, uint8_t priorityLevel);
static void UpdateTaskDelay(void);
static void ReturnHook(void * returnValue);
static void CreateTask(uint8_t * stackFrame,
                       uint32_t stackFrameSize,
					   uint32_t * stackPointer,
					   EntryPoint_t entryPoint,
					   void * parameter,
					   TaskState_t * taskState);
static void ProcessTaskEvent (uint32_t taskIndex, TaskEvent_t taskEvent);

/*==================[internal data definition]===============================*/

/** indice a la tarea actual */
static uint32_t CurrentTask = TASK_INVALID;


/** Estructura de definicion de la tarea idle. */
static const TaskDefinition_t TaskDefinitionIdle = {
		StackFrameIdle, STACK_IDLE_SIZE, IdleHook, 0, TASK_PRIORITY_IDLE
};

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void ProcessTaskEvent(uint32_t taskIndex, TaskEvent_t taskEvent){
	TaskControlBlock_t auxTaskControlBlock;

	switch (TaskControlList[taskIndex].taskState){
		case TASK_STATE_READY:
			if (taskEvent == TASK_EVENT_PREEMT){
				TaskControlList[taskIndex].taskState = TASK_STATE_RUNNING;
				RemoveTaskFromReadyList(taskIndex, TaskControlList[taskIndex].taskDefinition->priorityLevel);
			} else if (taskEvent == TASK_EVENT_BLOCK){
				TaskControlList[taskIndex].taskState = TASK_STATE_WAITING;
				RemoveTaskFromReadyList(taskIndex, TaskControlList[taskIndex].taskDefinition->priorityLevel);
			}
		break;
		case TASK_STATE_RUNNING:
			if (taskEvent == TASK_EVENT_SCHED){
				TaskControlList[taskIndex].taskState = TASK_STATE_READY;
				AddTaskToReadyList(taskIndex, TaskControlList[taskIndex].taskDefinition->priorityLevel);
			} else if (taskEvent == TASK_EVENT_RETURN){
				TaskControlList[taskIndex].taskState = TASK_STATE_ZOMBIE;
				RemoveTaskFromReadyList(taskIndex, TaskControlList[taskIndex].taskDefinition->priorityLevel);
			} else if (taskEvent == TASK_EVENT_RETURN_DEATACHED){
				TaskControlList[taskIndex].taskState = TASK_STATE_TERMINATED;
				RemoveTaskFromReadyList(taskIndex, TaskControlList[taskIndex].taskDefinition->priorityLevel);
			} else if (taskEvent == TASK_EVENT_BLOCK){
				TaskControlList[taskIndex].taskState = TASK_STATE_WAITING;
				RemoveTaskFromReadyList(taskIndex, TaskControlList[taskIndex].taskDefinition->priorityLevel);
			}
		break;
		case TASK_STATE_WAITING:
			if (taskEvent == TASK_EVENT_UNBLOCK){
				TaskControlList[taskIndex].taskState = TASK_STATE_READY;
				AddTaskToReadyList(taskIndex, TaskControlList[taskIndex].taskDefinition->priorityLevel);
			}
		break;
		case TASK_STATE_TERMINATED:
			if (taskEvent == TASK_EVENT_START){
				TaskControlList[taskIndex].taskDefinition = Os_TaskList + taskIndex;

				CreateTask(
						TaskControlList[taskIndex].taskDefinition->stackFrame,
						TaskControlList[taskIndex].taskDefinition->stackSize,
					  &(TaskControlList[taskIndex].stackPointer),
						TaskControlList[taskIndex].taskDefinition->entryPoint,
						TaskControlList[taskIndex].taskDefinition->parameter,
					  &(TaskControlList[taskIndex].taskState));

				AddTaskToReadyList(taskIndex, TaskControlList[taskIndex].taskDefinition->priorityLevel);
			}
		break;
		case TASK_STATE_ZOMBIE:
			if (taskEvent == TASK_EVENT_JOIN){
				TaskControlList[taskIndex].taskState = TASK_STATE_TERMINATED;
				RemoveTaskFromReadyList(taskIndex, TaskControlList[taskIndex].taskDefinition->priorityLevel);
			}
		break;
		case TASK_STATE_INVALID:

		break;
	}
}

/**
 * Cuando una tarea retorna debe ir a un lugar conocido como este Hook.
 */
static void ReturnHook(void * returnValue){
	while(1);
}

/**
 * Crea el contexto inicial para cada tarea del SO.
 * El valor donde se guardan los valores del stack de cada tarea es en el vector StackFrameX[STACK_SIZE]
 * @param stackFrame Vector de pila (frame).
 * @param stackFrameSize Tamaño expresado en bytes.
 * @param stackPointer Donde guardar el puntero de pila.
 * @param entryPoint Punto de entrada de la tarea (donde iniciara la primer instruccion)
 * @param parameter parametro de la tarea.
 */
static void CreateTask(uint8_t * stackFrame,
                       uint32_t stackFrameSize,
                       uint32_t * stackPointer,
                       EntryPoint_t entryPoint,
                       void * parameter,
                       TaskState_t * state){

	uint32_t * auxStackFrame = (uint32_t *)stackFrame;

	/* inicializo el frame en cero */
	bzero(auxStackFrame, stackFrameSize);

	/* último elemento del contexto inicial: xPSR
	 * necesita el bit 24 (T, modo Thumb) en 1
	 */
	auxStackFrame[stackFrameSize/4 - 1] = 1<<24;

	/* anteúltimo elemento: PC (entry point) */
	auxStackFrame[stackFrameSize/4 - 2] = (uint32_t)entryPoint;

	/* penúltimo elemento: LR (return hook) */
	auxStackFrame[stackFrameSize/4 - 3] = (uint32_t)ReturnHook;

	/* elemento -8: R0 (parámetro) */
	auxStackFrame[stackFrameSize/4 - 8] = (uint32_t)parameter;

	auxStackFrame[stackFrameSize/4 - 9] = EXC_RETURN;

	/* inicializo stack pointer inicial */
	*stackPointer = (uint32_t)&(auxStackFrame[stackFrameSize/4 - 17]);

	/* seteo estado inicial READY */
	*state = TASK_STATE_READY;
}

/** Actualiza el contador de delay de la tarea actual. */
static void UpdateTaskDelay(void){
	uint32_t taskIndex;
	for (taskIndex = 0; taskIndex < TASK_COUNT_OS; taskIndex++) {
		if ( (TaskControlList[taskIndex].taskState == TASK_STATE_WAITING) && (TaskControlList[taskIndex].waitingTime > 0)) {
			if (--TaskControlList[taskIndex].waitingTime == 0) {
				ProcessTaskEvent(taskIndex, TASK_EVENT_UNBLOCK);
			}
		}
	}
}

/**
 * Agrega una tarea a la lista de tareas ready (que pueden ser ejecutadas).
 * @param taskIndex indice de la tarea
 * @param priorityLevel prioridad de la tarea
 * @return estado de la operacion
 */
static uint32_t AddTaskToReadyList(uint32_t taskIndex, uint8_t priorityLevel){

	uint32_t taskIndexVolatile;

	if (priorityLevel >= 0 && priorityLevel < TASK_PRIORITY_COUNT){
		for (taskIndexVolatile = 0; taskIndexVolatile < TASK_COUNT_OS; taskIndexVolatile++){
			if (ReadyTasksList[priorityLevel][taskIndexVolatile] == TASK_INVALID){
				ReadyTasksList[priorityLevel][taskIndexVolatile] = taskIndex;
				break;
			}
		}
	}
	return 0;
}

/**
 * Remueve una tarea de la lista de tareas ready (ya no puede ser ejecutada).
 * @param taskIndex indice de la tarea
 * @param priorityLevel prioridad de la tarea
 * @return estado de la operacion
 */
static uint32_t RemoveTaskFromReadyList(uint32_t taskIndex, uint8_t priorityLevel){
	uint32_t taskIndexVolatile;

	if (priorityLevel >= 0 && priorityLevel < TASK_PRIORITY_COUNT){
		for (taskIndexVolatile = 0; taskIndexVolatile < TASK_COUNT_OS; taskIndexVolatile++){
			if (ReadyTasksList[priorityLevel][taskIndexVolatile] == taskIndex){
				// todo: hacer una API que tenga una funcion shift array para poner el primer elemento del arreglo en un indice valido
				ReadyTasksList[priorityLevel][taskIndexVolatile] = TASK_INVALID;

				for (taskIndexVolatile = 0; taskIndexVolatile < (TASK_COUNT_OS-1); taskIndexVolatile++){
					ReadyTasksList[priorityLevel][taskIndexVolatile] = ReadyTasksList[priorityLevel][taskIndexVolatile + 1];
				}
				ReadyTasksList[priorityLevel][TASK_COUNT_OS-1] = TASK_INVALID;

				break;
			}
		}
	}
	return 0;
}

/**
 * Hook donde va el procesador si ya no quedan tareas por ejecutar.
 * @param p parametro generico (no se utiliza).
 */
void * IdleHook(void * p){
	while (1) {
		__WFI();
	}
}

/**
 * Rutina de interrupcion del systic del sistema.
 * El nombre de las interrupciones esta en cr_startup_xx.c del uC del sistema.
 */
void SysTick_Handler(void){
	UpdateTaskDelay();
	Os_Schedule();
}

/*==================[external functions definition]==========================*/

/**
 * Genera un delay por software que el sistema operativo se encarga de manejar.
 * Cuando pasa el tiempo seteado por el usuario, le devuelve el control a la tarea que lo llamo.
 * @param milliseconds tiempo en milisegundos a esperar.
 */
void Os_Delay(uint32_t milliseconds){
	if (CurrentTask != TASK_INVALID) {
		TaskControlList[CurrentTask].waitingTime = milliseconds;
		ProcessTaskEvent(CurrentTask, TASK_EVENT_BLOCK);
		Os_Schedule();
	}
}

/**
 * Cambia el main stack pointer del sistema dependiendo del valor del stack pointer actual (hace el cambio de contexto).
 * En esta funcion se define la politica de scheduling del sistema operativo.
 * @param actualContext valor del msp (main stack pointer) actual. Cargado en r0 desde el ASM.
 * @return valor del stack pointer que se debe ejecutar (el ASM generado por GCC carga en r0 el valor del return)
 */
int32_t Os_GetNextContext(int32_t currentStackPointer){

	uint32_t stackPointerToReturn;
	uint8_t taskPriorityLevel;

	/** Si es una tarea valida...
	 * Guarda el stack pointer de la tarea actual.
	 * Si la tarea actual esta corriendo (En estado running)...
	 * Pone la tarea en estado ready
	 * Agrega la tarea a la lista de tareas listas para ejecutarse
	 * (Se realiza lo de agregar a la tarea ready para simplificar la logica de seleccion de tareas.*/
	if (CurrentTask < TASK_COUNT_OS) {
		TaskControlList[CurrentTask].stackPointer = currentStackPointer;
		if (TaskControlList[CurrentTask].taskState == TASK_STATE_RUNNING ){
			ProcessTaskEvent(CurrentTask, TASK_EVENT_SCHED);
		}
	}

	for (taskPriorityLevel = TASK_PRIORITY_HIGH; taskPriorityLevel >= TASK_PRIORITY_IDLE; taskPriorityLevel--){
		if (ReadyTasksList[taskPriorityLevel][0] <= TASK_IDLE){
			CurrentTask = ReadyTasksList[taskPriorityLevel][0];
			ProcessTaskEvent(CurrentTask, TASK_EVENT_PREEMT);
			stackPointerToReturn = TaskControlList[CurrentTask].stackPointer;
			break;
		}
	}

	return stackPointerToReturn;
}

/**
 * Activa la interrupcion por software del PendSV, de esta manera se procedera a
 * hacer el cambio de contexto.
 */
void Os_Schedule(void){
	/* activo PendSV para llevar a cabo el cambio de contexto */
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
	/* Instruction Synchronization Barrier: aseguramos que se
	 * ejecuten todas las instrucciones en  el pipeline
	 */
	__ISB();
	/* Data Synchronization Barrier: aseguramos que se
	 * completen todos los accesos a memoria
	 */
	__DSB();
}

/**
 * Inicializa el sistema operativo. En esta funcion se configura adecuadamente el
 * core del microcontrolador, se inicia la tarea idle junto con su stack y contexto y se
 * inician las tareas definidas por el usuario.
 * @return no debe retornar
 */
int Os_Start(void){
	uint32_t taskIndex, priorityIndex;

	/* actualizo SystemCoreClock (CMSIS) */
	SystemCoreClockUpdate();

	/** Inicializa array de prioridades de tareas. */
	for (priorityIndex = 0; priorityIndex < TASK_PRIORITY_COUNT; priorityIndex++){
		for (taskIndex = 0; taskIndex < TASK_COUNT_OS; taskIndex++){
			ReadyTasksList[priorityIndex][taskIndex] = TASK_INVALID;
		}
	}

	/* inicializo contexto idle */
	TaskControlList[TASK_IDLE].taskDefinition = &TaskDefinitionIdle;

	/* inicializo contextos iniciales de cada tarea */
	for (taskIndex = 0; taskIndex < TASK_COUNT_OS; taskIndex++) {

		TaskControlList[taskIndex].taskState = TASK_STATE_TERMINATED;
		ProcessTaskEvent(taskIndex, TASK_EVENT_START);
	}

	/* configuro PendSV con la prioridad más baja */
	NVIC_SetPriority(PendSV_IRQn, 255);
	SysTick_Config(SystemCoreClock / 1000);

	/* llamo al scheduler */
	Os_Schedule();

	return -1;
}


/*==================[end of file]============================================*/

//todo: que la tarea add task ponga a la tarea en estado ready

//todo: poner una API interna al OS que se llame changeTaskState y que se le pase el enum de estados y los cambie ahi dentro

//todo: buffer circular para agregar/quitar tareas

//todo: rebobinar la cinta cuando se deba poner al indice de la tarea actual en el primer lugar

/* todo: hacer una maquina de estados de tareas en el que reciban el indice de la tarea que lo llama,
el estado en que se encuentra la tarea y el evento que sucedio, y en funcion de eso que devuelva el nuevo
estado de la tarea en cuestion
*/
