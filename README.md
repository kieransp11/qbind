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

## TODO

- Write lazy spans which convert lazily on iteration and index access. 
- Write as many composable converters as possible
- Try writing converters for constructible types
- Investigate boost::hana as a way to rapidly bind templated functions.
