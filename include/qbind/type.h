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

/**
 * @brief Property based checked for type of K array
 */
class TypeClass
{
public:
    constexpr TypeClass(signed char t) noexcept
    :m_t(t)
    { }

    Type type() const
    {
        if (isAtom() || isVector())
        {
            return Type(abs(m_t));
        }
        std::ostringstream ss;
        ss << "No type available. " << static_cast<int>(m_t) << " is not an atom or vector.";
        throw std::runtime_error(ss.str());
    }

    constexpr bool isError() const noexcept
    {
        return m_t == -128;
    }

    constexpr bool isAtom() const noexcept
    {
        return -19 <= m_t && m_t <= -1 && m_t != -3;
    }

    constexpr bool isTuple() const noexcept
    {
        return m_t == 0;
    }

    constexpr bool isVector() const noexcept
    {
        return 1 <= m_t && m_t <= 19 && m_t != 3;
    }

    constexpr bool isTable() const noexcept
    {
        return m_t == 98;
    }

    constexpr bool isDictionary() const noexcept
    {
        return m_t == 99;
    }

    constexpr bool isUnknown() const noexcept
    {
        return !(isError() || isAtom() || isTuple() || isVector() || isTable() || isDictionary());
    }

    friend auto operator<<(std::ostream& os, TypeClass const& tc) -> std::ostream& {
        if (tc.isError())
            return os << "ERROR";
        if (tc.isAtom())
            return os << type_txt[abs(tc.m_t) - 1] << " Atom";
        if (tc.isTuple())
            return os << "Tuple";
        if (tc.isVector())
            return os << type_txt[tc.m_t - 1] << " Vector";
        if (tc.isTable())
            return os << "Table";
        if (tc.isDictionary())
            return os << "Dictionary";
        return os << "Unknown";
    }

    constexpr bool operator==(const TypeClass& rhs) const
    {
        return m_t == rhs.m_t;
    }
    constexpr bool operator!=(const TypeClass& rhs) const
    {
        return m_t != rhs.m_t;
    }

private:
    signed char m_t;

    static const char *type_txt[];
};

const char *TypeClass::type_txt[] = {
        "Boolean", "GUID", "", "Byte",            // 1 - 4
        "Short", "Int", "Long",                   // 5 - 7
        "Real", "Float",                          // 8 - 9
        "Char", "Symbol",                         // 10 - 11
        "Timestamp", "Month", "Date", "Datetime", // 12 - 15
        "Timespan", "Minute", "Second", "Time"};  // 16 - 19

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