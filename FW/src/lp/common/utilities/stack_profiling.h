// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef _ADSP_FW_STACK_PROFILING_H
#define _ADSP_FW_STACK_PROFILING_H

#define CANARY_PATTERN_SIZE 16

namespace dsp_fw
{
static const uint8_t CANARY_PATTERN[CANARY_PATTERN_SIZE] =
{
        0xCA, 0x01, 0xCA, 0x01, 0xCA, 0x01, 0xCA, 0x01,
        0xCA, 0x01, 0xCA, 0x01, 0xCA, 0x01, 0xCA, 0x01
}; 

/*!
  \def GenCanary
  Generates canary pattern on stack end, use CHECK_CANARY macro to verify after writing on stack
*/
#define GenCanary(stack_desc, canary_pattern) \
    assert((stack_desc)->size() > CANARY_PATTERN_SIZE); \
    memcpy_s((stack_desc)->data(), CANARY_PATTERN_SIZE, canary_pattern, CANARY_PATTERN_SIZE)

#if STACK_PROFILING
#define fill_pattern 0xDEADBEEF

/*!
  \brief Generates pattern on stack, which enables to find stack usage after task removal
  \param uint8_t adress of ByteArray stack descriptor 
  \param size_t space reserved for stack frame, context and etc
*/
static void GenStackPattern(ByteArray* stack_desc, size_t stack_reserved_space)
{
    size_t* pattern_start_ptr = reinterpret_cast<size_t*>(stack_desc->data() + CANARY_PATTERN_SIZE);
    size_t* pattern_end_ptr = reinterpret_cast<size_t*>(stack_desc->data() + stack_desc->size() - stack_reserved_space);
    for(size_t* pos = pattern_start_ptr; pos < pattern_end_ptr; pos++)
        *pos = fill_pattern;
}
#endif

inline ErrorCode VerifyCanary(const ByteArray& stack_desc, const uint8_t* const canary_pattern)
{
    return memcmp(stack_desc.data(), canary_pattern, CANARY_PATTERN_SIZE) == 0
            ? ADSP_SUCCESS : ADSP_OUT_OF_RESOURCES;
}

#if STACK_PROFILING
static size_t ProfileStack(const ByteArray& stack_desc)
{
    const size_t* pattern_start_ptr = reinterpret_cast<const size_t*>(stack_desc.data() + CANARY_PATTERN_SIZE);
    const size_t* pattern_end_ptr = reinterpret_cast<const size_t*>(stack_desc.data() + stack_desc.size());

    assert((pattern_end_ptr - pattern_start_ptr) > CANARY_PATTERN_SIZE);

    size_t pattern_left = 0;
    for(const size_t* pos = pattern_start_ptr; pos < reinterpret_cast<const size_t*>(pattern_end_ptr); pos++, pattern_left++)
        if(*pos != fill_pattern)
            break;

    return pattern_left * sizeof(uint32_t);
}
#endif
}
#endif //#ifndef _STACK_PROFILING_H
