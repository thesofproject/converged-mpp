// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#include <adsp_s_memory.h>
#include "adsp_std_defs.h"
#include "queue.h"

void QueueInit(SimpleQueue* queue, const void** buffer, size_t size)
{
    queue->rear = queue->front = queue->elements_count = 0;
    queue->elements_array = buffer;
    queue->size = size;
    memset(buffer, NULL, size * sizeof(void*));
}

ErrorCode Push(SimpleQueue* queue, const void* element)
{
    RETURN_EC_ON_FAIL(!IsFull(queue), ADSP_SIMPLE_QUEUE_FULL);
    queue->elements_array[queue->rear] = element;
    queue->rear = (queue->rear + 1) % queue->size;
    queue->elements_count += 1;
    FLOG(L_INFO, "Queue::Push count: %d rear: %d front: %d", queue->elements_count, queue->rear, queue->front);
    return ADSP_SUCCESS;
}

ErrorCode Pop(SimpleQueue* queue, const void** element)
{
    RETURN_EC_ON_FAIL(!IsFree(queue), ADSP_SIMPLE_QUEUE_EMPTY);
    if (element != NULL)
        *element = queue->elements_array[queue->front];
    queue->elements_array[queue->front] = NULL;
    queue->front = (queue->front + 1) % queue->size;
    queue->elements_count -= 1;
    FLOG(L_INFO, "Queue::Pop count: %d rear: %d front: %d", queue->elements_count, queue->rear, queue->front);
    return ADSP_SUCCESS;
}

ErrorCode Peek(const SimpleQueue* queue, const void** element)
{
    RETURN_EC_ON_FAIL(element != NULL, ADSP_ERROR_NULL_POINTER_AS_PARAM);
    RETURN_EC_ON_FAIL(!IsFree(queue), ADSP_SIMPLE_QUEUE_EMPTY);
    *element = queue->elements_array[queue->front];
    return ADSP_SUCCESS;
}

ErrorCode Remove(SimpleQueue* queue, const void* element)
{
    RETURN_EC_ON_FAIL(element != NULL, ADSP_ERROR_NULL_POINTER_AS_PARAM);
    RETURN_EC_ON_FAIL(!IsFree(queue), ADSP_SIMPLE_QUEUE_EMPTY);
    bool found_element_to_remove = false;
    for(size_t index = queue->front; index < queue->front + queue->elements_count; ++index)
    {
        const size_t this_element_index = index % queue->size;
        const size_t next_element_index = (index + 1) % queue->size;
        if (!found_element_to_remove)
        {
            found_element_to_remove = queue->elements_array[this_element_index] == element;
        }
        if (!found_element_to_remove) continue;
        // Reorder queue.
        queue->elements_array[this_element_index] = queue->elements_array[next_element_index];
    }
    // Remove last element and move rear pointer.
    queue->elements_array[queue->rear] = NULL;
    if (queue->rear == 0)
    {
#pragma frequency_hint NEVER
        queue->rear = queue->size - 1;
    }
    else
    {
        --queue->rear;
    }
    --queue->elements_count;
    return ADSP_SUCCESS;
}
