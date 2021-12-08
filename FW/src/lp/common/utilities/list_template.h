// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

/*!
  \file
  Two way list template declaration.
*/

#ifndef DSP_FW_UTILITIES_LIST_H
#define DSP_FW_UTILITIES_LIST_H

#include "adsp_std_defs.h"

namespace dsp_fw
{
template <class T>
class ListItem
{
public:
    ListItem(): next_item_(NULL), previous_item_(NULL)
    {        
    }
    void SetNextItem(T* const next_item)
    {
        next_item_ = next_item;
    }
    void SetPreviousItem(T* const previous_item)
    {
        previous_item_ = previous_item;
    }
    T* const next_item()
    {
        return next_item_;
    }
    T* const previous_item()
    {
        return previous_item_;
    }
protected:
    T* next_item_;
    T* previous_item_;
};

template <class T>
class List
{
public:
    List(): tail_(NULL), head_(NULL) 
    {
    }    
    void AddElement(T * const Item)
    {
        /* Check if existis in other list (zero element lists not verified) */
        //assert(Item->previous_item() == NULL);
        //assert(Item->next_item() == NULL);
        /* If list is empty set head and tail */
        if(items_counter_ == 0)
        {
            head_ = Item;
            Item->SetNextItem(NULL);
            Item->SetPreviousItem(NULL);
        }        
        else
        {
            Item->SetPreviousItem(tail_);
            tail_->SetNextItem(Item);
        }
        tail_ = Item;
        items_counter_++;
    }
    ErrorCode RemoveElement(T * const Item)
    {
        /*Check if element exists in list */
        if (!ExistInList(Item))
        {
            return ADSP_INVALID_REQUEST;
        }
        /* Check if head */
        if (NULL != Item->previous_item())
        {
            Item->previous_item()->SetNextItem(Item->next_item());
        }
        else
        {
            head_ = Item->next_item();
        }
        /* Check if tail */
        if (NULL != Item->next_item())
        {
            Item->next_item()->SetPreviousItem(Item->previous_item());
        }
        else
        {
            tail_ = Item->previous_item();
        }
        items_counter_--;
        return ADSP_SUCCESS;
    }
    T* head()
    {
        return head_;
    }
    T* tail()
    {
        return tail_;
    }
    uint16_t items_counter()
    {
        return items_counter_;
    }
private:
    uint16_t items_counter_;
    T* tail_;
    T* head_;
    bool ExistInList(T * const Item)
    {
        T* current_item = head_;

        while (NULL != current_item)
        {
            if (current_item == Item)
            {
                return true;
            }
            current_item = current_item->next_item();
        }
        return false;
    }
};
}

#endif //DSP_FW_UTILITIES_TWO_WAY_LIST_H
