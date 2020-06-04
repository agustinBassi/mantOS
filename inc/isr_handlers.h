/*
 * isr_handlers.h
 *
 *  Created on: 30/9/2016
 *      Author: mint
 */

#ifndef PROJECTS_OS8_INC_ISR_HANDLERS_H_
#define PROJECTS_OS8_INC_ISR_HANDLERS_H_


/*==================[inclusions]=============================================*/

#include "cmsis.h"

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/** Tipo definido para una funcion de usuario. Es un puntero a funcion. */
typedef void (* UserFunctionPointer_t)(void);

/** Estado que puede devolver una funcion asociada a las interrupciones.
 * Generalmente es utilizado para saber si una funcion de usuario pudo
 * vincularse o desvincularse de una interrupcion. */
typedef enum InterruptOperationState {
	INTERRUPT_STATE_OK,
	INTERRUPT_STATE_ERROR
} InterruptOperationState_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

InterruptOperationState_t 		IsrHandlers_AttachIrq 		(IRQn_Type irqName, UserFunctionPointer_t userFunctionPointer);

InterruptOperationState_t 		IsrHandlers_DeattachIrq 	(IRQn_Type irqName);

/*==================[end of file]============================================*/

#endif /* PROJECTS_OS8_INC_ISR_HANDLERS_H_ */
