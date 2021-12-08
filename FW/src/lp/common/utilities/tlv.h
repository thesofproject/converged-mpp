// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Intel Corporation. All rights reserved.

#ifndef ADSP_FW_TVL_TEMPLATE_H
#define ADSP_FW_TVL_TEMPLATE_H

namespace dsp_fw
{

/*!
  \brief Temlate of Type-Length-Value structure of data
*/
template<class T, size_t N=1> struct TLV
{
    TLV() : type(0), length(N * sizeof(T))
    {
#ifdef UT
        memset(value, 0, length);
#endif
    }

    explicit TLV(uint32_t type) : type(type), length(N * sizeof(T))
    {
#ifdef UT
        memset(value, 0, length);
#endif
    }

    /*!
      \brief creates TLV structure for N = 1
    */
    TLV(uint32_t type, const T& val) : type(type), length(N * sizeof(T))
    {
        C_ASSERT(N == 1);
        value[0] = val;
    }

    /*!
      \brief Retrieves ptr to next TLV struct. Size of total passed TLV needs to be validated at upper
      level (you can use TlvIterator for that).
    */
    TLV<T, N>* GetNext()
    {
        return reinterpret_cast<TLV<T, N>*>(&value[length/sizeof(T)]);
    }

    inline size_t GetTLVSize() const
    {
        return sizeof(type) + sizeof(length) + length;
    }

    uint8_t* GetNextTLVBegin()
    {
        return reinterpret_cast<uint8_t*> (this) + GetTLVSize();
    }

    // type of passed parameters (needs to be dispatched at upper layer.
    uint32_t type;
    // length of data (in bytes). needs to be equal to N * sizeof(T).
    uint32_t length;
    // array of parameters
    T value[N];
};

/*!
  \brief TlvIterator is structure usually allocated on stack with relatively small size that
  is supposed to help managing of TLV structure of any kind.
*/
struct TlvIterator
{
    /*!
      \brief ctor.
      \param ptr_to_tlv points to first passed TLV. If there is more TLV in single incoming buffer
                        then size_of_all_tlvs needs to be greater than length of first TLV
      \param size_of_all_tlvs
    */
    TlvIterator(void* ptr_to_tlv, size_t size_of_all_tlvs)
        : tlv(reinterpret_cast<TLVInt*>(ptr_to_tlv)), size(size_of_all_tlvs)
    {
    }

    TlvIterator(const void* ptr_to_tlv, size_t size_of_all_tlvs)
        : tlv(reinterpret_cast<TLVInt*>(const_cast<void*>(ptr_to_tlv))),
          size(size_of_all_tlvs)
    {
    }

    /*!
      \brief Checks whether current TLV structure pointer by tlv ptr is valid or not.
    */
    bool IsValid() const
    {
        return (size > 0) && (tlv->length != 0) && (size >= (int32_t)(tlv->length + offsetof(TLVInt, value)));
    }

    /*!
      \brief Retrieves type of current tlv structure.
    */
    uint32_t GetType() const { return tlv->type; }

    /*!
      \brief Retrieves length (in bytes) from current tlv structure.
    */
    uint32_t GetLength() const
    {
        return tlv->length;
    }

    /*!
      \brief Retrievs and casts values.
    */
    template<class T> T*  GetValueAsPtr()
    {
        if (sizeof(T) > tlv->length)
        {
        #pragma frequency_hint NEVER
            return NULL;
        }
        return reinterpret_cast<T*>(&tlv->value[0]);
    }

    template<class T> T* InitTlvOfArrayAndGetValueAsPtr(uint32_t type, uint32_t length, uint32_t items_count)
    {
        tlv->type = type;
        tlv->length = length;
        if ((sizeof(T) * items_count) > length)
        {
        #pragma frequency_hint NEVER
            return NULL;
        }
        return reinterpret_cast<T*>(&tlv->value[0]);
    }

    template<class T> T* InitTlvAndGetValueAsPtr(uint32_t type, uint32_t length)
    {
        tlv->type = type;
        tlv->length = length;
        return GetValueAsPtr<T>();
    }
    template<class T> void InitTlv(uint32_t type, const T& value)
    {
        T* _value = InitTlvAndGetValueAsPtr<T>(type, sizeof(T));
        *_value = value;
    }

    template<class T> const T& GetValueAs()
    {
        return reinterpret_cast<const TLV<T> *>(tlv)->value[0];
    }

    template<class T> bool IsValidAs() const { return (sizeof(T) <= tlv->length); }

    /*!
      \brief Check if given TLV would fit in the remaining space
    */
    template<class T> bool IsTlvValid() const { return size >= (int32_t)sizeof(TLV<T>); }

    /*!
      \brief Overloaded operator++ to make it work just like regular iterator
    */
    TlvIterator& operator++()
    {
        size_t last_size = tlv->length + sizeof(tlv->type) + sizeof(tlv->length);
        tlv = tlv->GetNext();
        size -= last_size;
        return *this;
    }

    /*!
      \brief Get integer value of address for the TLV that iterator is pointing to currently
      @return address
    */
    uint32_t GetCurrentTlvAddress() const
    {
        return reinterpret_cast<uint32_t>(tlv);
    }

private:
    typedef TLV<uint32_t> TLVInt;

    // ptr to currently processed TLV structure
    TLVInt* tlv;
    // total size of all tlv pointed by tlv
    int size;
};

// Calculate size of TLV containg <Value> of data_size size [Bytes].
#define TLV_SIZE(data_size) (sizeof(TLV<uint32_t, 1>) + data_size - sizeof(uint32_t))

} // namespace dsp_fw

#endif //#ifndef _ADSP_FW_TVL_TEMPLATE_H
