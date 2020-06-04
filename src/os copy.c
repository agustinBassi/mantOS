
/*==================[inclusions]=============================================*/

#include "os.h"
#include "board.h"


#include <string.h>

/*==================[macros and definitions]=================================*/

/** Numero de tareas utulizado por el OS. */
#define TASK_COUNT_OS 				(TASK_COUNT+1)

/** Valor de retorno de excepción a cargar en el LR. */
#define EXC_RETURN 					0xFFFFFFF9

/** Id de tarea idle. */
#define TASK_IDLE 					(TASK_COUNT_OS - 1)

/** Id de tarea inválida. */
#define TASK_INVALID 				(TASK_IDLE + 1)

/** Tamaño de stack de tarea idle. */
#define STACK_IDLE_SIZE 			256

/** ID de un semaforo invalido. */
#define SEMAPHORE_BINARY_INVALID 	SEMAPHORE_BINARY_COUNT+1

/** Posibles eventos que le pueden ocurrir a las tareas. */
typedef enum TaskEvent{
	TASK_EVENT_INVALID,
	TASK_EVENT_SCHED,
	TASK_EVENT_PREEMT,
	TASK_EVENT_RETURN,
	TASK_EVENT_RETURN_DEATACHED,
	TASK_EVENT_JOIN,
	TASK_EVENT_BLOCK,
	TASK_EVENT_UNBLOCK,
	TASK_EVENT_START,
	TASK_EVENT_TERMINATE
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

typedef struct SemaphoreBinary {
	uint32_t 					id;
	uint32_t 					ticsToWait;
} SemaphoreBinary_t;

/** estructura interna de control de tareas */
typedef struct TaskControlBlock {
	uint32_t 					stackPointer;
	const TaskDefinition_t * 	taskDefinition;
	TaskState_t 				taskState;
	uint32_t 					waitingTime;
	SemaphoreBinary_t			semaphoreBinary;
} TaskControlBlock_t;

/*==================[internal data declaration]==============================*/

/** Lista de tareas en estado ready que se mirara para seleccionar la siguiente tarea a ejecutar. */
static uint8_t 					ReadyTasksList 				[TASK_PRIORITY_COUNT][TASK_COUNT_OS];

/** Estructura interna de control de tareas. */
static TaskControlBlock_t 		TaskControlList				[TASK_COUNT_OS];

/** Contexto de la tarea idle. */
static uint8_t 					StackFrameIdle				[STACK_IDLE_SIZE];

/** Arreglo con los estados (TAKEN o GIVEN) de semaforos binariosS que posee el sistema. */
static SemaphoreBinaryState_t 	SemaphoresBinariesStates 	[SEMAPHORE_BINARY_COUNT];

/*==================[internal functions declaration]=========================*/

__attribute__ ((weak)) void * IdleHook(void * p);

static void 	ReturnHook							(void * returnValue);
static uint32_t AddTaskToReadyList 					(uint32_t taskIndex, uint8_t priorityLevel);
static uint32_t RemoveTaskFromReadyList 			(uint32_t taskIndex, uint8_t priorityLevel);
static void 	UpdateTasksDelay					(void);
static void 	UpdateTasksSemaphoresBinaries		(void);
static void 	ProcessTaskEvent					(uint32_t taskIndex, TaskEvent_t taskEvent);
static void 	CreateInitialTaskContext							(uint8_t * stackFrame,
													uint32_t stackFrameSize,
													uint32_t * stackPointer,
													EntryPoint_t entryPoint,
													void * parameter,
													TaskState_t * taskState);


/*==================[internal data definition]===============================*/

/** Estado en el que se puede encontrar el sistema operativo. */
static OsState_t OsState = OS_STATE_INVALID;

/** Indice a la tarea actual. */
static uint32_t RunningTask = TASK_INVALID;


/** Estructura de definicion de la tarea idle. */
static const TaskDefinition_t TaskDefinitionIdle = {
		StackFrameIdle, STACK_IDLE_SIZE, IdleHook, 0, TASK_PRIORITY_IDLE
};

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/**
 * Esta funcion es una maquina de estados de las tareas.
 * Define hacia que estado puede ir la tarea en funcion del estado en el que esta.
 * Realiza todas las tareas que se necesitan para procesar un evento.
 * Principalmente agrega o quita funciones a la lista de tareas ready.
 * Tambien llama al scheduler en los puntos de scheduling (cuando la tarea es bloqueada).
 * Tambien cambia los estados de las tareas.
 * @param taskIndex indice de la tarea sobre la cual se va a procesar el evento.
 * @param taskEvent evento que se quiere procesar.
 */
static void 	ProcessTaskEvent 		(uint32_t taskIndex, TaskEvent_t taskEvent){

	switch (TaskControlList[taskIndex].taskState){
		case TASK_STATE_READY:
			if (taskEvent == TASK_EVENT_SCHED){
				TaskControlList[taskIndex].taskState = TASK_STATE_RUNNING;
				RemoveTaskFromReadyList(taskIndex, TaskControlList[taskIndex].taskDefinition->priorityLevel);
			} else if (taskEvent == TASK_EVENT_BLOCK){
				TaskControlList[taskIndex].taskState = TASK_STATE_WAITING;
				RemoveTaskFromReadyList(taskIndex, TaskControlList[taskIndex].taskDefinition->priorityLevel);
				Os_Schedule();
			}  else if (taskEvent == TASK_EVENT_RETURN_DEATACHED){
				TaskControlList[taskIndex].taskState = TASK_STATE_TERMINATED;
				RemoveTaskFromReadyList(taskIndex, TaskControlList[taskIndex].taskDefinition->priorityLevel);
			}
		break;
		case TASK_STATE_RUNNING:
			if (taskEvent == TASK_EVENT_PREEMT){
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
				Os_Schedule();
			}
		break;
		case TASK_STATE_WAITING:
			if (taskEvent == TASK_EVENT_UNBLOCK){
				TaskControlList[taskIndex].taskState = TASK_STATE_READY;
				AddTaskToReadyList(taskIndex, TaskControlList[taskIndex].taskDefinition->priorityLevel);
			} else if (taskEvent == TASK_EVENT_RETURN_DEATACHED){
				TaskControlList[taskIndex].taskState = TASK_STATE_TERMINATED;
				RemoveTaskFromReadyList(taskIndex, TaskControlList[taskIndex].taskDefinition->priorityLevel);
			}
		break;
		case TASK_STATE_TERMINATED:
			if (taskEvent == TASK_EVENT_START){
				TaskControlList[taskIndex].taskDefinition = Os_TaskList + taskIndex;

				CreateInitialTaskContext(
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
static void 	ReturnHook				(void * returnValue){
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
static void 	CreateInitialTaskContext(
		uint8_t * 		stackFrame,
		uint32_t 		stackFrameSize,
		uint32_t * 		stackPointer,
		EntryPoint_t	entryPoint,
		void * 			parameter,
		TaskState_t * 	state){

	uint32_t * auxStackFrame = (uint32_t *)stackFrame;
	/** Inicializo el frame en cero. */
	bzero(auxStackFrame, stackFrameSize);
	/** Ultimo elemento del contexto inicial: xPSR. Necesita el bit 24 (T, modo Thumb) en 1. */
	auxStackFrame[stackFrameSize/4 - 1] = 1<<24;
	/** Anteúltimo elemento: PC (entry point). */
	auxStackFrame[stackFrameSize/4 - 2] = (uint32_t)entryPoint;
	/** Penúltimo elemento: LR (return hook). */
	auxStackFrame[stackFrameSize/4 - 3] = (uint32_t)ReturnHook;
	/** Elemento -8: R0 (parámetro) */
	auxStackFrame[stackFrameSize/4 - 8] = (uint32_t)parameter;
	/** Valor del LR al volver de la interrupcion. */
	auxStackFrame[stackFrameSize/4 - 9] = EXC_RETURN;
	/** Inicializa stack pointer inicial. */
	*stackPointer = (uint32_t)&(auxStackFrame[stackFrameSize/4 - 17]);
	/** Seteo estado inicial READY. */
	*state = TASK_STATE_READY;
}

/**
 * Actualiza el contador de delay de todas las tareas.
 * */
static void 	UpdateTasksDelay			(void){
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
 * Actualiza los semaforos binarios de todas las tareas.
 * Por un lado, desbloquea las tareas que estaban esperando por un semaforo que ya fue liberado.
 * Por otro lado descuenta el tiempo de espera de un semaforo para cada tarea y en caso que haya expirado el tiempo, desbloquea la tarea.
 */
static void 	UpdateTasksSemaphoresBinaries	(void){
	uint32_t taskIndex;
	/** Recorre todas las tareas para ver si tienen semaforos attachados. */
	for (taskIndex = 0; taskIndex < TASK_COUNT_OS; taskIndex++) {
		/** Si la tarea esta esperando la liberacion de un semaforo valido... */
		if ((TaskControlList[taskIndex].semaphoreBinary.id != SEMAPHORE_BINARY_INVALID)){
			/** Si el semaforo que tiene la tarea fue liberado desbloquea la tarea. */
			if ((SemaphoresBinariesStates[TaskControlList[taskIndex].semaphoreBinary.id] == SEMAPHORE_BINARY_STATE_GIVEN)){
				ProcessTaskEvent(taskIndex, TASK_EVENT_UNBLOCK);
			}
			/** Si el semaforo que tiene la tarea no se esta esperando infinitamente y se cumplio el tiempo de espera, desbloquea la tarea. */
			else if ((TaskControlList[taskIndex].semaphoreBinary.ticsToWait != OS_MAX_DELAY) &&
					(--TaskControlList[taskIndex].semaphoreBinary.ticsToWait == 0)){
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
static uint32_t AddTaskToReadyList 		(uint32_t taskIndex, uint8_t priorityLevel){

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
static uint32_t RemoveTaskFromReadyList (uint32_t taskIndex, uint8_t priorityLevel){
	uint32_t taskIndexVolatile;

	if (priorityLevel >= 0 && priorityLevel < TASK_PRIORITY_COUNT){
		for (taskIndexVolatile = 0; taskIndexVolatile < TASK_COUNT_OS; taskIndexVolatile++){
			if (ReadyTasksList[priorityLevel][taskIndexVolatile] == taskIndex){
				// todo: hacer una funcion que tenga una funcion shift array para poner el primer elemento del arreglo en un indice valido.
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
void * 			IdleHook				(void * p){
	while (1) {
		__WFI();
	}
}


/*==================[external functions definition]==========================*/

/**
 * Busca si en la lista de tareas ready hay una tarea de mayor prioridad que la de la tarea actual.
 * Esta funcion es utilizada en la funcion OsIrqHandlerDefault() en isr_handlers.c.
 * Sirve para que se pueda cambiar de contexto lo más pronto posible si luego de
 * ejecutar el handler de interrupcion se activo una tarea de mas alta prioridad.
 * @return 	TRUE si hay una tarea de mayor prioridad que la tarea que esta corriendo en estado ready.
 * 			FALSE caso contrario.
 */
Boolean_t			Os_IsHigherPriorityTaskReady	(void){
	Boolean_t isHigherPriorityTaskReady = FALSE;
	uint32_t taskPriorityLevel;
	/** El for es recorrido desde el indice de mayor prioridad hasta la prioridad de la tarea actual + 1. */
	for (taskPriorityLevel = TASK_PRIORITY_HIGH; taskPriorityLevel > TaskControlList[RunningTask].taskDefinition->priorityLevel; taskPriorityLevel--){
		if (ReadyTasksList[taskPriorityLevel][0] <= TASK_IDLE){
			isHigherPriorityTaskReady = TRUE;
			break;
		}
	}
	return isHigherPriorityTaskReady;
}

/**
 * Devuelve el estado en que esta corriendo actualmente el sistema operativo.
 * @return valor del estado del sistema (El posible valor esta en os.h).
 */
OsState_t			Os_GetOsState			(void){
	return OsState;
}

/**
 * Setea el estado actual del sistema.
 * @param osState estado a cambiar.
 */
void				Os_SetOsState			(OsState_t osState){
	OsState = osState;
}

/**
 * Toma (ocupa) o pone luz roja al semaforo.
 * Cuando un semaforo es tomado ninguna otra tarea puede tomarlo.
 * @param semaphoreId id del semaforo a tomar (uno de los semaforos dentro de SemaphoresBinaryStates).
 * @param waitingTime tiempo de espera por ese semaforo
 * 		OS_MAX_DELAY				-> espera para siempre.
 * 		Desde 0 a (OS_MAX_TIME - 1)	-> espera por los ticsToWait.
 * 		0							-> no espera.
 * @return estado del semaforo.
 * 		STATE_SEMAPHORE_TAKEN si el semaforo tenia luz roja.
 * 		STATE_SEMAPHORE_GIVEN si el semaforo tenia luz verde.
 */
 SemaphoreBinaryState_t Os_SemaphoreBinaryTake (uint8_t semaphoreBinaryId, uint32_t ticsToWait){
	 SemaphoreBinaryState_t semaphoreStateToReturn = SEMAPHORE_BINARY_STATE_TAKEN;
//	 uint32_t localRunningTask = RunningTask;

	 if (semaphoreBinaryId >= 0 && semaphoreBinaryId < SEMAPHORE_BINARY_COUNT){
		 if (SemaphoresBinariesStates[semaphoreBinaryId] == SEMAPHORE_BINARY_STATE_GIVEN){
			 /** Devuelve que el semaforo pudo ser tomado. */
			 semaphoreStateToReturn = SEMAPHORE_BINARY_STATE_GIVEN;
			 /** La tarea toma el semaforo y se cambia el estado a semaforo tomado. */
			 SemaphoresBinariesStates[semaphoreBinaryId] = SEMAPHORE_BINARY_STATE_TAKEN;
			 /** Hace que la tarea ya no tenga un semaforo asociado porque lo pudo tomar. */
			 TaskControlList[RunningTask].semaphoreBinary.id = SEMAPHORE_BINARY_INVALID;
		 } else if (ticsToWait != 0) {
			 TaskControlList[RunningTask].semaphoreBinary.id = semaphoreBinaryId;
			 TaskControlList[RunningTask].semaphoreBinary.ticsToWait = ticsToWait;
			 ProcessTaskEvent(RunningTask, TASK_EVENT_BLOCK);
			 semaphoreStateToReturn = SemaphoresBinariesStates[semaphoreBinaryId];
			 if (SemaphoresBinariesStates[semaphoreBinaryId] == SEMAPHORE_BINARY_STATE_GIVEN){
				 /** La tarea toma el semaforo y se cambia el estado a semaforo tomado. */
				 SemaphoresBinariesStates[semaphoreBinaryId] = SEMAPHORE_BINARY_STATE_TAKEN;
				 /** Hace que la tarea ya no tenga un semaforo asociado. */
				 TaskControlList[RunningTask].semaphoreBinary.id = SEMAPHORE_BINARY_INVALID;
			 }
		 } else {
			 /** El semaforo esta tomado y no se decide esperar a que se libere.*/
			 semaphoreStateToReturn = SEMAPHORE_BINARY_STATE_TAKEN;
		 }
	 }
	 return semaphoreStateToReturn;
 }

/**
 * Give (free) or put green light on semaphore.
 * @param semaphoreId semaphore to free.
 */
void 			Os_SemaphoreBinaryGive 		(uint8_t semaphoreBinaryId){
	if (semaphoreBinaryId >= 0 && semaphoreBinaryId < SEMAPHORE_BINARY_COUNT){
		SemaphoresBinariesStates [semaphoreBinaryId] = SEMAPHORE_BINARY_STATE_GIVEN;
		UpdateTasksSemaphoresBinaries();
	}
}

/**
 * Genera un delay por software que el sistema operativo se encarga de manejar.
 * Cuando pasa el tiempo seteado por el usuario, le devuelve el control a la tarea que lo llamo.
 * @param milliseconds tiempo en milisegundos a esperar.
 */
void 			Os_Delay				(uint32_t milliseconds){
	if (RunningTask != TASK_INVALID && milliseconds > 0) {
		TaskControlList[RunningTask].waitingTime = milliseconds;
		ProcessTaskEvent(RunningTask, TASK_EVENT_BLOCK);
	}
}

/**
 * Cambia el main stack pointer del sistema dependiendo del valor del stack pointer actual (hace el cambio de contexto).
 * En esta funcion se define la politica de scheduling del sistema operativo.
 * @param actualContext valor del msp (main stack pointer) actual. Cargado en r0 desde el ASM.
 * @return valor del stack pointer que se debe ejecutar (el ASM generado por GCC carga en r0 el valor del return)
 */
int32_t 		Os_GetNextContext		(int32_t currentStackPointer){

	uint32_t stackPointerToReturn;
	uint8_t taskPriorityLevel;

	/** Si es una tarea valida...
	 * 		Guarda el stack pointer de la tarea actual.
	 * 		Si la tarea actual esta corriendo (En estado running)...
	 * 			Pone la tarea en estado ready
	 * 			Agrega la tarea a la lista de tareas listas para ejecutarse
	 * 	(Se realiza lo de agregar a la tarea ready para simplificar la logica de seleccion de tareas a ejecutar.*/
	if (RunningTask < TASK_COUNT_OS) {
		TaskControlList[RunningTask].stackPointer = currentStackPointer;
		if (TaskControlList[RunningTask].taskState == TASK_STATE_RUNNING ){
			ProcessTaskEvent(RunningTask, TASK_EVENT_PREEMT);
		}
	}

	for (taskPriorityLevel = TASK_PRIORITY_HIGH; taskPriorityLevel >= TASK_PRIORITY_IDLE; taskPriorityLevel--){
		if (ReadyTasksList[taskPriorityLevel][0] <= TASK_IDLE){
			RunningTask = ReadyTasksList[taskPriorityLevel][0];
			ProcessTaskEvent(RunningTask, TASK_EVENT_SCHED);
			stackPointerToReturn = TaskControlList[RunningTask].stackPointer;
			break;
		}
	}

	return stackPointerToReturn;
}

/**
 * Activa la interrupcion por software del PendSV, de esta manera se procedera a
 * hacer el cambio de contexto.
 */
void 			Os_Schedule				(void){
	/** Activo PendSV para llevar a cabo el cambio de contexto. */
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
	/** Instruction Synchronization Barrier: asegura que se ejecuten todas las instrucciones en  el pipeline. */
	__ISB();
	/** Data Synchronization Barrier: asegura que se completen todos los accesos a memoria */
	__DSB();
}

/**
 * Inicializa el sistema operativo. En esta funcion se configura adecuadamente el
 * core del microcontrolador, se inicia la tarea idle junto con su stack y contexto y se
 * inician las tareas definidas por el usuario.
 * @return no debe retornar
 */
int 			Os_Start				(void){
	uint32_t auxIndex, priorityIndex;

	/** Actualiza SystemCoreClock (CMSIS) */
	SystemCoreClockUpdate();

	/** Inicializa array de prioridades de tareas. */
	for (priorityIndex = 0; priorityIndex < TASK_PRIORITY_COUNT; priorityIndex++){
		for (auxIndex = 0; auxIndex < TASK_COUNT_OS; auxIndex++){
			ReadyTasksList[priorityIndex][auxIndex] = TASK_INVALID;
		}
	}

	/** Inicializa el estado de los semaforos del sistema. */
	for (auxIndex = 0; auxIndex < SEMAPHORE_BINARY_COUNT; auxIndex++){
		SemaphoresBinariesStates[auxIndex] = SEMAPHORE_BINARY_STATE_TAKEN;
	}

	/** Inicializa contexto idle. */
	TaskControlList[TASK_IDLE].taskDefinition = &TaskDefinitionIdle;

	/** Inicializa contextos iniciales de cada tarea. */
	for (auxIndex = 0; auxIndex < TASK_COUNT_OS; auxIndex++) {

		TaskControlList[auxIndex].taskState = TASK_STATE_TERMINATED;
		TaskControlList[auxIndex].semaphoreBinary.id = SEMAPHORE_BINARY_INVALID;
		ProcessTaskEvent(auxIndex, TASK_EVENT_START);
	}

	/** Configura PendSV con la prioridad más baja. */
	NVIC_SetPriority(PendSV_IRQn, 255);
	SysTick_Config(SystemCoreClock / 1000);

	/** Pone el estado de sistema en modo tarea de usuario. */
	Os_SetOsState(OS_STATE_TASK);

	/** Llama al scheduler. */
	Os_Schedule();

	return -1;
}

/*==================[external functions definition of core interrupts]=======*/

void CoreIrqHandlerDefault (){
	/** Si sucede un error inesperado resetea el sistema. */
//	NVIC_SystemReset();
}

__attribute__ ((section(".after_vectors")))
void NMI_Handler(void) {
    while (1) {
    	CoreIrqHandlerDefault ();
    }
}
__attribute__ ((section(".after_vectors")))
void HardFault_Handler(void) {
    while (1) {
    	CoreIrqHandlerDefault ();
    }
}
__attribute__ ((section(".after_vectors")))
void MemManage_Handler(void) {
    while (1) {
    	CoreIrqHandlerDefault ();
    }
}
__attribute__ ((section(".after_vectors")))
void BusFault_Handler(void) {
    while (1) {
    	CoreIrqHandlerDefault ();
    }
}
__attribute__ ((section(".after_vectors")))
void UsageFault_Handler(void) {
    while (1) {
    	CoreIrqHandlerDefault ();
    }
}
__attribute__ ((section(".after_vectors")))
void SVC_Handler(void) {
    while (1) {
    	CoreIrqHandlerDefault ();
    }
}
__attribute__ ((section(".after_vectors")))
void DebugMon_Handler(void) {
    while (1) {
    }
}
//__attribute__ ((section(".after_vectors")))
//void PendSV_Handler(void) {
//    while (1) {
//    }
//}

/**
 * Rutina de interrupcion del systic del sistema.
 * El nombre de las interrupciones esta en cr_startup_xx.c del uC del sistema.
 */
__attribute__ ((section(".after_vectors")))
void 			SysTick_Handler			(void){
	UpdateTasksDelay();
	UpdateTasksSemaphoresBinaries();
	Os_Schedule();
}

__attribute__ ((section(".after_vectors")))
void IntDefaultHandler(void) {
    while (1) {
    }
}

/*==================[end of file]============================================*/

//todo: Preguntar como seria la sobrecarga de funciones para poder inicializar al OS con parametros desde Os_Start ()

//todo: que la tarea add task ponga a la tarea en estado ready

//todo: poner una API interna al OS que se llame changeTaskState y que se le pase el enum de estados y los cambie ahi dentro

//todo: buffer circular para agregar/quitar tareas

//todo: rebobinar la cinta cuando se deba poner al indice de la tarea actual en el primer lugar
