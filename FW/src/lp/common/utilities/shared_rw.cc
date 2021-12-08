// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#include "adsp_std_defs.h"

#include "shared_rw.h"
#include "core/core_context.h"
#include "debug_telemetry.h"

using namespace dsp_fw;

namespace MpUtils
{

ErrorCode SharedRW::Init(void* obj, size_t obj_size)
{
#if SUPPORTED(IMR)
    HALT_ON_FAIL_AND_REPORT_ERROR(!IS_IMR_ADDRESS(obj), ADSP_SPUTEX_ALLOCATED_IN_IMR);
#endif
    sputex_init(&rw_sputex_);

    obj_ = obj;
    obj_size_ = obj_size;

    assert(IS_ALIGNED(obj_, XCHAL_DCACHE_LINESIZE));
    assert(IS_ALIGNED(obj_size_, XCHAL_DCACHE_LINESIZE));

    //obj_ has been constructed flush it to memory
    arch_cpu_dcache_region_writeback(this, sizeof(SharedRW));
    arch_cpu_dcache_region_writeback(obj_, obj_size_);

    return ADSP_SUCCESS;
}

void SharedRW::acquire()
{
    void* stack_ptr = 0; READ_CPU_REG("a1", stack_ptr);
    while (!sputext_try_lock(&rw_sputex_))
    {
        // when current code is executed on interrupt OR in critical section and
        // sputex is acquired by current core it means that core will never escape from
        // this critical section - deadlock has been detected
        if (_xtos_get_intlevel() != 0 && sputex_owner(&rw_sputex_) == get_prid())
        {
            // dump important variables to make debug easier
            uint32_t a0 = 0; READ_CPU_REG("a0", a0);

            TelemetryWndData* telemetry_data = MasterCoreServices::Get()->GetMemoryServices()->GetTelemetryData();
            const size_t prid = get_prid();
            telemetry_data->deadlock_info[prid].register_a0 = a0;
            telemetry_data->deadlock_info[prid].register_a1 = (uint32_t)stack_ptr;
            telemetry_data->deadlock_info[prid].cached_stack_ptr = (uint32_t)register_a1_;
            HALT_ON_FAIL_AND_REPORT_ERROR(false, ADSP_DEADLOCK_DETECTED);
        }
    }
    register_a1_ = stack_ptr;
    SHARED_OUT("core " << xmp_prid() << ": acquired  w\n");
    arch_cpu_dcache_region_invalidate(obj_, obj_size_);
}

void SharedRW::lightacquire()
{
    void* stack_ptr = 0; READ_CPU_REG("a1", stack_ptr);
    while (!sputext_try_lock(&rw_sputex_))
    {
        // when current code is executed on interrupt OR in critical section and
        // sputex is acquired by current core it means that core will never escape from
        // this critical section - deadlock has been detected
        if (_xtos_get_intlevel() != 0 && sputex_owner(&rw_sputex_) == get_prid())
        {
            // dump important variables to make debug easier
            uint32_t a0 = 0; READ_CPU_REG("a0", a0);

            TelemetryWndData* telemetry_data = MasterCoreServices::Get()->GetMemoryServices()->GetTelemetryData();
            const size_t prid = get_prid();
            telemetry_data->deadlock_info[prid].register_a0 = a0;
            telemetry_data->deadlock_info[prid].register_a1 = (uint32_t)stack_ptr;
            telemetry_data->deadlock_info[prid].cached_stack_ptr = (uint32_t)register_a1_;
            HALT_ON_FAIL_AND_REPORT_ERROR(false, ADSP_DEADLOCK_DETECTED);
        }
    }
    arch_cpu_dcache_region_invalidate(obj_, obj_size_);
}

void SharedRW::release()
{
    if (sputex_owner(&rw_sputex_) == xmp_prid())
    {
        // it is saver to also invalidate obj_ to make sure that dirty bit in L1$ controller is also cleared.
        // It might be also better to free cache way for data prefetching.
        arch_cpu_dcache_region_writeback_inv(obj_, obj_size_);
        SHARED_OUT("core " << xmp_prid() << ": releases\n");
        register_a1_ = NULL;
        sputex_unlock(&rw_sputex_);
    }
}

void SharedRW::lightrelease()
{
    if (sputex_owner(&rw_sputex_) == xmp_prid())
    {
        //arch_cpu_dcache_region_writeback(obj_, obj_size_);
        // it is saver to also invalidate obj_ to make sure that dirty bit in L1$ controller is also cleared.
        // It might be also better to free cache way for data prefetching.
        sputex_unlock(&rw_sputex_);
    }
}

void SharedRW::invalidate()
{
    arch_cpu_dcache_region_invalidate(obj_, obj_size_);
}

void SharedRW::write_back()
{
    arch_cpu_dcache_region_writeback(obj_, obj_size_);
}

}
