// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef CPP_SIMPLE_QUEUE_H
#define CPP_SIMPLE_QUEUE_H

#include "queue.h"

template<typename T, size_t SIZE>
class CppSimpleQueue
{
public:
    CppSimpleQueue() {
        ResetQueue();
    }

    void ResetQueue()
    {
        this->elements_pointer = 0;
        QueueInit(this->GetQueue(), this->buffer, SIZE);
    }

    ErrorCode QPush(const T* element) { return Push(this->GetQueue(), element); }
    ErrorCode QPop(T** element) { return Pop(this->GetQueue(), (const void **)element); }
    bool IsEmpty(void) { return this->queue.elements_count == 0; }
    T* GetFreeElement(bool force = false) {
        T* result = NULL;
        T* trash = NULL;

        if (!IsFull(this->GetQueue())) {
            result = &this->elements[this->elements_pointer++];
            this->elements_pointer = this->elements_pointer % SIZE;
        }
        else if (force)
        {
            this->QPop(&trash);
            result = this->GetFreeElement(false);
        }

        return result;
    }
protected:
    SimpleQueue* GetQueue(void) { return &this->queue; }
private:
    SimpleQueue queue;
    const void* buffer[SIZE];
    T elements[SIZE];
    size_t elements_pointer;
};

#endif /* CPP_SIMPLE_QUEUE_H */
