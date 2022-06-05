#pragma once
#include <optional>

#include <kx/kx.h>

#include "k.h"
#include "type.h"

namespace qbind
{

template <Type2 T>
class Vector
{
public:

static constexpr TypeClass TypeInfo{static_cast<short int>(T)};

using Underlier = typename internal::c_type<T>::Underlier;

using reference = Underlier &;
using const_reference = const Underlier &;

using pointer = Underlier*;
using const_pointer = const Underlier*;


using ptr_to_const = const char *;
using ref_to_ptr_to_const = const char *&;
using ptr_to_ptr_to_const = const char **;
// Const reference to const char *. This comes from const T& === T const&.
using const_ref_to_ptr_to_const = const char *const &;
using const_ptr_to_ptr_to_const = const char *const *;

// Initialise from a K object
Vector(K data)
:m_ptr(std::move(data))
{
    if (!m_ptr)
        throw std::runtime_error("K is empty");
    const auto tc = m_ptr.typeClass();
    if (!tc.isVector())
    {
        std::ostringstream ss;
        ss << "K is not a vector. Received type: " << tc;
        throw std::runtime_error(ss.str());
    }
    if (tc.type() != T)
    {
        const TypeClass expected_tc(static_cast<signed char>(T));
        std::ostringstream ss;
        ss << "Expected K to be " << expected_tc << " but found " << tc;
        throw std::runtime_error(ss.str());
    }
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
    auto it = static_cast<Underlier*>(m_ptr.data());
    if constexpr (std::is_same_v<char*, Underlier>)
    {
        std::transform(first, last, it, ss);
    }
    else
    {
        std::copy(first, last, it);
    }
}

Vector(std::initializer_list<Underlier> list)
{
    m_ptr = K{ktn(static_cast<signed char>(T), list.size())};
    auto it = static_cast<Underlier*>(m_ptr.data());
    if constexpr (std::is_same_v<char*, Underlier>)
    {
        std::transform(list.begin(), list.end(), it, ss);
    }
    else
    {
        std::copy(list.begin(), list.end(), it);
    }
}

template<Type2 Q=T, typename = typename std::enable_if<Q == Type2::Char>::type>
Vector(char *str, std::optional<int64_t> size = std::nullopt)
{
    if (str == nullptr)
        throw std::runtime_error("Receive nullptr");
    if (size.value_or(0) < 0)
        throw std::runtime_error("Length must be non-negative");
    m_ptr = K{size.has_value() ? kpn(str, size.value()) : kp(str)};
}

// Element access for non symbols. No risk of editing in place here as there
// is no interning. This interface is effectively a normal vector/array
// at
DISABLE_IF_SYMBOL
reference at(size_t pos) 
{
    check_in_range();
    return *(static_cast<Underlier *>(m_ptr.data()) + pos);
}

DISABLE_IF_SYMBOL
const_reference at(size_t pos) const 
{
    check_in_range();
    return *(static_cast<Underlier *>(m_ptr.data()) + pos);
}

// []
DISABLE_IF_SYMBOL
reference operator[](size_t pos) 
{
    return *(static_cast<Underlier *>(m_ptr.data()) + pos);
}

DISABLE_IF_SYMBOL
const_reference operator[](size_t pos) const 
{
    return *(static_cast<Underlier *>(m_ptr.data()) + pos);
}

// front
DISABLE_IF_SYMBOL
reference front() 
{
    return *static_cast<Underlier *>(m_ptr.data());
}

DISABLE_IF_SYMBOL
const_reference front() const 
{
    return *static_cast<Underlier *>(m_ptr.data());
}

// back
DISABLE_IF_SYMBOL
reference back() 
{
    return *(static_cast<Underlier *>(m_ptr.data()) + m_ptr.size() - 1);
}

DISABLE_IF_SYMBOL
const_reference back() const 
{
    return *(static_cast<Underlier *>(m_ptr.data()) + m_ptr.size() - 1);
}

// data
DISABLE_IF_SYMBOL
pointer data() 
{
    return static_cast<Underlier *>(m_ptr.data());
}

DISABLE_IF_SYMBOL
const_pointer data() const 
{
    return static_cast<Underlier *>(m_ptr.data());
}

// Element access for symbols. Have to be careful about editing here
// due to the need to intern data. const versions of methods may return
// vanilla types, but non-const must return a ref-wrapper that when assigned
// to interns the presented data.
// at
ENABLE_IF_SYMBOL
SymbolReference at(size_t pos) 
{
    check_in_range();
    return {static_cast<char**>(m_ptr.data()) + pos};
}

ENABLE_IF_SYMBOL
const_ref_to_ptr_to_const at(size_t pos) const 
{
    check_in_range();
    return *(static_cast<char **>(m_ptr.data()) + pos);
}

// []
ENABLE_IF_SYMBOL
SymbolReference operator[](size_t pos) 
{
    return {static_cast<char**>(m_ptr.data()) + pos};
}

ENABLE_IF_SYMBOL
const_ref_to_ptr_to_const operator[](size_t pos) const 
{
    return *(static_cast<char **>(m_ptr.data()) + pos);
}

// front
ENABLE_IF_SYMBOL
SymbolReference front() 
{
    return {static_cast<char**>(m_ptr.data())};
}

ENABLE_IF_SYMBOL
const_ref_to_ptr_to_const front() const 
{
    return *static_cast<char**>(m_ptr.data());
}

// back
ENABLE_IF_SYMBOL
SymbolReference back() 
{
    return {static_cast<char**>(m_ptr.data()) + m_ptr.size() - 1};
}

ENABLE_IF_SYMBOL
const_ref_to_ptr_to_const back() const 
{
    return *(static_cast<char **>(m_ptr.data()) + m_ptr.size() - 1);
}

// data
// ENABLE_IF_SYMBOL
// pointer data() 
// {
//     return static_cast<Underlier *>(m_ptr.data());
// }

ENABLE_IF_SYMBOL
const_ptr_to_ptr_to_const data() const 
{
    return static_cast<Underlier *>(m_ptr.data());
}

// Iterators
// begin/cbegin
// end/cend
// rbegin/crbegin
// rend//crend

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
// clear, insert, emplace, erase, emplace_back, pop_back, resize, swap
// don't make sense given memory management.
void push_back(Underlier value)
{
    if constexpr(isSymbol<T>::value)
    {
        m_ptr.join_symbol(value);
    }
    else
    {
        m_ptr.join_atom(&value);
    }
}

void push_back(Vector<T> vec)
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

K m_ptr;
};

}