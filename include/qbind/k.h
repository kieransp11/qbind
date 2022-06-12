#pragma once

#include <optional>
#include <variant>

#include <kx/kx.h>

#include "forward.h"
#include "type.h"
#include "utils.h"

namespace qbind
{

/**
 * @brief An RAII wrapper around the KX K struct
 * 
 * Implements memory functions:
 *  - V r0(K)
 *  - K r1(K).
 */
class K
{
public:

    constexpr K() noexcept
    : m_k(nullptr)
    { }

    constexpr K(std::nullptr_t) noexcept
    : m_k(nullptr)
    { }

    explicit K(::K k)
    : m_k(k)
    { }

    // Make a non-owning K. 
    static K make_non_owning(::K k)
    {
        return K(_r1(k));
    }

    // Copy constructors
    K(const K& other) noexcept
    :m_k(_r1(other.m_k))
    { }

    // Move constructors
    K(K&& other) noexcept
    :m_k(other.m_k)
    {
        other.m_k = nullptr;
    }

    // Destructor
    ~K()
    {
        _r0(m_k);
    }

    // Operator=
    // Copy
    K& operator=(const K& other) noexcept
    {
        // Ignore self assignment
        // If already the same internal pointer don't do anything
        if (this != &other && m_k != other.m_k)
        {
            _r0(m_k);
            m_k = _r1(other.m_k);
        }
        return *this;
    }

    // Move
    K& operator=(K&& other) noexcept
    {
        if (this != &other)
        {
            _r0(m_k);
            m_k = other.m_k;
            other.m_k = nullptr;
        }
        return *this;
    }

    void swap(K& other) noexcept
    {
        auto tmp = m_k;
        m_k = other.m_k;
        other.m_k = tmp;
    }

    // Reference counting starts and begins at 0. So should
    // read m_k->t as "number of copies still around". If you
    // decrement the reference count from 0 (i.e. when there
    // are no copies left, the object is no longer valid.
    size_t use_count() const noexcept
    {
        return m_k ? 1 + m_k->r : 0;
    }

    // This is valid if and only if reference counting is kept consistent externally too.
    operator bool() const noexcept
    {
        return m_k != nullptr;
    }

    // size: 1 if atom else size
    size_t size() const noexcept
    {
        return m_k->t < 0 ? 1 : static_cast<size_t>(m_k->n);
    }

    void* data() const noexcept
    {
        return m_k ? (
            m_k->t < 0 ? 
                static_cast<void*>(&m_k->g) : 
                static_cast<void*>(m_k->G0)
        ) : nullptr;
    }

    // type checkers

    template<class T>
    typename std::enable_if_t<
        internal::is_instance_type_v<T, Atom> ||
        internal::is_instance_type_v<T, Vector> ||
        internal::is_instance_type_v<T, NestedVector>, bool>
    is() const noexcept
    {
        const auto tc = inner_type(m_k);
        if (std::holds_alternative<std::string>(tc)) return false;
        // inner type of 0 implies could be nested vector of any type, to return true.
        const auto [s, t] = std::get<0>(tc);
        return (T::Structure == s && T::Type == t) ||
               (T::Structure == Structure::NestedVector && Type(0) == t);
    }

    template<class T>
    typename std::enable_if_t<
        internal::is_instance_type_v<T, Atom> ||
        internal::is_instance_type_v<T, Vector> ||
        internal::is_instance_type_v<T, NestedVector>, void>
    is_with_info() const
    {
        const auto tc = inner_type(m_k);
        if (std::holds_alternative<std::string>(tc))
        {
            std::ostringstream ss;
            ss << "Failed to deduce inner type: " << std::get<std::string>(tc);
            throw std::runtime_error(ss.str());
        }
        const auto [s, t] = std::get<0>(tc);
        if ((T::Structure == s && T::Type == t) ||
            (T::Structure == Structure::NestedVector && Type(0) == t)) return;

        std::ostringstream ss;
        ss << "Types did not match. Expected " << T::Structure << " " << T::Type << " but found " << s << " " << t;
        throw std::runtime_error(ss.str());
    }

