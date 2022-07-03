#pragma once

#include "k.h"
#include "type.h"

#include "atom.h"
#include "vector.h"
#include "nested_vector.h"

#include "utils.h"

// A dictionary comes from two lists.
// We have two types of the lists: simple (aka vector), general (aka tuple).

// Maps are allowable in all four forms:
// simple -> simple
// simple -> general
// simple -> nested
// nested -> simple
// nested -> nested
// nested -> general
// general -> simple
// general -> nested
// general -> general.
namespace qbind
{

namespace internal
{

/**
 * @brief Abstraction over keys.
 * 
 * Find the index of a value if present. Provides the correct access
 * depending on the key structure type.
 */
template<typename T>
class Keys
{
public:

// Vector/Nested Vector

/**
 * @brief Get index of value in Vector/NestedVector
 * 
 * TODO: Need == on nested vector
 */
template <typename U>
typename std::enable_if_t<
    is_instance_type_v<T, Vector> || is_instance_type_v<T, NestedVector>, 
    size_t>
static find(const T& keys, U value)
{
    // impl for vector
    if constexpr (is_instance_type_v<T, Vector>)
    {
        static_assert(std::is_same_v<U, T::value>, "Cannot search flat vector for type it doesn't contain");
        return std::find(keys.begin(), keys.end(), value) - keys.begin();
    }
    // impl for nested vector
    for (size_t idx = 0; idx < keys.size(); ++ idx)
    {
        auto &idx_v = keys[{idx}];
        if (std::holds_alternative<U>(idx_v) && idx_v == value)
            return idx;
    }
    return keys.size();
}

template <typename U>
typename std::enable_if_t<
    is_instance_type_v<T, Vector> || is_instance_type_v<T, NestedVector>, 
    size_t>
static count(const T& keys, U value)
{
    // impl for vector
    if constexpr (is_instance_type_v<T, Vector>)
    {
        static_assert(std::is_same_v<U, T::value>, "Cannot search flat vector for type it doesn't contain");
        return std::count(keys.begin(), keys.end(), value);
    }
    size_t count = 0;
    // impl for nested vector
    for (size_t idx = 0; idx < keys.size(); ++ idx)
    {
        auto &idx_v = keys[{idx}];
        if (std::holds_alternative<U>(idx_v) && idx_v == value)
            ++count;
    }
    return count;
}

// Tuple

template<typename U>
typename std::enable_if_t<is_instance_class_v<T, Tuple>, size_t>
static find(const T& keys, U value)
{
    return T::template find_impl(keys, value);
}

template<typename U>
typename std::enable_if_t<is_instance_class_v<T, Tuple>, size_t>
static count(const T& keys, U value)
{
    return T::template count_impl(keys, value);
}

private:

template<class U, class... Types>
static size_t find_impl(const T& keys, U value)
{
    size_t idx = 0;
    ([&] ()
    {
        if constexpr (std::is_same_v<U,Types>)
        {
            if (value == keys.template get<idx>())
                return idx;
        }
        ++idx;            
    } (), ...);
    return idx;
}

template<class U, class... Types>
static size_t count_impl(const T& keys, U value)
{
    size_t idx = 0;
    size_t count = 0;
    ([&] ()
    {
        if constexpr (std::is_same_v<U,Types>)
        {
            if (value == keys.template get<idx>())
                ++count;
        }
        ++idx;            
    } (), ...);
    return count;
}

};

template<class T>
class Values
{
public:

typename std::enable_if_t<
    is_instance_type_v<T, Vector> || is_instance_type_v<T, NestedVector>, 
    typename T::reference>
static get(T& vs, size_t idx)
{
    return vs[idx];
}

typename std::enable_if_t<
    is_instance_type_v<T, Vector> || is_instance_type_v<T, NestedVector>,  
    typename T::const_reference>
static cget(const T& vs, size_t idx)
{
    return vs[idx];
}

/**
 * @brief Make the desired type from the underlying ::K array. The
 * constructor of the desired type should throw if theres any issues.
 * 
 * @tparam U 
 */
template <typename U>
typename std::enable_if_t<is_instance_class_v<T, Tuple>, U>
static get(const K &vs, size_t idx)
{
    return K::make_non_owning(vs.data<::K>()[idx]);
};

template <typename U>
typename std::enable_if_t<is_instance_class_v<T, Tuple>, const U>
static cget(const K &vs, size_t idx)
{
    return K::make_non_owning(vs.data<::K>()[idx]);
}

};

}

// There are effectively 9 types of dictionary we define over qbind types
//
//                | Vector | Nested Vector | Tuple |
// ---------------+--------+---------------+-------+
//         Vector |
//  Nested Vector |
//          Tuple |
//
// Keys can be Vector/Nested Vector/Tuple. Lookup initially involves mapping
// from value to index. We then map from index to an output type and value.
//
// Getting the index is always simple. For a vector iterate. For a nested
// vector iterate the top level and find where the key value given is
// congruent to the value in the nested vector. For tuple get all indices
// where the key type matches the tuple element types, then check for
// congruence.
//
// Mapping from index to value is more difficult. For Vector/Nested Vector
// values we can return the value returned by operator[].
// For Tuples mapping to type is harder. For example:
// Dictionary<Tuple<Atom<Int>,Atom<Int>,Atom<Double>>,
//                  Atom<Int>,Atom<Long>,Atom<Long>>.
// We can accept an Atom<Int> as key but won't know until runtime if the
// result type is Atom<Int> or Atom<Long>. Therefore in the case where the
// value type is a tuple an expected result type must be provided.
template<class TKey, class TValue>
class Dictionary
{
public:
    using KeyType = TKey;
    using ValueType = TValue;
    static constexpr Structure structure = Structure::Dictionary;

