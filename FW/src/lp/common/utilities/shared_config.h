// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef _ADSP_FW_SHARED_CONFIG_H
#define _ADSP_FW_SHARED_CONFIG_H

#include "platform/memory_defs.h"
#include "shared_rw.h"

namespace dsp_fw
{
// template used for handling run-time configuration parameters
template<size_t N> class SharedConfig
{
public:
    SharedConfig():
        new_config_(false), param_id_(0)
    {
        shared_desc_.Init(this, sizeof(SharedConfig<N>) - sizeof(MpUtils::SharedRW));
    }

    void SetNew(uint32_t param_id, uint32_t data)
    {
        shared_desc_.acquire();
        new_config_ = true;
        param_id_ = param_id;
        data_[0] = data;
        shared_desc_.release();
    }

    void SetNew(uint32_t param_id, uint32_t* data_ptr, size_t data_size)
    {
        shared_desc_.acquire();
        new_config_ = true;
        param_id_ = param_id;
        memcpy_s(data_, data_size, data_ptr, data_size);
        shared_desc_.release();
    }

    void Release()
    {
        shared_desc_.acquire();
        new_config_ = false;
        shared_desc_.release();
    }

    bool IsNewCfgAvailable()
    {
        shared_desc_.invalidate();
        return new_config_;
    }

    uint32_t GetParamId()
    {
        shared_desc_.invalidate();
        return param_id_;
    }

    uint32_t* GetData()
    {
        shared_desc_.invalidate();
        return data_;
    }

    size_t GetDataSize()
    {
        shared_desc_.invalidate();
        return sizeof(data_);
    }

private:
    // flag indicating whether new config has been set
    bool new_config_;
    // module specific parameter id
    uint32_t param_id_;
    // necessary data for setting property
    uint32_t data_[N];

    DCACHE_ALIGN MpUtils::SharedRW shared_desc_;
};

}

#endif //#ifndef _ADSP_FW_SHARED_CONFIG_H
