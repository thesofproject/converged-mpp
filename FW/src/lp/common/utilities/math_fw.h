// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef __MATH_FW__
#define __MATH_FW__

// Obtaining low and high part of uint16_t
#define UINT16_GET_LOW_PART(x)  (x & 0xFF)
#define UINT16_GET_HIGH_PART(x) ((x >> 8) & 0xFF)

float powf_fw(float base, float exponent);
float logf_fw(float arg);
float log10f_fw(float arg);
int abs_fw(int n);
float fabsf_fw(float n);
float expf_fw(float arg);
float sqrtf_fw(float arg);
float ceilf_fw(float arg);
float floorf_fw(float arg);
float sinf_fw(float arg);
float cosf_fw(float arg);
float atanf_fw(float arg);

#endif /* __FW_MATH__ */
