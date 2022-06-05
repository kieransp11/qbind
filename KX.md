# The KX KDB API is defined in k.h

Most of the functions fall in to a few basic categories.

## Making connections
- hostname: 0.0.0.0 opens a unix domain socket.
- port
- credentials: [username]:[password]
- timeout: Use 0 for no timeout
- capability: Bitfield (1 = 1 TB limit, 2 = use TLS)
```
I khpunc(S hostname,I port,S credentials,I timeout,I capability),
I khpun(const S hostname, I port, const S credentials, I timeout)
I khpu(const S hostname, I port, const S credentials)
I khp(const S hostname, I port)
```
Close the handle
```
V kclose(I)
```
Evaluate query
```
K k(I handle, const S s, …)
```
Evaluate query using valist
```
K vak(I handle,const S,va_list)
```

TODO: K sslInfo(K x)

## Memory management
Decrement an object's reference count.

If x->r is 0, x is unusable after the r0(x) call, and the memory pointed to by it may have been freed.

Note: Reference counting stats and ends with 0, not 1. This means it is best to think of k0::r as the number known copies being tracked.

This will SEGFAULT on a null pointer.
```
DONE V r0(K)
```

Increment an object's reference count.
```
DONE K r1(K)
```

Release the memory allocated for the thread’s pool.

Call when the thread is about to complete, releasing the memory allocated for that thread’s pool.
```
DONE V m9(V)
```

Set the lock for interning symbols in multithreaded environments
```
DONE I setm(I m)
```

## Symbol utilities
Intern a (null-terminated) string.
- string: String to intern.
- count: Number of character to intern.
```
S sn(S string, I count)
S ss(S string)
```

## Atom creation
Create an atom of type I.
```
DONE K ka(I type)
```
There are also type specific helpers.
```
DONE: K kb(I) / boolean
DONE: K kc(I) / char
DONE: K kd(I) / date
DONE: K ke(F) / real (take F instead of E)
DONE: K kf(F) / double
DONE: K kg(I) / byte
DONE: K kh(I) / short
DONE: K ki(I) / int
DONE: K kj(J) / long
DONE: K ks(S) / create symbol (Intern before as precaution)
DONE: K kt(I) / create time
DONE: K ktj(-KP, J) / create timestamp
DONE: K ktj(-KN, J) / create timespan
DONE: K ku(U) / create guid
DONE: K kz(F) / datetime
```

## Vector creation
Creating a vector can be done using:
```
DONE: K ktn(I type, J length)
DONE: K kp(S) / char-array from string.
DONE: K kpn(S x, J n) / char-array from string with n chars.
```

## Vector appending
All of the below methods append the right hand argument to the left hand argument, returning the potentially reallocated array.
```
/ value to list
DONE: K ja(K* x, V*)
/ K object to mixed list
DONE: K jk(K* x, K y)
/ interned symbol to a symbol list
DONE: K js(K* x, S s)
Append two lists of the same type
DONE: K jv(K* x, K y)
```

## Other creation
Create a dictionary from two k objects
```
K xD(K x, K y)
```
Create a table from a dictionary object
```
K xT(K x)
```
Create keyed table
```
K knt(J n, K x)
```
Create a simple table from a keyed table
```
K ktd(K x)
```

DONE: K knk(I n, …)
SKIP: K vaknk(I, va_list)

## Error handling
Capture (and reset) error string into usual error object, e.g.

Example: `K x=ee(dot(a,b));if(xt==-128)printf("error %s\n", x->s);`

If a function returns type K and has the option to return NULL, the user should wrap the call with ee, and check for the error result,  also considering that the error string pointer (x->s) may also be NULL. e.g.

​`K x=ee(dot(a,b));if(xt==-128)printf("error %s\n", x->s?x->s:"");`

Otherwise the error status within the interpreter may still be set, resulting in the error being signalled incorrectly elsewhere in kdb+.

Calling `ee(…)` has the side effect of clearing the interpreter’s error 
status for the NULL result path.
```
K ee(K)
```

TODO: K krr(const S)
TODO: K orr(const S)

////////////////////////////////////////////////////////////////////////////////

## Serialization
Verify that the byte vector x is a valid IPC message. Decompressed data only. x is not modified. Returns 0 if not valid.
```
I okx(K)
```

TODO: b9, d9.

## Date-format
Encode a year/month/day as a q date, e.g. 0==ymd(2000, 1, 1)
```
I ymd(I,I,I)
```
Convert a date to a yyyymmdd integer.
```
I dj(I)
```

## Callback Management (Shared library only)
https://code.kx.com/q/interfaces/c-client-for-q/#callbacks
Remove the callback of descriptor.
- descriptor: Descriptor to remove callback on.
- f: Call kclose on descriptor if 1 (kclose is called for sd0).
```
V sd0(I descriptor);
V sd0x(I descriptor, I f)
```
Set function on loop
Put the function K f(I d){…} on the q main event loop given a socket d.

If d is negative, the socket is switched to non-blocking.

The function f should return NULL or a pointer to a K object, and its reference count will be decremented. (It is the return value of f that will be r0’d – and only if not null.)

Shared library only.

On success, returns int K object containing d. On error, NULL is returned, d is closed.
```
K sd1(I d, f)
```

## Miscellaneous
Get version release date as yyyymmdd.
```
I ver()
```

TODO: dl
TODO: dot
TODO: sslInfo