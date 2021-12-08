// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#include "imr_exec_gate.h"
#include "debug_utilities/logger.h"
#include "core/core_context.h"
#include "power_manager/power_manager_service.h"
#include "scheduler/dp_scheduler/edf_scheduler.h"
#include "scheduler/dp_scheduler/threaded_task.h"
#include "debug_telemetry.h"

#if SUPPORTED(IMR)

namespace dsp_fw
{

bool ImrExecGate()
{
    if (CoreServices::Get() != NULL)
    {
#if !UT && !SIMULATION
        ThreadedTask* const current_task = CoreServices::Get()->GetCurrentThread();

        if (current_task != NULL && !current_task->IsHighLatency())
        {
            FLOG(L_HIGH, "Call from prohibited task! core_id = %d. Pointer to current thread = 0x%8.8X",
                    get_prid(), (uint32_t)current_task);
            return false;
        }

        if (!CoreServices::Get()->GetPowerManagerService()->IsIMRRequested())
        {
            FLOG(L_HIGH, "Call from IMR requested! core_id = %d. Pointer to current thread = 0x%8.8X",
                        get_prid(), (uint32_t)current_task);
            TelemetryWndData* const telemetry_data = MemoryServices::GetTelemetryData();
            if (IS_BIT_SET(telemetry_data->assert_info.mode, 1))
            {
                FORCE_CRASH_DUMP();
            }
        }

#endif /* !UT && !SIMULATION */
    }
    return true;
}

void PostImrExec(bool _allowed)
{
}

} //namespace dsp_fw

#endif // SUPPORTED(IMR)
