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
    
    static constexpr TypeClass TypeInfo{static_cast<short int>(T), true};

    using Underlier = typename internal::c_type<T>::Underlier;

    // What is actually returned (const accessors)
    using result = std::conditional_t<T == Type::Symbol, const char *, Underlier>;    

    // Const reference to const char *. This comes from const T& === T const&.
    using const_ref_to_ptr_to_const = const char *const &;

    using result_t = std::variant<
        std::monostate, 
        std::conditional_t<T == Type::Symbol, SymbolReference, Underlier &>, 
        Atom<T>, 
        Vector<T>, 
        NestedVector<T>>;
    using const_result_t = std::variant<
        std::monostate, 
        std::conditional_t<T == Type::Symbol, const char*, const Underlier &>, 
        const Atom<T>, 
        const Vector<T>, 
        const NestedVector<T>>;

    NestedVector(K data)
    :m_ptr(std::move(data))
    {
        check_type(m_ptr);
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

        m_ptr = K{knk(sizeof...(Args),
                    args.get().get()...
                    )};
    }

    // at: Will throw instead of monostate.
    result_t at(const std::vector<size_t>& idxs)
    {
        return get_idx<true>(idxs);
    }

    const_result_t at(const std::vector<size_t>& idxs) const
    {
        auto res = get_const_result(get_idx<true>(idxs));
    }

    // []: returns monostate if index is not available
    result_t operator[](const std::vector<size_t>& idxs)
    {
        return get_idx<false>(idxs);
    }

    const_result_t operator[](const std::vector<size_t>& idxs) const
    {
        auto res = get_const_result(get_idx<false>(idxs));
    }

    // front and back do with [] like vector.
    result_t front()
    {
        return this[0];
    }

    const_result_t front() const
    {
        return get_const_result(this[0]);
    }

    result_t back()
    {
        return this[m_ptr.size() - 1];
    }

    const_result_t back() const
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
    void push_back(Underlier value)
    {
        if (!m_ptr.typeClass().isVector())
            throw std::runtime_error("Nested vector is not flat, cannot append underlier.");
        if constexpr(T == Type::Symbol)
        {
            m_ptr.join_symbol(value);
        }
        else
        {
            m_ptr.join_atom(&value);
        }
    }

    // Add atom if root of nested vector is tuple
    void push_back(Atom<T> value)
    {
        if (!m_ptr.typeClass().isTuple())
            throw std::runtime_error("Nested vector is flat, cannot append atom to flat vector.");
        m_ptr.tuple_append(value.get());
    }

    // Add vector if root of nested vector is tuple
    void push_back(Vector<T> value)
    {
        if (!m_ptr.typeClass().isTuple())
            throw std::runtime_error("Nested vector is flat, cannot append vector to flat vector.");
        m_ptr.tuple_append(value.get());
    }

    // Add vector if root of nested vector is tuple
    void push_back(NestedVector<T> value)
    {
        if (!m_ptr.typeClass().isTuple())
            throw std::runtime_error("Nested vector is flat, cannot append nested vector to flat vector.");
        m_ptr.tuple_append(value.get());
    }

    // Operator
    // ==
    // <=> : Lexicographical ordering

    K get() const
    {
        return m_ptr;
    }

private:

    K m_ptr;

    // KX get_idx
    static ::K get_idx(const ::K k, const size_t idx)
    {
        return reinterpret_cast<::K *>(k->G0)[idx];
    }

    // Qbind get_idx
    static ::K get_idx(const K k, const size_t idx)
    {
        return reinterpret_cast<::K *>(k.data())[idx];
    }

    static void check_type_impl(::K k)
    {
        // dont allow null pointers
        if (!k)
            throw std::runtime_error("Nested vector contains nullptr");
        // vector or atom.
        if (abs(k->t) == static_cast<signed char>(T))
            return;
        // wrong type
        if (k->t != 0)
        {
            std::ostringstream ss;
            ss << "Nested Vector of type " << TypeClass::name(T) << " should not contain " << TypeClass(k->t);
            throw std::runtime_error(ss.str());
        }
        // recurse nested
        for (auto i = 0; i < k->n; ++i)
            check_type(get_idx(k, i));
    }


    // for type checking
    template <class... Args>
    friend class Tuple;

    static void check_type(K k)
    {
        if (!k)
            throw std::runtime_error("K is empty");
        auto tc = k.typeClass();
        if (tc.isVector() && tc.type() == T)
            return;
        if (tc.isTuple())
        {
            for (auto i = 0; i < k.size(); ++i)
            {
                check_type_impl(get_idx(k, i));
            }
            return;
        }
        throw std::runtime_error("K must be vector or tuple at first level");
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
    result_t get_idx(const std::vector<size_t>& idxs)
    {
        // catch all. Technically not indexing is this.
        if (idxs.empty())
            return this;

        if (idxs.front() >= m_ptr.size())
            return return_or_throw<throws>("Index out of range on first index.");

        if (m_ptr.typeClass().isVector())
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

    const_result_t get_const_result(result_t res)
    {
        if (std::holds_alternative<std::monostate>(res)) return std::monostate{};
        if (std::holds_alternative<Atom<T>>(res)) return std::get<Atom<T>>(res);
        if (std::holds_alternative<Vector<T>>(res)) return std::get<Vector<T>>(res);
        return std::get<std::conditional_t<T==Type::Symbol, SymbolReference, Underlier &>>(res);
    }
};
}