#pragma once

#include <optional>

#include <kx/kx.h>

#include "k.h"
#include "symbol.h"
#include "type.h"

namespace qbind
{

// TODO: Thing what Type* is for an iterator operator->. Iterators should probably use <Type T> enum rather than actual type.
//  Make iterator just for vector, then different onces for nested vector?

// TODO: Implement comparisons on vector.

template <Type T>
class Vector
{
public:

    static constexpr Type type = T;
    static constexpr Structure structure = Structure::Vector;

    // To access underlying data
    using underlier         = typename internal::c_type<T>::underlier;
    using value             = typename internal::c_type<T>::value;
    using reference         = typename internal::c_type<T>::reference;
    using const_reference   = typename internal::c_type<T>::const_reference;
    using pointer           = typename internal::c_type<T>::pointer;
    using const_pointer     = typename internal::c_type<T>::const_pointer;

    // Initialise from a K object
    Vector(K data)
    :m_ptr(std::move(data))
    {
        if (!m_ptr)
            throw std::runtime_error("K is empty");
        m_ptr.is_with_info<Vector<T>>();
    }

    // Create empty array of set length
    Vector(int64_t length)
    {
        if (length < 0)
        {
            throw std::runtime_error("Length must be non-negative");
        }
        m_ptr = K{ktn(static_cast<signed char>(T), length)};
    }

    // Create from another collection
    template<class iterator_type>
    Vector(iterator_type first, iterator_type last)
    {
        if (last < first)
        {
            throw std::runtime_error("Length must be non-negative");
        }
        m_ptr = K{ktn(static_cast<signed char>(T), last-first)};
        auto it = m_ptr.data<underlier>();
        std::transform(first, last, it, to_underlier);
    }

    Vector(std::initializer_list<value> list)
    {
        m_ptr = K{ktn(static_cast<signed char>(T), list.size())};
        auto it = m_ptr.data<underlier>();
        std::transform(list.begin(), list.end(), it, to_underlier);
    }

    template<qbind::Type Q=T, typename = typename std::enable_if<Q == Type::Char>::type>
    Vector(char *str, std::optional<int64_t> size = std::nullopt)
    {
        if (str == nullptr)
            throw std::runtime_error("Receive nullptr");
        if (size.value_or(0) < 0)
            throw std::runtime_error("Length must be non-negative");
        m_ptr = K{size.has_value() ? kpn(str, size.value()) : kp(str)};
    }

    // Element access. 
    // For non-symbols and const vectors of symbols this is effectively a normal 
    // vector/array. For non-const symbols we have to be careful about editing 
    // here due to the need to intern data. const versions of methods may return
    // vanilla types, but non-const must return a ref-wrapper that when assigned
    // to interns the presented data.
    // at
    reference at(size_t pos) 
    {
        check_in_range();
        return m_ptr.data<underlier>()[pos];
    }

    const_reference at(size_t pos) const 
    {
        check_in_range();
        return m_ptr.data<underlier>()[pos];
    }

    // []
    reference operator[](size_t pos) 
    {
        return m_ptr.data<underlier>()[pos];
    }

    const_reference operator[](size_t pos) const 
    {
        return m_ptr.data<underlier>()[pos];
    }

    // front
    reference front() 
    {
        return m_ptr.data<underlier>()[0];
    }

    const_reference front() const 
    {
        return m_ptr.data<underlier>()[0];
    }

    // back
    reference back() 
    {
        return m_ptr.data<underlier>()[m_ptr.size() - 1];
    }

    const_reference back() const 
    {
        return m_ptr.data<underlier>()[m_ptr.size() - 1];
    }

    // data
    pointer data() 
    {
        return m_ptr.data<underlier>();
    }

    const_pointer data() const 
    {
        return m_ptr.data<underlier>();
    }

    // Iterators
    // begin/cbegin
    // end/cend
    // rbegin/crbegin
    // rend/crend

    // Capacity
    // reserve, capacity, and shrink_to_fit don't make sense given memory management.
    bool empty() const noexcept
    {
        return size() != 0;
    }

    size_t size() const noexcept
    {
        return m_ptr.size();
    }

    size_t max_size() const noexcept
    {
        return std::numeric_limits<decltype(::k0::n)>::max();
    }

    // Modifiers
    // erase, pop_back, resize don't make sense given memory management.
    // TODO: clear, insert, emplace, emplace_back, swap

    void push_back(value value)
    {
        m_ptr.join_atom(to_underlier(value));
    }

    void push_back(const Vector<T>& vec)
    {
        m_ptr.join_lists(vec.m_ptr);
    }

    // Operators
    // ==
    // <=> : Lexicographical ordering

    K get() const
    {
        return m_ptr;
    }

private:

    void check_in_range(size_t pos)
    {
        if (pos >= m_ptr.size())
            throw std::out_of_range("Attempted to access index " + std::to_string(pos) + " but length is " + std::to_string(size()));
    }

    static constexpr typename internal::c_type<T>::to_underlier to_underlier;

    K m_ptr;
};

namespace v
{
using b = qbind::Vector<Type::Boolean>;
using g = qbind::Vector<Type::GUID>;
using x = qbind::Vector<Type::Byte>;
using h = qbind::Vector<Type::Short>;
using i = qbind::Vector<Type::Int>;
using j = qbind::Vector<Type::Long>;
using e = qbind::Vector<Type::Real>;
using f = qbind::Vector<Type::Float>;
using c = qbind::Vector<Type::Char>;   
using s = qbind::Vector<Type::Symbol>;
using p = qbind::Vector<Type::Timestamp>; 
using m = qbind::Vector<Type::Month>; 
using d = qbind::Vector<Type::Date>;
using z = qbind::Vector<Type::Datetime>; 
using n = qbind::Vector<Type::Timespan>;
using u = qbind::Vector<Type::Minute>;
using v = qbind::Vector<Type::Second>;  
using t = qbind::Vector<Type::Time>;   
} // namespace v

namespace vector
{
using Boolean   = qbind::Vector<Type::Boolean>;
using GUID      = qbind::Vector<Type::GUID>;
using Byte      = qbind::Vector<Type::Byte>;
using Short     = qbind::Vector<Type::Short>;
using Int       = qbind::Vector<Type::Int>;
using Long      = qbind::Vector<Type::Long>;
using Real      = qbind::Vector<Type::Real>;
using Float     = qbind::Vector<Type::Float>;
using Char      = qbind::Vector<Type::Char>;   
using Symbol    = qbind::Vector<Type::Symbol>;
using Timestamp = qbind::Vector<Type::Timestamp>; 
using Month     = qbind::Vector<Type::Month>; 
using Date      = qbind::Vector<Type::Date>;
using Datetime  = qbind::Vector<Type::Datetime>; 
using Timespan  = qbind::Vector<Type::Timespan>;
using Minute    = qbind::Vector<Type::Minute>;
using Second    = qbind::Vector<Type::Second>;  
using Time      = qbind::Vector<Type::Time>; 
} // namespace vector

using String = qbind::Vector<Type::Char>;

}