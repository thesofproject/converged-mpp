// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

/*!
  \file
  Definition of different types of circullar buffers.
*/

#ifndef DSP_FW_FW_UTILITIES_CIRCULAR_BUFFERS_H_
#define DSP_FW_FW_UTILITIES_CIRCULAR_BUFFERS_H_

#include <stdint.h>
#include "adsp_error.h"
#include "utilities/circular_buffer_template.h"

namespace dsp_fw
{
/*!
  \brief Predefined type of circular buffer consisting of bytes.
*/
typedef CircularBuffer<uint8_t> ByteCircularBuffer;

/*!
  \brief Predefined type of circular buffer consisting of dwords.
*/
typedef CircularBuffer<uint32_t>DwordCircularBuffer;

/*!
  \brief ByteArraySized is linear array for local AAC codec input storage.
  Its size
  Inherited size_ is considered as maximum allowable array size. //formerly local_in_size received from codec API as max size request
  Current valid data size is represented by filled_size_
*/
class ByteArraySized: public ByteArray
{
public:
    /**
     * Default ctor to provide two-stage initialization completed by Init() call.
     */
    ByteArraySized() :filled_size_(0) {}

    inline ErrorCode fill_in_cir(dsp_fw::ByteCircularBuffer* cir_in_buffer)
    {
        //local_array should be always shifted left (to its begining) before buffer filling
        //8-byte chunks are copied into buffer

        ByteArray in;
        uint32_t copy_cnt;
        uint32_t local_free_aligned = 0;

        if (get_free() < 8)
        {
            return ADSP_BUSY; //not an error, ADSP_SUCCESS ???
        }

        /** Here it is assumed that input buffer is always 8-bytes aligned
         * and chunks of 8 byte data are copied
         * Local_buffer (so local_free) is not aligned
         */
        while ( (get_free() > 7) && (cir_in_buffer->GetDataSize() > 7) )
        {
            local_free_aligned = get_free() - (get_free() % 8);
            in.Detach();
            cir_in_buffer->GetReadableBuffer(&in);
            copy_cnt = min(local_free_aligned, in.size());
            //fill Local_buffer
            memcpy_s( data() + data_size(), copy_cnt, in.data(), copy_cnt );
            cir_in_buffer->ReadCommit( copy_cnt, true );
            //resize Local_buffer
            filled_size_ = filled_size_ + copy_cnt;
            //assert (filled_size_ <= size() );
        }
        return ADSP_SUCCESS;
    }

    /**
     * After buffered data consumption the remaining data must be left shifted
     * to retain buffer pointer aligned.
     */
    inline ErrorCode shift_left(uint32_t consumed) {
        if (filled_size_ < consumed)
        {
            return ADSP_ERROR_INVALID_PARAM;
        }
        memmove(data(), data() + (size_t)consumed, (filled_size_ - consumed) * sizeof(uint8_t));
        filled_size_ = filled_size_ - consumed;
        return ADSP_SUCCESS;
    };

    inline size_t get_free(void) {
        return size() - filled_size_;
    }

    inline size_t data_size() const {
        return filled_size_;
    }
private:
    size_t filled_size_;
};

}

#endif /* DSP_FW_FW_UTILITIES_CIRCULAR_BUFFERS_H_ */
