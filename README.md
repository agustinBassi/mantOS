# mantOS

Author: Agustin Bassi - 2018

# Motivation

This project starts as academic project. Once the project was deliver, the RTOS worked really well, so it was decided to improve and make it publicly available.

Besides, it was intended to be implemented in a simple way importing four files to a project and compile it with the rest of application code.

Other and more complex RTOS like FreeRTOS or OSEK must be compiled with complex makefiles that make them difficult to use in beginners.

# Description

The project consists in a ultra-light real time operating system for ARM Cortex-M processors.

The RTOS consists in three parts:

1. An Assembler file called "PendSVHanler" used for switch context between tasks maintaining their states.
2. The RTOS files which are os_config.h, os.h and os.c.
3. The user application program which uses the RTOS in a really simple manner.

> **_NOTE:_**  This project has been builded within the [CIAA Project](http://www.proyecto-ciaa.com.ar/index_en.html) context. CIAA Project is an open source embedded platform oriented for industrial developments. Although of that, this RTOS can runs over any ARM Cortex-M microprocessor.

# Operating System description

This RTOS works in a preemtive way, this means that a task can be interrupted if there is a task with major priority ready to run (managed by the kernel).

To properly interrupt a task the kernel implements a mechanism to save each task context (stack pointer, variables, states and others) in order to resume the tasks later.

A task can be interrupted for two reasons: 
* A major priority task is ready to be executed.
* By timeout if there are other tasks with the same priority ready to execute, because the same priority tasks are executed in Round Robin way.

### IPC mechanism

To communicate with kernel system and to syncronize events and data between task, some common IPC (interprocess Communication Protocol) are implemented:

* Semaphore: This mechanisms serves to ...
* Mutex: This mechanisms serves to ...
* Queue: This mechanisms serves to ...

### Tasks states

The tasks states can be ...

To explain better the possible tasks states, in the figure below are shown the tasks flow.

# Using RTOS

The RTOS usage is shown in the main.c file. Starting by importing the os.h and os_config.h headers with the lines below.

Declare application tasks that must have the interfaces as follows.


Define application tasks putting relating code in each one.

Call the system initialization in the setup process. Note that there are no code in the main loop. This is because the kernel is in charge to manage tasks and execute them in the desired way.


# Contributing

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update tests as appropriate.

# License

[GPL]


