#pragma once

#include "forward.h"
#include "k.h"
#include "utils.h"

namespace qbind
{

class Converter
{
public:
    // Enforce only qbind types are being accepted in to methods.
    template<class T>
    static T to_cpp(::K arr)
    {
        static_assert(  internal::is_instance_type<T, Atom>::value ||
                        internal::is_instance_type<T, Vector>::value ||
                        internal::is_instance_class<T, Tuple>::value,
                        "T must be a qbind wrapper type: Atom, Vector, Tuple, Map, Table, KeyedTable");
        // Must be non-owning as kdb decrements arguments ref count on completion anyway.
        // This avoid a double-free by incrementing the ref-count when making as a non-owning K.
        return {K::make_non_owning(arr)};
    }

    // Enforce only qbind types are being returned from methods (if anything returned).
    template<class T>
    static ::K to_q(T value)
    {
        static_assert(  internal::is_instance_type<T, Atom>::value ||
                        internal::is_instance_type<T, Vector>::value ||
                        internal::is_instance_class<T, Tuple>::value,
                        "T must be a qbind wrapper type: Atom, Vector, Tuple, Map, Table, KeyedTable");
        auto res = value.get().release();
        // If the k object is one of the arguments passed in it must be r1'ed before return.
        // This is here for the add example only where we assign to an existing atom.
        // TODO: Fix converter so that it knows if it needs to increment by one on return.
        return res;
        //r1(res);
    }
};

}