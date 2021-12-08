// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef _ADSP_FW_SIMPLE_MEM_ALLOC_H
#define _ADSP_FW_SIMPLE_MEM_ALLOC_H

#include "utilities/array.h"
#include "error_handling.h"

namespace dsp_fw
{

class SimpleMemAlloc
{
public:
    static const size_t NATIVE_ALIGNMENT_BOUNDARY = sizeof(uint32_t);
    static const size_t MAX_ALIGNMENT_BOUNDARY = 16;

    SimpleMemAlloc() :begin_free_(NULL) {}

    void Init(uint8_t* buf, size_t size)
    {
        begin_free_ = buf;
        buf_.Init(buf, size);
    }

    void* Alloc(uint32_t alignment, uint32_t size)
    {
        size_t al_space = (alignment - (reinterpret_cast<uint32_t>(begin_free_) % alignment)) % alignment;

        if ((begin_free_ + size + al_space) >
            (buf_.data() + buf_.size()))
        {
            return NULL;
        }

        begin_free_ += al_space;
        void* p = begin_free_;

        begin_free_ += size;

        return p;
    }

    ErrorCode Alloc(uint32_t alignment, uint32_t size, ByteArray* arr)
    {
        void* p = Alloc(alignment, size);
        if (NULL == p)
            return ADSP_OUT_OF_RESOURCES;
        arr->Init(reinterpret_cast<uint8_t*>(p), size);
        return ADSP_SUCCESS;
    }

    void Reset()
    {
        begin_free_ = buf_.data();
    }

    size_t GetUsage() const
    {
        return reinterpret_cast<uint32_t>(begin_free_) - reinterpret_cast<uint32_t>(buf_.data());
    }

    size_t GetUnusedMemSize() const
    {
        return reinterpret_cast<uint32_t>(buf_.data() + buf_.size()) -
               reinterpret_cast<uint32_t>(begin_free_);
    }

private:
    ByteArray buf_;
    uint8_t* begin_free_;
};

}  // namespace dsp_fw

inline void* operator new(size_t buffer_size, dsp_fw::SimpleMemAlloc* const pool) throw()
{
    return pool->Alloc(dsp_fw::SimpleMemAlloc::NATIVE_ALIGNMENT_BOUNDARY, buffer_size);
}

inline void* operator new[](size_t buffer_size, dsp_fw::SimpleMemAlloc* const pool) throw()
{
    return pool->Alloc(dsp_fw::SimpleMemAlloc::NATIVE_ALIGNMENT_BOUNDARY, buffer_size);
}

inline void* operator new(size_t buffer_size, dsp_fw::SimpleMemAlloc* const pool, uint32_t alignment) throw()
{
    assert(alignment >= dsp_fw::SimpleMemAlloc::NATIVE_ALIGNMENT_BOUNDARY);
    RETURN_EC_ON_FAIL(alignment >= dsp_fw::SimpleMemAlloc::NATIVE_ALIGNMENT_BOUNDARY, NULL);
    return pool->Alloc(alignment, buffer_size);
}

inline void* operator new[](size_t buffer_size, dsp_fw::SimpleMemAlloc* const pool, uint32_t alignment) throw()
{
    assert(alignment >= dsp_fw::SimpleMemAlloc::NATIVE_ALIGNMENT_BOUNDARY);
    RETURN_EC_ON_FAIL(alignment >= dsp_fw::SimpleMemAlloc::NATIVE_ALIGNMENT_BOUNDARY, NULL);
    return pool->Alloc(alignment, buffer_size);
}

#endif  //#ifndef _ADSP_FW_SIMPLE_MEM_ALLOC_H
