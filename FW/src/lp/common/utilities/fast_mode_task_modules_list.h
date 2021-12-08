// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef ADSP_FW_FMT_MODULES_LIST_H
#define ADSP_FW_FMT_MODULES_LIST_H

#include "adsp_error.h"
#include "error_handling.h"
#include "scheduler/dp_scheduler/fast_mode_task.h"
#include "module/module_instance.h"
#include "ixc/fast_task_config.h"
#include "management/firmware_manager_mng.h"

namespace dsp_fw
{

template<int MAX_COUNT>
class FastModeTaskModulesList
{
    static const size_t FAST_MODE_TASK_MAX_MODULES_COUNT = 16;

public:
    explicit inline FastModeTaskModulesList(ModuleInstance* mi_ptr) :
    mi_ptr_(mi_ptr) { }

    ErrorCode PrepareFmtModulesList(
        uint32_t outpin_idx, const FastModeTaskConfig* modules_to_prepare,
        ModuleInstance** last_copier_mi)
    {
        RETURN_EC_ON_FAIL(NULL != modules_to_prepare, ADSP_ERROR_NULL_POINTER_AS_PARAM);
        RETURN_EC_ON_FAIL(NULL != last_copier_mi, ADSP_ERROR_NULL_POINTER_AS_PARAM);
        RETURN_EC_ON_FAIL(outpin_idx <  MAX_COUNT, ADSP_INVALID_PARAM);
        ErrorCode ec = ADSP_SUCCESS;

        dsp_fw_mgmt::ModuleManager* module_manager =
        dsp_fw_mgmt::FirmwareManagerMng::GetMngInstanceWrapped()->GetModuleManager();
        ModuleInstance* mi = NULL;

        if (modules_to_prepare->number_of_modules != 0)
        {
            /* Add current module at begnining of the list */
            list_item(outpin_idx)->elem = mi_ptr_;
            ec = modules_list(outpin_idx)->PushBack(list_item(outpin_idx));
            RETURN_ON_ERROR(ec);
        }

        for (size_t module_desc_idx = 0; module_desc_idx < modules_to_prepare->number_of_modules;
            ++module_desc_idx)
        {
            mi = module_manager->GetModuleInstance(
                    modules_to_prepare->module_instance_ids[module_desc_idx].module_id,
                    modules_to_prepare->module_instance_ids[module_desc_idx].instance_id);

            RETURN_EC_ON_FAIL(mi != NULL, ADSP_KPB_INVALID_MODULE_INSTANCE);

            ModuleInstanceListItem* new_list_item_ptr;

            ec = AllocFmtModuleListItem(mi, &new_list_item_ptr);
            RETURN_ON_ERROR(ec);
            /* Should be catch by status check above */
            HALT_ON_FAIL_AND_REPORT_ERROR(new_list_item_ptr != NULL, ADSP_OUT_OF_RESOURCES);

            new_list_item_ptr->elem = mi;
            ec = modules_list(outpin_idx)->PushBack(new_list_item_ptr);
            RETURN_ON_ERROR(ec);
        }

        *last_copier_mi = mi;

        return ec;
    }

    /* Note: this one cannot fail. */
    inline void ClearFmtModulesList(uint32_t outpin_idx)
    {
        /* Note: this should be validated in layer above. */
        assert(outpin_idx < MAX_COUNT);
        modules_list(outpin_idx)->Reset(true);
    }

    inline ModuleInstanceList* modules_list(size_t outpin_id)
    {
        /* Checked on multiple occasion by layers above */
        assert(outpin_id < MAX_COUNT);
        return &modules_list_[outpin_id];
    }

    inline ModuleInstance* last_list_module(size_t outpin_id)
    {
        /* Checked on multiple occasion by layers above */
        assert(outpin_id < MAX_COUNT);

        ModuleInstanceListItem* item = modules_list(outpin_id)->GetTail();
        if (item != NULL)
        {
            return item->elem;
        }
        return NULL;
    }

private:
    inline ModuleInstanceListItem* list_item(size_t outpin_id)
    {
        /* Checked on multiple occasion by layers above */
        assert(outpin_id < MAX_COUNT);
        return &list_item_[outpin_id];
    }

    inline ErrorCode AllocFmtModuleListItem(ModuleInstance* mi_ptr, ModuleInstanceListItem** item)
    {
        for (size_t module_slot_idx = 0; module_slot_idx < FAST_MODE_TASK_MAX_MODULES_COUNT;
                        ++module_slot_idx)
        {
            if (modules_list_item_[module_slot_idx].elem == mi_ptr)
            {
                return ADSP_INVALID_REQUEST;
            }
        }

        for (size_t module_slot_idx = 0; module_slot_idx < FAST_MODE_TASK_MAX_MODULES_COUNT;
                        ++module_slot_idx)
        {
            if (NULL == modules_list_item_[module_slot_idx].elem)
            {
                modules_list_item_[module_slot_idx].elem = mi_ptr;
                *item = &modules_list_item_[module_slot_idx];
                return ADSP_SUCCESS;
            }
        }
        return ADSP_OUT_OF_RESOURCES;
    }

    ModuleInstanceList modules_list_[MAX_COUNT];
    ModuleInstanceListItem modules_list_item_[FAST_MODE_TASK_MAX_MODULES_COUNT];
    ModuleInstanceListItem list_item_[MAX_COUNT];
    ModuleInstance* mi_ptr_;
};

}

#endif //#define ADSP_FW_FMT_MODULES_LIST_H
