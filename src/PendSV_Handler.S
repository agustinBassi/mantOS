	.syntax unified
	/* Make the PendSV_Hanlder globally available */
	.global PendSV_Handler
	/* declare access to routine for selecting the next context */
	.extern Os_GetNextContext
	.text
	.thumb_func
PendSV_Handler:
	/* save the rest of the context in this registers*/
	push {r4-r11,lr}
	/* call OS_GetNextContext passing as a parameter to it
	the current context. It is equivalent to do:

	nextContext = getNextContext(actualContext);

	By convention (AAPCS) the parameter must be passed in R0
	and returned must be returned in R0
	*/
	/* R0 = msp */
	mrs r0, msp 
	/* R0 = next pointer in the stack.
	Bbranch with link (bl) instruction is used */
	bl Os_GetNextContext 
	/* msp = r0 */
	msr msp, r0 
	/* get new context*/
	pop {r4-r11,lr}
	/* Return from the interruption.
	It must recovery the context builded in Task_Create function. */
	bx lr 
