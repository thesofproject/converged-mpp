// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef MPUTILS_SHARED_RW_H_
#define MPUTILS_SHARED_RW_H_

#include "cpp_backward_compatibility.h"
#include "sputex.h"
#include "platform/memory_defs.h"

#ifdef TRACK_SHARED
#define SHARED_OUT(x) myout << x
#else
#define SHARED_OUT(x)
#endif

namespace MpUtils
{

/*!
  \brief SharedRW class is simple decorator that shall be used for objects shared between
         cores and use only rw operations.
  \note Shared must be aligned to XCHAL_DCACHE_LINESIZE
  \note shared obj must be aligned to XCHAL_DCACHE_LINESIZE
  \note Shared must not be invalidated by SW.
  \note If Shared is member of shared obj, must be last member. Then create Shared with
        obj_size = sizeof(SharedObj) - sizeof(Shared).
*/
class SharedRW
{
public:
    /*!
      \brief Constructs SharedRW obj.
    */
     SharedRW()
     : obj_(NULL), obj_size_(0), register_a1_(NULL) {
         sputex_init(&rw_sputex_);
     }

    ~SharedRW(){}

    /*!
      \param obj               ptr to shared object
      \param obj_size          size of shared object.
    */
    ErrorCode Init(void* obj, size_t obj_size);

    /*!
      \brief Acquires sputex. If already acquired by the same core in critical section
      then deadlock will be reported (by fw dump)
    */
    void acquire();

    void lightacquire();

    /*!
      \brief there can be only one read-write access at a time and
      it's exclusive (no read or write), so release can be called without
      arguments
    */
    void release();
    void lightrelease();

    void invalidate();
    void write_back();

private:
    SharedRW(const SharedRW&);
    const SharedRW& operator=(const SharedRW&);

    // ptr to shared object
    void* obj_;
    // size of shared object
    size_t obj_size_;

    sputex rw_sputex_;

    // dumped register a1 (stack) for debug purposes
    void* register_a1_;
    DCACHE_ALIGN int al[0];
};

C_ASSERT(sizeof(SharedRW) == XCHAL_DCACHE_LINESIZE);

}

namespace dsp_fw
{
class LockWithCS
{
public:
    LockWithCS(MpUtils::SharedRW& shared_desc) : shared_desc_(shared_desc)
    {
        // Enter Critical section
        old_int_level_ =  _xtos_set_intlevel(GET_VALUE(CS_INT_LEVEL));
        // acquire lock
        shared_desc_.acquire();
    }

    ~LockWithCS()
    {
        // release lock
        shared_desc_.release();
        // Leave Critical Section
        _xtos_set_intlevel(old_int_level_);
    }

private:
    MpUtils::SharedRW& shared_desc_;
    uint32_t old_int_level_;
};
class LockWithoutCS
{
public:
    LockWithoutCS(MpUtils::SharedRW& shared_desc) : shared_desc_(shared_desc)
    {
        // acquire lock
        shared_desc_.acquire();
    }

    ~LockWithoutCS()
    {
        // release lock
        shared_desc_.release();
    }
private:
    MpUtils::SharedRW& shared_desc_;
};

class LightLockWithCS
{
public:
    LightLockWithCS(MpUtils::SharedRW& shared_desc) : shared_desc_(shared_desc)
    {
        shared_desc_.lightacquire();
    }
    ~LightLockWithCS()
    {
        shared_desc_.lightrelease();
    }
private:
    MpUtils::SharedRW& shared_desc_;
};

class LightLockWithoutCS: public LightLockWithCS
{
public:
    LightLockWithoutCS(MpUtils::SharedRW& shared_desc): LightLockWithCS(shared_desc){}
};

}

#endif /*MPUTILS_SHARED_RW_H_*/
