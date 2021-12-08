// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef _ASDP_FW_LIST_H
#define _ASDP_FW_LIST_H

#include "utilities/bitmap.h"
#include <core/kernel/memory/memory_pool.h>
#include "simple_mem_alloc.h"

namespace dsp_fw
{

/*!
  \brief Template for simple unidirectional list.
  List may be associated with a MemoryPool
  for automatic allocation of memory buffers
  for items being inserted.
 
  Passing new elements by copy enables type conversions
  and usually ptrs are kept as T so there is no
  overhead.
*/
template<class T>
class UniList
{
public:

    /*!
      \brief List item.
    */
    struct Item
    {
        /*!
          \brief Object. Typically T is a pointer, so there is
          no overhead of copy.
        */
        T elem;
        /*!
          \brief Pointer to the next element.
        */
        Item* next;
        /*!
          \brief Constructs new list item and stores
          reference to the object.
        */
        explicit Item(T& elem) :
            elem(elem), next(NULL)
        {
        }
        Item(): elem(NULL), next(NULL) {}
    };

    /*!
      \brief Creates empty list.
    */
    UniList() :size_(0), head_(NULL), tail_(NULL) {}

    /*!
      \brief Reset list to initial state
    */
    void Reset()
    {
        size_ = 0;
        head_ = NULL;
        tail_ = NULL;
    }

    /*!
      \brief Retrieves list of the list.
      \return Size of the list.
    */
    size_t GetSize() const { return size_; }

    /*!
      \brief Retrieves a head of the list.
      \return Pointer to the head of the list.
    */
    Item* GetHead() { return head_; }

    /*!
      \brief Retrieves a tail of the list.
      \return Pointer to the tail of the list.
    */
    Item* GetTail() { return tail_; }

    /*!
      \brief Rerieves a head of the list.
      \return Pointer to the head of the list.
    */
    const Item* GetHead() const { return head_; }

    /*!
      \brief Retrieves a tail of the list.
      \return Pointer to the tail of the list.
    */
    const Item* GetTail() const { return tail_; }

    /*!
      \brief Inserts a new element at the beginning of the list.
      \param item Pointer to the item being inserted.
      \return ADSP_SUCCESS Insertion suceeded.
    */
    ErrorCode PushFront(Item* item)
    {
        if (NULL == head_)
        {
            head_ = item;
            tail_ = item;
        }
        else
        {
            item->next = head_;
            head_ = item;
        }
        ++size_;
        return ADSP_SUCCESS;
    }

    /*!
      \brief Inserts a new element at the beginning of the list.
      This version allocates space for a list item
      referring the object being inserted.
      \param mem_pool Pointer to the memory pool used to allocate.
      \param elem Pointer to the element being inserted.
      \return ASDP_OUT_OF_RESOURCES Memory allocation failed.
      \return ASDP_SUCCESS Insertion succeeded.
    */
    ErrorCode PushFront(memory_pool_s* mem_pool, T elem)
    {
        Item* it = new(mem_pool) Item(elem);
        if (NULL == it)
            return ADSP_OUT_OF_RESOURCES;
        return PushFront(it);
    }

    /*!
      \brief Inserts a new element at the end of the list.
      \param item Pointer to the item being inserted.
      \return ADSP_SUCCESS Insertion suceeded.
    */
    ErrorCode PushBack(Item* item)
    {
        if (NULL == head_)
        {
            head_ = item;
        }
        if (NULL != tail_)
        {
            tail_->next = item;
        }
        tail_ = item;
        ++size_;
        return ADSP_SUCCESS;
    }

    /*!
      \brief Inserts a new element at the end of the list.
      This version allocates space for a list item
      referring the object being inserted.
      \param mem_pool Pointer to the memory pool used to allocate.
      \param elem Pointer to the element being inserted.
      \return ASDP_OUT_OF_RESOURCES Memory allocation failed.
      \return ASDP_SUCCESS Insertion succeeded.
    */
    ErrorCode PushBack(memory_pool_s* mem_pool, T elem)
    {
        Item* it = new(mem_pool) Item(elem);
        if (NULL == it)
            return ADSP_OUT_OF_RESOURCES;
        return PushBack(it);
    }

    ErrorCode PushBack(SimpleMemAlloc* pool, T elem)
    {
        Item* it = new(pool) Item(elem);
        if (NULL == it)
            return ADSP_OUT_OF_RESOURCES;
        return PushBack(it);
    }

