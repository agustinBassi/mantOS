
/*
 *
 */

/*
 * Definicion del tipo de datos como puntero a funcion
 * typedef void (* IsrFunctionPointer_t)(void);
 */

/*
 * Ejecutar la funcion
 * 	(* voidFunctionPointer(0})();
 * */

/*
 * asignar una funcion en el vector
 * 	voidFunctionPointer(0} = funcion del usuario
 */



/*==================[inclusions]=============================================*/

#include "os.h"
#include "isr_handlers.h"

/*==================[macros and definitions]=================================*/

/** @brief: Cantidad de interrupciones (en el NVIC que esta en cmsis_xxxx.h) que tiene el dispositivo. */
#if defined(edu_ciaa_nxp)
	#define DEVICE_INTERRUPTS_COUNT 52
#else
	#define DEVICE_INTERRUPTS_COUNT 52
#endif

/*==================[internal data declaration]==============================*/

/** @brief: Vector con los posibles punteros a funcion de usuario para cada interrupcion del dispositivo. */
static UserFunctionPointer_t UserFunctionPointersVector [DEVICE_INTERRUPTS_COUNT];

/*==================[internal functions declaration]=========================*/

static void OsDefaultIrqHandler (IRQn_Type irqName);

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/**
 * Handler por defecto del sistema operativo.
 * Todos los handlers de interrupcion del hardware donde corra el OS
 * llaman a esta funcion pasandole como parametro el nombre de la interrupcion.
 * @param irqName Nombre de la interrupcion del nvic de cmsis (en cmsis_43xx.h).
 */
static void 		OsDefaultIrqHandler 				(IRQn_Type irqName){
OsState_t osState = OS_STATE_INVALID;

	/** Obtiene el estado del sistema operativo. */
	osState = Os_GetOsState();
	/** Pone al OS en estado ISR. */
	Os_SetOsState(OS_STATE_ISR);
	/** Chequea que la interrupcion este attachada a una funcion de usuario valida. */
	if (UserFunctionPointersVector[irqName] != NULL){
		UserFunctionPointersVector[irqName]();
	}
	/** Pone al OS en estado TASK. */
	Os_SetOsState(OS_STATE_TASK);
	/** Si en el manejo de IRQ de usuario se habilito una tarea de mayor prioridad, llama al scheduler. */
	if (Os_IsHigherPriorityTaskReady() == TRUE){
		Os_Schedule();
	}
}

/*==================[external functions definition]==========================*/

/**
 * Asocia una funcion de usuario al handler de una interrupcion.
 * La configuracion de dicha interrupcion la debe realizar el usuario en su aplicacion.
 * @param irqName Nombre de la interrupcion de cmsis (en cmsis_43xx.h).
 * @param userFunctionPointer puntero a la funcion de usuario que se llamara cuando ocurra la interrupcion.
 * @return devuelve el estado de la operacion.
 * 			0 (OK) si pudo agregar correctamente el handler.
 * 			1 (ERROR) si la interrupcion ya tenia asociado una funcion de usuario.
 */
InterruptOperationState_t 	IsrHandlers_AttachIrq 		(IRQn_Type irqName, UserFunctionPointer_t userFunctionPointer){

	InterruptOperationState_t interruptOperationState = INTERRUPT_STATE_ERROR;

	/** Chequea si el vector de interrupciones no tiene ya una funcion de usuario asociada,
	 *  y tambien si la funcion de usuario no es nula. */
	if (UserFunctionPointersVector[irqName] == NULL && userFunctionPointer != NULL){
		UserFunctionPointersVector[irqName] = userFunctionPointer;
		interruptOperationState = INTERRUPT_STATE_OK;
	}

	return interruptOperationState;
}

/**
 * Desasocia una funcion de usuario con una interrupcion.
 * @param irqName nombre de la interrupcion de cmsis (en cmsis_43xx.h).
 * @return devuelve el estado de la operacion.
 * 			0 (OK) si pudo desasociar correctamente la funcion de usuario con la interrupcion.
 * 			1 (ERROR) si el handler de interrupcion no estaba asociado a ninguna funcion de usuario.
 */
InterruptOperationState_t 	IsrHandlers_DeattachIrq 	(IRQn_Type irqName){
	InterruptOperationState_t interruptOperationState = INTERRUPT_STATE_ERROR;

	/** Chequea si la interrupcion tenia una funcion de usuario asociada. */
	if (UserFunctionPointersVector[irqName] != NULL){
		UserFunctionPointersVector[irqName] = NULL;
		interruptOperationState = INTERRUPT_STATE_OK;
	}

	return interruptOperationState;
}


