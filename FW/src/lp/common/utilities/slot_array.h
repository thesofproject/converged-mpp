// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef _ADSP_FW_SLOT_ARRAY_H
#define _ADSP_FW_SLOT_ARRAY_H

#include "utilities/bitmap.h"
#include "adsp_std_defs.h"

namespace dsp_fw
{

template<class T, size_t N> class SlotArray
{
public:
    /*!
      \brief Indicates invalid index.
    */
    static const size_t INVALID_INDEX = (size_t)-1;

    /*!
      \brief Default ctor.
      Note that default ctor of type T is called for all entries.
    */
    SlotArray()
    {
    }

    /*!
      \brief Retrieves size of the array.
    */
    size_t GetSize() const
    {
        return sizeof(items_);
    }

    /*!
      \brief Retrieves number of free slots.
    */
    size_t GetFreeCount() const
    {
        return bitmap_.GetFreeCount();
    }

    /*!
      \brief Inserts the item into the first unoccupied slot
             and returns index of this slots. If there are no
             free slot INVALID_INDEX is returned.
     
      \note search starts always from the beginnig of the array.
            Thus ForEach() processes items in order they were inserted to the array.
    */
    size_t Insert(const T& new_item)
    {
        size_t idx = bitmap_.Alloc();
        if (idx == INVALID_INDEX)
            return idx;
        items_[idx] = new_item;
        return idx;
    }

    /*!
      \brief Frees slot at the specified index.
      If slot is unoccuppied, invalid paramter error is returned.
      Dtor is called, but if T is a pointer type, the object
      is not deleted, pointer is just wiped off. Owner of the object
      is responsisble for deleting the object.
    */
    ErrorCode FreeAt(size_t index)
    {
        return bitmap_.Free(index);
    }

    /*!
      \brief Direct access to the element at specified index.
      We cannot just return the reference since exception
      cannot be thrown if unoccupied slot is accessed.

      \return Ptr or NULL if slot unoccupied.
    */
    const T* GetAt(size_t index) const
    {
        if (bitmap_.isFree(index))
            return NULL;
        return &items_[index];
    }

    /*!
      \return Ptr or NULL if slot unoccupied.
    */
    T* GetAt(size_t index)
    {
        if (bitmap_.isFree(index))
            return NULL;
        return &items_[index];
    }

    /*!
      \param       index starts from index
      \param[out]  ret_index retrieved index
      \return      Ptr to first occupied slot
    */
    T* GetFirstUsed(size_t index, size_t* ret_index)
    {
       for (size_t i = index; i < N; ++i)
       {
           if (!bitmap_.isFree(i))
           {
               *ret_index = i;
               return &items_[i];
           }
       }
       return NULL;
    }

    T* Allocate()
    {
        for (size_t idx = 0;; ++idx)
        {
            if (bitmap_.isFree(idx))
            {
                bitmap_.AllocAt(idx);
                return &items_[idx];
            }
        }
        return NULL;
    }
    /*!
      \brief Runs the item_func for each occupied slot.
    */
    template<class ItemFunction> ErrorCode ForEach(ItemFunction item_func)
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (!bitmap_.isFree(i))
                item_func(items_[i]);
        }
        return ADSP_SUCCESS;
    }

    /*!
      \brief Runs the item_func with extra arg for each occupied slot.
    */
    template<class ItemFunction, class ItemFunctionArg> ErrorCode ForEach(ItemFunction item_func, ItemFunctionArg arg)
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (!bitmap_.isFree(i))
                item_func(items_[i], arg);
        }
        return ADSP_SUCCESS;
    }

    /*!
      \brief Runs the item_func with 2 extra args for each occupied slot.
    */
    template<class ItemFunction,
             class ItemFunctionArg1,
             class ItemFunctionArg2> ErrorCode ForEach(ItemFunction item_func, ItemFunctionArg1 arg1, ItemFunctionArg2 arg2)
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (!bitmap_.isFree(i))
                item_func(items_[i], arg1, arg2);
        }
        return ADSP_SUCCESS;
    }

    /*!
      \brief Returns index of first item that equals specified argument.
      Comparison is done using std operator==.
    */
    size_t Find(const T& item)
    {

        for (size_t i = 0; i < N; ++i)
        {
            if (item == items_[i])
                return i;
        }
        return INVALID_INDEX;
    }

    /*!
      \brief removes item from array
      Comparison is done using std operator==.
    */
    ErrorCode Remove(const T& item)
    {
        for (size_t idx = 0; idx < N; ++idx)
        {
            if (item == items_[idx])
            {
                bitmap_.Free(idx);
                return ADSP_SUCCESS;
            }
        }
        return ADSP_NOT_FOUND;
    }

    /*!
      \brief removes item from array
    */
    ErrorCode Remove(const T* item)
    {
        for (size_t idx = 0; idx < N; ++idx)
        {
            if (item == &items_[idx])
            {
                bitmap_.Free(idx);
                return ADSP_SUCCESS;
            }
        }
        return ADSP_NOT_FOUND;
    }

    /*!
      \brief Returns index of first item that meets criteria defined by compare
      (bool result of compare is true).
    */
    template<class Comparator, class V> size_t Find(Comparator compare, const V& v);

private:
    /*!
      \brief Array of items allocated here.
    */
    T items_[N];

    Bitmap<N> bitmap_;
};

}

#endif //#ifndef _ADSP_FW_SLOT_ARRAY_H