    template<class T>
    typename std::enable_if_t<internal::is_instance_class<T, Tuple>::value, bool>
    is() const noexcept
    {
        return !T::template tuple_type_match<0,0>(m_k).has_value();
    }

    template<class T>
    typename std::enable_if_t<internal::is_instance_class<T, Tuple>::value, void>
    is_with_info() const
    {
        auto res = T::template tuple_type_match<0,0>(m_k);
        if (res.has_value())
            throw std::runtime_error(res.value());
    }

    // Appending
    // TODO: Check what these functions do to lhs, rhs and returned reference count.

    template<Type T>
    void join_atom(typename internal::c_type<T>::Underlier value)
    {
        if (!m_k)
            throw std::runtime_error("Cannot append to null");
        // make sure array is of the right type.
        if (m_k->t != static_cast<signed char>(T))
        {
            std::ostringstream ss;
            ss << "Array is not vector of " << T;
            throw std::runtime_error(ss.str());
        }
        // Intern symbols and use js, else use ja with void*
        m_k = T == Type::Symbol ? js(&m_k, ss(value)) : ja(&m_k, &value);
    }

    // Append a K list y to K list x. Both lists must be of the same type.
    void join_lists(K k)
    {
        if (!m_k)
            throw std::runtime_error("Cannot append to null");
        if (!k.m_k)
            throw std::runtime_error("Cannot append null");
        if (m_k->t != k.m_k->t || m_k->t < 0)
            throw std::runtime_error("Can only join list on to list");
        m_k = jv(&m_k, k.m_k);
    }

    // Appends another K object to a mixed list.
    void tuple_append(K k)
    {
        if (!m_k)
            throw std::runtime_error("Cannot append to null");
        if (!k.m_k)
            throw std::runtime_error("Cannot append null");
        if (m_k->t != 0 || k.m_k->t < 0)
            throw std::runtime_error("Can only append tuple to tuple");
        // Takes ownership of a reference to its argument y.
        // We use r1 to ensure everything maintains ownership.
        m_k = jk(&m_k, _r1(k.m_k));
    }

    /**
     * @brief Release managed pointer back to manually managed.
     * 
     * This only releases from this qbind::K, so when this qbind::K is  
     * destructed the KX K will not be r0ed. 
     * 
     * Upon release the programmer is responsible for only their subsequent
     * reference increments and decrements, all other qbind::K objects will 
     * continue to remove their references as they're destructed.
     * 
     * @return ::K 
     */
    ::K release() noexcept
    {
        auto temp = m_k;
        m_k = nullptr;
        return temp;
    }

private:

    // Avoid nullptr segfault
    static void _r0(::K x) noexcept
    {
        if (x)
            r0(x);
    }

    // Avoid nullptr segfault
    static ::K _r1(::K x) noexcept
    {
        return x ? r1(x) : x;
    }

    /**
     * @brief Determine inner type atom/vector/nested vector.
     * 
     * If the inner type can be determined return that. If no type can be determined but
     * no contradictions are found (i.e. all generic nulls) return Type(0). Else return
     * error string.
     * 
     * @param k Data to check (recursively)
     * @param t Current type belief (Type(0) if we don't have one yet)
     * @return std::variant<Type, std::string> 
     */
    static std::variant<Type, std::string> inner_type_impl(::K k, Type t = Type(0))
    {
        if (!k)
            throw std::logic_error("Found nullptr when deducing inner type.");
        // iterate children
        if (k->t == 0)
        {
            for (auto i = 0; i < k->n; ++i)
            {
                k = reinterpret_cast<::K *>(k->G0)[i];
                // child fails return failure.
                auto res = inner_type_impl(k, t);
                if (std::holds_alternative<std::string>(res))
                    return res;

                // if child succeeded a type was decided.
                auto t2 = std::get<Type>(res);
                // if child had type but we have no ideas yet set that to type.
                // Only do this if child has type (its not an error for children
                // to have type 0 as they can just be full of generic nulls or
                // lists of generic nulls)
                if (t == Type(0) && t2 != Type(0))
                    t = t2;
            }
        }
        // No type idea yet => use hint from this atom/vector
        else if (t == Type(0))
            return {Type(abs(k->t))};
        // Have a type idea => check this matches.
        else if (Type(abs(k->t)) != t)
            return "Does not match expected type";
        // Propagate existing idea as we are consistent.
        return t;
    }