/** Todas los handlers de interrupcion llaman al handler de IRQ del sistema operativo.
 * Cada funcion que llama al OS_IrqHandler lo hace pasandole como parametro el nombre
 * de la interrupcion del nvic que tiene asociada. Estos nombres (con sufijo IRQn) estas en cmsis_43xx.h */

void DAC_IRQHandler			(void){OsDefaultIrqHandler(DAC_IRQn);}
void M0APP_IRQHandler		(void){OsDefaultIrqHandler(M0APP_IRQn);}
void DMA_IRQHandler			(void){OsDefaultIrqHandler(DMA_IRQn);}
void FLASH_EEPROM_IRQHandler(void){OsDefaultIrqHandler(RESERVED1_IRQn);}
void ETH_IRQHandler			(void){OsDefaultIrqHandler(ETHERNET_IRQn);}
void SDIO_IRQHandler		(void){OsDefaultIrqHandler(SDIO_IRQn);}
void LCD_IRQHandler			(void){OsDefaultIrqHandler(LCD_IRQn);}
void USB0_IRQHandler		(void){OsDefaultIrqHandler(USB0_IRQn);}
void USB1_IRQHandler		(void){OsDefaultIrqHandler(USB1_IRQn);}
void SCT_IRQHandler			(void){OsDefaultIrqHandler(SCT_IRQn);}
void RIT_IRQHandler			(void){OsDefaultIrqHandler(RITIMER_IRQn);}
void TIMER0_IRQHandler		(void){OsDefaultIrqHandler(TIMER0_IRQn);}
void TIMER1_IRQHandler		(void){OsDefaultIrqHandler(TIMER1_IRQn);}
void TIMER2_IRQHandler		(void){OsDefaultIrqHandler(TIMER2_IRQn);}
void TIMER3_IRQHandler		(void){OsDefaultIrqHandler(TIMER3_IRQn);}
void MCPWM_IRQHandler		(void){OsDefaultIrqHandler(MCPWM_IRQn);}
void ADC0_IRQHandler		(void){OsDefaultIrqHandler(ADC0_IRQn);}
void I2C0_IRQHandler		(void){OsDefaultIrqHandler(I2C0_IRQn);}
void SPI_IRQHandler			(void){OsDefaultIrqHandler(SPI_INT_IRQn);}
void I2C1_IRQHandler		(void){OsDefaultIrqHandler(I2C1_IRQn);}
void ADC1_IRQHandler		(void){OsDefaultIrqHandler(ADC1_IRQn);}
void SSP0_IRQHandler		(void){OsDefaultIrqHandler(SSP0_IRQn);}
void SSP1_IRQHandler		(void){OsDefaultIrqHandler(SSP1_IRQn);}
void UART0_IRQHandler		(void){OsDefaultIrqHandler(USART0_IRQn);}
void UART1_IRQHandler		(void){OsDefaultIrqHandler(UART1_IRQn);}
void UART2_IRQHandler		(void){OsDefaultIrqHandler(USART2_IRQn);}
void UART3_IRQHandler		(void){OsDefaultIrqHandler(USART3_IRQn);}
void I2S0_IRQHandler		(void){OsDefaultIrqHandler(I2S0_IRQn);}
void I2S1_IRQHandler		(void){OsDefaultIrqHandler(I2S1_IRQn);}
void SPIFI_IRQHandler		(void){OsDefaultIrqHandler(SPI_INT_IRQn);}
void SGPIO_IRQHandler		(void){OsDefaultIrqHandler(SGPIO_INT_IRQn);}
void GPIO0_IRQHandler		(void){OsDefaultIrqHandler(PIN_INT0_IRQn);}
void GPIO1_IRQHandler		(void){OsDefaultIrqHandler(PIN_INT1_IRQn);}
void GPIO2_IRQHandler		(void){OsDefaultIrqHandler(PIN_INT2_IRQn);}
void GPIO3_IRQHandler		(void){OsDefaultIrqHandler(PIN_INT3_IRQn);}
void GPIO4_IRQHandler		(void){OsDefaultIrqHandler(PIN_INT4_IRQn);}
void GPIO5_IRQHandler		(void){OsDefaultIrqHandler(PIN_INT5_IRQn);}
void GPIO6_IRQHandler		(void){OsDefaultIrqHandler(PIN_INT6_IRQn);}
void GPIO7_IRQHandler		(void){OsDefaultIrqHandler(PIN_INT7_IRQn);}
void GINT0_IRQHandler		(void){OsDefaultIrqHandler(GINT0_IRQn);}
void GINT1_IRQHandler		(void){OsDefaultIrqHandler(GINT1_IRQn);}
void EVRT_IRQHandler		(void){OsDefaultIrqHandler(EVENTROUTER_IRQn);}
void CAN1_IRQHandler		(void){OsDefaultIrqHandler(C_CAN1_IRQn);}
//void VADC_IRQHandler		(void){OsDefaultIrqHandler(RESERVED6_IRQn);}
void ATIMER_IRQHandler		(void){OsDefaultIrqHandler(ATIMER_IRQn);}
void RTC_IRQHandler			(void){OsDefaultIrqHandler(RTC_IRQn);}
void WDT_IRQHandler			(void){OsDefaultIrqHandler(WWDT_IRQn);}
void M0SUB_IRQHandler		(void){OsDefaultIrqHandler(M0SUB_IRQn);}
void CAN0_IRQHandler		(void){OsDefaultIrqHandler(C_CAN0_IRQn);}
void QEI_IRQHandler			(void){OsDefaultIrqHandler(QEI_IRQn);}

