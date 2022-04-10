#ifndef KX
#define KX

typedef char*S,C;
typedef unsigned char G;
typedef short H;
typedef int I;
typedef long long J;
typedef float E;
typedef double F;
typedef void V;

#ifdef __cplusplus
extern"C"{
#endif

#if !defined(KXVER)
#error "Set KXVER=3 for kdb+3.0 or standalone c-api after 2011-04-20. Otherwise set KXVER=2. e.g. #define KXVER 3 or gcc -DKXVER=3"
#endif

#if KXVER>=3
    typedef struct k0{
        signed char m,a,    // m,a are for internal use.
                    t;      // The object's type
        C u;                // The object's attribute flags
        I r;                // The object's reference count
        union{
            // The atoms are held in the following members:
            G g;
            H h;
            I i;
            J j;
            E e;
            F f;
            S s;
            // The following members are used for more complex data.
            struct k0*k;
            struct{
                J n;        // number of elements in vector
                G G0[1];
            };
        };
    }*K;

    typedef struct{G g[16];}U;

    #define kU(x) ((U*)kG(x))
    #define xU ((U*)xG)
            /**
             * @brief Create GUID
             */
    extern K ku(U),
            /**
             * @brief Create a table keyed by n first columns if number of 
             * columns exceeds n.
             *          
             * Returns null if the argument x is not a table.
             */
             knt(J,K),
            /**
             * @brief create vector of type I and length J
             */
             ktn(I,J),
            /**
             * @brief create fixed-length string
             * 
             * Create a char array from a string of length n.
             */
             kpn(S,J);
            /**
             * @brief toggle symbol lock
             * 
             * Set whether interning symbols uses a lock: m is either 0 or 1.
             * 
             * Returns the previously set value.
             */
    extern I setm(I),ver();
    #define DO(n,x)	{J i=0,_i=(n);for(;i<_i;++i){x;}}

#else

    typedef struct k0{
        I r;                // The object's reference count
        H t,u;              // The object's type and attribute flags
        union{
            // The atoms are held in the following members:
            G g;
            H h;
            I i;
            J j;
            E e;
            F f;
            S s;
            // The following members are used for more complex data.
            struct k0*k;
            struct{
                I n;        // number of elements in vector
                G G0[1];
            };
        };
    }*K;
            /**
             * @brief create vector of type I and length J
             */
    extern K ktn(I,I),
            /**
             * @brief create fixed-length string
             * 
             * Create a char array from a string of length n.
             */
             kpn(S,I);

    #define DO(n,x)	{I i=0,_i=(n);for(;i<_i;++i){x;}}
#endif

#ifdef __cplusplus
}
#endif

//#include<string.h>
// vector accessors, e.g. kF(x)[i] for float&datetime
#define kG(x)	((x)->G0)
#define kC(x)	kG(x)
#define kH(x)	((H*)kG(x))
#define kI(x)	((I*)kG(x))
#define kJ(x)	((J*)kG(x))
#define kE(x)	((E*)kG(x))
#define kF(x)	((F*)kG(x))
#define kS(x)	((S*)kG(x))
#define kK(x)	((K*)kG(x))

//      type bytes qtype     ctype  accessor
#define KB 1  // 1 boolean   char   kG
#define UU 2  // 16 guid     U      kU
#define KG 4  // 1 byte      char   kG
#define KH 5  // 2 short     short  kH
#define KI 6  // 4 int       int    kI
#define KJ 7  // 8 long      long   kJ
#define KE 8  // 4 real      float  kE
#define KF 9  // 8 float     double kF
#define KC 10 // 1 char      char   kC
#define KS 11 // * symbol    char*  kS

#define KP 12 // 8 timestamp long   kJ (nanoseconds from 2000.01.01)
#define KM 13 // 4 month     int    kI (months from 2000.01.01)
#define KD 14 // 4 date      int    kI (days from 2000.01.01)

#define KN 16 // 8 timespan  long   kJ (nanoseconds)
#define KU 17 // 4 minute    int    kI
#define KV 18 // 4 second    int    kI
#define KT 19 // 4 time      int    kI (millisecond)

#define KZ 15 // 8 datetime  double kF (DO NOT USE)

// table,dict
#define XT 98 //   x->k is XD
#define XD 99 //   kK(x)[0] is keys. kK(x)[1] is values.

