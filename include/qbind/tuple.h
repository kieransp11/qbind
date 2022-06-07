#pragma once

#include <kx/kx.h>

#include "k.h"
#include "type.h"
#include "utils.h"

namespace qbind
{

template<class... Types>
class Tuple
{
public:

    Tuple(K data)
    :m_ptr(std::move(data))
    {
        if (!m_ptr)
            throw std::runtime_error("K is empty");
        check_type_match(m_ptr.get());
    }

    Tuple(Types&&... elements)
    {
        size_t idx = 0;

        ([&] ()
        {
            // Do things in your "loop" lambda
            if (!elements.get())
            {
                throw std::runtime_error("K behind value at index " + std::to_string(idx) + " is empty.");
            }
            ++idx;
        } (), ...);

        // get returns an independent copy of the pointer so knk can take ownership
        m_ptr = K{knk(sizeof...(Types),
                    elements.get().get()...
                    )};
    }

    //  TODO: Move to std::get like method.
    template <size_t Idx>
    typename std::tuple_element<Idx, Tuple<Types...>>::type get()
    {
        return {K::make_non_owning(get_idx(Idx))};
    }

    template <size_t Idx>
    const typename std::tuple_element<Idx, Tuple<Types...>>::type get() const
    {
        return {K::make_non_owning(get_idx(Idx))};
    }

    template<typename T>
    typename std::tuple_element<
        internal::index_of_v<T, Types...>, 
        Tuple<Types...>
    >::type get()
    {
        return T{K::make_non_owning(
            get_idx(
                internal::index_of_v<T, Types...>
            )
        )};
    }

    template<typename T>
    const typename std::tuple_element<
        internal::index_of_v<T, Types...>, 
        Tuple<Types...>
    >::type get() const
    {
        return T{K::make_non_owning(
            get_idx(
                internal::index_of_v<T, Types...>
            )
        )};
    }

    // TODO: Move to std::tuple_cat like method.
    /**
     * @brief On append a new tuple is returned. The existing tuple remains as
     * a view. Therefore an edit to a common index will edit both tuples.
     * 
     * Type checking on this is already done, so fine to mutate m_ptr underneath
     * as long as the returned tuple is constructed in a valid state. That leaves
     * this as an appropriate constrained view.
     */
    template<typename T>
    Tuple<Types..., T> cat(T value)
    {
        m_ptr.tuple_append(value.get().get());
        return {m_ptr};
    }

    template<class... Types2>
    Tuple<Types..., Types2...> cat(Tuple<Types2...> other)
    {
        for (size_t i = 0; i < other.m_ptr.size(); ++i)
        {
            m_ptr.tuple_append(other.get_idx(i));
        }
        return {m_ptr};
    }

    static constexpr TypeClass TypeInfo{0};

    K get() const
    {
        return m_ptr;
    }

private:

    K m_ptr;

    ::K get_idx(size_t pos)
    {
        return *(static_cast<::K *>(m_ptr.data()) + pos);
    }

    // Type checking

    template<size_t Depth, size_t Idx, class Type, class... InnerTypes>
    static void check_type_match_impl(::K data)
    {
        ::K entry = *(reinterpret_cast<::K*>(data->G0) + Idx);
        // If type is a tuple recurse depth and start at index 0
        if constexpr (internal::is_instance_class<Type, Tuple>::value)
        {
            Type::template check_type_match<Depth+1, 0>(entry);
            return;
        }

        // TODO: Map<Vector, Vector>, Map<Vector, Tuple>, Map<Tuple, Vector>, Map<Tuple, Tuple>
        // TODO: Table<Args...>.
        // TODO: Keyed Table ??

        // If type is atom/vector check type info matches.
        if (TypeClass(entry->t) != Type::TypeInfo)
        {
            std::ostringstream ss;
            ss << "At depth " << Depth << " Index " << Idx << " Expected " << Type::TypeInfo << " Found " << TypeClass(entry->t);
            throw std::runtime_error(ss.str());
        }

        // if there's more entries keep going
        if constexpr (0 < sizeof...(InnerTypes))
            check_type_match_impl<Depth, Idx + 1, InnerTypes...>(data);
    }

    /**
     * @brief Start iterating across the tuples types checking their TypeInfo
     * across the provided KX array object.
     * 
     * @tparam Depth: Depth of search, starting at 0.
     * @tparam Idx: Idx in arguments, starting at 0.
     * @param data 
     */
    template<size_t Depth, size_t Idx>
    static ::K check_type_match(::K data)
    {
        if (data->t != 0)
            throw std::runtime_error("Not tuple at Depth: " + std::to_string(Depth) + " Index: " + std::to_string(Idx));
        if (data->n != sizeof...(Types))
            throw std::runtime_error("Bad length at Depth: " + std::to_string(Depth) + " Index: " + std::to_string(Idx) +
                                    "Expected: " + std::to_string(sizeof...(Types)) + " Found: " + std::to_string(data->n));
        check_type_match_impl<Depth, Idx, Types...>(data);
        return data;
    }

};

// TODO: make_tuple method.

} // namespace qbind

// Specialisations for structured bindings
namespace std
{
    template<class ...Types>
    struct tuple_size<qbind::Tuple<Types...>> : integral_constant<size_t, sizeof...(Types)> {};

    template<size_t Idx, class ...Types>
    struct tuple_element<Idx, qbind::Tuple<Types...>> : std::tuple_element<Idx, std::tuple<Types...>> {};
}