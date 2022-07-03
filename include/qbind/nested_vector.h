#pragma once

#include <variant>

#include <kx/kx.h>

#include "atom.h"
#include "k.h"
#include "utils.h"
#include "vector.h"

namespace qbind
{

/**
 * @brief A nested vector of homogeneous type.
 * 
 * All values in the object must be an atom, vector, or vector
 * containing only those of type T, recursively. Depth may not
 * be uniform.
 * 
 * Must be at least iterable at first level. i.e. may be flat vector.
 * 
 * @tparam T 
 */
template <Type T>
class NestedVector
{
public:
    
    static constexpr Type type = T;
    static constexpr Structure structure = Structure::NestedVector;

    using underlier = typename internal::c_type<T>::underlier;
    using value     = typename internal::c_type<T>::value;
    using reference = std::variant<
        std::monostate,                 // Index does not exist
        typename internal::c_type<T>::reference, // Index inside vector
        Atom<T>,                        // Atom
        Vector<T>,                      // Vector
        NestedVector<T>>;               // Nested Vector
    using const_reference = std::variant<
        std::monostate,                         // Index does not exist
        typename internal::c_type<T>::const_reference,   // Index inside vector
        const Atom<T>,                          // Atom
        const Vector<T>,                        // Vector
        const NestedVector<T>>;                 // Nested Vector
    // No pointer or const_pointer as not exposing data

    NestedVector(K data)
    :m_ptr(std::move(data))
    {
        if (!m_ptr)
            throw std::runtime_error("K is empty");
        m_ptr.is_with_info<NestedVector<T>>();
    }

    template<class... Args>
    NestedVector(Args&&... args)
    {
        size_t idx = 0;

        ([&] ()
        {
            if (!args.get())
            {
                throw std::runtime_error("K behind value at index " + std::to_string(idx) + " is empty.");
            }
            ++idx;
            static_assert(std::is_same_v<Args,Atom<T>> ||
                          std::is_same_v<Args,Vector<T>> ||
                          std::is_same_v<Args,NestedVector<T>> , "Not correct type");
        } (), ...);

        // get returns an independent copy of the pointer so knk can take ownership
        m_ptr = K{knk(sizeof...(Args),
                    args.get().release()...
                    )};
    }

    // at: Will throw instead of monostate.
    reference at(const std::vector<size_t>& idxs)
    {
        return get_idx<true>(idxs);
    }

    const_reference at(const std::vector<size_t>& idxs) const
    {
        auto res = get_const_result(get_idx<true>(idxs));
    }

    // []: returns monostate if index is not available
    reference operator[](const std::vector<size_t>& idxs)
    {
        return get_idx<false>(idxs);
    }

    const_reference operator[](const std::vector<size_t>& idxs) const
    {
        auto res = get_const_result(get_idx<false>(idxs));
    }

    // front and back do with [] like vector.
    reference front()
    {
        return this[0];
    }

    const_reference front() const
    {
        return get_const_result(this[0]);
    }

    reference back()
    {
        return this[m_ptr.size() - 1];
    }

    const_reference back() const
    {
        return get_const_result(this[m_ptr.size() - 1]);
    }

    // skip data

    // Iterators
    // begin/cbegin
    // end/cend
    // rbegin/crbegin
    // rend/crend

    // Capacity
    // reserve, capacity, and shrink_to_fit don't make sense given memory management.
    bool empty() const noexcept
    {
        return size() != 0;
    }

    size_t size() const noexcept
    {
        return m_ptr.size();
    }

    size_t max_size() const noexcept
    {
        return std::numeric_limits<decltype(::k0::n)>::max();
    }

    // Modifiers
    // erase, emplace_back, swap don't make sense given memory management.
    // TODO: clear, insert, emplace, emplace_back, swap

    /**
     * @brief Add underlier if root of nested vector is flat vector
     * 
     * @param value Value to add.
     */
    void push_back(value value)
    {
        if (!m_ptr.is<Vector<T>>())
            throw std::runtime_error("Nested vector is not flat, cannot append underlier.");
        m_ptr.join_atom(to_underlier(value));
    }

