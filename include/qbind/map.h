#pragma once

#include <limits>
#include <functional>

#include "span.h"

#include <unordered_map>

namespace qbind
{

// forward declare
template<class Key, class T, class KeyEqual>
class Map;

template<typename KeyItType, typename ValueItType>
class MapIterator
{
public:
    using value_type = typename ValueItType::value_type;
    using key_type = typename KeyItType::value_type;
    using difference_type = int64_t;

    using value_it = ValueItType;
    using key_it = KeyItType;

    using value_reference = typename ValueItType::reference;
    using key_reference = typename KeyItType::reference;
    using iterator_category = std::random_access_iterator_tag;

    MapIterator() : m_key_it({}), m_value_it({}), m_index(0) {}
    MapIterator(KeyItType k, ValueItType v, difference_type idx = 0)
        :m_key_it(k)
        ,m_value_it(v)
        ,m_index(idx)
    {} 

    // Value access
    std::pair<key_reference, value_reference> operator*() const 
    {
        return std::make_pair(
            key_reference(*(m_key_it+m_index)),
            value_reference(*(m_value_it+m_index))
        );
    }
    std::pair<key_reference, value_reference> operator[](difference_type n) const
    {
        return std::make_pair(
            key_reference(*(m_key_it+m_index+n)),
            value_reference(*(m_value_it+m_index+n))
        );
    }

    // Forwards / backwards
    MapIterator &operator++() { ++m_index; return *this; }
    MapIterator &operator--() { --m_index; return *this; }
    MapIterator operator++(int) { auto tmp(*this); ++m_index; return tmp; }
    MapIterator operator--(int) { auto tmp(*this); --m_index; return tmp; }

    // Arithmetic
    difference_type operator-(const MapIterator &other) const { return m_index - other.m_index; }
    MapIterator operator+(difference_type n) const { return {m_key_it, m_value_it, m_index + n}; }
    MapIterator operator-(difference_type n) const { return {m_key_it, m_value_it, m_index - n}; }

    friend inline MapIterator operator+(difference_type n, const MapIterator& other)
    {
        return {other.m_key_it, other.m_value_it, other.m_index + n};
    }
    friend inline MapIterator operator-(difference_type n, const MapIterator& other)
    {
        return {other.m_key_it, other.m_value_it, other.m_index - n};
    }

    MapIterator &operator+=(difference_type n) { m_index += n;  return *this; }
    MapIterator &operator-=(difference_type n) { m_index -= n;  return *this; }

    // Comparison
    bool operator==(const MapIterator &rhs) const { return m_key_it == rhs.m_key_it && m_value_it == rhs.m_value_it && m_index == rhs.m_index; }
    bool operator!=(const MapIterator &rhs) const { return m_key_it == rhs.m_key_it && m_value_it == rhs.m_value_it && m_index != rhs.m_index; }
    bool operator< (const MapIterator &rhs) const { return m_key_it == rhs.m_key_it && m_value_it == rhs.m_value_it && m_index <  rhs.m_index; }
    bool operator> (const MapIterator &rhs) const { return m_key_it == rhs.m_key_it && m_value_it == rhs.m_value_it && m_index >  rhs.m_index; }
    bool operator<=(const MapIterator &rhs) const { return m_key_it == rhs.m_key_it && m_value_it == rhs.m_value_it && m_index <= rhs.m_index; }
    bool operator>=(const MapIterator &rhs) const { return m_key_it == rhs.m_key_it && m_value_it == rhs.m_value_it && m_index >= rhs.m_index; }

private:
    key_it m_key_it;
    value_it m_value_it;
    difference_type m_index;
};

/**
 * TODO: Implement in terms of span iterators, not pointers. Remove offset from span<T>
 * @brief Implements an unordered multi map interface
 * 
 * @tparam Key 
 * @tparam T 
 * @tparam KeyEqual 
 */
template<class Key, class T, class KeyEqual = std::equal_to<Key>>
class Map
{
    using value_type = std::remove_cv_t<typename T::Target>;
    using key_type = std::remove_cv_t<typename Key::Target>;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using key_reference = Ref<Key>;
    using key_const_reference = ConstRef<Key>;
    using value_reference = Ref<T>;
    using value_const_reference = ConstRef<T>;
    using iterator = MapIterator<typename Span<Key>::iterator, typename Span<T>::iterator>;
    using reverse_iterator = MapIterator<typename Span<Key>::reverse_iterator, typename Span<T>::reverse_iterator>;
    using const_iterator = MapIterator<typename Span<Key>::const_iterator, typename Span<T>::const_iterator>;
    using const_reverse_iterator = MapIterator<typename Span<Key>::const_reverse_iterator, typename Span<T>::const_reverse_iterator>;

    Map(Span<Key> keys, Span<T> values, KeyEqual key_equal = KeyEqual())
        : m_arr(nullptr)
        , m_keys(std::move(keys))
        , m_values(std::move(values))
        , m_equal(std::move(key_equal))
    {
        if (m_keys.size() != m_values.size())
            throw std::invalid_argument("Keys and values must be the same size");
    }

    Map(K arr, KeyEqual key_equal = KeyEqual()) noexcept
        : m_arr(arr)
        , m_keys(Span<Key>(UntypedSpan::SymmetricMemory(
            ((K*)(arr->G0))[0]
        )))
        , m_values(Span<T>(UntypedSpan::SymmetricMemory(
            ((K*)(arr->G0))[1]
        )))
        , m_equal(std::move(key_equal))
    {}

