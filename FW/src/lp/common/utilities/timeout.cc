// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#include "adsp_std_defs.h"
#include "timeout.h"
#include "core/core_context.h"
#include "core/lp_timer.h"

bool wait_with_timeout(volatile uint32_t* const blocking_param_ptr,
                       const uint32_t unblocking_value,
                       const uint32_t unblocking_mask,
                       const int timeout)
{
    const size_t start_timestamp = xthal_get_ccount();
    const size_t end_timestamp = start_timestamp + timeout;  // overflow allowed
    bool timed_out = false;
    do
    {
        if (((*blocking_param_ptr) & unblocking_mask) == (unblocking_value & unblocking_mask))
        {
            // condition is matched
            return false;
        }
        if (timeout > 0)
        {
            const size_t timestamp = xthal_get_ccount();
            if (end_timestamp < start_timestamp)
            {
                // end expected after overflow
                timed_out = (timestamp >= end_timestamp) && (timestamp < start_timestamp);
            }
            else
            {
                timed_out = timestamp >= end_timestamp;
            }
        }
    } while (!timed_out);
    return timed_out;
}

namespace dsp_fw
{
PollingTimer::PollingTimer(PollingTimer::Units units, size_t expiration_time)
{
    uint64_t add_on = expiration_time;
    if (units == PollingTimer::US)
    {
        add_on = add_on * GET_VALUE(XTAL_FREQUENCY) / 1000000;
    }
    else if (units == PollingTimer::MS)
    {
        add_on = add_on * GET_VALUE(XTAL_FREQUENCY) / 1000;
    }
#if !defined(UT) && FW_BUILD
    lp_timer_s* timer = CoreServices::Get()->GetLowPowerTimer();
    uint64_t current_timestamp = lp_timer_get_wall_clk_value(timer);
    expiration_time_ = current_timestamp + add_on;
#else
    expiration_time_ = add_on;
#endif
}

bool PollingTimer::expired() const
{
#if !defined(UT) && FW_BUILD
    return lp_timer_get_wall_clk_value(CoreServices::Get()->GetLowPowerTimer()) > expiration_time_;
#else
    return false;
#endif
}

}
