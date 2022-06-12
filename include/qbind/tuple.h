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
        m_ptr.is_with_info<Tuple<Types...>>(m_ptr);
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

    static constexpr Structure Structure = Structure::Tuple;

    K get() const
    {
        return m_ptr;
    }

private:

    K m_ptr;

    ::K get_idx(size_t pos)
    {
        return static_cast<::K *>(m_ptr.data())[pos];
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