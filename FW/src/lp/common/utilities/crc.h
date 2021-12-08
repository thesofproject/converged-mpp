// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef CRC_H
#define CRC_H
#include <stdint.h>

EXTERN_C_BEGIN

uint8_t CalculateCRC(uint8_t const* data, uint32_t data_len);

EXTERN_C_END

#endif // CRC_H
