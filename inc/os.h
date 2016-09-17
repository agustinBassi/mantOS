#ifndef _OS_H_
#define _OS_H_

/*==================[inclusions]=============================================*/

#include "board.h"
#include "os_config.h"

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/** prototipo de las tareas o hilos de ejecucion o threads */
typedef void* (*EntryPoint_t)(void *);

typedef enum TaskLevelPriority {
	TASK_PRIORITY_IDLE,
	TASK_PRIORITY_LOW,
	TASK_PRIORITY_MEDIUM,
	TASK_PRIORITY_HIGH,
	TASK_PRIORITY_COUNT
} TaskLevelPriority_t;

/** estructura de definicón de tareas */
typedef struct TaskDefinition {
	uint8_t * 			stackFrame;
	uint32_t 			stackSize;
	EntryPoint_t 		entryPoint;
	void  * 			parameter;
	TaskLevelPriority_t priorityLevel;
} TaskDefinition_t;



/*==================[external data declaration]==============================*/

extern const TaskDefinition_t Os_TaskList[TASK_COUNT];

/*==================[external functions declaration]=========================*/

int Os_Start(void);
void Os_Schedule(void);
void Os_Delay(uint32_t milliseconds);

/*==================[end of file]============================================*/
#endif /* #ifndef _OS_H_ */