/*==================[end of file]============================================*/






///** Inicializar las interrupciones del micro donde corra el sistema. */
//void IsrHandlers_InitInterrupt (IRQn_Type irqName){
//
//	switch (irqName){
//	case PIN_INT0_IRQn:
//		    Chip_SCU_PinMux(2,0,MD_PUP|MD_EZI,FUNC4);  /* GPIO5[0], LED0R */
//			Chip_SCU_PinMux(2,1,MD_PUP|MD_EZI,FUNC4);  /* GPIO5[1], LED0G */
//			Chip_SCU_PinMux(2,2,MD_PUP|MD_EZI,FUNC4);  /* GPIO5[2], LED0B */
//			Chip_SCU_PinMux(2,10,MD_PUP|MD_EZI,FUNC0); /* GPIO0[14], LED1 */
//			Chip_SCU_PinMux(2,11,MD_PUP|MD_EZI,FUNC0); /* GPIO1[11], LED2 */
//			Chip_SCU_PinMux(2,12,MD_PUP|MD_EZI,FUNC0); /* GPIO1[12], LED3 */
//
//			Chip_GPIO_SetDir(LPC_GPIO_PORT, 5,(1<<0)|(1<<1)|(1<<2),1);
//			Chip_GPIO_SetDir(LPC_GPIO_PORT, 0,(1<<14),1);
//			Chip_GPIO_SetDir(LPC_GPIO_PORT, 1,(1<<11)|(1<<12),1);
//
//			Chip_GPIO_ClearValue(LPC_GPIO_PORT, 5,(1<<0)|(1<<1)|(1<<2));
//			Chip_GPIO_ClearValue(LPC_GPIO_PORT, 0,(1<<14));
//			Chip_GPIO_ClearValue(LPC_GPIO_PORT, 1,(1<<11)|(1<<12));
//
//			/* Switches */
//			Chip_SCU_PinMux(1,0,MD_PUP|MD_EZI|MD_ZI,FUNC0); /* GPIO0[4], SW1 */
//			Chip_SCU_PinMux(1,1,MD_PUP|MD_EZI|MD_ZI,FUNC0); /* GPIO0[8], SW2 */
//			Chip_SCU_PinMux(1,2,MD_PUP|MD_EZI|MD_ZI,FUNC0); /* GPIO0[9], SW3 */
//			Chip_SCU_PinMux(1,6,MD_PUP|MD_EZI|MD_ZI,FUNC0); /* GPIO1[9], SW4 */
//
//			Chip_GPIO_SetDir(LPC_GPIO_PORT, 0,(1<<4)|(1<<8)|(1<<9),0);
//			Chip_GPIO_SetDir(LPC_GPIO_PORT, 1,(1<<9),0);
//			Chip_PININT_Init(LPC_GPIO_PIN_INT);
//			Chip_SCU_GPIOIntPinSel(0,0,4);
//			Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH0);
//			Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT,PININTCH0);
//			Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT,PININTCH0);
//			NVIC_EnableIRQ(PIN_INT0_IRQn);
//		break;
//	default:
//
//		break;
//
//	}
//
//}







