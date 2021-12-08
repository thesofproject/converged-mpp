// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#include "adsp_std_defs.h"
#include "thread_lock.h"
#include "core/core_context.h"
#include "scheduler/dp_scheduler/thread_conditional_block.h"

namespace dsp_fw
{

#define INVALID_TH_CONTEXT ((ThreadedTask*)0xffffffff)

static const ThreadedTask* get_thread()
{
    const ThreadedTask* th = NULL;
#if !defined(UT)
    CoreServices* core_services = CoreServices::Get();
    if (core_services)
    {
        th = core_services->GetCurrentThread();
    }
#endif // !defined(UT)
    return th;
}

ThreadSafe::ThreadSafe() : thread_(INVALID_TH_CONTEXT), reference_counter_(0)
{
}

ThreadSafe::Lock::Lock(ThreadSafe* th)
{
    bool in_loop = true;

    do
    {
        ENTER_CRITICAL_SECTION(0);
        ths_ = th;
        const ThreadedTask* cur_th = get_thread();
        if (ths_->reference_counter_)
        {
            if (cur_th == ths_->thread_)
            {
                ths_->reference_counter_ += 1;
                in_loop = false;
            }
            else
            {
                // locked by another thread.
                in_loop = true;
            }
        }
        else
        {
            assert(ths_->thread_ == INVALID_TH_CONTEXT);
            ths_->thread_ = cur_th;
            ths_->reference_counter_ += 1;
            in_loop = false;
        }
        LEAVE_CRITICAL_SECTION(0);
        if (in_loop)
        {
            const size_t cached_int_level = _xtos_get_intlevel();
            HALT_ON_FAIL(cached_int_level == 0);

            BlockCurrentThreadedTask blockade(
                (volatile uint32_t*)&(ths_->thread_),
                (uint32_t)INVALID_TH_CONTEXT);
        }
    } while (in_loop);
}

ThreadSafe::Lock::~Lock()
{
    ENTER_CRITICAL_SECTION(0);
    const ThreadedTask* cur_th = get_thread();
    HALT_ON_FAIL(ths_->reference_counter_ != 0);
    HALT_ON_FAIL(cur_th == ths_->thread_);
    ths_->reference_counter_ -= 1;
    if (ths_->reference_counter_ == 0)
    {
        ths_->thread_ = INVALID_TH_CONTEXT;
    }
    LEAVE_CRITICAL_SECTION(0);
}
}
