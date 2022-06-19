#pragma once

#include <iterator>

#include "type.h"

namespace qbind
{

/**
 * @brief Generic iterator
 * 
 * @tparam T: Kx type of array
 * @tparam Const: If the iterator is const
 * @tparam Reverse: If the iterator is reverse
 */
template<Type T, bool Const, bool Reverse>
class Iterator
{
public:
    using iterator_category = std::contiguous_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = typename internal::c_type<T>::value;
    using pointer           = std::conditional_t<Const,
                                typename internal::c_type<T>::const_pointer,
                                typename internal::c_type<T>::pointer>;
    using reference         = std::conditional_t<Const,
                                typename internal::c_type<T>::const_reference,
                                typename internal::c_type<T>::reference>;

    Iterator(pointer ptr) : m_ptr(ptr) {}

    inline Iterator& operator+=(difference_type rhs) { m_ptr = add(rhs); return *this;}
    inline Iterator& operator-=(difference_type rhs) { m_ptr = sub(rhs); return *this;}
    
    inline reference operator*() const {return *m_ptr;}
    inline pointer operator->() const {return m_ptr;}
    inline reference operator[](difference_type rhs) const {return m_ptr[direction*rhs];}
    
    // pre-increment/decrement operators
    inline Iterator& operator++() {inc(m_ptr); return *this;}
    inline Iterator& operator--() {dec(m_ptr); return *this;}
    // post-increment/decrement operators
    inline Iterator operator++(int) {Iterator tmp(*this); inc(m_ptr); return tmp;}
    inline Iterator operator--(int) {Iterator tmp(*this); dec(m_ptr); return tmp;}

    inline difference_type operator-(const Iterator& rhs) const {return direction*(m_ptr-rhs.ptr);}
    inline Iterator operator+(difference_type rhs) const {return Iterator(add(rhs));}
    inline Iterator operator-(difference_type rhs) const {return Iterator(sub(rhs));}
    friend inline Iterator operator+(difference_type lhs, const Iterator& rhs) {return Iterator(rhs.add(lhs));}
    friend inline Iterator operator-(difference_type lhs, const Iterator& rhs) {return Iterator(rhs.sub(lhs));}
    
    auto operator<=>(const Iterator& rhs) const
    {
        if (m_ptr == rhs.m_ptr)
            return 0;
        // rhs.m_ptr < m_ptr & forward => direction = 1 => return 1
        // rhs.m_ptr < m_ptr & backward => direction = -1 => return -1
        if (rhs.m_ptr < m_ptr)
            return direction;
        // m_ptr < rhs.m_ptr & forward => direction = 1 => return -1
        // m_ptr < rhs.m_ptr & backward => direction = -1 => return 1
        return -direction;
    }

private:
    pointer m_ptr;

    struct Forward
    {
        static constexpr difference_type value = 1;
    };

    struct Backward
    {
        static constexpr difference_type value = -1;
    };

    static constexpr difference_type direction = std::conditional_t<Reverse, Backward, Forward>::value;

    // increment by reference. Equivalent to ++ptr
    static inline pointer inc(pointer & p)
    {
        return p += direction;
    }

    // decrement by reference. Equivalent to --ptr
    static inline pointer dec(pointer& p)
    {
        return p -= direction;
    }

    // add. Equivalent to m_ptr + n
    pointer add(difference_type n)
    {
        return m_ptr + (direction * n);
    }

    // sub. Equivalent to m_ptr - n
    pointer sub(difference_type n)
    {
        return m_ptr - (direction * n);
    }
};

}