    static Map SymmetricMemory(K arr)
    {
        return Map(r1(arr));
    }

    constexpr K to_q() const
    {
        if (!m_arr) xD(m_keys.to_q(), m_values.to_q());
        return r1(m_arr);
    }

    /**
     * @brief Copy
     */
    constexpr Map(const Map& other) noexcept
        : m_arr(other.m_arr ? r1(other.m_arr) : nullptr)
        , m_keys(other.m_keys)
        , m_values(other.m_values)
        , m_equal(std::move(m_equal))
    {}

    constexpr Map& operator=(const Map& other) noexcept
    {
        if (this != &other)
        {
            r0(m_arr);
            m_arr = other.m_arr ? r1(other.m_arr) : nullptr;
            m_keys = other.m_keys;
            m_values = other.m_values;
            m_equal = other.m_equal;
        }
        return *this;
    }

    /**
     * @brief Move - use memberwise move
     */
    constexpr Map(Map&& other) noexcept
        : m_arr(other.m_arr)
        , m_keys(std::move(other.m_keys))
        , m_values(std::move(other.m_values))
        , m_equal(std::move(other.m_equal))
    {
        other.m_arr = nullptr;
        other.m_keys = {UntypedSpan()};
        other.m_values = {UntypedSpan()};
    }

    constexpr Map& operator=(Map&& other) noexcept
    {
        if (this != &other)
        {
            if (m_arr)
                r0(m_arr);
            m_arr = other.m_arr ? r1(other.m_arr) : nullptr;
            m_keys = std::move(other.m_keys);
            m_values = std::move(other.m_values);
            m_equal = std::move(other.m_equal);
            other.m_arr = nullptr;
            other.m_keys = {UntypedSpan()};
            other.m_values = {UntypedSpan()};
        }
        return *this;
    }

    iterator begin() noexcept               { return MapIterator(m_keys.begin(), m_values.begin()); }
    const_iterator cbegin() const noexcept  { return MapIterator(m_keys.cbegin(), m_values.cbegin()); };

    iterator end() noexcept                 { return MapIterator(m_keys.begin(), m_values.begin(), size()); }
    const_iterator cend() const noexcept    { return MapIterator(m_keys.cbegin(), m_values.cbegin(), size()); }

    reverse_iterator rbegin() noexcept                  { return MapIterator(m_keys.begin(), m_values.begin(), size() - 1); }
    const_reverse_iterator crbegin() const noexcept     { return MapIterator(m_keys.cbegin(), m_values.cbegin(), size() - 1); }

    reverse_iterator rend() noexcept                    { return MapIterator(m_keys.begin(), m_values.begin(), -1); }
    const_reverse_iterator crend() const noexcept       { return MapIterator(m_keys.cbegin(), m_values.cbegin(), -1); }

    [[nodiscard]] bool empty() const noexcept
    {
        return m_keys.empty();
    }
    size_type size() const noexcept
    {
        return m_keys.size();
    }

    // Skip modifier functions

    value_reference at(const key_type& key)
    {
        const auto idx = key_index(key);
        if (idx == m_keys.size())
            throw std::out_of_range("Key not found");
        return m_values[idx];
    }
    value_const_reference at(const key_type& key) const
    {
        const auto idx = key_index(key);
        if (idx == m_keys.size())
            throw std::out_of_range("Key not found");
        return m_values[idx];  
    }

    constexpr value_reference operator[]( const key_type& key ) const noexcept
    {
        return m_values[key_index(key)];
    }
    constexpr value_reference operator[]( key_type&& key ) const noexcept
    {
        return m_values[key_index(key)];
    }
    constexpr value_reference operator[]( typename Key::iterator key ) const noexcept
    {
        return m_values[key - m_keys.begin()];
    }
    constexpr value_reference operator[]( typename Key::const_iterator key ) const noexcept
    {
        return m_values[key - m_keys.cbegin()];
    }

    template< class K >  
    size_type count( const K& x ) const
    {
        return std::count_if(
            m_keys.begin(),
            m_keys.end(),
            [&k = x, &eq = m_equal](const auto &x)
            {
                return eq(k, x);
            });
    }

    template< class K > 
    iterator find( const K& key )
    {
        return std::find_if(
            m_keys.begin(), 
            m_keys.end(), 
            [&k=key, &eq=m_equal](const auto &x)
            {
                return eq(k, x);
            });
    }
    template< class K > 
    const_iterator find( const K& key ) const
    {
        return std::find_if(
            m_keys.cbegin(), 
            m_keys.cend(), 
            [&k=key, &eq=m_equal](const auto &x)
            {
                return eq(k, x);
            });
    }

    template< class K > 
    bool contains( const K& x ) const
    {
        return key_index(x) != m_keys.size();
    }

    // template< class K >
    // std::pair<iterator,iterator> equal_range( const K& x );
    // template< class K >
    // std::pair<const_iterator,const_iterator> equal_range( const K& x ) const;

    KeyEqual key_eq() const { return m_equal; };

    Span<Key> keys() const { return m_keys; };
    Span<T> values() const { return m_values; };

private:
    /**
     * @brief Pointer to k0. Populated if created from converter, else null.
     * Used to monitor ownership.
     */
    K m_arr;
    Span<Key> m_keys;
    Span<T> m_values;
    KeyEqual m_equal;

    template<class K>
    size_t key_index(const K& key)
    {
        return std::find_if(
            m_keys.begin(),
            m_keys.end(),
            [&k=key, &eq=m_equal](const auto &x)
            {
                return eq(k, x);
            }) - m_keys.begin();
    }

};
}