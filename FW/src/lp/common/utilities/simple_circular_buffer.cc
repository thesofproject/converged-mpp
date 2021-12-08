// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#include "simple_circular_buffer.h"
#include <xtensa/tie/xt_hifi3.h>

namespace dsp_fw
{

void copy_from_cb(uint8_t* out, size_t s_out, const uint8_t* in, size_t bytes)
{
    size_t n_samples = bytes / 4;
    const ae_int32x2* sin = reinterpret_cast<const ae_int32x2*> ( in );
    ae_int32x2* sout = reinterpret_cast<ae_int32x2*> ( out );
    ae_int32x2 vs;
    if (!IS_ALIGNED(sin, 8))
    {
        AE_L32_XC(vs, reinterpret_cast<const ae_int32*> (sin), 4 );
        AE_S32_L_IP(vs, reinterpret_cast<ae_int32*> (sout), 4 );
        n_samples--;
    }
    ae_valign align_out = AE_ZALIGN64();
    for (size_t i = 0; i < n_samples / 2; i++ )
    {
        AE_L32X2_XC( vs, sin, 8 );
        AE_SA32X2_IP( vs, align_out, sout );
    }
    AE_SA64POS_FP( align_out, sout );
    if ( n_samples % 2 )
    {
        AE_L32_XC(vs, reinterpret_cast<const ae_int32*> (sin), 0 );
        AE_S32_L_IP(vs, reinterpret_cast<ae_int32*> (sout), 0 );
    }
}

void copy_from_cb_(uint8_t* out, size_t s_out, const uint8_t* in, size_t n_samples)
{
    const ae_int24x2* sin = reinterpret_cast<const ae_int24x2*> ( in );
    ae_int24x2* sout = reinterpret_cast<ae_int24x2*> ( out );
    ae_int24x2 vs;
    ae_valign align_in;
    ae_valign align_out = AE_ZALIGN64();
    AE_LA24X2POS_PC( align_in, sin );
    for (size_t i = 0; i < n_samples / 6; i++ )
    {
        AE_LA24X2_IC( vs, align_in, sin );
        AE_SA24X2_IP( vs, align_out, sout );
    }
    size_t rest = n_samples % 6;
    if ( rest )
    {
        AE_LA24X2_IC( vs, align_in, sin );
        ae_int32 d32;
        if ( rest >= 3 )
        {
            AE_SA24_IP( AE_MOVAD32_H( AE_MOVINT32X2_FROMINT24X2( vs ) ), align_out, sout );
            d32 = AE_MOVAD32_L( AE_MOVINT32X2_FROMINT24X2( vs ) );
            rest -= 3;
        }
        else
        {
            d32 = AE_MOVAD32_H( AE_MOVINT32X2_FROMINT24X2( vs ) );
        }
        AE_SA64POS_FP( align_out, sout );
        if ( 0 == rest-- ) return;
        out = reinterpret_cast<uint8_t*> ( sout );
        *(out++) = static_cast<uint8_t> ( AE_MOVAD32_L( d32 ) );
        if ( 0 == rest ) return;
        *out = static_cast<uint8_t> ( AE_MOVAD32_L( AE_SRLA32( d32, 8 ) ) );
        return;
    }
    AE_SA64POS_FP( align_out, sout );
}

ErrorCode RtCircularBuffer::PushDataFromCb(ByteArray* buffer)
{
    kw_assert(buffer != NULL);
    const size_t size = buffer->size();
    if (ba_.size() < (data_in_buffer_ + size))
    {
        return ADSP_SUCCESS;
    }
    RETURN_EC_ON_FAIL(ba_.size() >= size, ADSP_OUT_OF_RESOURCES);
    if (size <= (uint32_t)(ba_.data_end() - write_ptr_))
    {
        copy_from_cb(write_ptr_, ba_.size(), buffer->data(), size );
        write_ptr_ += size;
    }
    else
    {
        uint32_t non_wrapped_size = (uint32_t)(ba_.data_end() - write_ptr_);
        uint32_t reminder_size = size - non_wrapped_size;

        copy_from_cb(write_ptr_, ba_.size(), buffer->data(), non_wrapped_size );
        uint32_t c_beg = (uint32_t)AE_GETCBEGIN0();
        uint32_t c_end = (uint32_t)AE_GETCEND0();
        uint32_t b_beg = (uint32_t)buffer->data();
        if ((b_beg + non_wrapped_size) < c_end)
        {
            copy_from_cb(ba_.data(), ba_.size(), buffer->data() + non_wrapped_size, reminder_size );
        }
        else
        {
            size_t delta = (b_beg + non_wrapped_size) - c_end;
            uint8_t* new_begin = (uint8_t*)(c_beg + delta);
            copy_from_cb(ba_.data(), ba_.size(), new_begin, reminder_size );
        }
        write_ptr_ = ba_.data() + reminder_size;
    }

    if (write_ptr_ == ba_.data_end())
    {
        write_ptr_ = ba_.data();
    }

    data_in_buffer_ += size;
    if (data_in_buffer_ > ba_.size())
    {
        HALT_ON_ERROR(ADSP_CIRCULAR_BUFFER_OVERRUN);
        return ADSP_SUCCESS;
    }

    return ADSP_SUCCESS;
}

ErrorCode RtCircularBuffer::ReadData(ByteArray* buffer, size_t size)
{
    // buffer is not circular
    // buffer is given
    // uint8_t* rp = read_ptr_;
    RETURN_EC_ON_FAIL(ba_.size() >= size, ADSP_OUT_OF_RESOURCES);
    RETURN_EC_ON_FAIL(size <= data_in_buffer_, ADSP_CIRCULAR_BUFFER_UNDERRUN);

    void* cached_c_beg = AE_GETCBEGIN0();
    void* cached_c_end = AE_GETCEND0();

    AE_SETCBEGIN0(ba_.data());
    AE_SETCEND0(ba_.data_end());

    copy_from_cb(buffer->data(), size, read_ptr(), size);

    AE_SETCBEGIN0(cached_c_beg);
    AE_SETCEND0(cached_c_end);

    data_in_buffer_ -= size;

    return ADSP_SUCCESS;
}

} //namespace dsp_fw
