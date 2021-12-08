// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#include "adsp_std_defs.h"
#include "registerable_callback.h"

namespace dsp_fw
{

C_ASSERT(REGISTERABLE_CALLBACK_MAX_SUPPORTED_ARGS == 2);

ErrorCode RegisterableCallback::Init(AdspRegisterableCallbackData* data,
                                     const void* function_ptr,
                                     size_t argc,
                                     void** argv,
                                     bool rets_val,
                                     uint32_t expected_ret_val,
                                     uint32_t* ret_val_store,
                                     const void* recovery_function_ptr)
{
    RETURN_EC_ON_FAIL(argc <= REGISTERABLE_CALLBACK_MAX_SUPPORTED_ARGS, ADSP_REGISTERABLE_CALLBACK_TOO_MANY_ARGS);
    RETURN_EC_ON_FAIL(!(argc != 0 && argv == NULL), ADSP_REGISTERABLE_CALLBACK_NULL_ARV);
    RETURN_EC_ON_FAIL(!(argc == 0 && argv != NULL), ADSP_REGISTERABLE_CALLBACK_INCOSISTENT_PARAMS);

    data->callback =  (AdspCallback)function_ptr;
    data->argc = argc;
    for (size_t idx = 0; idx < argc; ++idx)
    {
        data->argv[idx] = argv[idx];
    }
    data->ret_val = rets_val;
    data->expected_ret_val = expected_ret_val;
    data->ret_val_store = ret_val_store;
    data->recovery = (AdspRecovery)recovery_function_ptr;
    return ADSP_SUCCESS;
}
void RegisterableCallback::Execute(AdspRegisterableCallbackData* data)
{
    uint32_t ret_val = 0;
    Execute(data, ret_val);
    return;
}

void RegisterableCallback::Execute(AdspRegisterableCallbackData* data, uint32_t &callback_retval)
{
    uint32_t ret_val = 0;
    bool is_executed = ExecuteCallback(data, ret_val);

    if(!is_executed)
    {
        return;
    }

    callback_retval = ret_val;

    if (is_executed && data->ret_val && (ret_val != data->expected_ret_val))
    {
        ExecuteRecovery(data);
    }

    return;
}

bool RegisterableCallback::ExecuteCallback(AdspRegisterableCallbackData* data, uint32_t &ret_val)
{
    bool is_executed = false;
    if (data->callback == NULL)
    {
        assert(false);
        return is_executed;
    }

    if (data->argc == 0)
    {
        ret_val = data->callback();
        is_executed = true;
    }
    if (data->argc == 1)
    {
        ret_val = data->callback(data->argv[0]);
        is_executed = true;
    }
    if (data->argc == 2)
    {
        ret_val = data->callback(data->argv[0], data->argv[1]);
        is_executed = true;
    }

    if (is_executed && data->ret_val_store != NULL)
    {
        *data->ret_val_store = ret_val;
    }

    return is_executed;
}

void RegisterableCallback::ExecuteRecovery(AdspRegisterableCallbackData* data)
{
    if (data->recovery != NULL)
    {
        if (data->argc == 0)
        {
            data->recovery();
        }
        if (data->argc == 1)
        {
            data->recovery(data->argv[0]);
        }
        if (data->argc == 2)
        {
            data->recovery(data->argv[0], data->argv[1]);
        }
    }
    return;
}

}
