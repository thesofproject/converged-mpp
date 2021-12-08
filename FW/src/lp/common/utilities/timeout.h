// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef FW_SRC_BASE_FW_UTILITIES_TIMEOUT_H_
#define FW_SRC_BASE_FW_UTILITIES_TIMEOUT_H_
/*!
  \brief Wait for a certain value to appear in given memory location.
  The wait will timeout after the specified amount of CC if the condition is not met.
  \param  blocking_param_ptr pointer to the memory location to check
  \param  unblocking_value   expected value
  \param  unblocking_mask    mask for checking only some bits
  \param  timeout            value of timeout in CC. inifite if <= 0
  \return                    flag meaning if timeout occured
*/
bool wait_with_timeout(volatile uint32_t* const blocking_param_ptr,
                       const uint32_t unblocking_value,
                       const uint32_t unblocking_mask = 0xFFFFFFFF,
                       const int timeout = -1);

namespace dsp_fw
{

/*!
  \brief PollingTimer implements timer which expires after certain amount of time.
*/
class PollingTimer
{
public:
    enum Units
    {
        US = 0,
        MS = 1,
    };
    /*!
      \brief Ctor of PollingTimer
     
      \param expiration_time value of wallclock after which timer expires
    */
    explicit PollingTimer(Units units, size_t expiration_time);

    /*!
      \brief returns information if timer is expired
    */
    bool expired() const;
private:
    uint64_t expiration_time_;
};

}

#endif  // FW_SRC_BASE_FW_UTILITIES_TIMEOUT_H_
