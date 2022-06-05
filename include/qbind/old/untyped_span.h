#pragma once

#include <kx/kx.h>
#include <iostream>

#include "qbind/type.h"

namespace qbind
{

template <class UnderlierType, class TargetType>
struct Adapter;

/**
 * @brief A type-erased container with correct move semantics for K
 */
class UntypedSpan
{
public:
    //  constants and types
    using element_type           = void;
    using value_type             = std::remove_cv_t<void>;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    using pointer                = void *;
    using const_pointer          = const void *;
    //using reference              = void &;
    //using const_reference        = const void &;

    /**
     * @brief default
     */
    constexpr UntypedSpan() noexcept
        :m_arr(nullptr)
        ,m_start(0)
        ,m_end(0)
    {}

    /**
     * @brief Copy
     */
    constexpr UntypedSpan(const UntypedSpan& other) noexcept
        :m_arr(other.m_arr ? r1(other.m_arr) : nullptr)
        ,m_start(other.m_start)
        ,m_end(other.m_end)
    { };

    constexpr UntypedSpan& operator=(const UntypedSpan& other) noexcept
    {
        if (this != &other)
        {
            r0(m_arr);
            m_arr = r1(other.m_arr);
            m_start = other.m_start;
            m_end = other.m_end;
        }
        return *this;
    }

    /**
     * @brief move - use memberwise move
     */ 
    constexpr UntypedSpan(UntypedSpan &&other) noexcept
        : m_arr(other.m_arr)
        , m_start(other.m_start)
        , m_end(other.m_end)
    {
        other.m_arr = nullptr;
        other.m_start = 0;
        other.m_end = 0;
    };

    /**
     * @brief Use non-default move assignment as default is not constexpr.
     */
    constexpr UntypedSpan& operator=(UntypedSpan &&other) noexcept
    {
        if (this != &other)
        {
            r0(m_arr);
            m_arr = r1(other.m_arr);
            m_start = other.m_start;
            m_end = other.m_end;
            other.m_arr = nullptr;
            other.m_start = 0;
            other.m_end = 0;
        }
        return *this;
    };

    /**
     * @brief A base pointer to the data. 
     * 
     * This pointer is not suitable for indexing. The pointer points to the
     * atom value or base of the array.
     * 
     * @return constexpr pointer 
     */
    constexpr void* data() const noexcept
    {
        return m_arr ? (
            m_arr->t < 0 ? 
                static_cast<void*>(&m_arr->g) : 
                static_cast<void*>(m_arr->G0)
        ) : nullptr;
    }

    constexpr size_type size() const noexcept
    {
        return m_end - m_start;
    }
    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return size() == 0;
    };

    template<size_t Count>
    UntypedSpan first() const
    {
        if (Count > size())
            throw std::length_error("Attempted to make subview bigger than parent view");
        return UntypedSpan(m_arr, m_start, Count);
    }
    UntypedSpan first(size_type Count) const
    {
        if (Count > size())
            throw std::length_error("Attempted to make subview bigger than parent view");
        return UntypedSpan(m_arr, m_start, Count);
    }

    template<size_t Count>
    UntypedSpan last() const
    {
        if (Count > size())
            throw std::length_error("Attempted to make subview bigger than parent view");
        return UntypedSpan(m_arr, m_end - Count, Count);
    }
    UntypedSpan last( size_type Count ) const
    {
        if (Count > size())
            throw std::length_error("Attempted to make subview bigger than parent view");
        return UntypedSpan(m_arr, m_end - Count, Count);
    }

    template<size_t Offset, size_t Count>
    UntypedSpan subspan() const
    {
        if (Offset + Count > size())
            throw std::length_error("Attempted to make subview which overhangs parent view");
        return UntypedSpan(m_arr, m_start + Offset, Count);
    }
    UntypedSpan subspan(size_type Offset, size_type Count) const
    {
        if (Offset + Count > size())
            throw std::length_error("Attempted to make subview which overhangs parent view");
        return UntypedSpan(m_arr, m_start + Offset, Count);
    }

    // TODO: Is marking this constexpr for Column literal type okay?
    constexpr ~UntypedSpan()
    {
        if (m_arr) r0(m_arr);
    }

    /**
     * @brief The offset of the base pointer of the K array this span starts
     * from. Add this to data after reinterpret casting the pointer to the
     * appropriate type to get to base of the span.
     */
    constexpr size_t offset() const
    {
        return m_start;
    }

    /**
     * @brief Make a subview. No checks on indexing done on construction.
     */
    constexpr UntypedSpan(K arr, size_t Offset, size_t Count) noexcept
        :m_arr(arr ? r1(arr) : nullptr)
        ,m_start(Offset)
        ,m_end(Offset+Count)
    {}   

    /**
     * @brief Takes ownership of array. This is to be used only by the
     * qbind::Converter
     */
    constexpr UntypedSpan(K arr) noexcept
        :m_arr(arr)
        ,m_start(0)
        ,m_end(size(arr))
    { }

    /**
     * @brief Sometimes we want symmetric number of reference count increments
     * and decrements, i.e. creating refs for iterators. Making a qbind::Ref of
     * a K object inside an iterator should not destroy it.
     * 
     * This ensures the lifetime of the span is extended until the last iterator
     * operating on it, or the array itself, is destructed. 
     */
    static UntypedSpan SymmetricMemory(K arr)
    {
        return UntypedSpan(r1(arr));
    }

    K to_q() const
    {
        return r1(m_arr);
    }

private:

    static inline constexpr size_t size(K arr)
    {
        return arr ? (arr->t < 0 ? 1 : arr->n ): 0;
    }

    /**
     * @brief Pointer to k0. May be null.
     */
    K m_arr;
    /**
     * @brief Start index.
     */
    size_t m_start;
    /**
     * @brief End index (i.e. one after last like iterator semantics)
     */
    size_t m_end;

    /**
     * @brief Friend as mixed arrays have elements of type untyped span.
     */
    friend class Adapter<K, UntypedSpan> ;

    /**
     * @brief Friend to allow to do runtime check on construction that
     * conversion is correct.
     */
    template <class QType, class Adapter>
    friend class Span;
};
}