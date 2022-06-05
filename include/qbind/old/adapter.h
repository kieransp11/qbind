#pragma once

#include <array>
#include <chrono>

#include <kx/kx.h>

#include "untyped_span.h"

namespace qbind
{

/**
 * @brief Specialise this class to implement a conversion
 * 
 * @tparam UnderlierType : Type in the underlying array
 * @tparam TargetType : Type to convert the underlying element to
 */
template<class UnderlierType, class TargetType>
struct Adapter
{
    using Underlier = UnderlierType;
    using Target = TargetType;

    // UntypedSpan makeAtom(Target&&) const
    // {
    //     constexpr auto false_ = [](){ return false; };
    //     static_assert(false_(), "No default atom constructor specified");
    // }

    // UntypedSpan makeVector(int64_t size) const
    // {
    //     constexpr auto false_ = [](){ return false; };
    //     static_assert(false_(), "No default vector constructor specified.");
    // }

    /**
     * @brief Set the value using forwarding.
     * 
     * @param u : Underlier to set to. Do any deallocation as necessary
     * @param v : Value to set underlier to
     */
    void setValue(Underlier& u, Target&& v) const
    {
        throw std::invalid_argument( "No known conversion to kdb type");
    }

    /**
     * @brief Set the value using a copy.
     * 
     * @param u : Underlier to set to. Do any deallocation as necessary
     * @param v : Value to set underlier to
     */
    void setValue(Underlier& u, const Target& v) const
    {
        throw std::invalid_argument("No known conversion to kdb type");
    }

    /**
     * @brief Get the underlying value at the target type.
     * 
     * @param u : Underlying value
     * @return const Target : Value of underlying converted to target
     */
    const Target getValue(const Underlier& u) const
    {
        throw std::invalid_argument("No known conversion to cpp type");
    }

};

////////////////////////////////////////////////////////////////////////////////
// Adapters for the default underlying types
////////////////////////////////////////////////////////////////////////////////

    // UntypedSpan makeAtom(type v) const                               \
    // {                                                                \
    //     return atomConstructor;                                      \
    // }                                                                \
    //                                                                  \
    // UntypedSpan makeVector(int64_t size) const                       \
    // {                                                                \
    //     return ktn(vectorType, size);                                \
    // }                                                                \

#define DEFAULT_QBIND_CONVERSION(type)                                   \
    template <>                                                          \
    struct Adapter<type, type>                                           \
    {                                                                    \
        using Underlier = type;                                          \
        using Target = type;                                             \
                                                                         \
        constexpr void setValue(Underlier &u, Target &&t) const noexcept \
        {                                                                \
            u = std::forward<Target>(t);                                 \
        }                                                                \
                                                                         \
        constexpr Target getValue(Underlier u) const noexcept            \
        {                                                                \
            return u;                                                    \
        }                                                                \
    };

DEFAULT_QBIND_CONVERSION(bool);
DEFAULT_QBIND_CONVERSION(uint8_t);
DEFAULT_QBIND_CONVERSION(int16_t);
DEFAULT_QBIND_CONVERSION(int32_t);
DEFAULT_QBIND_CONVERSION(int64_t);
DEFAULT_QBIND_CONVERSION(float);
DEFAULT_QBIND_CONVERSION(double);
DEFAULT_QBIND_CONVERSION(char);
DEFAULT_QBIND_CONVERSION(char*);

template<>
struct Adapter<U, std::array<uint8_t, 16>>
{
    using Underlier = U;
    using Target = std::array<uint8_t, 16>;

    /**
     * @brief Set the value using forwarding.
     * 
     * @param u : Underlier to set to. Do any deallocation as necessary
     * @param v : Value to set underlier to
     */
    constexpr void setValue(Underlier& u, Target&& v) const
    {
        u = reinterpret_cast<Underlier&>(v);
    }

    /**
     * @brief Set the value using a copy.
     * 
     * @param u : Underlier to set to. Do any deallocation as necessary
     * @param v : Value to set underlier to
     */
    constexpr void setValue(Underlier& u, const Target& v) const
    {
        u = reinterpret_cast<Underlier&>(const_cast<Target&>(v));
    }

    /**
     * @brief Get the underlying value at the target type.
     * 
     * @param u : Underlying value
     * @return const Target : Value of underlying converted to target
     */
    const Target getValue(const Underlier& u) const
    {
        return reinterpret_cast<Target&>(const_cast<Underlier&>(u));
    }
};

////////////////////////////////////////////////////////////////////////////////
// Non-default converters to more useful types
////////////////////////////////////////////////////////////////////////////////

// Symbol -> string view
template<>
struct Adapter<char *, std::string_view>
{
    using Underlier = char *;
    using Target = std::string_view;

    void setValue(Underlier& u, const Target& v) const
    {
        // interns the symbol
        u = ss(const_cast<char*>(v.data()));
    }

    const Target getValue(const Underlier& u) const
    {
        return std::string_view(u);
    }
};

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
struct Adapter<int64_t, chrono::Timestamp>
{
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

template<>
struct Adapter<int32_t, chrono::Month>
{
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

template<>
struct Adapter<int32_t, chrono::Date>
{
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

#define TIME_QBIND_CONVERSION(underlier, target)                              \
    template <>                                                               \
    struct Adapter<underlier, target>                                         \
    {                                                                         \
        using Underlier = underlier;                                          \
        using Target = target;                                                \
                                                                              \
        constexpr void setValue(Underlier &u, const Target& t) const noexcept \
        {                                                                     \
            u = t.count();                                                    \
        }                                                                     \
                                                                              \
        constexpr Target getValue(Underlier u) const noexcept                 \
        {                                                                     \
            return Target{u};                                                 \
        }                                                                     \
    };

TIME_QBIND_CONVERSION(int64_t, chrono::Timespan);
TIME_QBIND_CONVERSION(int32_t, chrono::Minute);
TIME_QBIND_CONVERSION(int32_t, chrono::Second);
TIME_QBIND_CONVERSION(int32_t, chrono::Time);

// For errors
template<>
struct Adapter<char*, std::runtime_error>
{
    using Underlier = char *;
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

// For mixed
template<>
struct Adapter<K, UntypedSpan>
{
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

// TODO: Make atom needs to be a struct with an operator so that
// the MakeAtom<T>::make(T) can be instantiated if an untyped span/
// span/map/table isn't returned. This will help with constructions
// for i.e. std::tuple
//
// Type has underlying, and code. 
//  - That means you can construct vector fine.
//  - 

// IterAdapter -> Allows K object to be accessed as the target type with the
//                underlying K object type
// AtomAdapter -> Take a C++ object and convert it to the atom with the right
//                type. Has to be separate as this setting K object type properly.
//                This will be able to use static assert for default implementation
//                as it just uses return type of function and not visit to over
//                specify number of template instantiations.

}