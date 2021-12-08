// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef MPUTILS_SPUTEX_H_
#define MPUTILS_SPUTEX_H_

#include "fwkernel/arch/sputex.h"

namespace MpUtils
{

#if 0
/*!
  \brief Name and interface after C++11 std::mutex 
*/
class sputex
{
public:
    typedef xmp_atomic_int_t* native_handle_type;

    sputex()
    {
        xmp_atomic_int_init(&spin_lock_, 0);
    }
    
    void lock()
    {
        XMP_SIMPLE_SPINLOCK_ACQUIRE(&spin_lock_);
    }

    bool try_lock()
    {
        return (xmp_atomic_int_conditional_set(&spin_lock_, 0, xmp_prid() + 1) == 0);
    }

    void unlock()
    {
        // equivalent to XMP_SIMPLE_SPINLOCK_RELEASE(&spin_lock_)
        while (xmp_atomic_int_conditional_set(&spin_lock_, (int)xmp_prid() + 1, 0) != (int)xmp_prid() + 1)
            xmp_spin();
    }
    
    
    native_handle_type native_handle() { return (&spin_lock_); }
    
    /*
     * 0 core0, 1 core1, XMP_NO_OWNER not owned
     * Not present in C++11 mutex interface.
     */
    unsigned int owner()
    {
        return XMP_SIMPLE_SPINLOCK_OWNER(&spin_lock_);
    }

    void* operator new(size_t size, void* buf)
    {
        assert(buf != NULL);
        return buf;
    }
private:
    sputex(const sputex&);
    const sputex& operator=(const sputex&);
    xmp_atomic_int_t spin_lock_;
};
C_ASSERT(sizeof(sputex) == sizeof(uint32_t));
#endif
/*
 * Name and interface after C++11 std::lock_guard 
 */
class lock_guard
{
public:
    lock_guard(sputex& obj) : s_(obj)
    {
        sputex_lock(&s_);
    }
    
    ~lock_guard()
    {
        sputex_unlock(&s_);
    }
private:
    lock_guard(const lock_guard&);
    const lock_guard& operator=(const lock_guard&);
    sputex& s_;
};

}

#endif /*SPUTEX_H_*/
