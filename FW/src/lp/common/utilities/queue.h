// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef FW_SRC_BASE_FW_UTILITIES_QUEUE_H_
#define FW_SRC_BASE_FW_UTILITIES_QUEUE_H_

#include <misc/utils.h>
#include <stdint.h>
#include <stddef.h>
#include "syntax_sugar.h"
#include "adsp_assert.h"
#include "adsp_error.h"

/*!
  \brief SimpleQueue is a FIFO container.
  It encapsulates all logic required to push, peek and pop elements from fifo.
  Memory for elements is maintained out of the queue.
 */
typedef struct _SimpleQueue
{
    uint32_t rear;
    uint32_t front;
    uint32_t elements_count;
    size_t size;
    const void** elements_array;
} SimpleQueue;

/*!
  \brief Initialize queue
  \param queue  SimpleQueue structure to Initialize
  \param buffer container for the queue elements
  \param size   Number of elements
*/
void QueueInit(SimpleQueue* queue, const void** buffer, size_t size);
/*!
  \brief Get number of elements in the queue
  \param  queue Pointer to SimpleQueue structure
  \return       number of elements
*/
 FORCE_INLINE inline uint32_t GetElementsCount(const SimpleQueue* queue)
 {
     return queue->elements_count;
 }
/*!
  \brief Check whether queue is full
  \param  queue Pointer to SimpleQueue structure
  \return       True / False
*/
 FORCE_INLINE inline uint32_t IsFull(const SimpleQueue* queue)
 {
     return queue->elements_count >= queue->size;
 }
/*!
  \brief Check whether queue is empty
  \param  queue Pointer to SimpleQueue structure
  \return       True / False
*/
 FORCE_INLINE inline uint32_t IsFree(const SimpleQueue* queue)
 {
     return queue->elements_count == 0;
 }
/*!
  \brief Retrieve the oldest element but don't remove it from queue.
  \param  queue   Pointer to SimpleQueue structure
  \param  element <out> container for retrieved element
  \return         error code
*/
ErrorCode Peek(const SimpleQueue* queue, const void** element);
/*!
  \brief Push element to the queue.
  \param  queue   Pointer to SimpleQueue structure
  \param  element element to push
  \return         error code
*/
ErrorCode Push(SimpleQueue* queue, const void* element);
/*!
  \brief Retrieve the oldest element.
  \param  queue   Pointer to SimpleQueue structure
  \param  element <out> container for retrieved element. Can be NULL.
  \return         error code
*/
ErrorCode Pop(SimpleQueue* queue, const void** element);
/*!
  \brief Remove element from the queue.
  \param  queue   Pointer to SimpleQueue structure
  \param  element element to remove
  \return         error code
*/
ErrorCode Remove(SimpleQueue* queue, const void* element);

#ifdef __cplusplus

namespace dsp_fw
{

template<class T, size_t N>
class Queue
{
public:
    Queue() : elements_cnt(0), position(0) {}

    T append(T element)
    {
        T popping_out = T();
        if (elements_cnt == N)
        {
            // element should pop out
            popping_out = elements[position];
        }
        elements[position] = element;
        elements_cnt = min(elements_cnt + 1, N);
        position = (position + 1) % N;
        return popping_out;
    }

    size_t len() const
    {
        return elements_cnt;
    }
    size_t size() const
    {
        return N;
    }

    void clear()
    {
        position = 0;
        elements_cnt = 0;
    }

    const T operator[](size_t index) const
    {
        size_t index_in_elements = calculate_index_in_elements(index);
        return elements[index_in_elements];
    }
    T& operator[](size_t index)
    {
        size_t index_in_elements = calculate_index_in_elements(index);
        return elements[index_in_elements];
    }

    T sum() const
    {
        T sum = T();
        for (size_t index = 0; index < len(); index++)
        {
            sum += this->operator [](index);
        }
        return sum;
    }
private:
    /*!
      \brief Calculate index of required queue element in elements array according to current position.
      \note The following implementation is valid since popping is NOT allowed for this queue.
     
      \param  index required element in queue. '0' returns first (the oldest item)
      \return       index in elements[] of the required element in queue
    */
    inline size_t calculate_index_in_elements(size_t index) const
    {
        assert(index < elements_cnt);
        if (elements_cnt == N)
        {
           index = (position + index) % elements_cnt;
        }
        return index;
    }
    T elements[N];
    size_t elements_cnt;
    uint32_t position;
};

}  // namespace dsp_fw

#endif  // __cplusplus
#endif  // FW_SRC_BASE_FW_UTILITIES_QUEUE_H_
