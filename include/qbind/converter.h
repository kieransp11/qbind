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
        
        // If the k object is one of the arguments passed in it must be r1'ed before return.
        // However, we r1 all arguments in to_cpp. The non-returned arguments will be r0'ed
        // by their K destructor. Release means the K will be left with a nullptr meaning when
        // it is destructed it doesn't call r0, meaning the original r1 of the argument is
        // preserved.
        return value.get().release();
    }
};

}