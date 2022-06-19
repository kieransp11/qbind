#pragma once

#include <string.h>
#include <string_view>

#include <kx/kx.h>

namespace qbind
{

namespace internal
{

struct intern
{
    [[nodiscard]]
    char * operator()(const std::string_view& value) const noexcept
    {
        return sn(const_cast<char *>(value.data()), value.size());
    }
};

} // internal

/**
 * @brief Thin wrapper to provide reference access to interned symbols.
 *
 * There are three types of "strings" we care about:
 *  - const char*: Null terminated character array.
 *  - string_view: Non-owning view over buffer.
 *  - string: Owning buffer over data.
 *
 * We cannot mutate the data inside a symbol, so can only return const char*.
 * string_view implicitly constructs from const char* and is immutable. string
 * implicitly constructs from const char*. When it does this it copies the data,
 * so will own its own data without messing with symbol buffer.
 *
 * Therefore on read return const char* as it converts implicitly to what we want
 * and has correct semantics.
 *
 * On write const char* implicitly converts to string_view, as does string.
 * Therefore on write we accept string_view. As string_view is not necessarily
 * null-terminated we intern string_view::size() charactors;
 */
class SymbolReference
{
public:

    // Construct from a reference to a pre-interned symbol.
    SymbolReference(char*& ptr)
    :m_ptr(ptr)
    { }

    // Read

    /**
     * @brief Can't mutate data pointed to internally by reading.
     *      string_view: const char* converts implicitly with no copy.
     *      string:      const char* converts implicitly with copy.
     * @return const char * 
     */
    operator const char *() const { return m_ptr; }

    // Compare

    // Compare to other symbol references.
    auto operator<=>(const SymbolReference& rhs) const
    {
        return strcmp(m_ptr, rhs.m_ptr);
    }

    // std::string_view
    auto operator<=>(const std::string_view& rhs) const
    {
        // compare rhs's data that may not be null terminated
        auto res = strncmp(m_ptr, rhs.data(), rhs.size());
        // if m_ptr isn't null at index rhs.size() it means it is longer than
        // rhs, therefore lexicographically greater.
        return (res == 0) ? m_ptr[rhs.size()] != '\0' : res;
    }

    // Write
    SymbolReference& operator=(const std::string_view& value)
    {
        m_ptr = intern(value);
        // sn(const_cast<char *>(value.data()), value.size());
        return *this;
    }

private:
    static constexpr internal::intern intern;
    char *&m_ptr;
};

std::ostream& operator<<(std::ostream& os, const SymbolReference &s)
{
    return os << s;
}

/**
 * @brief Thin wrapper around a pointer to a symbol.
 *
 * This is used for pointer and const_pointer types for symbol.
 * If we used T=char* for symbol ::data() then we would:
 *  - return T* = char** for ::data()
 *  - return const T* = const char** for ::data() const
 *
 * In other words it means this type must be a pointer to SymbolReference
 * or a pointer to const SymbolReference. As we cannot use pointer type
 * notation we have:
 *  - SymbolPointer<false> == char* *;
 *  - SymbolPointer<true>  == const char* *;
 *  - const SymbolPointer<false> == char* const*;
 *  - const SymbolPointer<true> == const char* const*;
 */
template<bool Const>
class SymbolPointer
{
public:
    using return_type = std::conditional_t<Const, const SymbolReference, SymbolReference>;
    using difference_type = int64_t;

    SymbolPointer(char** ptr)
    :m_ptr(ptr)
    { }

    inline SymbolPointer &operator+=(difference_type rhs) { m_ptr += rhs; return *this;}
    inline SymbolPointer &operator-=(difference_type rhs) { m_ptr -= rhs; return *this;}

    inline return_type operator*() const { return {*m_ptr}; }
    inline return_type operator->() const { return {*m_ptr}; }
    inline return_type operator[](difference_type rhs) const { return m_ptr[rhs]; }

    // pre-increment/decrement operators
    inline SymbolPointer& operator++() {++m_ptr; return *this;}
    inline SymbolPointer& operator--() {--m_ptr; return *this;}
    // post-increment/decrement operators
    inline SymbolPointer operator++(int) {SymbolPointer tmp(*this); ++m_ptr; return tmp;}
    inline SymbolPointer operator--(int) {SymbolPointer tmp(*this); --m_ptr; return tmp;}

    inline difference_type operator-(const SymbolPointer& rhs) const {return m_ptr - rhs.m_ptr;}
    inline SymbolPointer operator+(difference_type rhs) const {return SymbolPointer(m_ptr + rhs);}
    inline SymbolPointer operator-(difference_type rhs) const {return SymbolPointer(m_ptr - rhs);}
    friend inline SymbolPointer operator+(difference_type lhs, const SymbolPointer& rhs) {return SymbolPointer(lhs + rhs.m_ptr);}
    friend inline SymbolPointer operator-(difference_type lhs, const SymbolPointer& rhs) {return SymbolPointer(lhs - rhs.m_ptr);}

    inline bool operator<=>(const SymbolPointer &rhs) const { return m_ptr - rhs.m_ptr; }

private:
    char **m_ptr;
};

} // qbind