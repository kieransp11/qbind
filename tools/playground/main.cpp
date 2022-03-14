#include <iostream>
#include <type_traits>

#include "qbind/function.h"

// by value, const by value
// by ref, const ref
// by rvalue ref, cont rvalue ref
int64_t add(
    int16_t i, const int32_t j, 
    int16_t& k, const int32_t& l,
    int16_t&& m, const int32_t&& n)
{
    return i + j;
}

template<class T>
struct Converter
{
    T convert()
    {
        throw std::invalid_argument("Bad argument");
    }
};

template<>
struct Converter<int>
{
    int convert()
    {
        std::cout << "Convert to int" << std::endl;
        return 0;
    }
};

#define STRi(x) #x
#define STR(x) STRi(x)

int main()
{

    // result can be put through c++filt -t [output] to get type with issue
    // however the result doesn't change between const, const ref, universal ref/ rvalue ref
    // These should be detectable by traits though.
    std::cout << "Return: " << typeid(qbind::ResultType<decltype(add)>).name() << std::endl;
    std::cout << "Type | Const | lvalue ref | rvalue ref" << std::endl;
    std::cout << typeid(qbind::ArgType<decltype(add), 0>).name()
              << std::is_const_v<qbind::ArgType<decltype(add), 0>>
              << std::is_lvalue_reference_v<qbind::ArgType<decltype(add), 0>>
              << std::is_rvalue_reference_v<qbind::ArgType<decltype(add), 0>> << std::endl;

    // This doesn't seem to give true for a const lvalue - doesn't matter though as its copy either way
    std::cout << typeid(qbind::ArgType<decltype(add), 1>).name()
              << std::is_const_v<qbind::ArgType<decltype(add), 1>>
              << std::is_lvalue_reference_v<qbind::ArgType<decltype(add), 1>>
              << std::is_rvalue_reference_v<qbind::ArgType<decltype(add), 1>> << std::endl;

    std::cout << typeid(qbind::ArgType<decltype(add), 2>).name()
              << std::is_const_v<std::remove_reference_t<qbind::ArgType<decltype(add), 2>>>
              << std::is_lvalue_reference_v<qbind::ArgType<decltype(add), 2>>
              << std::is_rvalue_reference_v<qbind::ArgType<decltype(add), 2>> << std::endl;

    std::cout << typeid(qbind::ArgType<decltype(add), 3>).name()
              << std::is_const_v<std::remove_reference_t<qbind::ArgType<decltype(add), 3>>>
              << std::is_lvalue_reference_v<qbind::ArgType<decltype(add), 3>>
              << std::is_rvalue_reference_v<qbind::ArgType<decltype(add), 3>> << std::endl;

    std::cout << typeid(qbind::ArgType<decltype(add), 4>).name()
              << std::is_const_v<std::remove_reference_t<qbind::ArgType<decltype(add), 4>>>
              << std::is_lvalue_reference_v<qbind::ArgType<decltype(add), 4>>
              << std::is_rvalue_reference_v<qbind::ArgType<decltype(add), 4>> << std::endl;

    std::cout << typeid(qbind::ArgType<decltype(add), 5>).name()
              << std::is_const_v<std::remove_reference_t<qbind::ArgType<decltype(add), 5>>>
              << std::is_lvalue_reference_v<qbind::ArgType<decltype(add), 5>>
              << std::is_rvalue_reference_v<qbind::ArgType<decltype(add), 5>> << std::endl;


    // Converter<int> x;
    // x.convert();
    // Converter<float> y;
    // y.convert();

}
