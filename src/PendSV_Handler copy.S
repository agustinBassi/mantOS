#define USING_FPU 0xFFFFFFE9

	.syntax unified
	.global PendSV_Handler

	/* declaro acceso a la rutina de seleccion de proximo contexto */
	.extern Os_GetNextContext

	.text
	.thumb_func
PendSV_Handler:

	tst lr, 0x10
	it eq
		vpusheq {s16-s31}



	/* guardamos el resto del contexto */
	push {r4-r11,lr}

	/* llamo a getNextContext pasándole como parámetro al
	contexto actual, esto equivale a hacer:
	nextContext = getNextContext(actualContext);

	Por convención (AAPCS), paso el parámetro en R0 y devuelvo
	el valor en R0.
	*/
	mrs r0, msp /* r0 = msp */
	bl Os_GetNextContext /* r0 = siguiente puntero de pila
	uso la instrucción bl (branch with link) */

	msr msp, r0 /* msp = r0 */

	pop {r4-r11,lr}


	tst lr, 0x10
	it eq
		vpopeq {s16-s31}


	bx lr /* retorno de interrupción,
debería recuperar el contexto armado en task_create */
