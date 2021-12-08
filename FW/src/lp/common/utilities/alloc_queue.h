// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef ADSP_FW_UTILITIES_ALLOC_QUEUE_H
#define ADSP_FW_UTILITIES_ALLOC_QUEUE_H

#include "module/module_instance.h"
#include "scheduler/queues/audio_queue.h"
#include "scheduler/queues/dp_queue.h"
#include "scheduler/queues/ref_queue.h"
#include "simple_mem_alloc.h"
#include <core/kernel/memory/memory_pool.h>
#include "dispatchers/module_instance_dispatcher.h"

namespace dsp_fw
{
#define FIRST_REFERENCE_PIN_IDX 1
/*!
  \brief Allocates the queue from the assigned mem pool.

  \param src_mod   Downstream module in the processing flow.
  \param out_pin   Output pin index of the downstream module to be bound to the queue.
  \param dst_mod   Upstream module in the processing flow.
  \param in_pin    Input pin index of the upstream module to be bound to the queue.

  \return ptr to allocated queue (NULL if there is no resource available)
*/
template<class MEM_POOL>
static AudioQueue* AllocQueue(MEM_POOL pool, const ModuleInstance* src_mod, size_t out_pin, const ModuleInstance* dst_mod, size_t in_pin)
{
    const bool is_queue_shared = src_mod->GetCoreId() != MASTER_CORE_ID || dst_mod->GetCoreId() != MASTER_CORE_ID;
    const size_t instance_alignment = XCHAL_DCACHE_LINESIZE;

    bool is_ext_queue_req = false;
    AudioQueue* queue = NULL;
    size_t queue_size = sizeof(DPQueue);

    const size_t dst_ibs = ModuleInstanceDispatcher::GetIbs(dst_mod, in_pin);
    const size_t src_obs = ModuleInstanceDispatcher::GetObs(src_mod, out_pin);

    if(in_pin < FIRST_REFERENCE_PIN_IDX)
    {
        is_ext_queue_req = (dst_mod->GetModuleDpQueueType() == ModuleInstance::ADVANCED_QUEUE) ||
                           (src_mod->GetModuleDpQueueType() == ModuleInstance::ADVANCED_QUEUE);

        queue = new(pool, instance_alignment) DPQueue(dst_ibs,
                                                      src_obs,
                                                      is_ext_queue_req,
                                                      is_queue_shared,
                                                      is_queue_shared);
    }
    else
    {
        // Note: all reference queues have to be extended type to allow runtime binding
        is_ext_queue_req = true;
        queue = new(pool, instance_alignment) RefQueue(dst_ibs,
                                                       src_obs,
                                                       is_ext_queue_req,
                                                       is_queue_shared,
                                                       is_queue_shared);
        queue_size = sizeof(RefQueue);
    }

    if (NULL == queue) return NULL;

    size_t q_buf_size = queue->GetTotalBufferSize();
    ByteArray buf(new(pool, instance_alignment) uint8_t[q_buf_size], q_buf_size);

    // In case of buffer allocation error, queue object is not deleted.
    // mem pool assigned to ppl does not support deleting single objects
    // The whole ppl is deleted by the mgr in case of any error like this one,
    // so there is no mem leak
    if (buf.data() == NULL)
    {
        REPORT_ERROR_NO_LOG(ADSP_ALLOC_QUEUE_FATAL_ERROR_NO_MEMORY);
        return NULL;
    }
    if (queue->Init(buf) != ADSP_SUCCESS)
    {
        REPORT_ERROR_NO_LOG(ADSP_ALLOC_QUEUE_FATAL_ERROR_NOT_INITIALIZED);
        return NULL;
    }

    if (is_queue_shared)
    {
        queue_size = ROUND_UP(queue_size, XCHAL_DCACHE_LINESIZE);
        // writeback queue. Needed to use arch_cpu_dcache_region_writeback since is allocated using
        arch_cpu_dcache_region_writeback(queue, queue_size);
        // convert queue addr from L2 to alised L2
        // it will simplify queue sync mechanism
        queue = SRAM_TO_SRAM_ALIAS(queue);
    }

    return queue;
}

} // namespace dsp_fw

#endif /* ADSP_FW_UTILITIES_ALLOC_QUEUE_H */
