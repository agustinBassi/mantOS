#ifndef _OS_H_
#define _OS_H_

/*==================[inclusions]=============================================*/

#include "board.h"
#include "os_config.h"

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

// Prototype of tasks or threads
typedef void* (*EntryPoint_t)(void *);

// Possible priority task levels
typedef enum TaskLevelPriority {
	TASK_PRIORITY_IDLE,
	TASK_PRIORITY_LOW,
	TASK_PRIORITY_MEDIUM,
	TASK_PRIORITY_HIGH,
	TASK_PRIORITY_COUNT
} TaskLevelPriority_t;

// Struct used to define a task
typedef struct TaskDefinition {
	uint8_t *           stackFrame;
	uint32_t            stackSize;
	EntryPoint_t        entryPoint;
	void  *             parameter;
	TaskLevelPriority_t priorityLevel;
} TaskDefinition_t;

/*==================[external data declaration]==============================*/

extern const TaskDefinition_t Os_TaskList[TASK_COUNT];

/*==================[external functions declaration]=========================*/

// Starting point of the kernel
int  Os_Start    (void);
// Schedule a task (send it to dispatcher)
void Os_Schedule (void);
// The delay must be controlled by the kernel
void Os_Delay    (uint32_t milliseconds);

/*==================[end of file]============================================*/

#endif /* #ifndef _OS_H_ */
