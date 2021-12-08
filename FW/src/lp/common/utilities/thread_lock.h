// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef ADSP_FW_UTILITIES_SIMPLE_LOCK_H
#define ADSP_FW_UTILITIES_SIMPLE_LOCK_H

namespace dsp_fw
{

class ThreadedTask;

/*!
  \brief Guardian that protects shared objects.

  Example:
  \code
  class MyObject: public ThreadSafe
  {
      void Function_MulitThreadAccess()
      {
          ThreadSafe::Lock lock(this);
          do_something1();
          // call below will detect that the same thread is
          // trying to access MyObjecy
          this->Function2_MulitThreadAccess();
      }
      void Function2_MulitThreadAccess()
      {
          ThreadSafe::Lock lock(this);
          do_something2();
      }
  };
  \endcode

  \note Protects only object that is shared between different threads within the
  same core (XMP is not supported).
*/
class ThreadSafe
{
protected:
    ThreadSafe();

    class Lock
    {
    public:
        Lock(ThreadSafe* th);
        ~Lock();
    private:
        ThreadSafe* ths_;
    };

    /*!
      \brief Pointer to ThreadedTask that is using ThreadSafe.
      \note Valid only is reference_counter_ != 0.
    */
    const ThreadedTask* thread_;
    size_t reference_counter_;
};
}

#endif // ADSP_FW_UTILITIES_SIMPLE_LOCK_H
