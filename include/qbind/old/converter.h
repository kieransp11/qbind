#pragma once

#include <tuple>
#include <utility>

#include <kx/kx.h>

#include "type.h"
#include "span.h"
#include "map.h"
#include "utils.h"

namespace qbind
{

template <class, template <class...> class>
struct is_instance : public std::false_type {};

template <class...Ts, template <class...> class U>
struct is_instance<U<Ts...>, U> : public std::true_type {};

class Converter
{
public:
    // TODO:
    // SFINAE here for
    // - instance of type (getting atoms/singletons)
    // - span<T> (getting arrays where T is instance of Type)
    // - map<K,V> (getting map where K and V are instances of Type)
    // - table<Column...> (getting table for multiple columns)
    // - tuple<any of the above...>

    // The methods should do recursive type checks and construct results as
    // necessary. 
    template<class T>
    auto to_cpp(K arr)
    {
        if constexpr (is_instance<T, Span>::value)
            return Visit([](auto qtype, auto arr)
                         { 
                             return span_to_cpp<T>(qtype, arr); 
                         },
                         arr);
        // else if constexpr (is_instance<T, Map>::value)
        //     return Visit([](auto qtype, auto arr)
        //                 { 
        //                     return map_to_cpp<T>(qtype, arr); 
        //                 },
        //                 arr);
        // else if constexpr (is_instance<T, Table>::value)
        //     return Visit([](auto qtype, auto arr)
        //                 { 
        //                     return table_to_cpp<T>(qtype, arr); 
        //                 },
        //                 arr);
        else if constexpr (is_instance<T, std::tuple>::value)
            return Visit([](auto qtype, auto arr)
                        {
                            return tuple_to_cpp<T>(qtype, arr);
                        },
                        arr);
        else // assume atom
            return Visit([](auto qtype, auto arr)
                         { 
                             return atom_to_cpp<T>(qtype, arr); 
                         },
                         arr);
    }

    template<class T>
    K to_q(T value)
    {
        // is_instance<T, Table>::value
        // if constexpr (is_instance<T, Map>::value)                  return value.to_q();
        if constexpr (is_instance<T, std::tuple>::value)           return tuple_to_q(value);
        else if constexpr (is_instance<T, Span>::value)                     return value.span().to_q();
        // Atom - only accept default values.
        else if constexpr (std::is_same_v<T, UntypedSpan>)               return value.to_q();
        else if constexpr (std::is_same_v<T, bool>)                      return kb(value);
        else if constexpr (std::is_same_v<T, std::array<uint8_t, 16>>)   return ku(U{.g = value});
        else if constexpr (std::is_same_v<T, uint8_t>)                   return kg(value);
        else if constexpr (std::is_same_v<T, int16_t>)                   return kh(value);
        else if constexpr (std::is_same_v<T, int32_t>)                   return ki(value);
        else if constexpr (std::is_same_v<T, int64_t>)                   return kj(value);
        else if constexpr (std::is_same_v<T, float>)                     return ke(value);
        else if constexpr (std::is_same_v<T, double>)                    return kf(value);
        else if constexpr (std::is_same_v<T, char>)                      return kc(value);
        else if constexpr (std::is_same_v<T, std::string_view>)          return ks(value.data());
        else if constexpr (std::is_same_v<T, chrono::Timestamp>)         return ktj(-KP, (value - chrono::epoch).count());
        else if constexpr (std::is_same_v<T, chrono::Month>)
        {
            const auto [y, m] = value;
            return kti(-KM, ((static_cast<int>(y) - 2000) * 12) + (static_cast<unsigned>(m) - 1));
        }
        else if constexpr (std::is_same_v<T, chrono::Date>)              return kd((value - chrono::epoch).count());
        else if constexpr (std::is_same_v<T, chrono::Timespan>)          return ktj(-KN, value.count());
        else if constexpr (std::is_same_v<T, chrono::Minute>)            return kti(-KU, value.count());
        else if constexpr (std::is_same_v<T, chrono::Second>)            return kti(-KV, value.count());
        else if constexpr (std::is_same_v<T, chrono::Time>)              return kti(-KT, value.count());
        else
        {
            constexpr auto false_ = [](){ return false; };
            static_assert(false_(), "No known conversion to kdb type");    
        }
    }

private:

    template<class T, class QType>
    static T atom_to_cpp(QType type, K arr)
    {
        UntypedSpan span(arr);
        if (span.size() != 1)
            throw std::invalid_argument("Cannot convert non-singleton vector to atom");
        Adapter<typename QType::Underlier, T> adapter;
        // converts singleton or atom
        return adapter.getValue(*reinterpret_cast<
            typename QType::Underlier*>(span.data()));
    }

    template<class T, class QType>
    static T span_to_cpp(QType type, K arr)
    {
        if (arr->t == XT)
            throw std::invalid_argument("Cannot convert table to span");
        if (arr->t == XD)
            throw std::invalid_argument("Cannot convert table to dictionary");
        return T(UntypedSpan(arr));
    }

    template<class T>
    static T map_to_cpp(K arr)
    {
        if (arr->t != XD)
            throw std::invalid_argument("Array is not a map");
        return T(arr);
    }

    template<class T>
    static T table_to_cpp(K arr)
    {
        if (arr->t != XT)
            throw std::invalid_argument("Array is not a table");
    }

    /**
     * @brief Convert a mixed list to a tuple
     * 
     * @tparam T: The tuple type
     * @tparam QType: Type of list (expect mixed)
     * @param type: type to convert to (expect mixed)
     * @param arr: Mixed array with same length as tuple
     * @return T: Tuple containing each element of arr converted.
     */
    template<class T, class QType>
    static T tuple_to_cpp(QType type, K arr)
    {
        if (arr->t != 0)
            throw std::invalid_argument("Only mixed arrays can be converted to tuples");
        if (std::tuple_size<T>{} != arr->n)
            throw std::invalid_argument("Tuple arity does not match array length");

        return tuple_to_cpp_impl(arr, T(), std::make_index_sequence<std::tuple_size<T>{}>{});
        // TODO: should this decrement reference count of arr here as it is no longer used
    }

    template<class ...Args, size_t ... Idxs>
    static std::tuple<Args...> tuple_to_cpp_impl(K arr, std::tuple<Args...> t, std::index_sequence<Idxs...> idxs)
    {
        return std::make_tuple(
            Converter{}.to_cpp<Args>(((K*)(arr->G0))[Idxs])...
        );
    }

    /**
     * @brief Take the elements of a tuple and convert them individually into a mixed list.
     * 
     * @tparam Args: Types of elements in the tuple
     * @param value: The tuple
     * @return K: Mixed list containing the conversions from the tuple.
     */
    template<class ...Args>
    static K tuple_to_q(std::tuple<Args...> value)
    {
        return tuple_to_q_impl(std::move(value), std::make_index_sequence<sizeof...(Args)>{});
    }

    template<class ...Args, size_t ...Idxs>
    static K tuple_to_q_impl(std::tuple<Args...> value, std::tuple<Args...> t, std::index_sequence<Idxs...> idxs)
    {
        knk(sizeof...(Args),
            Converter{}.to_q<Args>(std::get<Idxs>(value))...);
    }
};

}