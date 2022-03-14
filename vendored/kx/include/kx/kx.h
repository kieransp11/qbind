#pragma once

#include "kx/k.h"

#ifdef __cplusplus
extern"C"{
#endif

#if KXVER>=3
    #undef kU(x)
    #undef xU

    #undef DO(n,x)

#else

    #undef DO(n,k)

#endif

#ifdef __cplusplus
}
#endif

// vector accessors, e.g. kF(x)[i] for float&datetime
#undef kG(x)
#undef kC(x)
#undef kH(x)
#undef kI(x)
#undef kJ(x)
#undef kE(x)
#undef kF(x)
#undef kS(x)
#undef kK(x)

// nulls(n?) and infinities(w?)
#undef nh
#undef wh
#undef ni
#undef wi
#undef nj
#undef wj
#if defined(WIN32) || defined(_WIN32)
#undef nf
#undef wf
#undef finite
extern double log(double);
#else  
#undef nf
#undef wf
#undef closesocket(x)
#endif 

// remove more clutter
#undef O
#undef R
#undef Z
#undef P(x,y)
#undef U(x)
#undef SW
#undef CS(n,x)
#undef CD

#undef ZV
#undef ZK
#undef ZH
#undef ZI
#undef ZJ
#undef ZE
#undef ZF
#undef ZC
#undef ZS

#undef K1(f)
#undef K2(f)
#undef TX(T,x)
#undef xr
#undef xt
#undef xu
#undef xn
#undef xx
#undef xy
#undef xg
#undef xh
#undef xi
#undef xj
#undef xe
#undef xf
#undef xs
#undef xk
#undef xG
#undef xH
#undef xI
#undef xJ
#undef xE
#undef xF
#undef xS
#undef xK
#undef xC
#undef xB