    // Add atom if root of nested vector is tuple
    void push_back(Atom<T> value)
    {
        if (m_ptr.is<Atom<T>>() || m_ptr.is<Vector<T>>())
            throw std::runtime_error("Nested vector is not tuple-based, cannot append atom.");
        m_ptr.tuple_append(value.get());
    }

    // Add vector if root of nested vector is tuple
    void push_back(Vector<T> value)
    {
        if (m_ptr.is<Atom<T>>() || m_ptr.is<Vector<T>>())
            throw std::runtime_error("Nested vector is not tuple-based, cannot append atom.");
        m_ptr.tuple_append(value.get());
    }

    // Add vector if root of nested vector is tuple
    void push_back(NestedVector<T> value)
    {
        if (m_ptr.is<Atom<T>>() || m_ptr.is<Vector<T>>())
            throw std::runtime_error("Nested vector is not tuple-based, cannot append atom.");
        m_ptr.tuple_append(value.get());
    }

    // Operator
    // ==
    bool operator==(const NestedVector& rhs) const
    {
        return m_ptr == rhs.m_ptr;
    }
    bool operator!=(const NestedVector& rhs) const
    {
        return m_ptr != rhs.m_ptr;
    }

    K get() const
    {
        return m_ptr;
    }

private:

    static constexpr typename internal::c_type<T>::to_underlier to_underlier;

    K m_ptr;

    // KX get_idx
    static ::K get_idx(const ::K k, const size_t idx)
    {
        return reinterpret_cast<::K *>(k->G0)[idx];
    }

    // Qbind get_idx
    static ::K get_idx(const K k, const size_t idx)
    {
        return k.data<::K>()[idx];
    }

    template<bool throws>
    std::monostate return_or_throw(std::string err)
    {
        if constexpr (throws)
        {
            throw std::runtime_error(std::move(err));
        }
        return std::monostate{};
    }

    template<bool throws>
    reference get_idx(const std::vector<size_t>& idxs)
    {
        // catch all. Technically not indexing is this.
        if (idxs.empty())
            return this;

        if (idxs.front() >= m_ptr.size())
            return return_or_throw<throws>("Index out of range on first index.");

        if (m_ptr.is<Atom<T>>())
            return return_or_throw<throws>("Attempted to index atom on first index.");

        if (m_ptr.is<Vector<T>>())
        {
            if (idxs.size() != 1)
                return return_or_throw<throws>("Not on last index. Attempted to index flat vector on index 0");
            return Vector<T>(m_ptr)[idxs.front()];
        }

        ::K current_ptr = get_idx(m_ptr, idxs.front());

        auto it = std::next(idxs.cbegin(), 1);
        while (it != idxs.cend())
        {
            if (!current_ptr)
                return return_or_throw<throws>("Found nullptr inside nested vector.");
            if (current_ptr->t < 0)
                return return_or_throw<throws>("Attempted to index atom at index " + std::to_string(it - idxs.begin()));
            if (*it >= current_ptr->n)
                return return_or_throw<throws>("Index out of range at index " + std::to_string(it - idxs.begin()));
            if (current_ptr->t > 0)
            {
                if (it != std::prev(idxs.end(), 1))
                    return return_or_throw<throws>("Not on last index. Attempted to index flat vector on index " + std::to_string(it - idxs.begin()));
                return Vector<T>(K::make_non_owning(current_ptr))[idxs.front()];
            }
        
            current_ptr = get_idx(current_ptr, *it);
            ++it;
        }

        if (!current_ptr)
            return return_or_throw<throws>("Found nullptr inside nested vector.");

        auto k = K::make_non_owning(current_ptr);
        if (current_ptr->t < 0)
            return Atom<T>{std::move(k)};
        if (current_ptr->t > 0)
            return Vector<T>{std::move(k)};
        return NestedVector<T>{std::move(k)};
    }

    const_reference get_const_result(reference res)
    {
        if (std::holds_alternative<std::monostate>(res)) return std::monostate{};
        if (std::holds_alternative<Atom<T>>(res)) return std::get<Atom<T>>(res);
        if (std::holds_alternative<Vector<T>>(res)) return std::get<Vector<T>>(res);
        if (std::holds_alternative<NestedVector<T>>(res)) return std::get<NestedVector<T>>(res);
        return std::get<typename internal::c_type<T>::reference>(res);
    }
};
}