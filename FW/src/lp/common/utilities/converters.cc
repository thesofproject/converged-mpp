// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#include "converters.h"
#include <xt_hifi_defs.h>

void copy_32b_cb_to_24b(int8_t* out, const int8_t* in, size_t n_samples)
{
    const ae_f24x2* sin = (const ae_f24x2*)( in );
    ae_f24x2* sout = (ae_f24x2*)( out );

    ae_f24x2 vs;
    ae_valign align_out = AE_ZALIGN64();

    if (!IS_ALIGNED(sin, 8))
    {
        AE_L32F24_XC(vs, (const ae_f24*)sin, 4 );
        AE_SA24_IP(vs, align_out, sout);
        n_samples--;
    }

    for (size_t i = 0; i < n_samples / 2; i++ )
    {
        AE_L32X2F24_XC(vs, sin, 8 );
        AE_SA24X2_IP(vs, align_out, sout);
    }
    AE_SA64POS_FP( align_out, sout );

    if (n_samples % 2)
    {
        AE_L32X2F24_XC(vs, sin, 4 );
        ae_f24 tmp = AE_MOVAD32_H(AE_MOVINT24X2_FROMF24X2(vs));
        AE_SA24_IP(tmp, align_out, sout);
        AE_SA64POS_FP( align_out, sout );
    }
}

void copy_32b_to_24b(int8_t* out, const int8_t* in, size_t n_samples)
{
    const ae_int32* in_ptr = (const ae_int32*)(in);

    ae_valign align_out = AE_ZALIGN64( );
    for (size_t i = 0; i < n_samples; i++)
    {
        ae_int32 d32 = in_ptr[i] >> 8;
        ae_int24 d24 = AE_MOVINT24_FROMINT32( d32 );
        AE_SA24_L_IP( d24, align_out, out );
    }
    AE_SA64POS_FP( align_out, out );
}

void copy_24b_to_32b(int8_t* out, const int8_t* in, size_t n_samples)
{
    debug_assert(out != NULL);
    debug_assert(in != NULL);
    debug_assert(n_samples != 0);

    ae_int32x2* out_ptr = (ae_int32x2*)(out);

    ae_valign align_in = AE_LA64_PP(in);
    int i = 0;
    ae_int24x2 d24 = AE_ZERO24();
    if (!IS_ALIGNED(out_ptr, 8))
    {
        AE_LA24_IP(d24, align_in, in);
        ae_int32x2 d320 = d24;
        int higher = AE_MOVAD32_H(d320);
        *(ae_int32*)(out_ptr) = higher << 8;
        out_ptr = (ae_int32x2*)(out + 4);
        ++i;
    }
    // process two samples in single iteration to increase performance
    while (i < (int)n_samples - 1)
    {
        AE_LA24X2_IP(d24, align_in, in);
        ae_int32x2 d320 = d24;
        d320 = AE_SLAI32(d320, 8);
        AE_S32X2_IP(d320, out_ptr, 8);
        i += 2;
    }
    if (i != (int)n_samples)
    {
        AE_LA24X2_IP(d24, align_in, in);
        ae_int32x2 d320 = d24;
        int higher = AE_MOVAD32_H(d320);
        *(ae_int32*)(out_ptr) = higher << 8;
    }
}

void copy_16b_cb_to_16b(int16_t* out, const int16_t* in, size_t n_samples)
{
    const ae_int16x4* sin = reinterpret_cast<const ae_int16x4*> ( in );
    ae_int16x4* sout = reinterpret_cast<ae_int16x4*> ( out );
    ae_int16x4 vs;
    ae_valign align_in;
    ae_valign align_out = AE_ZALIGN64();
    AE_LA16X4POS_PC( align_in, sin );
    for (size_t i = 0; i < n_samples / 4; i++ )
    {
        AE_LA16X4_IC( vs, align_in, sin );
        AE_SA16X4_IP( vs, align_out, sout );
    }
    AE_SA64POS_FP( align_out, sout );
    size_t rest = n_samples % 4;
    if ( rest )
    {
        AE_LA16X4_IC( vs, align_in, sin );
        if ( 1 == rest )
        {
            ae_int16 d16 = AE_MOVAD16_3( vs );
            AE_S16_0_I(d16, reinterpret_cast<ae_int16*> (sout), 0 );
            return;
        }
        ae_int32 d32 = AE_MOVINT32_FROMINT16X4( AE_SHORTSWAP( vs ) );
        AE_S32_L_IP(d32, reinterpret_cast<ae_int32*> (sout), 4 );
        if ( 3 == rest )
        {
            ae_int16 d16 = AE_MOVAD16_1( vs );
            AE_S16_0_I(d16, reinterpret_cast<ae_int16*> (sout), 0 );
            return;
        }
    }
}

void copy_32b_to_16b(int16_t* out, const int32_t* in, size_t n_samples)
{
    uint32_t i = 0;
    const ae_int32* in_ptr = (const ae_int32*)(in);
    ae_int16* out_ptr = (ae_int16*)(out);

    if (!IS_ALIGNED(&in_ptr[i], 8))
    {
        ae_int32 d32 = in_ptr[i++] >> 16;
        ae_int16 d16 = AE_MOVINT16_FROMINT32( d32 );
        AE_S16_0_IP( d16, out_ptr, 2);
    }
    // going to process4 samples in single loop to increase perf
    ae_int16x4* out_ptr2 = (ae_int16x4*)(out_ptr);
    const ae_int16x4* in_ptr2 = (const ae_int16x4*)(&in_ptr[i]);
    ae_valign align_out = AE_ZALIGN64();
    for (; i < n_samples - 3; i+=4)
    {
        ae_int16x4 d32x2_0 = *(in_ptr2++);
        ae_int16x4 d32x2_1 = *(in_ptr2++);
        ae_int16x4 d16x4 = AE_SEL16_6420(d32x2_0, d32x2_1);
        AE_SA16X4_IP(d16x4 , align_out, out_ptr2);
    }
    AE_SA64POS_FP( align_out, out_ptr2 );
    out_ptr = (ae_int16*)(out_ptr2);
    while (i != n_samples)
    {
        ae_int32 d32 = in_ptr[i] >> 16;
        ae_int16 d16 = AE_MOVINT16_FROMINT32( d32 );
        AE_S16_0_IP( d16, out_ptr, 2 );
        i++;
    }
}

void copy_24b_to_16b(int8_t* out, const int8_t* in, size_t n_samples)
{
    for (size_t idx = 0; idx < n_samples; ++idx)
    {
        out[idx*2] = in[idx * 3 + 1];
        out[idx*2+1] = in[idx * 3 + 2];
	}
}