    /*!
      \brief Inserts given list at the end of the list.
      \param item Reference to list to add.
      \retval ADSP_SUCCESS Insertion suceeded.
    */
    ErrorCode PushBack(UniList<T>& list)
    {
        if (NULL == head_)
        {
            head_ = list.GetHead();
        }
        if (NULL != tail_)
        {
            tail_->next = list.GetHead();
        }
        tail_ = list.GetTail();
        size_ += list.GetSize();
        return ADSP_SUCCESS;
    }

private:
    /* Current size of the list. */
    size_t size_;
    /* Pointer to the head of the list. */
    Item* head_;
    /* Pointer to the tail of the list. */
    Item* tail_;
};

template<class T>
class BiList
{
public:
    struct Item
    {
        T elem;
        Item* prev;
        Item* next;

        explicit Item(T& elem) :
            elem(elem), prev(NULL), next(NULL)
        {
        }

        explicit Item() :
            elem(NULL), prev(NULL), next(NULL)
        {
        }
    };
    BiList() :size_(0), head_(NULL), tail_(NULL) {}
    size_t GetSize() const { return size_; }
    Item* GetHead() { return head_; }
    const Item* GetHead() const { return head_; }
    Item* GetTail() { return tail_; }
    const Item* GetTail() const { return tail_; }

    ErrorCode PushBack(Item* item)
    {
        if (NULL == head_)
        {
            head_ = item;
            item->prev = NULL;
            item->next = NULL;
        }
        if (NULL != tail_)
        {
            tail_->next = item;
            item->prev = tail_;
            item->next = NULL;
        }
        tail_ = item;
        ++size_;
        return ADSP_SUCCESS;
    }

    ErrorCode PushBack(memory_pool_s* mem_pool, T elem)
    {
        Item* it = new(mem_pool) Item(elem);
        if (NULL == it)
            return ADSP_OUT_OF_RESOURCES;
        return PushBack(it);
    }

    void Reset(bool clear_items = false)
    {
        if (clear_items)
        {
            /* Clear all the items. */
            Item* item = head_;
            while (NULL != item)
            {
                Item *next_item = item->next;
                item->elem = NULL;
                item->prev = NULL;
                item->next = NULL;
                item = next_item;
            }
        }

        size_ = 0;
        head_ = NULL;
        tail_ = NULL;
    }

private:
   size_t size_;
   Item* head_;
   Item* tail_;
};

/*!
  \brief bi-directional list with pre allocated array
*/
template<class T, size_t N>
class BiListPreAlloc
{
public:
    struct Item
    {
        T elem;
        Item* prev;
        Item* next;
        Item() :
            prev(NULL), next(NULL)
        {
        }
        explicit Item(T& elem) :
            elem(elem), prev(NULL), next(NULL)
        {
        }
    };
    BiListPreAlloc() :size_(0), head_(NULL), tail_(NULL){}
    size_t GetSize() const { return size_; }
    size_t GetFreeSize() const { return N - size_; }
    Item* GetHead() { return head_; }
    const Item* GetHead() const { return head_; }
    Item* GetTail() { return tail_; }
    const Item* GetTail() const { return tail_; }

    ErrorCode PushBack(T& elem)
    {
        size_t idx = bitmap_.Alloc();
        RETURN_EC_ON_FAIL(INVALID_INDEX_BITMAP != idx, ADSP_LIST_CANNOT_PUSH_BACK_ELEMENT);
        ErrorCode ec = ADSP_LIST_CANNOT_PUSH_BACK_ELEMENT;
        ENTER_CRITICAL_SECTION(PUSH_BACK);
        ++size_;
        if (NULL == head_)
        {
            head_ = &items_[idx];
            head_->prev = NULL;
            head_->next = NULL;
            head_->elem = elem;
            tail_ = head_;
            ec = ADSP_SUCCESS;
        }
        else if (NULL != tail_)
        {
            tail_->next = &items_[idx];
            items_[idx].elem = elem;
            items_[idx].prev= tail_;
            items_[idx].next = NULL;
            tail_ = tail_->next;
            ec = ADSP_SUCCESS;
        }
        LEAVE_CRITICAL_SECTION(PUSH_BACK);
        return ec;
    }