#ifdef __cplusplus
    #include<cstdarg>
    extern"C"{
    extern V m9();
#else
    #include<stdarg.h>
    extern V m9(V);
#endif
        /**
         * @brief Connect with capability.
         * 
         * Standalone apps only. Available only from the c/e libs and not as a 
         * shared library loaded into kdb+.
         * 
         * @param S hostname: 0.0.0.0 opens a unix domain socket.
         * @param I port
         * @param S credentials: [username]:[password]
         * @param I timeout: Use 0 for no timeout.
         * @param I capability bitfield (1 = 1 TB limit, 2 = use TLS)
         * @return I 
         *   0   Authentication error
         *  -1   Connection error
         *  -2   Timeout error
         *  -3   OpenSSL initialization failed (see SSL info for details)
         */
extern I khpunc(S,I,S,I,I),
        /**
         * @brief Connect with timeout.
         * 
         * Same as connect with capability except cannot specify a capability.
         * As a result OpenSSL errors will not be returned.
         */
         khpun(const S,I,const S,I),
        /**
         * @brief Construct without a timeout.
         * 
         * Same as connect with timeout where timeout=0.
         */
         khpu(const S,I,const S),
        /**
         * @brief Connect anonymously
         * 
         * Same as connect without timer where credentias is "".
         */
         khp(const S,I),
        /**
         * @brief Verify that the byte vector x is a valid IPC message.
         * 
         * @param K message: Decompressed data only. Message is not modified. 
         * @return I Returns 0 if not valid.
         */
         okx(K),
        /**
         * @brief Encode a year/month/day as a q date, e.g. 0==ymd(2000, 1, 1)
         */
         ymd(I,I,I),
        /**
         * @brief Converts a q date to a yyyymmdd integer.
         */
         dj(I);
        /**
         * @brief Decrement an object‘s reference count.
         * 
         * If x->r is 0, x is unusable after the r0(x) call, and the memory 
         * pointed to by it may have been freed.
         * 
         * Note: Reference counting starts and ends with 0, not 1.
         */
extern V r0(K),
        /**
         * @brief Remove the callback on d and call kclose.
         * 
         * Shared library only.
         */
         sd0(I),
        /**
         * @brief Remove the callback on d and call kclose on d if f is 1.
         * 
         * Shared library only. Since V3.0 2013.04.04.
         */
         sd0x(I d,I f),
        /**
         * @brief Disconnect.
         * With the release of c.o with V2.6, c.o now tracks the connection
         * type (pre V2.6, or V2.6+). Hence, to close the connection, you must 
         * call kclose (instead of close or closeSocket): this will clean up 
         * the connection tracking and close the socket.
         *  
         * Standalone apps only. 
         */
         kclose(I);
        /**
         * @brief Intern n chars from a string.
         * 
         * @param S string: data to be interned.
         * @param I count: number of characters to intern.
         * @returns an interned string and should be used to store the string
         *          in a symbol vector.
         */
extern S sn(S,I),
        /**
         * @brief Intern a null-terminated string.
         * 
         * @param S string: null-terminated data to be interned.
         * @returns: An interned string and should be used to store the string
         *           in a symbol vector.
         */
         ss(S);
        /**
         * @brief error string
         * 
         * Capture (and reset) error string into usual error object, e.g.
         * 
         * @example: K x=ee(dot(a,b));if(xt==-128)printf("error %s\n", x->s);
         * 
         * 
         * If a function returns type K and has the option to return NULL, the
         * user should wrap the call with ee, and check for the error result, 
         * also considering that the error string pointer (x->s) may also be 
         * NULL. e.g.
         * 
        ​ * K x=ee(dot(a,b));if(xt==-128)printf("error %s\n", x->s?x->s:"");
         *
         * Otherwise the error status within the interpreter may still be set, 
         * resulting in the error being signalled incorrectly elsewhere in kdb+.
         * 
         * Calling ee(…) has the side effect of clearing the interpreter’s error 
         * status for the NULL result path.
         */
extern K ee(K),
        /**
         * @brief create timestamp or timespan.
         * 
         * @param I: -KP or -KN indicating type.
         * @param J: Create a timestamp or timespan from a number of nanos.
         * 
         */
         ktj(I, J),
        /**
         * @brief Create an atom of type I.
         */
         ka(I),
        /**
         * @brief Create boolean.
         */
         kb(I),
        /**
         * @brief Create byte
         */
         kg(I),
        /**
         * @brief Create short
         */
         kh(I),
        /**
         * @brief Create int 
         */
         ki(I),
        /**
         * @brief Create long
         */
         kj(J),
        /**
         * @brief Create real
         * 
         */
         ke(F),
        /**
         * @brief Create float
         */
         kf(F),
        /**
         * @brief Create char
         */
         kc(I),
        /**
         * @brief Create symbol
         */
         ks(S),
        /**
         * @brief Create data
         */
         kd(I),
        /**
         * @brief Create datetime
         */
         kz(F),
        /**
         * @brief Create time
         */
         kt(I),
        /**
         * @brief Put the function K f(I d){…} on the q main event loop given a
         *        socket d.
         *  
         * If d is negative, the socket is switched to non-blocking.
         *  
         * The function f should return NULL or a pointer to a K object, and 
         * its reference count will be decremented. (It is the return value of
         * f that will be r0’d – and only if not null.)
         * 
         * Shared library only.
         *  
         * On success, returns int K object containing d. On error, NULL is 
         * returned, d is closed.
         */
         sd1(I, K (*)(I)),
        /**
         * @brief Dynamic link
         * 
         * Function takes a C function that would take n K objects as arguments
         * and returns a K object. Shared library only.
         *  
         * Returns a q function.
         * @param f function: Function taking J K arguments 
         */
         dl(V *f, J),
        /**
         * @brief Create a mixed list.
         * 
         * @param I length
         * @param ... Takes ownership of references to arguments.
         */
         knk(I, ...),
        /**
         * @brief Create a char array from a string.
         */
         kp(S),
        /**
         * @brief Join value
         * 
         * Appends a raw value to a list. x points to a K object, which may be
         * reallocated during the function. The contents of x, i.e. *x, will be
         * updated in case of reallocation.
         * 
         * @returns pointer to the (potentially reallocated) K object.
         */
         ja(K *, V *),
        /**
         * @brief Appends an interned string s to a symbol list.
         * 
         * @returns pointer to the (potentially reallocated) K object.
         */
         js(K *, S),
        /**
         * @brief Join K objects.
         * 
         * Appends another K object to a mixed list. Takes ownership of a 
         * reference to its argument y.
         * 
         * Returns a pointer to the (potentially reallocated) K object.
         */
         jk(K *, K),
        /**
         * @brief Join K lists.
         * 
         * Append a K list y to K list x. Both lists must be of the same type.
         * 
         * @returns pointer to the (potentially reallocated) K object.
         */
         jv(K *k, K),
        /**
         * @brief evaluate. Takes ownership of references to its arguments.
         * 
         * handle>0, sends sync message to handle, to evaluate a string or 
         * function with parameters, and then blocks until a message of any 
         * type is received on handle. It can return NULL (indicating a 
         * network error) or a pointer to a K object. k(handle,(S)NULL) does 
         * not send a message, and blocks until a message of any type is 
         * received on handle. If that object has type -128, it indicates an 
         * error, accessible as a null-terminated string in r->s. When you have 
         * finished using this object, it should be freed by calling r0(r).
         * 
         * handle<0, this is for async messaging, and the return value can be 
         * either 0 (network error) or non-zero (success). This result should 
         * not be passed to r0.
         * 
         * handle==0 is valid only for a plugin, and executes against the kdb+ 
         * process in which it is loaded.
         * 
         * Note that a k call will block until a message is sent/received 
         * (handle!=0) or evaluated (handle=0).
         * 
         * @param I: handle
         * @param S: Command to be evaluated.
         * @param ...: Optional parameters are either local (shared library 
         *             only) or remote. The last argument must be NULL.
         * @returns depends on handle
         */
         k(I, const S, ...),
        /**
         * @brief Create a table from a dictionary object.
         * 
         * Will r0(x) and return null if it is unable to form a valid table. 
         * Takes ownership of a reference to its argument.
         */
         xT(K),
        /**
         * @brief Create a dictionary from two K objects.
         * 
         * Takes ownership of references to the arguments.
         * 
         * If y is null, will r0(x) and return null.
         */
         xD(K, K),
        /**
         * @brief Create simple table.
         * 
         * Create a simple table from a keyed table.
         *  
         * Takes ownership of a reference to its argument.
         */
         ktd(K),
        /**
         * @brief Increment an object‘s reference count.
         */
         r1(K),
        /**
         * @brief Signal C error
         * 
         * Kdb+ recognizes an error returned from a C function via the 
         * function’s return value being 0, combined with the value of a global
         * error indicator that can be set by calling krr with a 
         * null-terminated string. As krr records only the passed pointer, you 
         * should ensure that the string remains valid after the return from 
         * your code into kdb+ – typically you should use static storage for 
         * the string. (Thread-local if you expect to amend the error string 
         * from multiple threads.) The strings "stop", "abort" and "stack" are 
         * reserved values and krr must not be called with those.
         * 
         * Do not call krr() and then return a valid pointer! For convenience, 
         * krr returns 0, so it can be used directly
         */
         krr(const S),
        /**
         * @brief signal system error
         * 
         * Similar to krr, this appends a system-error message to string S 
         * before passing it to krr.
         * 
         * The system error message looks at errno/GetLastError and, if set, 
         * will format using strerror/FormatMessage.
         * 
         * The user error string is copied to a static, thread-local buffer 
         * and, as such, is valid until the next call to orr from that thread. 
         * However, the total message size (including both user and system 
         * error) is limited to 255 characters and is truncated if it exceeds 
         * this limit.
         */
         orr(const S),
        /**
         * @brief The same as the q function Apply, i.e. .[x;y]. Shared library 
         * only.
         * 
         * On success, returns a K object with the result of the . application. 
         * On error, NULL is returned. See ee for result-handling example.
         */
         dot(K, K),
        /**
         * @brief Serialise
         * 
         * Uses q IPC and mode capabilities level, where mode is:
         *  - -1 : valid for V3.0+ for serializing/deserializing within the 
         *         same process
         *    0  : unenumerate, block serialization of timespan and timestamp 
         *         (for working with versions prior to V2.6)
         *    1  : retain enumerations, allow serialization of timespan and 
         *         timestamp: Useful for passing data between threads
         *    2  : unenumerate, allow serialization of timespan and timestamp
         *    3  : unenumerate, compress, allow serialization of timespan and 
         *         timestamp
         * 
         * On success, returns a byte-array K object with serialized 
         * representation. On error, NULL is returned; use ee to retrieve error 
         * string.
         */
         b9(I, K),
        /**
         * @brief Deserialize
         * 
         * The byte array x is not modified.
         * 
         * On success, returns deserialized K object. On error, NULL is 
         * returned; use ee to retrieve the error string.
         */
         d9(K),
        /**
         * @brief A dictionary of settings similar to -26!x, or an error if SSL
         *        initialisation failed.
         * 
         * @return null if there was an error initializing the OpenSSL lib.
         */
         sslInfo(K x),
        /**
         * @brief va_list version of knk
         * 
         * va_list is as defined in stdarg.h, included by k.h
         */
         vaknk(I, va_list),
        /**
         * @brief va_list version of k
         * 
         * va_list is as defined in stdarg.h, included by k.h
         */
         vak(I, const S, va_list);

#ifdef __cplusplus 
}
#endif

