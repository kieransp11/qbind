#include <iostream>
#include "qbind/atom.h"
#include "qbind/tuple.h"
#include "qbind/function.h"

namespace q = qbind;
namespace a = qbind::a;
namespace v = qbind::v;

a::j add(a::j x, a::j y)
{
    return {static_cast<int64_t>(x) + static_cast<int64_t>(y)};
    //x = static_cast<int64_t>(x) + static_cast<int64_t>(y);
    //return x;
}

q::Tuple<a::j> emplace(a::j x)
{
    return {std::move(x)};
}

// Export a function that will return the sum of two longs
QBIND_FN_EXPORT(add, qadd, 2, 1)
QBIND_FN_EXPORT(emplace, qemplace, 1, 1)

extern "C"
{

::K getRef(::K x)
{
    std::cout << x << " " << x->r << std::endl;
    return r1(x);
}
}

// Example usage:
// ./xb b && cp out/lib/libqbind.kdb.tests.dylib $CONDA_PREFIX/q/m64/libqbind.kdb.tests.so

// rlwrap q

/**
Neither variants leak under these tests but when returning a new K value
if you r it immediately you get 0, where as if you edit and return you get one.

Assumed explanation: Compositions of C functions are called without a return to the q
runtime? Hence r[myadd[2;3]] when r shows ref count 0 is because the result from myadd
has no copies as it goes straight from myadd into r without returning to the q runtime
and being copied in to r. This also applies for r[qem[1]].
However, passing a value made in to a c function always increments the ref count. i.e. r[1] == 1;

r: `libqbind.kdb.tests 2:(`getRef;1); myadd: `libqbind.kdb.tests 2:(`qadd;2); qem:`libqbind.kdb.tests 2:(`qemplace;1);

r[myadd[2;3]];
x:2;
y:2;
r[myadd[x;2]];
r[x];
r[y];
r[myadd[x;y]];
r[x];
r[y];
x:myadd[x;y];
r[x];
r[y];
*/

// clear environment
// rm $CONDA_PREFIX/q/m64/libqbind.kdb.tests.so