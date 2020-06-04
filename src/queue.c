/* Copyright 2015, Nicolás Dassieu Blanchet.
 * Copyright 2016, Pablo Ridolfi.
 * All rights reserved.
 *
 * This file is part of a project.
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

/** @brief OSEK-OS simple queue implementation.
 **
 **/

/*==================[inclusions]=============================================*/

#include "queue.h"

/*==================[macros and definitions]=================================*/



/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

int32_t Queue_Init(Queue_t * queue, uint32_t semaphoreQueueId)
{
	int32_t rv = -1;
	queue->tail = 0;
	queue->head = 0;
	queue->semaphoreBinaryId = semaphoreQueueId;

	return rv;
}

void Queue_PutItem(Queue_t * queue, QueueItem_t item){

	while(((queue->head+1)%QUEUE_LEN) == queue->tail){

	}

	queue->data[queue->head] = item;

	queue->head = (queue->head+1)%QUEUE_LEN;

	Os_SemaphoreBinaryGive(queue->semaphoreBinaryId);

}

QueueItem_t Queue_GetItem(Queue_t * queue){

	Os_SemaphoreBinaryTake(queue->semaphoreBinaryId, OS_MAX_DELAY);

	QueueItem_t item = queue->data[queue->tail];
	queue->tail = (queue->tail+1)%QUEUE_LEN;



	return item;
}

/*==================[end of file]============================================*/
