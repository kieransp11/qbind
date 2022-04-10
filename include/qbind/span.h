#pragma once

#include <sstream>
#include <kx/kx.h>

#include "qbind/untyped_span.h"
#include "qbind/ref.h"

namespace qbind
{

// forward declare
template <typename T>
class Span;

template<typename RefType>
class SpanIterator
{
public:
    /**
     * @brief Type of value being exposed
     */
    using value_type = typename RefType::Target;
    using difference_type = int64_t;
    /**
     * @brief Pointer to underlying element
     */
    using pointer = typename RefType::Underlier*;
    /**
     * @brief Ref<Type> which can be used for indexing.
     */
    using reference = RefType;
    using iterator_category = std::random_access_iterator_tag;
 
    SpanIterator() : m_ptr(nullptr), m_index(0) {}
    SpanIterator(pointer ptr, difference_type idx = 0) :m_ptr(ptr) ,m_index(idx) {}

    // Value access
    reference operator*() const { return reference{*(m_ptr + m_index)}; }
    reference operator[](difference_type n) const { return reference{*(m_ptr + m_index + n)}; }

    // Forwards / backwards
    SpanIterator &operator++() { ++m_index; return *this; }
    SpanIterator &operator--() { --m_index; return *this; }
    SpanIterator operator++(int) { auto tmp(*this); ++m_index; return tmp; }
    SpanIterator operator--(int) { auto tmp(*this); --m_index; return tmp; }

    // Arithmetic
    difference_type operator-(const SpanIterator &other) const { return m_index - other.m_index; }
    SpanIterator operator+(difference_type n) const { return {m_ptr, m_index + n}; }
    SpanIterator operator-(difference_type n) const { return {m_ptr, m_index - n}; }

    friend inline SpanIterator operator+(difference_type n, const SpanIterator& other)
    {
        return {other.m_ptr, other.m_index + n};
    }
    friend inline SpanIterator operator-(difference_type n, const SpanIterator& other)
    {
        return {other.m_ptr, other.m_index - n};
    }

    SpanIterator &operator+=(difference_type n) { m_index += n;  return *this; }
    SpanIterator &operator-=(difference_type n) { m_index -= n;  return *this; }

    // Comparison
    bool operator==(const SpanIterator &rhs) const { return m_ptr == rhs.m_ptr && m_index == rhs.m_index; }
    bool operator!=(const SpanIterator &rhs) const { return m_ptr == rhs.m_ptr && m_index != rhs.m_index; }
    bool operator< (const SpanIterator &rhs) const { return m_ptr == rhs.m_ptr && m_index <  rhs.m_index; }
    bool operator> (const SpanIterator &rhs) const { return m_ptr == rhs.m_ptr && m_index >  rhs.m_index; }
    bool operator<=(const SpanIterator &rhs) const { return m_ptr == rhs.m_ptr && m_index <= rhs.m_index; }
    bool operator>=(const SpanIterator &rhs) const { return m_ptr == rhs.m_ptr && m_index >= rhs.m_index; }

private:
    pointer m_ptr;
    difference_type m_index;
};

template <typename T>
class Span
{
public:   
    //  constants and types
    using element_type              = T;
    using value_type                = std::remove_cv_t<typename T::Target>;
    using size_type                 = size_t;
    using difference_type           = ptrdiff_t;
    using reference                 = Ref<T>;
    using const_reference           = ConstRef<T>;
    using iterator                  = SpanIterator<Ref<T>>;
    using reverse_iterator          = SpanIterator<Ref<T>>;
    using const_iterator            = SpanIterator<ConstRef<T>>;
    using const_reverse_iterator    = SpanIterator<ConstRef<T>>;

    constexpr iterator       begin()    const noexcept { return SpanIterator<reference>(        data()); }
    constexpr const_iterator cbegin()   const noexcept { return SpanIterator<const_reference>(  data()); }
    constexpr iterator       end()      const noexcept { return SpanIterator<reference>(        data(), size()); }
    constexpr const_iterator cend()     const noexcept { return SpanIterator<const_reference>(  data(), size()); }

    constexpr reverse_iterator       rbegin()   const noexcept { return SpanIterator<reference>(        data(), size()-1); }
    constexpr const_reverse_iterator crbegin()  const noexcept { return SpanIterator<const_reference>(  data(), size()-1); }
    constexpr reverse_iterator       rend()     const noexcept { return SpanIterator<reference>(        data(), -1); }
    constexpr const_reverse_iterator crend()    const noexcept { return SpanIterator<const_reference>(  data(), -1); }

    constexpr reference front() const { return reference(*data()); }
    constexpr reference back() const { return reference(*(data() + size() - 1)); }
    reference at(size_type idx) const
    {
        if (idx < m_span.size())
            return reference(*(data() + idx));
        std::ostringstream s;
        s << "Attempted to access index " << std::to_string(idx)
          << " of a span containing" << std::to_string(m_span.size());
        throw std::out_of_range(s.str());
    }
    constexpr reference operator[](size_type idx) const
    {
        return reference(*(data() + idx));
    }

    constexpr size_type size() const noexcept 
    { 
        return m_span.size(); 
    }
    [[nodiscard]] constexpr bool empty() const noexcept 
    { 
        return m_span.empty(); 
    }

    template<size_t Count>
    Span<T> first() const
    {
        return {m_span.first<Count>()};
    }
    Span<T> first(size_type Count) const
    {
        return {m_span.first(Count)};
    }

    template<size_t Count>
    Span<T> last() const
    {
        return {m_span.last<Count>()};
    }
    Span<T> last( size_type Count ) const
    {
        return {m_span.last(Count)};
    }

    template<size_t Offset, size_t Count>
    Span<T> subspan() const
    {
        return {m_span.subspan<Offset, Count>()};
    }
    Span<T> subspan(size_type Offset, size_type Count) const
    {
        return {m_span.subspan(Offset, Count)};
    }

    constexpr Span()
        :m_span(UntypedSpan())
    {}

    constexpr Span(const UntypedSpan& span)
        :m_span(std::move(span))
    {
        if (std::abs(m_span.m_arr->t) != T::Code)
            throw std::invalid_argument("Span's underlying array does not match type specified");
    }

    constexpr Span(UntypedSpan&& span)
        :m_span(std::move(span))
    {
        if (std::abs(m_span.m_arr->t) != T::Code)
            throw std::invalid_argument("Span's underlying array does not match type specified");
    }

    UntypedSpan span() const
    {
        return m_span;
    }

private:
    using underlier =                   typename T::Underlier;
    using underlier_pointer =           underlier *;
    using underlier_const_pointer  =    const underlier *;

    /**
     * @brief Private as it gives a raw pointer to an underlying value.
     */
    constexpr underlier_pointer data() const noexcept 
    { 
        return reinterpret_cast<underlier_pointer>(m_span.data()) + m_span.offset(); 
    }

    UntypedSpan m_span;
};
}