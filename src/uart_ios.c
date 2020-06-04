/* Copyright 2016, Pablo Ridolfi
 * All rights reserved.
 *
 * This file is part of Workspace.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "uart_ios.h"
#include "os.h"

uint8_t rxbuf[UART_BUF_SIZE];
uint8_t txbuf[UART_BUF_SIZE];

RINGBUFF_T rrb;
RINGBUFF_T trb;

//void uart_irq(void)
void uartInterrupt(void)
{
	Chip_UART_IRQRBHandler(LPC_USART2, &rrb, &trb);
}

void uartIosInit(void)
{
	/* UART2 (USB-UART) */
	Chip_UART_Init(LPC_USART2);
	Chip_UART_SetBaud(LPC_USART2, 115200);

	Chip_UART_TXEnable(LPC_USART2);

	Chip_SCU_PinMux(7, 1, MD_PDN, FUNC6);              /* P7_1: UART2_TXD */
	Chip_SCU_PinMux(7, 2, MD_PLN|MD_EZI|MD_ZI, FUNC6); /* P7_2: UART2_RXD */

	Chip_UART_IntEnable(LPC_USART2, UART_IER_RBRINT);
//	os_attach_irq(USART2_IRQn, uart_irq);

	RingBuffer_Init(&rrb, rxbuf, 1, UART_BUF_SIZE);
	RingBuffer_Init(&trb, txbuf, 1, UART_BUF_SIZE);
}

uint32_t uartIosSend(void * data, int datalen)
{
	return Chip_UART_SendRB(LPC_USART2, &trb, data, datalen);
}

uint32_t uartIosRecv(void * data, int datalen)
{
	return Chip_UART_ReadRB(LPC_USART2, &rrb, data, datalen);
}
