#include <iostream>
#include <type_traits>
#include <sstream>
#include <stdexcept>

#include <string>
#include <vector>

#include <kx/kx.h>

#include "qbind/memory_manager.h"
#include "qbind/atom.h"
#include "qbind/vector.h"
#include "qbind/tuple.h"

void print(std::string x)
{
    std::cout << x << std::endl;
}

// using namespace qbind::literals;

constexpr bool strings_equal(const char* a, char const * b) {
    return std::string_view(a)==b;
}

void task1()
{
    std::cout << "in task 1" << std::endl;
}

/**
 * @brief Check element access:
 * 
 * TODO: at: get and assign. check throws.
 * []: get and assign.
 * front(): get and TODO: assign.
 * back(): get and TODO: assign.
 * TODO: data()
 * 
 */
void check_element_access()
{
    qbind::Vector<qbind::Type::Short> vec{1, 2, 3, 4, 5};

    std::cout << std::to_string(vec.front()) << " " << std::to_string(vec.back()) << std::endl;
    for (auto i = 0; i < vec.size(); ++i)
    {
        std::cout << std::to_string(vec[i]);
        vec[i] = (i + 1) * 2;
        std::cout << " -> " << std::to_string(vec[i]) << std::endl;
    }

    qbind::Vector<qbind::Type::Symbol> vec2{"hello", "bonjour", "hallo"};

    std::cout << vec2.front() << " " << vec2.back() << std::endl;
    for (auto i = 0; i < vec2.size(); ++i)
    {
        std::cout << vec2[i];
        vec2[i] = "foobarbaz";
        std::cout << " -> " << vec2[i] << std::endl;
    }
  
}

qbind::Tuple<
    qbind::Atom<qbind::Type::Boolean>,
    qbind::Atom<qbind::Type::Long>
> check_tuple_type_checking()
{
    ::K inner = knk(1, kb(1));

    ::K mixed = knk(3, kb(1), ktn(KJ, 5), inner);

    auto b = qbind::Atom<qbind::Type::Boolean>{true};
    auto l = qbind::Atom<qbind::Type::Long>{999};

    return qbind::Tuple<qbind::Atom<qbind::Type::Boolean>,
                 qbind::Atom<qbind::Type::Long>>{
        std::move(b), std::move(l)};

    // qbind::Tuple<
    //     qbind::Atom<qbind::Type::Boolean>,
    //     qbind::Vector<qbind::Type::Long>,
    //     qbind::Tuple<
    //         qbind::Atom<qbind::Type::Long>
    //     >
    // >::check_type_match<0,0>(mixed);

    //qbind::Tuple<qbind::Atom<qbind::Type::Boolean>, qbind::Vector<qbind::Type::Long>>::check_type_match<0,0>(ki(5));
}

