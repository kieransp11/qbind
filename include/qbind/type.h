#pragma once

#include <sstream>
#include <stdexcept>

#include <kx/kx.h>

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
            "Boolean", "GUID", "", "Byte",            // 1 - 4
            "Short", "Int", "Long",                   // 5 - 7
            "Real", "Float",                          // 8 - 9
            "Char", "Symbol",                         // 10 - 11
            "Timestamp", "Month", "Date", "Datetime", // 12 - 15
            "Timespan", "Minute", "Second", "Time"};  // 16 - 19
    return os << type_txt[static_cast<signed char>(t) - 1];     
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
}

namespace internal
{

/**
 * @brief Map Type to underlying type.
 */
template<Type T> struct c_type;

template<> struct c_type<Type::Boolean> { using Underlier = bool; };
template<> struct c_type<Type::GUID>    { using Underlier = std::array<uint8_t, 16>; };
template<> struct c_type<Type::Byte>    { using Underlier = uint8_t; };
template<> struct c_type<Type::Short>   { using Underlier = int16_t; };
template<> struct c_type<Type::Int>     { using Underlier = int32_t; };
template<> struct c_type<Type::Long>    { using Underlier = int64_t; };
template<> struct c_type<Type::Real>    { using Underlier = float; };
template<> struct c_type<Type::Float>   { using Underlier = double; };
template<> struct c_type<Type::Char>    { using Underlier = char; };
template<> struct c_type<Type::Symbol>  { using Underlier = char *; };
template<> struct c_type<Type::Timestamp> { using Underlier = int64_t; };
template<> struct c_type<Type::Month>   { using Underlier = int32_t; };
template<> struct c_type<Type::Date>    { using Underlier = int32_t; };
template<> struct c_type<Type::Datetime> { using Underlier = double;};
template<> struct c_type<Type::Timespan> { using Underlier = int64_t; };
template<> struct c_type<Type::Minute>  { using Underlier = int32_t; };
template<> struct c_type<Type::Second>  { using Underlier = int32_t; };
template<> struct c_type<Type::Time>    { using Underlier = int32_t; };

}

}