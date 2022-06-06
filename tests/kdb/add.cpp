#include "qbind/atom.h"
#include "qbind/function.h"

namespace q = qbind;
namespace a = qbind::a;
namespace v = qbind::v;

a::j add(a::j x, a::j y)
{
    return {static_cast<int64_t>(x) + static_cast<int64_t>(y)};
    //return x;
}

// Export a function that will return the sum of two longs
QBIND_FN_EXPORT(add, qadd, 2, 1)

// Example usage:
// ./xb b
// cp out/lib/libqbind.kdb.tests.dylib $CONDA_PREFIX/q/m64/libqbind.kdb.tests.so

// rlwrap q

// myadd: `libqbind.kdb.tests 2:(`qadd;2)
// myadd[2;4]

// clear environment
// rm $CONDA_PREFIX/q/m64/libqbind.kdb.tests.so