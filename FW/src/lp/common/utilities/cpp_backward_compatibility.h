// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef MPUTILS_CPPBACKWARDCOMPATIBILITY_H_
#define MPUTILS_CPPBACKWARDCOMPATIBILITY_H_
#if (__cplusplus < 201103L)

#define S_ASSERT_CONCAT1(a, b) a##b
#define S_ASSERT_CONCAT2(a, b) S_ASSERT_CONCAT1(a, b)
#define S_ASSERT(e) enum { S_ASSERT_CONCAT2(assert_line_, __LINE__) = 1/(!!(e)) }

#define static_assert(x, y) S_ASSERT(x)

#define constexpr const

#define final

#define override

#endif
#endif /*MPUTILS_CPPBACKWARDCOMPATIBILITY_H_*/

