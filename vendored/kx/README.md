# KX

**Source Repository:** [KxSystems/kdb](https://github.com/KxSystems/kdb)

KX provide the necessary components for interacting with kdb. These components are written to conform to C11, not C++. As such some C idioms leak. To combat this we have provided `kx.h`. This includes `k.h` but unsets most of the macros which are unhelpful. Likewise we have formatted `k.h` itself to make it easier to reference.

All objects (at present for Linux and Mac) are shipped in this vendored folder. A CMake find module has been written and included in the root `cmake` folder of this project. 