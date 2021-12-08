// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef _ADSP_FW_CONVERTERS_H
#define _ADSP_FW_CONVERTERS_H

#include "adsp_std_defs.h"

void copy_32b_cb_to_24b(int8_t* out, const int8_t* in, size_t n_samples);
void copy_24b_to_32b(int8_t* out, const int8_t* in, size_t n_samples);
void copy_32b_to_24b(int8_t* out, const int8_t* in, size_t n_samples);
void copy_24b_to_16b(int8_t* out, const int8_t* in, size_t n_samples);
void copy_16b_cb_to_16b(int16_t* out, const int16_t* in, size_t n_samples);
void copy_32b_to_16b(int16_t* out, const int32_t* in, size_t n_samples);

#endif //_ADSP_FW_CONVERTERS_H