//
//
//
//__attribute__ ((section(".after_vectors")))
//void NMI_Handler(void) {
//    while (1) {
//    }
//}
//__attribute__ ((section(".after_vectors")))
//void HardFault_Handler(void) {
//    while (1) {
//    }
//}
//__attribute__ ((section(".after_vectors")))
//void MemManage_Handler(void) {
//    while (1) {
//    }
//}
//__attribute__ ((section(".after_vectors")))
//void BusFault_Handler(void) {
//    while (1) {
//    }
//}
//__attribute__ ((section(".after_vectors")))
//void UsageFault_Handler(void) {
//    while (1) {
//    }
//}
//__attribute__ ((section(".after_vectors")))
//void SVC_Handler(void) {
//    while (1) {
//    }
//}
//__attribute__ ((section(".after_vectors")))
//void DebugMon_Handler(void) {
//    while (1) {
//    }
//}
//__attribute__ ((section(".after_vectors")))
//void PendSV_Handler(void) {
//    while (1) {
//    }
//}
//__attribute__ ((section(".after_vectors")))
//void SysTick_Handler(void) {
//    while (1) {
//    }
//}
//
//__attribute__ ((section(".after_vectors")))
//void IntDefaultHandler(void) {
//    while (1) {
//    }
//}





//
//void (* const userIrqHandler(})(void) = {
//    // Chip Level - LPC43 (M4)
//    DAC_IRQHandler,           // 16
//    M0CORE_IRQHandler,        // 17
//    DMA_IRQHandler,           // 18
//    0,           				// 19
//    FLASH_EEPROM_IRQHandler,   // 20 ORed flash Bank A, flash Bank B, EEPROM interrupts
//    ETH_IRQHandler,           // 21
//    SDIO_IRQHandler,          // 22
//    LCD_IRQHandler,           // 23
//    USB0_IRQHandler,          // 24
//    USB1_IRQHandler,          // 25
//    SCT_IRQHandler,           // 26
//    RIT_IRQHandler,           // 27
//    TIMER0_IRQHandler,        // 28
//    TIMER1_IRQHandler,        // 29
//    TIMER2_IRQHandler,        // 30
//    TIMER3_IRQHandler,        // 31
//    MCPWM_IRQHandler,         // 32
//    ADC0_IRQHandler,          // 33
//    I2C0_IRQHandler,          // 34
//    I2C1_IRQHandler,          // 35
//    SPI_IRQHandler,           // 36
//    ADC1_IRQHandler,          // 37
//    SSP0_IRQHandler,          // 38
//    SSP1_IRQHandler,          // 39
//    UART0_IRQHandler,         // 40
//    UART1_IRQHandler,         // 41
//    UART2_IRQHandler,         // 42
//    UART3_IRQHandler,         // 43
//    I2S0_IRQHandler,          // 44
//    I2S1_IRQHandler,          // 45
//    SPIFI_IRQHandler,         // 46
//    SGPIO_IRQHandler,         // 47
//    GPIO0_IRQHandler,         // 48
//    GPIO1_IRQHandler,         // 49
//    GPIO2_IRQHandler,         // 50
//    GPIO3_IRQHandler,         // 51
//    GPIO4_IRQHandler,         // 52
//    GPIO5_IRQHandler,         // 53
//    GPIO6_IRQHandler,         // 54
//    GPIO7_IRQHandler,         // 55
//    GINT0_IRQHandler,         // 56
//    GINT1_IRQHandler,         // 57
//    EVRT_IRQHandler,          // 58
//    CAN1_IRQHandler,          // 59
//    0,                        // 60
//    VADC_IRQHandler,          // 61
//    ATIMER_IRQHandler,        // 62
//    RTC_IRQHandler,           // 63
//    0,                        // 64
//    WDT_IRQHandler,           // 65
//    M0SUB_IRQHandler,         // 66
//    CAN0_IRQHandler,          // 67
//    QEI_IRQHandler,           // 68
//};
//
//

//void ResetISR(void);
//WEAK void NMI_Handler(void);
//WEAK void HardFault_Handler(void);
//WEAK void MemManage_Handler(void);
//WEAK void BusFault_Handler(void);
//WEAK void UsageFault_Handler(void);
//WEAK void SVC_Handler(void);
//WEAK void DebugMon_Handler(void);
//WEAK void PendSV_Handler(void);
//WEAK void SysTick_Handler(void);
//WEAK void IntDefaultHandler(void);

//#include "cmsis.h"









