/*==================[inclusions]=============================================*/

#include "board.h"
#include "os.h"

#include <string.h>

/*==================[macros and definitions]=================================*/

/** TASK COUNT number used by OS and task user count + 1*/
#define TASK_COUNT_OS       (TASK_COUNT+1)
/** return value of exception to load in LR */
#define EXC_RETURN          0xFFFFFFF9
/** id for idle task */
#define TASK_IDLE           (TASK_COUNT_OS - 1)
/** id for invalid task */
#define TASK_INVALID        (TASK_IDLE + 1)
/** Idle task stack size */
#define STACK_IDLE_SIZE     256

/** Possible events that can happen to tasks. */
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

/** possible states of a task */
typedef enum TaskState {
    TASK_STATE_INVALID,
    TASK_STATE_READY,
    TASK_STATE_RUNNING,
    TASK_STATE_WAITING,
    TASK_STATE_ZOMBIE,
    TASK_STATE_TERMINATED
} TaskState_t;

/** internal task control structure */
typedef struct TaskControlBlock {
    uint32_t                 stackPointer;
    const TaskDefinition_t * taskDefinition;
    TaskState_t              taskState;
    uint32_t                 waitingTime;
} TaskControlBlock_t;

/*==================[internal data declaration]==============================*/

/** List of tasks in ready state that will be looked at to select the next */
uint8_t ReadyTasksList [TASK_PRIORITY_COUNT][TASK_COUNT_OS];
/** internal task control structure */
static TaskControlBlock_t TaskControlList[TASK_COUNT_OS];
/** idle task context */
uint8_t StackFrameIdle[STACK_IDLE_SIZE];

/*==================[internal functions declaration]=========================*/

__attribute__ ((weak)) void * IdleHook(void * p);

static uint32_t AddTaskToReadyList      (uint32_t taskIndex, uint8_t priorityLevel);
static uint32_t RemoveTaskFromReadyList (uint32_t taskIndex, uint8_t priorityLevel);
static void     UpdateTaskDelay         (void);
static void     ReturnHook              (void * returnValue);
static void     ProcessTaskEvent        (uint32_t taskIndex, TaskEvent_t taskEvent);
static void     CreateTask              (uint8_t * stackFrame, uint32_t stackFrameSize,
                                         uint32_t * stackPointer, EntryPoint_t entryPoint,
                                         void * parameter, TaskState_t * taskState);

/*==================[internal data definition]===============================*/

/** index to current task */
static uint32_t CurrentTask = TASK_INVALID;
/** Definition structure of the idle task. */
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
 * When a task returns it must go to a place known as this Hook.
 */
static void ReturnHook(void * returnValue){
    while(1);
}

/**
* Create the initial context for each OS task.
* The value where the stack values of each task are stored is in the vector StackFrameX [STACK_SIZE]
* @param stackFrame Stack vector (frame).
* @param stackFrameSize Size expressed in bytes.
* @param stackPointer Where to save the stack pointer.
* @param entryPoint Task entry point (where the first instruction will start)
* @param parameter task parameter.
 */
static void CreateTask(uint8_t * stackFrame, uint32_t stackFrameSize,
                       uint32_t * stackPointer, EntryPoint_t entryPoint,
                       void * parameter, TaskState_t * state){

    uint32_t * auxStackFrame = (uint32_t *)stackFrame;

    /* inicializo el frame en cero */
    bzero(auxStackFrame, stackFrameSize);
    /* último elemento del contexto inicial: xPSR
     * necesita el bit 24 (T, modo Thumb) en 1 */
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

/** Update the delay counter for the current task. */
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
* Add a task to the list of ready tasks (that can be executed).
* @param taskIndex index of the task
* @param priorityLevel task priority
* @return operation status
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
* Remove a task from the ready task list (it can no longer be executed).
* @param taskIndex index of the task
* @param priorityLevel task priority
* @return operation status
 */
static uint32_t RemoveTaskFromReadyList(uint32_t taskIndex, uint8_t priorityLevel){
    uint32_t taskIndexVolatile;

    if (priorityLevel >= 0 && priorityLevel < TASK_PRIORITY_COUNT){
        for (taskIndexVolatile = 0; taskIndexVolatile < TASK_COUNT_OS; taskIndexVolatile++){
            if (ReadyTasksList[priorityLevel][taskIndexVolatile] == taskIndex){
                // TODO: make an API that has a shift array function to put the first element of the array in a valid index
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
* Hook where the processor goes if there are no tasks left to execute.
* @param p generic parameter (not used).
 */
void * IdleHook(void * p){
    while (1) {
        __WFI();
    }
}

/**
* System systic interrupt routine.
* The name of the interrupts is in cr_startup_xx.c of the system uC.
 */
void SysTick_Handler(void){
    UpdateTaskDelay();
    Os_Schedule();
}

/*==================[external functions definition]==========================*/

/**
* It generates a delay by software that the operating system is in charge of handling.
* When the time set by the user passes, it returns control to the task that called it.
* @param milliseconds time in milliseconds to wait.
 */
void Os_Delay(uint32_t milliseconds){
    if (CurrentTask != TASK_INVALID) {
        TaskControlList[CurrentTask].waitingTime = milliseconds;
        ProcessTaskEvent(CurrentTask, TASK_EVENT_BLOCK);
        Os_Schedule();
    }
}

/**
* Change the main stack pointer of the system depending on the current stack pointer value (does the context change).
* This function defines the scheduling policy of the operating system.
* @param actualContext value of the current msp (main stack pointer). Loaded in r0 from ASM.
* @return value of the stack pointer to be executed (the ASM generated by GCC loads the return value in r0)
 */
int32_t Os_GetNextContext(int32_t currentStackPointer){

    uint32_t stackPointerToReturn;
    uint8_t taskPriorityLevel;

    /** 
     * If it is a valid task ...
     * Save the stack pointer of the current task.
     * If the current task is running (In running state) ...
     * Put the task in ready state
     * Add the task to the list of ready-to-run tasks
     * (Adding to the ready task is done to simplify the task selection logic.
     * */
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
* Activate the software interruption of the PendSV, in this way we will proceed to
* make the context switch.
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
* Initialize the operating system. In this function, the
* core of the microcontroller, the idle task starts with its stack and context and
* start user-defined tasks.
* @return must not return
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

// TODO: the task add task puts the task in ready state
// TODO: put an internal API to the OS that is called changeTaskState and that passes the enum of states and changes them there
// TODO: circular buffer to add / remove tasks
// TODO: rewind the tape when it should be put to the current task index first
// TODO: make a task state machine in which they receive the index of the task that calls it,
// the state of the task and the event that happened, and based on that it returns the new
// status of the task in question
