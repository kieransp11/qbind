#pragma once

#include <sstream>
#include <stdexcept>

#include <kx/kx.h>

#include "symbol.h"

namespace qbind 
{

/**
 * @brief The possible atomic types for values in KDB
 * 
 * signed char like the t field of the k0 struct.
 */
enum class Type : signed char
{
    Boolean = KB,
    GUID = UU,
    Byte = KG,
    Short = KH,
    Int = KI,
    Long = KJ,
    Real = KE,
    Float = KF,
    Char = KC,
    Symbol = KS,
    Timestamp = KP,
    Month = KM,
    Date = KD,
    Datetime = KZ, // deprecated
    Timespan = KN,
    Minute = KU,
    Second = KV,
    Time = KT,
};

std::ostream& operator<<(std::ostream& os, const Type &t) 
{
    static const char *type_txt[] = {
            "Boolean", "GUID", "UNKNOWN", "Byte",     // 1 - 4
            "Short", "Int", "Long",                   // 5 - 7
            "Real", "Float",                          // 8 - 9
            "Char", "Symbol",                         // 10 - 11
            "Timestamp", "Month", "Date", "Datetime", // 12 - 15
            "Timespan", "Minute", "Second", "Time"};  // 16 - 19
    const auto idx = static_cast<signed char>(t) - 1;
    return os << ((idx < 0 || 18 < idx) ? "UNKNOWN" : type_txt[idx]);
}

enum class Structure : char
{
    Error,          // -128
    Atom,           // -19 to -1 except -3
    Tuple,          // 0
    Vector,         // 1 to 19 except 3
    NestedVector,   // 0 where all children are 0 or same type
    Table,          // 98
    Dictionary,     // 99
    KeyedTable
};

std::ostream& operator<<(std::ostream& os, const Structure &s) 
{
    switch (s)
    {
        case Structure::Error:          return os << "ERROR";
        case Structure::Atom:           return os << "Atom";
        case Structure::Tuple:          return os << "Tuple";
        case Structure::Vector:         return os << "Vector";
        case Structure::NestedVector:   return os << "Nested Vector";
        case Structure::Table:          return os << "Table";
        case Structure::Dictionary:     return os << "Dictionary";
        case Structure::KeyedTable:     return os << "Keyed Table";
    }
    return os << "UNKNOWN";
}

namespace internal
{

/**
 * @brief std::identity in C++20.
 */
struct identity
{
    template<typename T>
    [[nodiscard]] 
    constexpr T&& operator()(T&& t) const noexcept 
    { 
        return std::forward<T>(t); 
    }
};

/**
 * @brief Map Type to underlying type.
 * 
 * value uses: vector constructor from initialiser list.
 *             vector assign to from initialiser list.
 *             vector overloads of assign.
 *             vector overloads of insert.
 *             vector overloads of push back.
 *             vector overloads of resize.
 * 
 * underlier     : Type of the underlying kx k0 array.
 * value         : Type to expose publically
 * reference     : Mutable reference to value in array.
 * const_referenc: Const reference to value in array.
 * pointer       : Mutable pointer to data element.
 * const_pointer : Immutable pointer to data element.
 * to_underlier  : underlier f(value)
 */
template<Type T> struct c_type;

#define C_TYPE_TRAITS(type)                \
    using underlier = type;                \
    using value = type;                    \
    using reference = value &;             \
    using const_reference = const value &; \
    using pointer = value *;               \
    using const_pointer = const value *;   \
    using to_underlier = identity;

template<> struct c_type<Type::Boolean> { C_TYPE_TRAITS(bool) };
template<> struct c_type<Type::GUID>    
{
    using guid = std::array<uint8_t, 16>;
    C_TYPE_TRAITS(guid)
};
template<> struct c_type<Type::Byte>    { C_TYPE_TRAITS(uint8_t) };
template<> struct c_type<Type::Short>   { C_TYPE_TRAITS(int16_t) };
template<> struct c_type<Type::Int>     { C_TYPE_TRAITS(int32_t) };
template<> struct c_type<Type::Long>    { C_TYPE_TRAITS(int64_t) };
template<> struct c_type<Type::Real>    { C_TYPE_TRAITS(float); };
template<> struct c_type<Type::Float>   { C_TYPE_TRAITS(double); };
template<> struct c_type<Type::Char>    { C_TYPE_TRAITS(char); };
template<> struct c_type<Type::Symbol>  
{ 
    using underlier = char *;
    using value = std::string_view;
    using reference = SymbolReference;
    using const_reference = const SymbolReference;
    using pointer = SymbolPointer<false>;
    using const_pointer = SymbolPointer<true>;
    using to_underlier = intern;
};
template<> struct c_type<Type::Timestamp> { C_TYPE_TRAITS(int64_t) };
template<> struct c_type<Type::Month>   { C_TYPE_TRAITS(int32_t) };
template<> struct c_type<Type::Date>    { C_TYPE_TRAITS(int32_t) };
template<> struct c_type<Type::Datetime> { C_TYPE_TRAITS(double) };
template<> struct c_type<Type::Timespan> { C_TYPE_TRAITS(int64_t) };
template<> struct c_type<Type::Minute>  { C_TYPE_TRAITS(int32_t) };
template<> struct c_type<Type::Second>  { C_TYPE_TRAITS(int32_t) };
template<> struct c_type<Type::Time>    { C_TYPE_TRAITS(int32_t) };

}

}