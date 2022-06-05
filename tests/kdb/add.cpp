// #include "kx/kx.h"
// #include <iostream>
// #include "qbind/converter.h"
// #include "qbind/function.h"

// int64_t add(int64_t x, int64_t y)
// {
//     return x + y;
// }

// // Export a function that will return the sum of two longs
// QBIND_FN_EXPORT(add, qadd, 2, 1)

// Example usage:
// ./xb b
// cp out/tests/kdb/libqbind.kdb.tests.dylib $CONDA_PREFIX/q/m64/libqbind.kdb.tests.so

// start a q session with rlwrap q

// q)myadd: `libqbind.kdb.tests 2:(`add;2)
// q)myadd[2;4]

// clear environment
// rm $CONDA_PREFIX/q/m64/libqbind.kdb.tests.so