// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

/*!
  \file
  Circular buffer template declaration.
*/

#ifndef DSP_FW_UTILITIES_CIRCULAR_BUFFER_TEMPLATE_H
#define DSP_FW_UTILITIES_CIRCULAR_BUFFER_TEMPLATE_H

#include <memory>
#include "adsp_std_defs.h"
#include "utilities/array.h"
#include "memory.h"

#if defined(DEBUG)
    #define CB_DBG_COUNTERS_DECLARE() DebugCounters debug_counters_;
    #define CB_DBG_COUNTERS_INIT() memset(&debug_counters_, 0, sizeof(debug_counters_))
    #define CB_DBG_COUNTERS_INC(cnt, inc) debug_counters_.cnt += inc;
#else
    #define CB_DBG_COUNTERS_DECLARE()
    #define CB_DBG_COUNTERS_INIT()
    #define CB_DBG_COUNTERS_INC(cnt, inc)
#endif

namespace dsp_fw
{

template <class T> class CircularBuffer
{
public:
    /*!
      \brief Constructor used to create an instace working on the memory region
             indicated by the poninter & size.
      \param[in]   array              Repesents memory region on which
                                      the circular buffer will operate
                                      (e.g. existing circular buffer.)
      \return Nothing.
    */
    CircularBuffer(const Array<T>& array)
        :array_(array),
         data_size_(0),
         logical_size_(array_.size()),
         preceding_array_()

    {
        if(array.size() == 0 || array.data() == NULL)
        {
            /* Assert - can not throw exception */
            //assert(false);
        }
        CB_DBG_COUNTERS_INIT();

    }

    /*!
      \brief Constructor used to create an instace working on the memory region
             indicated by the poninter & size.
      \param[in]   array               Repesents memory region on which
                                       the circular buffer will operate
                                       (e.g. existing circular buffer.)
     
      \param[in]   preceding_array     Represents memory region just before
                                       circular buffer for storing data
                                       for decoders.
     
      \return Nothing.
    */
    CircularBuffer(const Array<T>& array, const Array<T>&preceding_array )
        :array_(array),
         data_size_(0),
         logical_size_(array_.size()),
         preceding_array_(preceding_array)
    {
        if(array.size() == 0 || array.data() == NULL)
        {
            /* Assert - can not throw exception */
            //assert(false);
        }
        if (preceding_array.size() == 0 || preceding_array.data() == NULL)
        {
            /* Assert - can not throw exception */
            //assert(false);
        }
        if (preceding_array.data() + preceding_array.size() != array.data())
        {
            /* Both arrays do not "stick" */
            //assert(false);
        }
        CB_DBG_COUNTERS_INIT();
    }


    /*! 
      \brief Constructor used to create an instace working on the memory region
             indicated by the poninter & size.
      \param[in]   array              Repesents memory region on which
                                      the circular buffer will operate
                                      (e.g. existing circular buffer.)
     
      \param[in]   read_position      Current Read Position  (allow to
                                      initiate circular buffer in desired
                                      state)
     
      \param[in]   data_size         Current data size  (allow to
                                     initiate circular buffer in desired
                                     state)
     
      \return Nothing.
    */
    CircularBuffer(const Array<T>& array, const size_t read_position, const size_t data_size) :
        array_(array), data_size_(data_size), logical_size_(array_.size()), preceding_array_()
    {
        if (data_size_ > array.size())
        {
            /* Assert - can not throw exception */
            //assert(false);
        }

        if (array.size() == 0 || array.data() == NULL)
        {
            /* Assert - can not throw exception */
            //assert(false);
        }
        if (ADSP_SUCCESS != read_pos_.SafeSet(read_position, array.size() ))
        {
            /* Assert - can not throw exception */
            //assert(false);
        }
        if (ADSP_SUCCESS != write_pos_.SafeSet((read_position + data_size) % array.size(),
            array.size()))
        {
            /* Assert - can not throw exception */
            //assert(false);
        }
        CB_DBG_COUNTERS_INIT();
    }

    /*!
      \brief Returns total physical size of the circular buffer. It may be different from
      the logical_size() if tail of the buffer is temporarily hidden (see GetWriteableBuffer()).
      \return Circular buffer size (no. of Ts).
    */
    size_t size() const
    {
        return array_.size();
    }

    /*!
      \brief Returns logical size of the circular buffer.
      Logical size may be different than physical size (size of the underlaying array_)
      when writer requests a buffer chunk that is larger than the end()-wp and there is
      requested amount of space available at the beginning of the buffer.
      The wp is set to the beginning of the buffer and the tail (end()-wp) is "hidden"
      by setting the logical size until the reader reaches logical_end().
      \return Circular buffer logical size (no. of Ts).
    */
    size_t logical_size() const
    {
        return logical_size_;
    }

    /*!
      \brief Returns pointer to the begining of circular buffer.
      \return Circular Buffer pointer.
    */
    const T* begin() const { return array_.data(); }
    T* begin() { return array_.data(); }

    /*!
      \brief Commits write operation and updates the current write position.
             If last_commit is true, remaining space locked for queued write, if any, is released.
     
      \param[in]   size               Size of the written memory (no. of Ts).
      \return Poper error code.
    */
    ErrorCode WriteCommit(const size_t size, const bool last_commit);

    /*!
      \brief Commits read operation and updates the current read position.
             If last_commit is true, remaining space locked for queued read, if any, is released.
     
      \param[in]   size               Size of the read memory (no. of Ts).
      \return Poper error code.
    */
    ErrorCode ReadCommit(const size_t size, const bool last_commit);


    /*!
      \brief Returns continous readable memory available in the circular buffer.
      \param[in]  size                         Requested size.
      \param[out]  pointer to array            Memmory array.
      \return proper error code
    */
    ErrorCode GetReadableBuffer(Array<T>* array, size_t size = 0);

    /*!
      \brief Returns continous writable memory available in the circular buffer.
      \param[in]  size                           Requested size.
      \param[out]  pointer to array              Memmory array.
      \return proper error code
    */
    ErrorCode GetWriteableBuffer(Array<T>* array, size_t size = 0);

    ErrorCode Unwind(Array<T>* buffer, size_t max_data_requested = 0);

    size_t GetPrecedingArraySize()
    {
        return preceding_array_.size();
    }


    /*!
      \brief Pushes element into the circular buffer. Updates write position.
      \param[in]  element            Element to be pushed to the buffer.
      \return Nothing.
    */
    ErrorCode Push(const T& element)
    {
        if(IsFull())
        {
            return ADSP_OUT_OF_RESOURCES;
        }
        if (write_pos_.HasQueued())
        {
            return ADSP_BUSY;
        }
        array_[write_pos_.IncWrapPos(1, logical_size())] = element;
        data_size_ ++;
        return ADSP_SUCCESS;

    }

    /*!
      \brief Pops (copies) element from the circular buffer to the location specified by element.
             Updates read position.
      \param[in]  element            Element taken from the buffer.
      \return Nothing.
    */
    ErrorCode Pop(T* element)
    {
        if(element == NULL)
        {
            //assert(false);
            return ADSP_ERROR_INVALID_PARAM;
        }
        if (IsEmpty())
        {
            return ADSP_OUT_OF_RESOURCES;
        }
        if (read_pos_.HasQueued())
        {
            return ADSP_BUSY;
        }
        *element = array_[read_pos_.IncWrapPos(1, logical_size())];
        if (read_pos_.get_pos() == 0 /* wrapped*/ && logical_size() < array_.size())
        {
            logical_size_ = array_.size(); //reset logical size on wrap
        }
        --data_size_;
        return ADSP_SUCCESS;
    }
    /*!
      \brief Returns number of entries in the circular buffer available for next read operation.
     
      \return Size of redable memory.
    */
    size_t GetDataSize() const
    {
        size_t value;
        uint32_t interrupt_level = XTOS_SET_INTLEVEL(7);
        value = data_size_ - read_pos_.get_queued_data_size();
        XTOS_RESTORE_INTLEVEL(interrupt_level);
        return value;
    }

    /*!
      \brief Returns overall free memory in the circular buffer.
             It is logical number of entries minus occupied entries
             and entries locked for pending write.
     
      \return Size of free memory.
    */
    size_t GetFreeDataSize() const
    {
        return logical_size() - (data_size_ + write_pos_.get_queued_data_size());
    }

    /*! 
      \brief Returns true if circular buffer is full.
    */
    bool IsFull() const { return logical_size() == GetDataSize(); }

    /*! 
      \brief Returns true if circular buffer is empty.
    */
    bool IsEmpty() const { return 0 == GetDataSize(); }

    bool IsWrapped() const
    {
        return (read_pos_.get_pos() + GetDataSize() > logical_size());
    }

    /*!
      \brief Returns maximum readable continous memory size from the current read position to the tail.
      @return Size of continous readable memory
    */
    size_t GetMaxReadableSize() const
    {
        size_t value;
        uint32_t interrupt_level = XTOS_SET_INTLEVEL(7);
        value = min(logical_size() - read_pos_.get_queued_pos(), GetDataSize());
        XTOS_RESTORE_INTLEVEL(interrupt_level);
        return value;
    }

    /*!
      \brief Returns maximum writeable continous memory size from the current write position to the tail.
             It does not try to wrap and search for writeable chunk at the beginning of array.
      @return Size of continous writeable memory
    */
    size_t GetMaxWriteableSize() const { return min(logical_size() - write_pos_.get_queued_pos(), GetFreeDataSize()); }



    /*!
      \brief Returns current read position.
      @return read_position
    */
    size_t GetReadPosition() const
    {
        return read_pos_.get_pos();
    }
    /*!
      \brief Returns amount of data queued to write
      @return write queued data
    */
    size_t GetWriteDataQueued() const
    {
        return write_pos_.get_queued_data_size();
    }

    /*!
      \brief Returns amount of data queued to read
      @return read queued data
    */
    size_t GetReadDataQueued() const
    {
        return read_pos_.get_queued_data_size();
    }

    /*!
      \brief Set read position
      @return proper error code
    */
    ErrorCode DisplaceReadPosition(const size_t new_read_position)
    {
        if (write_pos_.get_pos() != read_pos_.get_pos() && data_size_ == 0)
        {
            //assert(0);
        }
        if (logical_size() != array_.size())
        {
            return ADSP_BUSY;
        }

        size_t consumed_data = (new_read_position - read_pos_.get_pos()
                        + array_.size()) % array_.size();

        /* Check whether this not cause underruns */
        if (consumed_data == 0)
        {
            consumed_data = logical_size();
        }
        if (consumed_data > data_size_)
        {
            return ADSP_CIRCULAR_BUFFER_UNDERRUN;
        }
        ErrorCode error = read_pos_.SafeSet(new_read_position, array_.size());

        CB_DBG_COUNTERS_INC(displace_read_pos_, 1);
        CB_DBG_COUNTERS_INC(cumulated_read_queued_data_, consumed_data);

        if (ADSP_SUCCESS == error)
        {
            data_size_ = data_size_ - consumed_data;
        }
        return error;
    }


    /*!
      \brief Returns current write position.
      @return write_position
    */
    size_t GetWritePosition() const
    {
        return write_pos_.get_pos();
    }

    /*!
      \brief Commit data without queueing them
    */
    ErrorCode InsertData(const size_t size)
    {
        return DisplaceWritePosition( (write_pos_.get_pos() + size) % array_.size());
    }

    /*!
      \brief Set write position
      @return proper error code
    */
    ErrorCode DisplaceWritePosition(const size_t new_write_position)
    {
        if (logical_size() != array_.size())
        {
            return ADSP_BUSY;
        }

        size_t incoming_data = (new_write_position - write_pos_.get_pos()
                        + array_.size()) % array_.size();
        if (0 == incoming_data)
        {
            incoming_data = array_.size();
        }

        if (incoming_data + data_size_ > array_.size())
        {
            return ADSP_CIRCULAR_BUFFER_OVERRUN;
        }
        ErrorCode error = write_pos_.SafeSet(new_write_position, array_.size());
        CB_DBG_COUNTERS_INC(displace_write_pos_, 1);
        CB_DBG_COUNTERS_INC(cumulated_data_received_, incoming_data);
        if (ADSP_SUCCESS == error)
        {
            data_size_ = data_size_ + incoming_data;
        }
        return error;

    }
    /*!
      \brief Reset all positions in rings
    */
    void Reset()
    {
        write_pos_.Reset();
        read_pos_.Reset();
        data_size_ = 0;
        logical_size_ = array_.size();

    }

   ErrorCode ReInitialize( const size_t read_position, const size_t data_size)

    {
        if (data_size_ > array_.size())
        {
           return ADSP_ERROR_INVALID_PARAM;
        }
        this->Reset();
        if (ADSP_SUCCESS != read_pos_.SafeSet(read_position, array_.size() ))
        {
            return ADSP_FAILURE;
        }
        if (ADSP_SUCCESS != write_pos_.SafeSet((read_position + data_size) % array_.size(),
            array_.size()))
        {
         return ADSP_FAILURE;
        }
        data_size_ = data_size;
        return ADSP_SUCCESS;
    }

    /*!
      \brief Reconstructs existing circular buffer object with new underlying array.
    */
   ErrorCode ReConstruct(const Array<T>& array)
   {
       if(array.size() == 0 || array.data() == NULL)
       {
           return ADSP_ERROR_INVALID_PARAM;
       }

       write_pos_.Reset();
       read_pos_.Reset();
       data_size_ = 0;
       array_ = array;
       logical_size_ = array_.size();

       preceding_array_.Detach();

       return ADSP_SUCCESS;
   }

    /*!
      \brief Returns write position register address
             Such implementation is used for driver puprose.
             Application should have access to "write position address" only for Reading.
    */
    void const * GetWritePosAddress()
    {
        return write_pos_.GetPosAddress();
    }
    /*!
      \brief Returns read position register address.
             Such implementation is used for driver puprose.
             Application should have access to "read position address" only for Reading.
    */
    void const * GetReadPosAddress()
    {
        return read_pos_.GetPosAddress();;
    }

    virtual uint32_t GetMaxWritePosition() {return GetWritePosition();};

private:

    Array<T> array_;
    size_t data_size_;
    size_t logical_size_;
    Array<T> preceding_array_;

    class Position
    {
    public:
        Position() :pos_(0), queued_pos_(0), queued_data_size_(0) {}
        size_t get_pos() const { return pos_; }
        size_t get_queued_pos() const { return queued_pos_; }
        size_t get_queued_data_size() const { return queued_data_size_; }
        // this is unconditional set of both positions to the specified value
        void Set(size_t new_pos) { //assert(queued_data_size_ == 0); 
            queued_pos_ = pos_ = new_pos;
        }
        // this is safe check to the specified value
        ErrorCode SafeSet(size_t new_pos, size_t boundary)
        {
            if (new_pos > boundary)
            {
                //assert(false);
                return ADSP_ERROR_INVALID_PARAM;
            }
            if (HasQueued())
                return ADSP_BUSY;
            new_pos %= boundary;
            Set(new_pos);
            return ADSP_SUCCESS;
        }
        size_t IncWrapPos(size_t inc, size_t boundary)
        {
            size_t old_pos = pos_;
            Set((pos_+inc) % boundary);
            return old_pos;
        }
        void IncQueuedPos(size_t inc, size_t boundary)
        {
            queued_data_size_ += inc;
            queued_pos_ = (queued_pos_+inc)%boundary;
        }
        void SetQueuedPos(size_t inc)
        {
            queued_data_size_ += inc;
            queued_pos_ = inc;
        }
        bool HasQueued() const { return pos_ != queued_pos_; }
        /*!
          @return Flag indicataing whether position has been wrapped around boundary.
        */
        bool CommitQueued(size_t commit_size, size_t boundary)
        {
            if (commit_size > queued_data_size_)
            {
                //assert(false);
                return ADSP_ERROR_INVALID_PARAM;
            }
            queued_data_size_ -= commit_size;
            bool wrapping = pos_+commit_size >= boundary;
            pos_ = (pos_+commit_size) % boundary;
            return wrapping;
        }
        /**
         * Queued space is reset and queued pos is moved "back" to the pos.
         */
        void ResetQueued() { queued_data_size_ = 0; queued_pos_ = pos_; }
        void Reset() {queued_data_size_ = 0; queued_pos_ = 0;  pos_ = 0; }
        void const * GetPosAddress() const {return &pos_;}


    private:
        size_t pos_;
        size_t queued_pos_;
        size_t queued_data_size_;
    };

    Position read_pos_;
    Position write_pos_;

    /* Private default constructor - prevent from constructing CircularBuffer without array. */
    CircularBuffer();

    struct DebugCounters
    {
        uint32_t write_commit_count_;
        uint32_t read_commit_count_;
        uint32_t cumulated_data_received_;
        uint32_t cumulated_data_consumed_;
        uint32_t get_readable_buffer_count_;
        uint32_t cumulated_read_queued_data_;
        uint32_t get_writable_buffer_count_;
        uint32_t cumulated_write_queued_data_;
        uint32_t displace_read_pos_;
        uint32_t displace_write_pos_;
    };
    CB_DBG_COUNTERS_DECLARE();
};

template <class T>
ErrorCode CircularBuffer<T>::WriteCommit(const size_t size, const bool last_commit)
{
    // TODO: next check seems to not make sense?
    if (size + data_size_> logical_size())
    {
        //assert(false);
        return ADSP_CIRCULAR_BUFFER_OVERRUN;
    }
    write_pos_.CommitQueued(size, logical_size());
    uint32_t interrupt_level = XTOS_SET_INTLEVEL(7);
    data_size_ += size;

    uint32_t test = ((write_pos_.get_pos() - read_pos_.get_pos() + logical_size_)
                    % logical_size_);

    if ( (data_size_ %logical_size_) != test)
    {
        //assert(false);
    }
    XTOS_RESTORE_INTLEVEL(interrupt_level);
    CB_DBG_COUNTERS_INC(write_commit_count_, 1);
    CB_DBG_COUNTERS_INC(cumulated_data_received_, size);

    if (last_commit)
    {
        write_pos_.ResetQueued();
    }
    return ADSP_SUCCESS;
}

template <class T>
ErrorCode CircularBuffer<T>::ReadCommit(const size_t size, const bool last_commit)
{
    uint32_t interrupt_level = XTOS_SET_INTLEVEL(7);

    if (size > data_size_)
    {
        return ADSP_CIRCULAR_BUFFER_UNDERRUN;
    }
    bool wrapped = read_pos_.CommitQueued(size, logical_size());
    data_size_ -= size;
    // check if logical_size can be reset to physical boundary - this happens if read position was wrapper around
    // the buffer boundary
    // TODO: pop might need (confirmed) the same check to be performed
    if (wrapped && logical_size() < array_.size())
    {
        logical_size_ = array_.size();
    }

    CB_DBG_COUNTERS_INC(read_commit_count_, 1);
    CB_DBG_COUNTERS_INC(cumulated_data_consumed_, size);

    if (last_commit)
    {
        read_pos_.ResetQueued();
    }
    XTOS_RESTORE_INTLEVEL(interrupt_level);
    return ADSP_SUCCESS;
}

template <class T>
ErrorCode CircularBuffer<T>::GetReadableBuffer(Array<T>* buffer, size_t size)
{
    // make sure that buffer descriptor passed by the caller is clean
    if (buffer->data() != 0 || buffer->size() != 0)
    {
        //assert(false);
        return ADSP_ERROR_INVALID_PARAM;
    }

    CB_DBG_COUNTERS_INC(get_readable_buffer_count_, 1);

    size_t max_readable_size = GetMaxReadableSize();
    // handle special case where caller passes 0 to request max continuous readable space
    if (size == 0)
    {
        size = max_readable_size;
        if (size == 0)
            return ADSP_OUT_OF_RESOURCES;
    }
    else
    {
        if (size > max_readable_size)
            return ADSP_OUT_OF_RESOURCES;
    }
#if 0
    size_t available_data_size = GetDataSize();
    if (available_data_size> logical_size())
    {
        //assert(false);
        return ADSP_FAILURE;
    }
#endif
    uint32_t interrupt_level = XTOS_SET_INTLEVEL(7);
    buffer->Init(&array_[read_pos_.get_queued_pos()], size);
    read_pos_.IncQueuedPos(size, logical_size());
    XTOS_RESTORE_INTLEVEL(interrupt_level);
    CB_DBG_COUNTERS_INC(cumulated_read_queued_data_, size);
    return ADSP_SUCCESS;
}

template <class T>
ErrorCode CircularBuffer<T>::GetWriteableBuffer(Array<T>* buffer, size_t size)
{
    // make sure that buffer descriptor passed by the caller is clean
    if (buffer->data() != 0 || buffer->size() != 0)
    {
        //assert(false);
        return ADSP_ERROR_INVALID_PARAM;
    }

    CB_DBG_COUNTERS_INC(get_writable_buffer_count_, 1);

    size_t max_writeable_size = GetMaxWriteableSize();

    if(size == 0) // handle special case where caller passes 0 to request max continuous writeable space
    {
        size = max_writeable_size;
        if (size == 0)
            return ADSP_OUT_OF_RESOURCES;
    }

    T* fragment;
    // check if there is any chance to alloc chunk of requested size
    if (GetFreeDataSize() < size)
        return ADSP_OUT_OF_RESOURCES;
    // check if there is free space at the tail
    if (write_pos_.get_queued_pos() + size <= logical_size())
    {
        fragment = &array_[write_pos_.get_queued_pos()];
        write_pos_.IncQueuedPos(size, logical_size());
    }
    else if (size <= read_pos_.get_pos()) // otherwise check if there is a free chunk at the head
    {
        logical_size_ = write_pos_.get_queued_pos(); // hide the tail of the buffer ...
        fragment = array_.data(); // ... and wrap the write pointer
        write_pos_.SetQueuedPos(size);
    }
    else
        return ADSP_OUT_OF_RESOURCES;
#if 0
    if (queued_write_position_ + size <= logical_size_)
    {
        fragment_addrees = array_.data()+queued_read_position_;
        queued_write_position_ = WrapIncPosition(queued_write_position_, size);
    }
    else if (size <= read_position_)
    {
        fragment_addrees = array_.data();
        logical_size_ = queued_write_position_;
        queued_write_position_ = size;
    }
    else
    {
        return ADSP_OUT_OF_RESOURCES;
    }
#endif
    buffer->Init(fragment, size);
    CB_DBG_COUNTERS_INC(cumulated_write_queued_data_, size);
    return ADSP_SUCCESS;
}
template <class T>
ErrorCode CircularBuffer<T>::Unwind(Array<T>* buffer, size_t max_data_requested)
{
    /* make sure that buffer descriptor passed by the caller is clean */
    if (buffer->data() != 0 || buffer->size() != 0)
    {
        return ADSP_ERROR_INVALID_PARAM;
    }
    /* Check if no is queued to read */
    if (0 != GetReadDataQueued())
    {
        return ADSP_BUSY;
    }
    if (0 == max_data_requested)
    {
        max_data_requested = GetDataSize();
    }
    /*
     * If cirrucular buffer do not wraps call normal Get Readable Buffer or
     * if max data requested is less than max readeable size.
     */
    if (!IsWrapped() || max_data_requested <= GetMaxReadableSize())
    {
        return GetReadableBuffer(buffer, min(max_data_requested, GetMaxReadableSize()));
    }

    /* check if proceding array initialized */
    if (GetPrecedingArraySize() == 0 || preceding_array_.data() == NULL)
    {
        return ADSP_INVALID_REQUEST;
    }

    /* Else Calculate size  which will  be unwinded */
    size_t copy_size = GetMaxReadableSize();
    /* Check if there is enough space to copy */
    if (copy_size > preceding_array_.size())
    {
        return ADSP_OUT_OF_RESOURCES;
    }
    /* calculate destination pointer such that the data "sticks" */
    T* destintation_ptr = &(preceding_array_[preceding_array_.size() - copy_size]);
    memcpy_s(destintation_ptr, preceding_array_.size()*sizeof(T), &array_[read_pos_.get_pos()], copy_size*sizeof(T));
    /* Init unwinded buffer */
    buffer->Init(destintation_ptr, max_data_requested);
    /* Queue all data to read */
    read_pos_.IncQueuedPos(max_data_requested, logical_size());
    return ADSP_SUCCESS;

}

}
//namespace dsp_fw

#endif //DSP_FW_UTILITIES_CIRCULAR_BUFFER_TEMPLATE_H
