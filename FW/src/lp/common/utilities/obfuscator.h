// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef _OBFUSCATOR_H
#define _OBFUSCATOR_H

namespace dsp_fw
{
/*!
  \brief This class brings out to the compiler that a DERIVED_OBFUSCATED type is actually obfuscated by a OBFUSCATING type.
  This class shall be inherited by the obfuscated type and will
  provide the conversion functions towards the obfuscating type.
  Once part of parents of the obfuscated class, the "static_cast<OBFUSCATING*>" will work on DERIVED_OBFUCATED instances
  and should be preferred to the unsafe "reinterpret_cast<OBFUSCATING&>" counterpart.
  \remarks This class has to be template to take leverage of the CRTP.
*/
template <class DERIVED_OBFUSCATED, class OBFUSCATING>
struct Obfuscator
{
    /*!
      \brief Safely convert implicitly a DERIVED_OBFUSCATED instance reference into a OBFUSCATING instance pointer.
    */
    operator OBFUSCATING*()
    {
        // static_cast works here due to the CRTP.
        // Compiler is thus able to retrieve actual DERIVED_OBFUSCATED instance by itself.
        return reinterpret_cast<OBFUSCATING*>(static_cast<DERIVED_OBFUSCATED*>(this));
    }

    /*!
      \brief Safely convert implicitly a DERIVED_OBFUSCATED instance reference into a OBFUSCATING instance pointer.
    */
    operator const OBFUSCATING*() const
    {
        // static_cast works here due to the CRTP.
        // Compiler is thus able to retrieve actual DERIVED_OBFUSCATED instance by itself.
        return reinterpret_cast<const OBFUSCATING*>(static_cast<const DERIVED_OBFUSCATED*>(this));
    }
};

} //namespace dsp_fw


#endif //_OBFUSCATOR_H