    ErrorCode PutAfter(Item* it, T& elem)
    {
        if (it == NULL)
        {
            // undetermined behavior if list is not empty
            assert(head_ == NULL);
            assert(tail_ == NULL);
            return PushFront(elem);
        }
        size_t idx = bitmap_.Alloc();
        RETURN_EC_ON_FAIL(INVALID_INDEX_BITMAP != idx, ADSP_LIST_CANNOT_PUT_AFTER_ELEMENT);
        ENTER_CRITICAL_SECTION(PUT_AFTER);

        ++size_;
        Item* new_it = &items_[idx];
        new_it->elem = elem;
        Item* next_it = it->next;
        if (next_it == NULL)
        {
            new_it->next = NULL;
            new_it->prev = it;
            it->next = new_it;
            tail_ = new_it;
        }
        else
        {
            new_it->next = next_it;
            new_it->prev = it;
            next_it->prev = new_it;
            it->next = new_it;
        }

        LEAVE_CRITICAL_SECTION(PUT_AFTER);
        return ADSP_SUCCESS;
    }

    ErrorCode PutBefore(Item* it, T& elem)
    {
        if (it == NULL)
        {
            // undetermined behavior if list is not empty
            assert(head_ == NULL);
            assert(tail_ == NULL);
            return PushFront(elem);
        }
        size_t idx = bitmap_.Alloc();
        RETURN_EC_ON_FAIL(INVALID_INDEX_BITMAP != idx, ADSP_LIST_CANNOT_PUT_BEFORE_ELEMENT);
        ENTER_CRITICAL_SECTION(PUT_BEFORE);

        ++size_;
        Item* new_it = &items_[idx];
        new_it->elem = elem;
        Item* prev_it = it->prev;
        if (prev_it == NULL)
        {
            new_it->next = it;
            new_it->prev = NULL;
            it->prev = new_it;
            head_ = new_it;
        }
        else
        {
            new_it->next = it;
            new_it->prev = it->prev;
            it->prev = new_it;
            prev_it->next = new_it;

        }
        LEAVE_CRITICAL_SECTION(PUT_BEFORE);
        return ADSP_SUCCESS;
    }

    ErrorCode PushFront(T& elem)
    {
        size_t idx = bitmap_.Alloc();
        RETURN_EC_ON_FAIL(INVALID_INDEX_BITMAP != idx, ADSP_LIST_CANNOT_PUSH_FRONT_ELEMENT);

        ENTER_CRITICAL_SECTION(PUSH_FRONT);
        ++size_;
        if (NULL == head_)
        {
            head_ = &items_[idx];
            head_->prev = NULL;
            head_->next = NULL;
            head_->elem = elem;
            tail_ = head_;
        }
        else
        {
            items_[idx].elem = elem;
            items_[idx].prev = NULL;
            items_[idx].next = head_;
            head_->prev = &items_[idx];
            head_ = &items_[idx];
        }
        LEAVE_CRITICAL_SECTION(PUSH_FRONT);
        return ADSP_SUCCESS;
    }

    size_t Find(const T& elem) const
    {
        for (size_t idx = 0; idx < N; ++idx)
        {
            if (bitmap_.isFree(idx))
                continue;
            if (items_[idx].elem == elem)
            {
                return idx;
            }
        }
        return INVALID_INDEX_BITMAP;
    }

    ErrorCode Remove(T& elem)
    {
        for (size_t idx = 0; idx < N; ++idx)
        {
            if (bitmap_.isFree(idx))
                continue;
            if (items_[idx].elem == elem)
            {
                ENTER_CRITICAL_SECTION(REMOVE_ELEMENT);
                Item* prev = items_[idx].prev;
                Item* next = items_[idx].next;
                // remove head
                if (&items_[idx] == head_)
                {
                    head_ = next;
                    if (head_ != NULL)
                    head_->prev = NULL;
                }
                // remove tail
                if (&items_[idx] == tail_)
                {
                    tail_ = prev;
                    if (tail_ != NULL)
                    tail_->next = NULL;
                }
                if (prev != NULL && next != NULL)
                {
                    prev->next = next;
                    next->prev = prev;
                }
                bitmap_.Free(idx);
                --size_;
                LEAVE_CRITICAL_SECTION(REMOVE_ELEMENT);
                return ADSP_SUCCESS;
            }
        }
        return ADSP_CANNOT_REMOVE_ELEMENT_FROM_LIST;
    }
private:
    Item items_[N];
    size_t size_;
    Item* head_;
    Item* tail_;
    Bitmap<N> bitmap_;
};
}

#endif //#ifndef _ASDP_FW_LIST_H
