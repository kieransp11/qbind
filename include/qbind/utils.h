#pragma once

#include <type_traits>

#include "type.h"

namespace qbind::internal
{
    
template <class, template <class...> class>
struct is_instance_class : public std::false_type {};

template <class...Ts, template <class...> class U>
struct is_instance_class<U<Ts...>, U> : public std::true_type {};

template<class, template <Type...> class>
struct is_instance_type : public std::false_type {};

template <Type...Ts, template <Type...> class U>
struct is_instance_type<U<Ts...>, U> : public std::true_type {};

// Symbol helpers

#define ENABLE_IF_SYMBOL template <                             \
        bool Cond = T==Type::Symbol,                           \
        typename = typename std::enable_if<Cond, void>::type    \
    >

#define DISABLE_IF_SYMBOL template <                            \
        bool Cond = T!=Type::Symbol,                           \
        typename = typename std::enable_if<Cond, void>::type    \
    >

/**
 * @brief Find index of T in U,Us...
 */
template <typename T, typename U, typename... Us>
class index_of
{
public:
    static constexpr size_t value = type_to_idx<T, U, Us...>();

private:

    template<typename _T, typename _U, typename... _Us>
    constexpr size_t type_to_idx(size_t idx = 0)
    {
        if constexpr (std::is_same_v<T,U>)
            return idx;
        static_assert(sizeof...(Us) > 0, "Failed to find index of type");
        return type_to_idx<T, Us...>(idx + 1);        
    }
};

template <typename T, typename... Ts>
constexpr std::size_t index_of_v = index_of<T, Ts...>::value;

}