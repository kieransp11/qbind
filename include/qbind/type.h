#pragma once

#include <chrono>

#include <kx/kx.h>

#include "untyped_span.h"

namespace qbind 
{

namespace Q
{

/**
 * @brief K type used to map type codes to underlying types.
 * 
 * @tparam TypeCode : K type code
 * @tparam UnderlierType : Underlying type, as specified by k0 struct.
 */
template<size_t TypeCode, class UnderlierType>
struct Type
{
    static constexpr size_t Code = TypeCode;
    using Underlier = UnderlierType;
};

using Mixed =       Type<0, K>;
using Boolean =     Type<KB, bool>;
using Guid =        Type<UU, std::array<uint8_t, 16>>;
using Byte =        Type<KG, uint8_t>;
using Short =       Type<KH, int16_t>;
using Integer =     Type<KI, int32_t>;
using Long =        Type<KJ, int64_t>;
using Real =        Type<KE, float>;
using Float =       Type<KF, double>;
using Char =        Type<KC, char>;
using Symbol =      Type<KS, char *>;

using Timestamp =   Type<KP, int64_t>;
using Month =       Type<KM, int32_t>;
using Date =        Type<KD, int32_t>;

using Timespan =    Type<KN, int64_t>;
using Minute =      Type<KU, int32_t>;
using Second =      Type<KV, int32_t>;
using Time =        Type<KT, int32_t>;

using Datetime =    Type<KZ, double>;

using Error =       Type<128, char *>;

}

/**
 * @brief Specialise this class to implement a converting type.
 * 
 * @tparam QCode : Pairing of type code to native type
 * @tparam Target : Target type
 * @tparam Enable : If Underlier is not the same as Target
 */
template<
    class QType,
    class TargetType,
    typename Enable>
struct Type
{
    static constexpr size_t Code = QType::Code;
    using Underlier = typename QType::Underlier;
    using Target = TargetType;

    /**
     * @brief Set the value using forwarding. 
     * 
     * @param u : underlier to set to. Do any deallocation as necessary.
     * @param v : value to set underlier to.
     */
    constexpr void setValue(Underlier& u, Target&& v) const
    {
        throw std::invalid_argument("Steal conversion not implemented.");
    }

    /**
     * @brief Set the Value object
     * 
     * @param u : underlier to set to. Do any deallocation as necessary.
     * @param v : value to set underlier to.
     */
    constexpr void setValue(Underlier& u, const Target& v) const
    {
        throw std::invalid_argument("Copy conversion not implemented.");
    }

    const Target getValue(const Underlier& u) const
    {
        throw std::invalid_argument("Retrieval not defined."); 
    }
};

/**
 * @brief The trivial type with identity converters. (Based on std::identity)
 * 
 * @tparam Code 
 * @tparam Underlier 
 * @tparam Target 
 * @tparam std::enable_if<std::is_same<Underlier, Target>::value>::type 
 */
template<
    class QType,
    class TargetType
    >
struct Type<QType, TargetType,
            typename std::enable_if<std::is_same<typename QType::Underlier, TargetType>::value>::type>
{
    static constexpr size_t Code = QType::Code;
    using Underlier = typename QType::Underlier;
    using Target = TargetType;

    constexpr void setValue(Underlier& u, Target&& t) const noexcept
    {
        u = std::forward<Target>(t);
    }

    constexpr Target getValue(Underlier u) const noexcept
    {
        return u;
    }  
};

template <class QType>
using BasicType = Type<QType, typename QType::Underlier>;

using Boolean = BasicType<Q::Boolean>;
using Guid = BasicType<Q::Guid>;
using Byte = BasicType<Q::Byte>;
using Short = BasicType<Q::Short>;
using Integer = BasicType<Q::Integer>;
using Long = BasicType<Q::Long>;
using Real = BasicType<Q::Real>;
using Float = BasicType<Q::Float>;
using Char = BasicType<Q::Char>;

template<>
struct Type<Q::Symbol, std::string_view>
{
    static constexpr size_t Code = KS;
    using Underlier = char*;
    using Target = std::string_view;

    void setValue(Underlier& u, const Target& t) const
    {
        // intern the symbol
        u = ss(const_cast<char*>(t.data()));
    } 

    constexpr Target getValue(Underlier u) const noexcept
    {
        return std::string_view(u);
    }
};

using Symbol = Type<Q::Symbol, std::string_view>;

namespace chrono
{
    using Timestamp = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;
    using Month = std::pair<std::chrono::year, std::chrono::month>;
    using Date = std::chrono::time_point<std::chrono::system_clock, std::chrono::days>;

