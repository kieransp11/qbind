#pragma once

#include <kx/kx.h>

#include "k.h"
#include "type.h"

namespace qbind
{

/**
 * @brief A simple wrapper around the KX atom.
 * 
 * We intern strings as a precaution even though the documentation 
 * implies interning is needed only for symbol vectors.
 * 
 * @tparam T type you wish the atom to be.
 */
template<Type T>
class Atom
{
public:

    static constexpr Type Type = T;
    static constexpr Structure Structure = Structure::Atom;

    using Underlier = typename internal::c_type<T>::Underlier;

    // Initialise from a K object
    Atom(K data)
    :m_ptr(std::move(data))
    {
        if (!m_ptr)
            throw std::runtime_error("K is empty");
        m_ptr.is_with_info<Atom<T>>();
    }

    // Initialise from value
    Atom(Underlier v)
    {
        // Make an atom of the right type using the kx API and put
        // that in a temporary qbind atom as it is safe to assign to.
        Atom<T> tmp{K{ka(-static_cast<signed char>(T))}};
        tmp = v;
        // swap the contents of the qbind::Ks to initialise this atom.
        m_ptr.swap(tmp.m_ptr);
    }

    /**
     * @brief If this is a symbol atom only return const char * as a warning
     * this data should not be mutated/extended.
     * 
     * This conversion allows the atom to be printed directly.
     */
    ENABLE_IF_SYMBOL
    operator char const *() const { 
        return *static_cast<const char **>(m_ptr.data()); 
    }

    /**
     * @brief For non-symbol atoms return the underlying value freely.
     * 
     * This conversion allows the atom to be printed directly.
    */
    DISABLE_IF_SYMBOL
    operator Underlier() const { 
        return *static_cast<Underlier*>(m_ptr.data()); 
    }

    /**
     * @brief Assignment assumes ownership, i.e. char* will be interned
     * and all other values copied.
     */
    Atom<T>& operator=(Underlier value)
    {
        auto ptr = static_cast<Underlier*>(m_ptr.data());
        if constexpr (std::is_same_v<Underlier, char*>)
        {
            *ptr = ss(value);
        }
        else
        {
            *ptr = value;
        }
        return *this;
    }

    K get() const
    {
        return m_ptr;
    }

private:

    K m_ptr;
};

}