// nulls(n?) and infinities(w?)
#define nh ((I)0xFFFF8000)
#define wh ((I)0x7FFF)
#define ni ((I)0x80000000)
#define wi ((I)0x7FFFFFFF)
#define nj ((J)0x8000000000000000LL) 
#define wj 0x7FFFFFFFFFFFFFFFLL
#if defined(WIN32) || defined(_WIN32)
#define nf (log(-1.0))
#define wf (-log(0.0))
#define finite _finite
extern double log(double);
#else  
#define nf (0/0.0)
#define wf (1/0.0)
#define closesocket(x) close(x)
#endif 

// remove more clutter
#define O printf
#define R return
#define Z static
#define P(x,y) {if(x)R(y);}
#define U(x) P(!(x),0)
#define SW switch
#define CS(n,x)	case n:x;break;
#define CD default

#define ZV Z V
#define ZK Z K
#define ZH Z H
#define ZI Z I
#define ZJ Z J
#define ZE Z E
#define ZF Z F
#define ZC Z C
#define ZS Z S

#define K1(f) K f(K x)
#define K2(f) K f(K x,K y)
#define TX(T,x) (*(T*)((G*)(x)+8))
#define xr x->r
#define xt x->t
#define xu x->u
#define xn x->n
#define xx xK[0]
#define xy xK[1]
#define xg TX(G,x)
#define xh TX(H,x)
#define xi TX(I,x)
#define xj TX(J,x)
#define xe TX(E,x)
#define xf TX(F,x)
#define xs TX(S,x)
#define xk TX(K,x)
#define xG x->G0
#define xH ((H*)xG)
#define xI ((I*)xG)
#define xJ ((J*)xG)
#define xE ((E*)xG)
#define xF ((F*)xG)
#define xS ((S*)xG)
#define xK ((K*)xG)
#define xC xG
#define xB ((G*)xG)

#endif