    using Timespan = std::chrono::nanoseconds;
    using Minute = std::chrono::duration<int32_t, std::ratio<60>>;
    using Second = std::chrono::duration<int32_t>;
    using Time = std::chrono::duration<int32_t, std::milli>;

    using namespace std::literals;
    static constexpr auto epoch = static_cast<std::chrono::sys_days>(std::chrono::year_month_day(2000y, std::chrono::January, 1d));
}

/**
 * @brief Nanoseconds since 2000-01-01
 */
template<>
struct Type<Q::Timestamp, chrono::Timestamp>
{
    static constexpr size_t Code = KP;
    using Underlier = int64_t;
    using Target = chrono::Timestamp;

    void setValue(Underlier& u, const Target& t) const
    {
        u = (t - chrono::epoch).count();
    }

    constexpr Target getValue(Underlier u) const
    {
        return chrono::epoch + std::chrono::nanoseconds(u);
    }
};

using Timestamp = Type<Q::Timestamp, chrono::Timestamp>;

template<>
struct Type<Q::Month, chrono::Month>
{
    static constexpr size_t Code = KM;
    using Underlier = int32_t;
    using Target = chrono::Month;

    void setValue(Underlier& u, const Target& t) const
    {
        const auto [y, m] = t;
        u = ((static_cast<int>(y) - 2000) * 12) + (static_cast<unsigned>(m) - 1);
    }

    constexpr Target getValue(Underlier u) const
    {
        const auto m = u % 12;
        const auto y = ((u - m) / 12) + 2000;
        return {std::chrono::year(y), std::chrono::month(m + 1)};
    }
};

using Month = Type<Q::Month, chrono::Month>;

template<>
struct Type<Q::Date, chrono::Date>
{
    static constexpr size_t Code = KD;
    using Underlier = int32_t;
    using Target = chrono::Date;

    void setValue(Underlier& u, const Target& t) const
    {
        u = (t - chrono::epoch).count();
    }

    constexpr Target getValue(Underlier u) const
    {
        return chrono::epoch + std::chrono::days(u);
    }
};

using Date = Type<Q::Date, chrono::Date>;

template<class QType, class ChronoType>
struct TimeType
{
    static constexpr size_t Code = QType::Code;
    using Underlier = typename QType::Underlier;
    using Target = ChronoType;

    void setValue(Underlier& u, const Target& t) const
    {
        u = t.count();
    }

    constexpr Target getValue(Underlier u) const
    {
        return ChronoType{u};
    }
};

using Timespan = TimeType<Q::Timespan, chrono::Timespan>;
using Minute = TimeType<Q::Minute, chrono::Minute>;
using Second = TimeType<Q::Second, chrono::Second>;
using Time = TimeType<Q::Time, chrono::Time>;

// TODO: narrow cast for safety
// template<>
// class Type<KZ, double, chrono::Timestamp>
// {
// public:

//     void setValue(double& u, const chrono::Timestamp& t) const
//     {
//         u = (t - chrono::epoch).count();
//     }

//     constexpr chrono::Date getValue(double u) const
//     {

//         return chrono::epoch + std::chrono::days(u);
//     }
// };

// using DateTime = Type<KD, double, chrono::Timestamp>;

template<>
struct Type<Q::Error, std::runtime_error>
{
    static constexpr size_t Code = 128;
    using Underlier = char*;
    using Target = std::runtime_error;

    void setValue(Underlier& u, const Target& t) const
    {
        u = const_cast<char*>(t.what());
    }

    Target getValue(Underlier& u) const
    {

        return std::runtime_error(u);
    }
};

using Error = Type<Q::Error, std::runtime_error>;

class UntypedSpan; // forward

template<>
struct Type<Q::Mixed, UntypedSpan>
{
    static constexpr size_t Code = 0;
    using Underlier = K;
    using Target = UntypedSpan;

    /**
     * @brief Set the Value object
     * 
     * @param u 
     * @param t 
     */
    void setValue(Underlier& u, Target t) const
    {
        // preserve old pointer.
        auto old = u;
        // copy across internal m_arr to u
        u = t.m_arr;
        // now has an additional reference so keep track of it.
        r1(u);
        // Old pointer is losing a reference so decrement its reference count.
        r0(old);
    }

    /**
     * @brief Make a span over the K object we're handing out. 
     */
    Target getValue(Underlier& u) const
    {
        return UntypedSpan::SymmetricMemory(u);
    }
};

using Mixed = Type<Q::Mixed, UntypedSpan>;

}