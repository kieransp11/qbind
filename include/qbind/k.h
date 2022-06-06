#pragma once

#include <kx/kx.h>

#include "forward.h"
#include "type.h"
#include "utils.h"

namespace qbind
{

/**
 * @brief An RAII wrapper around the KX K struct
 * 
 * Implements memory functions:
 *  - V r0(K)
 *  - K r1(K).
 */
class K
{
public:

    constexpr K() noexcept
    : m_k(nullptr)
    { }

    constexpr K(std::nullptr_t) noexcept
    : m_k(nullptr)
    { }

    explicit K(::K k)
    : m_k(k)
    { }

    // Copy constructors
    K(const K& other) noexcept
    :m_k(_r1(other.m_k))
    { }

    // Move constructors
    K(K&& other) noexcept
    :m_k(other.m_k)
    {
        other.m_k = nullptr;
    }

    // Destructor
    ~K()
    {
        _r0(m_k);
    }

    // Operator=
    // Copy
    K& operator=(const K& other) noexcept
    {
        // Ignore self assignment
        // If already the same internal pointer don't do anything
        if (this != &other && m_k != other.m_k)
        {
            _r0(m_k);
            m_k = _r1(other.m_k);
        }
        return *this;
    }

    // Move
    K& operator=(K&& other) noexcept
    {
        if (this != &other)
        {
            _r0(m_k);
            m_k = other.m_k;
            other.m_k = nullptr;
        }
        return *this;
    }

    void swap(K& other) noexcept
    {
        auto tmp = m_k;
        m_k = other.m_k;
        other.m_k = tmp;
    }

    // Reference counting starts and begins at 0. So should
    // read m_k->t as "number of copies still around". If you
    // decrement the reference count from 0 (i.e. when there
    // are no copies left, the object is no longer valid.
    size_t use_count() const noexcept
    {
        return m_k ? 1 + m_k->r : 0;
    }

    // This is valid if and only if reference counting is kept consistent externally too.
    operator bool() const noexcept
    {
        return m_k != nullptr;
    }

    // Access members of the struct but don't leak reference counting.
    TypeClass typeClass() const noexcept
    {
        return TypeClass(m_k->t);
    }

    // size: 1 if atom else size
    size_t size() const noexcept
    {
        return m_k->t < 0 ? 1 : static_cast<size_t>(m_k->n);
    }

    void* data() const noexcept
    {
        return m_k ? (
            m_k->t < 0 ? 
                static_cast<void*>(&m_k->g) : 
                static_cast<void*>(m_k->G0)
        ) : nullptr;
    }

private:

    // Avoid nullptr segfault
    static void _r0(::K x) noexcept
    {
        if (x)
            r0(x);
    }

    // Avoid nullptr segfault
    static ::K _r1(::K x) noexcept
    {
        return x ? r1(x) : x;
    }

    // Vector appending: Type/nullity protection done in vector.
    // TODO: Check what these functions do to lhs, rhs and returned reference count.
    template <Type>
    friend class Vector;

    void join_atom(void* value)
    {
        m_k = ja(&m_k, value);
    }

    void join_symbol(char* symbol)
    {
        m_k = js(&m_k, ss(symbol));
    }

    void join_lists(K k)
    {
        m_k = jv(&m_k, k.m_k);
    }

    // Tuple appending: Type/nullity protection done in tuple.
    // TODO: Check what this function does to lhs, rhs and returned reference count.
    template <class... Types>
    friend class Tuple;

    void tuple_append(K k)
    {
        // docs mention taking ownership of rhs.
        m_k = jk(&m_k, _r1(k.m_k));
    }

    ::K get() const
    {
        return _r1(m_k);
    }

    friend class Converter;

    // For Converter and Tuple
    // Make a non-owning array. 
    static K make_non_owning(::K k)
    {
        return K(_r1(k));
    }

    // For Converter.
    // Use to get the internal pointer and return back to 
    // manual memory management. May return nullptr.
    ::K release() noexcept
    {
        auto temp = m_k;
        m_k = nullptr;
        return temp;
    }

private:

    ::K m_k;
};

}