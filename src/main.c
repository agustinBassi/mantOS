/*==================[inclusions]=============================================*/

#include "os.h"
#include "board.h"

/*==================[macros and definitions]=================================*/

// thread stack size
#define STACK_SIZE              512
#define DEFAULT_TASK_PARAMS     0xAABBCCDD
#define DEFAULT_RETURN_VALUE    0
#define TIMES_TO_LOOP           4

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

static void * Task1(void * param);
static void * Task2(void * param);
static void * Task3(void * param);
static void * Task4(void * param);
static void * Task5(void * param);

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/* Stack for each task */
uint8_t StackFrame1[STACK_SIZE];
uint8_t StackFrame2[STACK_SIZE];
uint8_t StackFrame3[STACK_SIZE];
uint8_t StackFrame4[STACK_SIZE];
uint8_t StackFrame5[STACK_SIZE];

// Define each OS Task associaten them to a function, stack and priority
const TaskDefinition_t Os_TaskList[TASK_COUNT] = {
    {StackFrame1, STACK_SIZE, Task1, (void *)DEFAULT_TASK_PARAMS, TASK_PRIORITY_LOW},
    {StackFrame2, STACK_SIZE, Task2, (void *)DEFAULT_TASK_PARAMS, TASK_PRIORITY_MEDIUM},
    {StackFrame3, STACK_SIZE, Task3, (void *)DEFAULT_TASK_PARAMS, TASK_PRIORITY_HIGH},
    {StackFrame4, STACK_SIZE, Task4, (void *)DEFAULT_TASK_PARAMS, TASK_PRIORITY_HIGH},
    {StackFrame5, STACK_SIZE, Task5, (void *)DEFAULT_TASK_PARAMS, TASK_PRIORITY_HIGH}
};

/*==================[internal functions definition]==========================*/

static void * Task1(void * param){
    // loops forever
    while (1) {
        Board_LED_Toggle(1);
        Os_Delay(140);
    }
    return (void *)DEFAULT_RETURN_VALUE; 
}

static void * Task2(void * param){
    // when this task finished to loop they won't be executed again
    int timesToLoop = TIMES_TO_LOOP;
    while (++timesToLoop) {
        Board_LED_Toggle(2);
        Os_Delay(650);
    }
    return (void *)DEFAULT_RETURN_VALUE; 
}

static void * Task3(void * param){
    // when this task finished to loop they won't be executed again
    int timesToLoop = TIMES_TO_LOOP;
    while (++timesToLoop) {
        Board_LED_Toggle(3);
        Os_Delay(800);
    }
    return (void *)DEFAULT_RETURN_VALUE; 
}

static void * Task4(void * param){
    // when this task finished to loop they won't be executed again
    int timesToLoop = TIMES_TO_LOOP;
    while (++timesToLoop) {
        Board_LED_Toggle(4);
        Os_Delay(900);
    }
    return (void *)DEFAULT_RETURN_VALUE; 

static void * Task5(void * param){
    // when this task finished to loop they won't be executed again
    int timesToLoop = TIMES_TO_LOOP;
    while (++timesToLoop) {
        Board_LED_Toggle(5);
        Os_Delay(250);
    }
    return (void *)DEFAULT_RETURN_VALUE;
}

/*==================[external functions definition]==========================*/

int main(void){
    // Initialize the board HW
    Board_Init();
    // Start OS
    Os_Start();
    // The previous call must never comeback here
    while(1);
}

/*==================[end of file]============================================*/
