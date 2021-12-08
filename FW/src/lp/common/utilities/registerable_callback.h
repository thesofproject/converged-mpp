// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef ADSP_FW_UTILITIES_REGISTERABLE_CALLBACK_H
#define ADSP_FW_UTILITIES_REGISTERABLE_CALLBACK_H

#include "intel_adsp/system_service_internal.h"

namespace dsp_fw
{

class RegisterableCallback
{
public:
    /*!
      \brief Initializes callback.

      Examples:
      \code
          Pipeline* ppl = NULL;
          (PUT GET PIPELINE CODE HERE);

          void* (Item::* f)() = &Pipeline::SetState;
          int fptr = (int&)f;
          void* argv[2];
          argv[0] = (void*)ppl;
          argv[1] = (void*)PPL_PAUSED;
          registerable_callback.Init(fptr, 2, args, true, ADSP_SUCCESS);
          (SWITCH TO OTHER THREAD);
          registered_callback->Execute()
      \endcode

      \param function_ptr ptr to function of any type casted to void*
      \param argc number of arguments that function is taking
      \param argv ptr to array of arguments casted to void*. inout args are not supported!
      \param rets_val determines whether function is returning value and needs to be compared
      \param expected_ret_val expected returning value (optional when rets_val is set)
      \param ret_val_store ptr where ret value is going to be stored (optional, when NULL wont work)
      \param recovery_function_ptr ptr to recovery function that is going to be called when
             returned value is not equal to expected. When NULL passed, no recovery is registered

      \return ADSP_SUCCESS
      \return ADSP_REGISTERABLE_CALLBACK_TOO_MANY_ARGS
      \return ADSP_REGISTERABLE_CALLBACK_NULL_ARV
      \return ADSP_REGISTERABLE_CALLBACK_INCOSISTENT_PARAMS
    */
    static ErrorCode Init(AdspRegisterableCallbackData* data,
                          const void* function_ptr, size_t argc, void** argv,
                          bool rets_val, uint32_t expected_ret_val = 0, uint32_t* ret_val_store = NULL,
                          const void* recovery_function_ptr = NULL);

    /*!
      \brief Executes callback
    */
    static void Execute(AdspRegisterableCallbackData* data);

    /*!
      \brief Executes callback and return retval by first parameter
    */
    static void Execute(AdspRegisterableCallbackData* data, uint32_t &callback_retval);
private:
    explicit RegisterableCallback()
    {
    }

    /*!
      \brief Executes callback
    */
    static bool ExecuteCallback(AdspRegisterableCallbackData* data, uint32_t &ret_val);

    /*!
      \brief Executes recovery
    */
    static void ExecuteRecovery(AdspRegisterableCallbackData* data);
};

} // dsp_fw

#endif /* ADSP_FW_UTILITIES_REGISTERABLE_CALLBACK_H */
