// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef ADSP_FW_SIMPLE_CIRCULAR_BUFFER_H
#define ADSP_FW_SIMPLE_CIRCULAR_BUFFER_H

#include "adsp_std_defs.h"
#include "utilities/array.h"

namespace dsp_fw
{

class RtCircularBuffer
{

public:
    RtCircularBuffer():
        buffer_active_(false),
        write_ptr_(NULL),
        read_ptr_(NULL),
        data_in_buffer_(0)
    {
    };

    void Init(uint8_t* buffer, size_t size)
    {
        assert(buffer!=NULL);
        ba_.Init(buffer, size);
        buffer_active_ = true;
        write_ptr_ = ba_.data();
        read_ptr_ = ba_.data();
    }

    ErrorCode Release()
    {
        ba_.Detach();
        buffer_active_ = false;
        return ADSP_SUCCESS;
    }

    /*!
      \brief Inserts data into CB from another circular buffer with HiFi enhancements.
    */
    ErrorCode PushDataFromCb(ByteArray* buffer);

    ErrorCode ReadData(ByteArray* buffer, size_t size);
    bool is_ba_active() const
    {
        return buffer_active_;
    }

    bool is_any_rt_buffered() const
    {
        return (data_in_buffer_ > 0 );
    }

    size_t data_in_buffer() const
    {
        return data_in_buffer_;
    }

    uint8_t* read_ptr()
    {
        uint8_t* rp;
        rp = ba_.size() + write_ptr_ -  data_in_buffer_;

        if(rp >= ba_.data_end())
        {
            rp -= ba_.size();
        }
        return rp;
    }

private:
    ByteArray ba_;
    bool buffer_active_;
    uint8_t* write_ptr_;
    uint8_t* read_ptr_;
    uint32_t data_in_buffer_;

};

} // namespace dsp_fw

#endif //ADSP_FW_SIMPLE_CIRCULAR_BUFFER_H
