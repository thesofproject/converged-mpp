// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef ADSP_FW_IMR_EXEC_GATE_H
#define ADSP_FW_IMR_EXEC_GATE_H

#include "adsp_std_defs.h"

namespace dsp_fw
{

#if SUPPORTED(IMR)
extern bool ImrExecGate();
extern void PostImrExec(bool _allowed);
#endif

/*!
  \note Here is great way from Bjarne Stroustrup, how to solve problem of wrapping member function in old C++98.
  http://www.stroustrup.com/wrapper.pdf
  This class provides transparent calling functions that are stored in IMR.
*/

#if SUPPORTED(IMR)

template<class T> class Call_proxy;

template<class T> class Wrap
 {
public:
    T* p; //holder for object
    Wrap(T* pp) :p(pp) { }
    Call_proxy<T> operator->()
    {
        if (ImrExecGate())
        {
            return Call_proxy<T>(p, true);
        }
        return Call_proxy<T>(p, false);
    }
#if 0
    T* direct() { return p; }

    void Set(T* pp)
    {
        p = pp;
    }
#endif
    void* operator new(size_t buffer_size, void* buffer) throw()
    {
        return buffer;
    }
};

template<class T> class Call_proxy
{
    T* p;
    bool allowed_;
public:
    Call_proxy(T* pp, bool _allowed) :p(pp), allowed_(_allowed){ }
    ~Call_proxy()
    {
        PostImrExec(allowed_);
    }
    T* operator->() { if(allowed_) return p; return NULL; }
};
#else

template<class T>
class Wrap
{
    T* p; //holder for object
public:
    Wrap(T* pp): p(pp) { }
    T* operator->() { return p; }

#if 0
    T* direct() { return p; }

    void Set(T* pp)
    {
        p = pp;
    }
#endif

    void* operator new(size_t buffer_size, void* buffer) throw()
    {
        return buffer;
    }
};

#endif

} //namespace dsp_fw



#endif // ADSP_FW_IMR_EXEC_GATE_H
