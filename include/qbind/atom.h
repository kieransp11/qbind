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

    static constexpr Type type = T;
    static constexpr Structure structure = Structure::Atom;

    using underlier = typename internal::c_type<T>::underlier;
    using value     = typename internal::c_type<T>::value;
    using reference = typename internal::c_type<T>::reference;

    // Initialise from a K object
    Atom(K data)
    :m_ptr(std::move(data))
    {
        if (!m_ptr)
            throw std::runtime_error("K is empty");
        m_ptr.is_with_info<Atom<T>>();
    }

    // Initialise from value
    Atom(value v)
    {
        // Make an atom of the right type using the kx API and put
        // that in a temporary qbind atom as it is safe to assign to.
        Atom<T> tmp{K{ka(-static_cast<signed char>(T))}};
        tmp = v;
        // swap the contents of the qbind::Ks to initialise this atom.
        m_ptr.swap(tmp.m_ptr);
    }

    /**
     * @brief Retrun value as reference. This allows all operators
     * defined on the reference type to be used.
     * 
     * For symbols the reference type is SymbolReference. This just
     * allows lexicographical operators. All other underlying types
     * are integral, so it opens up the normal operations you would
     * expect.
     * 
     * As all reference values are printable, atoms are printable.
     * 
     * @return reference 
     */
    operator reference() const {
        return m_ptr.data<underlier>()[0];
    }

    /**
     * @brief Returns value of atom as usable type.
     * If this is a symbol atom only return const char * as a warning
     * this data should not be mutated/extended.
     * 
     * For symbols return std::string_view as symbol content should
     * not be mutated/extended. The view is created by implicitly 
     * converting from char*. For all other types this is the
     * underlying type.
     */
    ENABLE_IF_SYMBOL
    operator value() const {
        return m_ptr.data<underlier>()[0];
    }

    /**
     * @brief Assignment assumes ownership, i.e. char* will be interned
     * and all other values copied.
     */
    Atom<T>& operator=(value value)
    {
        auto ptr = m_ptr.data<underlier>();
        *ptr = to_underlier(value);
        return *this;
    }

    K get() const
    {
        return m_ptr;
    }

private:

    static constexpr typename internal::c_type<T>::to_underlier to_underlier;

    K m_ptr;
};

}