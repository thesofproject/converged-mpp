// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef ADSP_FW_UTILITIES_CRITICAL_SECTION_H
#define ADSP_FW_UTILITIES_CRITICAL_SECTION_H

namespace dsp_fw
{

/*!
  \brief Enters critical section for whole block from the moment CSBlock is created on stack.
  Usage:
    YOUR CODE HERE
    CSBlock critical_section; // enters critical section
    YOUR CODE HERE
    return value; // before return is called compiler will call ~CSBlock() for critical_section
*/
class CSBlock
{
public:
    CSBlock()
    {
        old_int_level_ = _xtos_set_intlevel(GET_VALUE(CS_INT_LEVEL));
    }

    explicit CSBlock(uint32_t int_level)
    {
        old_int_level_ = _xtos_set_intlevel(int_level);
    }

    ~CSBlock()
    {
        _xtos_set_intlevel(old_int_level_);
    }
private:
    uint32_t old_int_level_;
};

} // namespace dsp_fw

#endif /* ADSP_FW_UTILITIES_CRITICAL_SECTION_H */
