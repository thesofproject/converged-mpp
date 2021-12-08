// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef MPUTILS_SHARED_H_
#define MPUTILS_SHARED_H_

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

struct access_read_t{};
struct access_write_t{};
struct access_readwrite_t{};

template<class T>
class SimpleShared
{
public:
    /*!
      \brief getters to retrieve shared object
    */
    T& get() { return obj_; }
    const T& get() const { return obj_; }

    /*!
      \brief getters to retrieve shared object casted to aliased memory address space
    */
    T& get_aliased() { return (T&)(*SRAM_TO_SRAM_ALIAS(&obj_)); }
    const T& get_aliased() const { return (T&)(*SRAM_TO_SRAM_ALIAS(&obj_)); }

    void set(const T& other) { obj_ = other; }
    void invalidate() { arch_cpu_dcache_region_invalidate(&obj_, sizeof(obj_)); }
    void write_back() { arch_cpu_dcache_region_writeback_inv(&obj_, sizeof(obj_));}
private:
    DCACHE_ALIGN T obj_;
    DCACHE_ALIGN uint8_t pad_[0];
};

/*!
  \brief Shared class is simple decorator that shall be used for objects shared between
        cores.
  \note Shared must be aligned to XCHAL_DCACHE_LINESIZE
  \note shared obj must be aligned to XCHAL_DCACHE_LINESIZE
  \note Shared must not be invalidated by SW.
  \note If Shared is member of shared obj, must be last member. Then create Shared with
        obj_size = sizeof(SharedObj) - sizeof(Shared).
*/
class Shared
{
public:
    /*!
      \brief Constructs Shared obj.
     
      \param obj       ptr to shared object
      \param obj_size  size of shared object.
    */
    Shared(void* obj, size_t obj_size) : obj_(obj), obj_size_(obj_size)
    {
        assert(IS_ALIGNED(this, XCHAL_DCACHE_LINESIZE));
        assert(IS_ALIGNED(obj_, XCHAL_DCACHE_LINESIZE));
        //obj_ has been constructed flush it to memory
        arch_cpu_dcache_region_writeback(obj_, obj_size_);
        //no active readers
        ctrl_block_.reader_cnt = 0;
        //let other cores know that they need to invalidate obj_
        ctrl_block_.invalidate_flags = kAllCoresMask & ~(1 << xmp_prid());
        //intial state is clean (flags handled above)
        ctrl_block_.possibly_dirty = false;
        //control block has been initialized flush it
        XMP_WRITE_BACK_ELEMENT(&ctrl_block_);
    }

    ~Shared() {}

    void acquire(const access_read_t& type)
    {
        lock_guard lock(ctrl_block_.write_sputex);
        XMP_INVALIDATE_ELEMENT(&ctrl_block_);
        ++ctrl_block_.reader_cnt;
        if(ctrl_block_.invalidate_flags & (1 << xmp_prid()))
        {
            arch_cpu_dcache_region_invalidate(obj_, obj_size_);
            ctrl_block_.invalidate_flags &= ~(1 << xmp_prid());
            SHARED_OUT("core " << xmp_prid() << ": acquired r , cnt:");
            SHARED_OUT(ctrl_block_.reader_cnt << ", invalidated\n");
        }
        else
        {
            SHARED_OUT("core " << xmp_prid() << ": acquired r , cnt:");
            SHARED_OUT(ctrl_block_.reader_cnt << "\n");
        }
        XMP_WRITE_BACK_ELEMENT(&ctrl_block_);
    }

    void acquire(const access_write_t& type)
    {
        acquire(type, false);
    }

    void acquire(const access_readwrite_t& type)
    {
        acquire(type, false);
    }

    /*!
      \brief there can be only one write access at a time and
      it's exclusive (no read either), so release can be called without
      arguments -> if write access is active release write, otherwise
      release read.
    */
    void release()
    {
        if(sputex_owner(&ctrl_block_.write_sputex) == get_prid())
        {
            //if acquired for writing we have most recent copy of ctrl_block
            if(ctrl_block_.possibly_dirty)
            {
                arch_cpu_dcache_region_writeback(obj_, obj_size_);
                ctrl_block_.invalidate_flags = kAllCoresMask & ~(1 << xmp_prid());
                SHARED_OUT("core " << xmp_prid() << ": releases ?w, dirty\n");
            }
            else
            {
                SHARED_OUT("core " << xmp_prid() << ": releases ?w, clean\n");
            }
            XMP_WRITE_BACK_ELEMENT(&ctrl_block_);
            sputex_unlock(&ctrl_block_.write_sputex);
        }
        else
        {
            lock_guard lock(ctrl_block_.write_sputex);
            XMP_INVALIDATE_ELEMENT(&ctrl_block_);
            SHARED_OUT("core " << xmp_prid() << ": releases r , cnt:");
            SHARED_OUT(ctrl_block_.reader_cnt << "\n");
            --ctrl_block_.reader_cnt;
            XMP_WRITE_BACK_ELEMENT(&ctrl_block_);
        }
    }

    static const int kAllCoresMask = 0x3;

    void invalidate()
    {
        XMP_INVALIDATE_ELEMENT(&ctrl_block_);
        arch_cpu_dcache_region_invalidate(obj_, obj_size_);
    }

    void write_back()
    {
        XMP_WRITE_BACK_ELEMENT(&ctrl_block_);
        arch_cpu_dcache_region_writeback(obj_, obj_size_);
    }
private:
    Shared(const Shared&);
    const Shared& operator=(const Shared&);

    void acquire(const access_write_t& type, bool defer_dirty)
    {
        sputex_lock(&ctrl_block_.write_sputex);
        XMP_INVALIDATE_ELEMENT(&ctrl_block_);
        while(ctrl_block_.reader_cnt > 0)
        {
            sputex_unlock(&ctrl_block_.write_sputex);
            xmp_spin();
            sputex_lock(&ctrl_block_.write_sputex);
        }

        ctrl_block_.possibly_dirty = (defer_dirty)?false:true;

        XMP_WRITE_BACK_ELEMENT(&ctrl_block_);
        SHARED_OUT("core " << xmp_prid() << ": acquired  w\n");
    }

    void acquire(const access_readwrite_t& type, bool defer_dirty)
    {
        const access_write_t access_write = access_write_t();
        acquire(access_write, defer_dirty);
        //we hold the lock and have invalidated/uptodate control block here
        if(ctrl_block_.invalidate_flags & (1 << xmp_prid()))
        {
            SHARED_OUT("core " << xmp_prid() << ": promoted rw, invalidated\n");
            arch_cpu_dcache_region_invalidate(obj_, obj_size_);
            ctrl_block_.invalidate_flags &= ~(1 << xmp_prid());
            XMP_WRITE_BACK_ELEMENT(&ctrl_block_);
        }
        else
        {
            SHARED_OUT("core " << xmp_prid() << ": promoted rw\n");
        }
    }

    struct SharedControlBlock
    {
        sputex write_sputex;
        int reader_cnt;
        int invalidate_flags;
        bool possibly_dirty;
    };

    static_assert(sizeof(SharedControlBlock) <= XCHAL_DCACHE_LINESIZE,
                  "ControlBlock bigger than cache line size!");

    SharedControlBlock ctrl_block_ DCACHE_ALIGN;
    // ptr to shared object
    void* const obj_;
    // size of shared object
    const size_t obj_size_;
};

}

#endif /*SHARED_H_*/