    Dictionary(K data)
    :m_ptr(std::move(data))
    {
        if (!m_ptr)
            throw std::runtime_error("K is empty");
        m_ptr.is_with_info<Dictionary<TKey, TValue>>();
        m_length = static_cast<::K *>(m_ptr.data())[0]->n;
    }

    Dictionary(TKey ks, TValue vs)
    {
        // Make sure same length. Conformance to types guaranteed.
        if (ks.size() != vs.size())
            throw std::runtime_error("Keys and value must be same length");
        m_ptr = K{xD(ks.get().release(), vs.get().release())};
        m_length = ks.get().size();
    }

    TKey keys() const
    {
        return {K::make_non_owning(static_cast<::K *>(m_ptr.data())[0])};
    }

    TValue values() const
    {
        return {K::make_non_owning(static_cast<::K *>(m_ptr.data())[1])};
    }

    // TODO: Implement for all
    // at
    template<typename Key>
    typename std::enable_if_t<!internal::is_instance_class_v<TValue, Tuple>, typename TKey::reference>
    get(Key&& key)
    {
        const auto idx = internal::Keys<TKey>::find(keys(), key);
        check_in_range(idx);
        return internal::Values<TValue>::get(values(), idx);
    }

    template<typename GetAs, typename Key>
    typename std::enable_if_t<internal::is_instance_class_v<TValue, Tuple>, GetAs>
    get(Key&& key)
    {
        const auto idx = internal::Keys<TKey>::find(keys(), key);
        check_in_range(idx);
        return internal::Values<TValue>::template get<GetAs>(values(), idx);
    }

    template<typename Key>
    typename std::enable_if_t<!internal::is_instance_class_v<TValue, Tuple>, typename TKey::const_reference>
    get(Key&& key) const
    {
        const auto idx = internal::Keys<TKey>::find(keys(), key);
        check_in_range(idx);
        return internal::Values<TValue>::cget(values(), idx);
    }

    template<typename GetAs, typename Key>
    typename std::enable_if_t<internal::is_instance_class_v<TValue, Tuple>, const GetAs>
    get(Key&& key) const
    {
        const auto idx = internal::Keys<TKey>::find(keys(), key);
        check_in_range(idx);
        return internal::Values<TValue>::template cget<GetAs>(values(), idx);
    }

    // []
    template<typename Key>
    typename std::enable_if_t<!internal::is_instance_class_v<TValue, Tuple>, typename TKey::reference>
    operator[](Key&& key)
    {
        const auto idx = internal::Keys<TKey>::find(keys(), key);
        return internal::Values<TValue>::get(values(), idx);
    }

    template<typename GetAs, typename Key>
    typename std::enable_if_t<internal::is_instance_class_v<TValue, Tuple>, GetAs>
    operator[](Key&& key)
    {
        const auto idx = internal::Keys<TKey>::find(keys(), key);
        return internal::Values<TValue>::template get<GetAs>(values(), idx);
    }

    template<typename Key>
    typename std::enable_if_t<!internal::is_instance_class_v<TValue, Tuple>, typename TKey::const_reference>
    operator[](Key&& key) const
    {
        const auto idx = internal::Keys<TKey>::find(keys(), key);
        return internal::Values<TValue>::cget(values(), idx);
    }

    template<typename GetAs, typename Key>
    typename std::enable_if_t<internal::is_instance_class_v<TValue, Tuple>, const GetAs>
    operator[](Key&& key) const
    {
        const auto idx = internal::Keys<TKey>::find(keys(), key);
        return internal::Values<TValue>::template cget<GetAs>(values(), idx);
    }

    // TODO: Only works for Vector/Nested Vector keys, Vector/Nested Vector values
    // Iterators
    // begin/cbegin
    // end/cend
    // rbegin/crbegin
    // rend/crend
    
    // Capatacity
    bool empty() const noexcept
    {
        return size() != 0;
    }

    size_t size() const noexcept
    {
        return m_length;
    }

    size_t max_size() const noexcept
    {
        return std::numeric_limits<decltype(::k0::n)>::max();
    }

    // Modifiers - Not supported
    // clear, insert, insert_or_assign, emplace, emplace_hint, try_emplace,
    // erase, swap, extract, merge

    // Lookup
    // count
    template<typename Key>
    size_t count(Key&& key) const
    {
        return internal::Keys<TKey>::count(keys(), key);
    }
    
    /**
     * @brief find: returns index over iterator as iterators aren't supported
     * on dictionarys with tuple keys or values.
     * 
     */
    template<typename Key>
    size_t find(Key&& key) const
    {
        return internal::Keys<TKey>::find(keys(), key);
    }

    // contains
    template<typename Key>
    bool contains(Key&& key) const
    {
        return internal::Keys<TKey>::find(keys(), key) < m_length;
    }

    // TODO: equal_range, lower_bound, upper_bound - Supported for vector keys. Give back range on keys to allow for tuple values.

    // // table
    // typename std::enable_if_t<std::is_same_v<TKey,Vector<Type::Symbol>>, void>
    // as_table() const
    // {
    //     // keys must be symbols.
    //     // the rhs may be any vector. Each entry is put in a column, so doesn't
    //     // matter if i.e. its a jagged array. It will have one record with each
    //     // element in a column.
    //     //K{xT(get().release())};
    // }

    K get() const
    {
        return m_ptr;
    }

private:

    void check_in_range(size_t pos)
    {
        if (pos >= m_ptr.size())
            throw std::out_of_range("Key lookup failed");
    }

    K m_ptr;
    size_t m_length;
};

}