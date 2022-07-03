# QBind

This project aims to provide easy export of C++ functions to kdb. It does this by providing an export Macro which will try to map between your functions arguments and return type, and the `K` array object used internally by kdb.

This is not designed to be a client library for kdb. As such no efforts are made to support OpenSSL connections etc. (although there is a CMake find module which you can use to help link the necessary components to make SSL connections to KDB). 

## Requirements

You will require the following to build this project:

- A C++ compiler
- CMake
- An internet connection (for CMake fetch content)
- Boost (Preprocessor, FunctionTypes, MPL)
- [Optional] Ninja for incremental builds.

## Overview

QBind is made up of several core concepts which build on top of each other. The concepts are as followed:

- `UntypedSpan`: An RAII wrapper around KX's `K` pointer.
- `Q::Type<Code, Underlier>`: These types map KDB type codes to underlying C types. Each existing KDB type has an alias provided for convenience.
- `Type<QType, Target>`: This type provides conversion methods from the underlying type of the QType to the target type and vice versa. There are three methods which are part of this interface:
    - `void setValue(Underlier& u, Target&& v)`: Move value into underlier referred to by `u`.
    - `void setValue(Underlier& u, const Target& v)`: Copy value into underlier referred to by `u`.
    - `Target getValue(const Underlier& u)`: Convert underling value into target type. A copy is taken. This is efficient even in the identity case as the size of Q atoms are all small (<= 16 bytes).

- `Ref<Type>`/`ConstRef<Type>`: A wrapped around a reference to the underlier of type which allows assigning to it and reading from it as the native type.
- `Span<Type>`: A span with the underlying type of the type. Backed by an untyped span which must have the same type code as the type.
- `SpanIterator<RefType>`: An iterator for `Span<Type>`. `RefType` is either `Ref<Type>` or `ConstRef<Type>`, indicating a const or non-const iterator.

This gives us traditional memory managed vectors with a familiar interface from `std::span`. We then also define `UntypedMap` and `UntypedTable` which encapsulate maps and tables respectively. These have typed counterparts `Map<KeySpan, ValueSpan>` and `Table<ColumType...>`. All of these can be then effective be constructed via visiting the defined `Q::Type` of a `K` object and checking dimensions/type codes at each level.

- Atoms: It will either be a true atom (negative type code) or a singleton list. By visiting on its type code we can get `Q::Type`, and by combining with that target type `Target` we get `Type<QType, Target>` which provides the necessary conversions.
- Vectors: These are mapped to spans. An atom can trivially be a span of length one. The check for an existing conversion is not done until element access.
- Map: An untyped Map is made up of a `K` object of type 99. This is a mixed list with two elements, keys and values. These can be made in to untyped spans.
- Table: An untyped Table is made up of a `K` object of type 98. This is a mixed list of two elements, columns names and column values. This can be made in to a symbol span and a mixed span. 

Mixed arrays are `K` objects filled with other `K` objects. In many cases the length and type of objects in a mixed array is assumed. For this reason we provide a convenience binding to `std::tuple`. This allows tables to be more conveniently defined as we can type check each element of the column value mixed array against the columns proposed value in the tuple. 

### FEAT:
- Implement table interface.
- Implement keyed table interface.