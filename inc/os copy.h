#ifndef _OS_H_
#define _OS_H_

/*==================[inclusions]=============================================*/

#include "os_config.h"
#include "isr_handlers.h"

/*==================[macros]=================================================*/

#define OS_MAX_DELAY	0xffffffff

/*==================[typedef]================================================*/

/** @brief: prototipo de las tareas, hilos de ejecucion o threads */
typedef void* (*EntryPoint_t)(void *);

/** @brief: Posibles estados en los que puede estar el sistema operativo a nivel global. */
typedef enum OsRunningState {
	OS_STATE_INVALID,
	OS_STATE_SYS,
	OS_STATE_TASK,
	OS_STATE_ISR
} OsState_t;

/** @brief: Posibles niveles de prioridad que puede tener una tarea. */
typedef enum TaskLevelPriority {
	TASK_PRIORITY_IDLE,
	TASK_PRIORITY_LOW,
	TASK_PRIORITY_MEDIUM,
	TASK_PRIORITY_HIGH,
	TASK_PRIORITY_COUNT
} TaskLevelPriority_t;

/** @brief: Estructura de definicón de tareas.
 * El usuario tiene que definir sus tareas bajo esta estructura. */
typedef struct TaskDefinition {
	uint8_t * 			stackFrame;
	uint32_t 			stackSize;
	EntryPoint_t 		entryPoint;
	void  * 			parameter;
	TaskLevelPriority_t priorityLevel;
} TaskDefinition_t;


/** @brief: Posibles estados en los que puede estar un semaforo binario. */
typedef enum SemaphoreState {
	SEMAPHORE_BINARY_STATE_GIVEN,
	SEMAPHORE_BINARY_STATE_TAKEN
} SemaphoreBinaryState_t;

/** @brief: Tipo de datos para definir a un booleano a partir de el tipo de datos ya definido Bool.
 * Es para no generar dependencia de tipo de datos. */
typedef Bool Boolean_t;

/*==================[external data declaration]==============================*/

extern const TaskDefinition_t Os_TaskList[TASK_COUNT];

/*==================[external functions declaration]=========================*/

int 						Os_Start						(void);
void 						Os_Schedule						(void);
void 						Os_Delay						(uint32_t milliseconds);
SemaphoreBinaryState_t 		Os_SemaphoreBinaryTake 			(uint8_t semaphoreBinaryId, uint32_t ticsToWait);
void 						Os_SemaphoreBinaryGive 			(uint8_t semaphoreId);
OsState_t					Os_GetOsState					(void);
void						Os_SetOsState					(OsState_t osState);
Boolean_t					Os_IsHigherPriorityTaskReady	(void);

/*==================[end of file]============================================*/
#endif /* #ifndef _OS_H_ */