int main()
{
    qbind::initialise();


    //std::tuple<int,long,short> t;
    

    // Checks
    //check_element_access();
    {
        auto x = check_tuple_type_checking();
        std::cout << "START" << std::endl;
    }

    char *str = "Hello world";
    auto ptr2 = ss(str);
    auto symbol = ks(ptr2);
    std::cout << (long)symbol->s << std::endl;
    std::cout << ptr2 << std::endl;
    std::cout << symbol->s << std::endl;

    qbind::K qsymbol(symbol);

    qbind::Atom<qbind::Type::Symbol> qatom("qsymbol");

    std::cout << "here" <<  (qatom.TypeInfo == qbind::TypeClass(-11)) << std::endl;

    // This throws if not const
    const char *qatomdata = qatom;
    std::cout << (long)qatomdata << std::endl;
    std::cout << qatomdata << std::endl;

    qatom = "Goodbye";
    const char *qatomdata2 = qatom;
    std::cout << qatomdata2 << std::endl;
    std::cout << qatom << std::endl;

    auto short_ = kh(125);
    qbind::K qshort(short_);
    qbind::Atom<qbind::Type::Short> qsatom(15);
    
    short h = qsatom;
    std::cout << qsatom << std::endl;


    auto badType = qbind::Type(99);
    std::cout << (int32_t)badType << std::endl;
    auto descAtom = ka(12);
    std::cout << descAtom << std::endl;
    descAtom->h = 45;
    std::cout << "Type: " << (int)descAtom->t << "\n"
              << "RefCount: " << descAtom->r << "\n"
              << "Length: " << descAtom->n << "\n";

    auto mixedAtom = ka(0);
    std::cout << "Type: " << (int)mixedAtom->t << "\n"
              << "RefCount: " << mixedAtom->r << "\n"
              << "Length: " << mixedAtom->n << "\n";    


    auto mixedList = ktn(0, 50);
    std::cout << "Type: " << (int)mixedList->t << "\n"
              << "RefCount: " << mixedList->r << "\n"
              << "Length: " << mixedList->n << "\n";
    ktn(-1, 50);

    std::thread t1(task1);
    t1.join();

    //auto x = qbind::Column("hello", qbind::Integer());
    // auto x = "hello"_ki;

    // std::cout << x.header() << std::endl;
    // auto atom = ki(99);
    // auto singleton = ktn(KI, 1);
    // // start and end with 0. The r0 on a ref count 0 object causes deallocation
    // std::cout << "ref count atom: " << atom->r << std::endl;
    // std::cout << "ref count atom: " << singleton->r << std::endl;

    // reinterpret_cast<int *>(singleton->G0)[0] = 99999;
    // std::cout << qbind::Converter().convert<int>(atom) << std::endl;
    // std::cout << qbind::Converter().convert<int>(singleton) << std::endl;

    // std::cout << "ref count atom: " << atom->r << std::endl;
    // std::cout << "ref count atom: " << singleton->r << std::endl;

    // auto arr = ktn(KI, 5);

    // auto ptr = reinterpret_cast<int*>(arr->G0);
    // ptr[0] = 1;
    // ptr[1] = 2;
    // ptr[2] = 3;
    // ptr[3] = 4;
    // ptr[4] = 5;

    // auto span = qbind::Converter().convert<qbind::Span<qbind::Integer>>(arr);
    // for (const auto &x: span)
    // {
    //     std::cout << x << std::endl;
    // }
    // span[3] = 999;
    // for (const auto &x: span)
    // {
    //     std::cout << x << std::endl;
    // }
    // std::cout << "done" << std::endl;

    // auto atom2 = ki(99);
    // std::cout << atom2->i << std::endl;
    // auto span2 = qbind::Converter().to_cpp<qbind::SimpleSpan<qbind::Integer>>(atom2); 
    // std::cout << atom2->i << std::endl;

    // std::cout << *&(atom2->i) << std::endl;

    // std::cout << std::to_string(atom->i) << std::endl;
    // std::cout << std::to_string(ptr[0]) << std::endl;
    // std::cout << std::to_string(ptr[4]) << std::endl;

    // void *atom_ptr = static_cast<void *>(&atom->g);
    // void *arr_ptr = static_cast<void *>(arr->G0);

    // std::cout << *reinterpret_cast<int *>(atom_ptr) << std::endl;
    // std::cout << *reinterpret_cast<int *>(arr_ptr) << std::endl;
    // std::cout << reinterpret_cast<int *>(arr_ptr)[4] << std::endl;

    //qbind::Type<qbind::Q::Integer, int> x;
    //qbind::BasicType<qbind::Q::Integer> x;
    //std::cout << std::to_string(x.getValue(5)) << std::endl;



    // int x = 100;
    // qbind::Ref<qbind::String> ref(x);
    // print(ref);
    // ref = "95";
    // print(ref);
    // x += 200;
    // print(ref);

    // std::cout << "Const ref " << std::endl;

    // qbind::ConstRef<qbind::String> cref(x);
    // print(cref);
    // cref = "95";
    // print(cref);
    

    // std::vector<int> vec{1, 2, 3, 4, 5};
    // auto it = vec.cbegin();
    // *it = 9;

    // std::cout << qbind::to_name(0) << std::endl;
    // std::cout << qbind::to_name(-5) << std::endl;
    // std::cout << qbind::to_name(14) << std::endl;
    // // qbind::detail::Converter<int, int> x;
    // // qbind::detail::Converter<int, double> y;

    // qbind::Type<0> type;
    // std::cout << type.name << std::endl;
    // qbind::Type<1> type1;
    // std::cout << type1.name << std::endl;
    // qbind::Type<2> type2;

    // // convert to Kdb
    // qbind::Convert<int>(0);
    // // convert to Cpp
    // //qbind::Convert<int>(static_cast<K>(nullptr));

    // // result can be put through c++filt -t [output] to get type with issue
    // // however the result doesn't change between const, const ref, universal ref/ rvalue ref
    // // These should be detectable by traits though.
    // std::cout << "Return: " << typeid(qbind::ResultType<decltype(add)>).name() << std::endl;
    // std::cout << "Type | Const | lvalue ref | rvalue ref" << std::endl;
    // std::cout << typeid(qbind::ArgType<decltype(add), 0>).name()
    //           << std::is_const_v<qbind::ArgType<decltype(add), 0>>
    //           << std::is_lvalue_reference_v<qbind::ArgType<decltype(add), 0>>
    //           << std::is_rvalue_reference_v<qbind::ArgType<decltype(add), 0>> << std::endl;

    // // This doesn't seem to give true for a const lvalue - doesn't matter though as its copy either way
    // std::cout << typeid(qbind::ArgType<decltype(add), 1>).name()
    //           << std::is_const_v<qbind::ArgType<decltype(add), 1>>
    //           << std::is_lvalue_reference_v<qbind::ArgType<decltype(add), 1>>
    //           << std::is_rvalue_reference_v<qbind::ArgType<decltype(add), 1>> << std::endl;

    // std::cout << typeid(qbind::ArgType<decltype(add), 2>).name()
    //           << std::is_const_v<std::remove_reference_t<qbind::ArgType<decltype(add), 2>>>
    //           << std::is_lvalue_reference_v<qbind::ArgType<decltype(add), 2>>
    //           << std::is_rvalue_reference_v<qbind::ArgType<decltype(add), 2>> << std::endl;

    // std::cout << typeid(qbind::ArgType<decltype(add), 3>).name()
    //           << std::is_const_v<std::remove_reference_t<qbind::ArgType<decltype(add), 3>>>
    //           << std::is_lvalue_reference_v<qbind::ArgType<decltype(add), 3>>
    //           << std::is_rvalue_reference_v<qbind::ArgType<decltype(add), 3>> << std::endl;

    // std::cout << typeid(qbind::ArgType<decltype(add), 4>).name()
    //           << std::is_const_v<std::remove_reference_t<qbind::ArgType<decltype(add), 4>>>
    //           << std::is_lvalue_reference_v<qbind::ArgType<decltype(add), 4>>
    //           << std::is_rvalue_reference_v<qbind::ArgType<decltype(add), 4>> << std::endl;

    // std::cout << typeid(qbind::ArgType<decltype(add), 5>).name()
    //           << std::is_const_v<std::remove_reference_t<qbind::ArgType<decltype(add), 5>>>
    //           << std::is_lvalue_reference_v<qbind::ArgType<decltype(add), 5>>
    //           << std::is_rvalue_reference_v<qbind::ArgType<decltype(add), 5>> << std::endl;


    // Converter<int> x;
    // x.convert();
    // Converter<float> y;
    // y.convert();

}
