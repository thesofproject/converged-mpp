// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef ADSP_FW_BUFFER_PROVIDER_H
#define ADSP_FW_BUFFER_PROVIDER_H

#include "adsp_std_defs.h"
#define IMR_MAX_BUFFERS 50

namespace dsp_fw
{

class BufferProvider
{
private:
    struct BufferInfo
    {
        uint8_t* pointer;
        unsigned int size;
    };

    uint8_t*         m_buffer;
    unsigned int  m_total_size;
    BufferInfo    m_hired[IMR_MAX_BUFFERS];
public:

    BufferProvider(uint8_t* _buffer, unsigned int _size)
    {
        m_buffer = _buffer;
        m_total_size = _size;

        for (int i = 0; i < IMR_MAX_BUFFERS; i++)
        {
            m_hired[i].pointer = NULL;
            m_hired[i].size = 0;
        }
    }

    uint8_t* GetBuffer(int size)
    {
        //ENTER_CRITICAL_SECTION(0);

        uint8_t* pointer = m_buffer;
        int found_index = -1;
        for (int i = 0; i < IMR_MAX_BUFFERS; i++)
        {
            if (m_hired[i].pointer != NULL)
            {
                if (found_index >= 0)
                {
                    if (m_hired[i].pointer - pointer >= size)
                    {
                        m_hired[found_index].pointer = pointer;
                        m_hired[found_index].size = size;
                        //LEAVE_CRITICAL_SECTION(0);
                        return pointer;
                    }
                    else
                    {
                        found_index = -1;
                        pointer = m_hired[i].pointer + m_hired[i].size;
                    }
                }
                else
                {
                    pointer = m_hired[i].pointer + m_hired[i].size;
                }
            }
            else
            {
                if (found_index == -1)
                    found_index = i;
            }
        }

        if (found_index >= 0 && (m_buffer + m_total_size - pointer) >= size)
        {
            m_hired[found_index].pointer = pointer;
            m_hired[found_index].size = size;
            //LEAVE_CRITICAL_SECTION(0);
            return pointer;
        }

        //LEAVE_CRITICAL_SECTION(0);
        return NULL;
    }

    void ReleaseBuffer(uint8_t* buffer)
    {
        //ENTER_CRITICAL_SECTION(0);
        int index = 0;
        while (m_hired[index].pointer != buffer && index < IMR_MAX_BUFFERS)
        {
            index++;
        }
        if (IMR_MAX_BUFFERS <= index)
        {
            //LEAVE_CRITICAL_SECTION(0);
            return;
        }
        m_hired[index].pointer = NULL;
        m_hired[index].size = 0;
        //LEAVE_CRITICAL_SECTION(0);
    }
};

}

#endif /* ADSP_FW_BUFFER_PROVIDER_H */
