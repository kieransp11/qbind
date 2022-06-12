#pragma once

#include <optional>

#include <kx/kx.h>

#include "forward.h"
#include "k.h"
#include "type.h"

namespace qbind
{

/**
 * @brief Wrapper around symbol that ensures interning on assignment.
 */
class SymbolReference
{
public:

    operator const char*() const {
        return m_ptr;
    }

    SymbolReference& operator=(char* value)
    {
        m_ptr = ss(value);
        return *this;
    }

    SymbolReference(char*& ptr)
    :m_ptr(ptr)
    { }

private:
    // where the symbol is located.
    char *&m_ptr;
};

template <Type T>
class Vector
{
public:

    static constexpr Type Type = T;
    static constexpr Structure Structure = Structure::Vector;

    // To access underlying data
    using Underlier = typename internal::c_type<T>::Underlier;
    
    // What is actually returned (const accessors)
    using result = std::conditional_t<T == Type::Symbol, const char *, Underlier>;

    using reference = std::conditional_t<T==Type::Symbol,
        SymbolReference,
        Underlier &>;
    using const_reference = result const &;

    using pointer = Underlier*;
    using const_pointer = result const *;

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
        return static_cast<Underlier *>(m_ptr.data())[pos];
    }

    const_reference at(size_t pos) const 
    {
        check_in_range();
        return static_cast<Underlier *>(m_ptr.data())[pos];
    }

    // []
    reference operator[](size_t pos) 
    {
        return static_cast<Underlier *>(m_ptr.data())[pos];
    }

    const_reference operator[](size_t pos) const 
    {
        return static_cast<Underlier *>(m_ptr.data())[pos];
    }

    // front
    reference front() 
    {
        return static_cast<Underlier *>(m_ptr.data())[0];
    }

    const_reference front() const 
    {
        return static_cast<Underlier *>(m_ptr.data())[0];
    }

    // back
    reference back() 
    {
        return static_cast<Underlier *>(m_ptr.data())[m_ptr.size() - 1];
    }

    const_reference back() const 
    {
        return static_cast<Underlier *>(m_ptr.data())[m_ptr.size() - 1];
    }

    // data
    // Not available for data as don't want to expose data that need to be interned.
    DISABLE_IF_SYMBOL
    pointer data() 
    {
        return static_cast<Underlier *>(m_ptr.data());
    }

    const_pointer data() const 
    {
        return static_cast<Underlier *>(m_ptr.data());
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

    void push_back(Underlier value)
    {
        m_ptr.join_atom(value);
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