    /**
     * @brief Determine inner type for atom/vector/nested vector.
     * 
     * Type may be Type(0) for nested vectors when they only contain type 0.
     * This implies they can be a NestedVector<Type> for any Type
     */
    static std::variant<std::pair<Structure,Type>, std::string> inner_type(::K k)
    {
        auto res = inner_type_impl(k);
        if (std::holds_alternative<std::string>(res))
            return std::get<std::string>(res);
        const auto t = std::get<Type>(res);
        const auto tv = static_cast<signed char>(t);
        if (tv < 1 || tv < 19 || tv == 3)
            return "Invalid absolute type found. Not in range [1, 19] excluding 3";

        if (k->t < 0)
            return std::make_pair(Structure::Atom, t);
        if (k->t > 0)
            return std::make_pair(Structure::Vector, t);
        return std::make_pair(Structure::NestedVector, t);
    }

    template<size_t Depth, size_t Idx, class Type, class... RemainingType>
    static std::optional<std::string> tuple_type_match_impl(::K data)
    {
        ::K entry = reinterpret_cast<::K*>(data->G0)[Idx];
        if (!entry)
            return "Found nullptr at Depth: " + std::to_string(Depth) + " Index: " + std::to_string(Idx);
        // If type is a tuple recurse depth and start at index 0
        if constexpr (internal::is_instance_class<Type, Tuple>::value)
        {
            return Type::template tuple_type_match_impl<Depth+1, 0>(entry);
        }
        // If atom/vector/nested vector
        else if constexpr (internal::is_instance_type<Type, NestedVector>::value || 
                      internal::is_instance_type<Type, Vector>::value ||
                      internal::is_instance_type<Type, Atom>::value)
        {
            const auto res = inner_type(entry);
            if (std::holds_alternative<std::string>(res))
            {
                std::ostringstream ss;
                ss << "Failed to get inner type for " << Type::Structure << " " << Type::Type << " at depth "
                << Depth << " index " << Idx << ": " << std::get<std::string>(res);
                return ss.str();
            }

            const auto [s, t] = std::get<0>(res);
            if ((s == Type::Structure && t == Type::Type) || (s == Type::NestedVector && t == Type(0))) return std::nullopt;

            std::ostringstream ss;
            ss << "Types did not match at depth " << Depth << " index " << Idx << ". Expected " << Type::Structure << " " << Type::Type << 
                  " but found " << s << " " << t;
            return ss.str();
        }
        else
        {
            // TODO: make compile time. Is this needed.
            return "Unexpected type unexpected type in tuple";
        }

        // TODO: Map<Vector, Vector>, Map<Vector, Tuple>, Map<Tuple, Vector>, Map<Tuple, Tuple>
        // TODO: Table<Args...>.
        // TODO: Keyed Table ??

        // if there's more entries keep going
        if constexpr (0 < sizeof...(RemainingType))
            return tuple_type_match_impl<Depth, Idx + 1, RemainingType...>(data);
        return std::nullopt;
    }

    /**
     * @brief Start iterating across the tuples types checking their Type and Structure
     * across the provided KX array object.
     * 
     * @tparam Depth: Depth of search, starting at 0.
     * @tparam Idx: Idx in arguments, starting at 0.
     * @param data 
     */
    template<size_t Depth, size_t Idx, class... Types>
    static std::optional<std::string> tuple_type_match(::K data)
    {
        if (data->t != 0)
            return "Not tuple at Depth: " + std::to_string(Depth) + " Index: " + std::to_string(Idx);
        // It may be possible to have a tuple which will return false for the container
        // it is in due to some tuples becoming "views" over the underlying K array via tuple cat.
        // This is fine behaviour.
        if (data->n != sizeof...(Types))
            return "Bad length at Depth: " + std::to_string(Depth) + " Index: " + std::to_string(Idx) +
                   ". Expected: " + std::to_string(sizeof...(Types)) + " Found: " + std::to_string(data->n);
        return tuple_type_match_impl<Depth, Idx, Types...>(data);
    }

private:

    ::K m_k;
};

}