#include <iostream>
#include <type_traits>
#include <sstream>
#include <stdexcept>

#include <string>
#include <vector>
#include <variant>

#include <kx/kx.h>

#include "qbind/memory_manager.h"
#include "qbind/atom.h"
#include "qbind/dictionary.h"
#include "qbind/vector.h"
#include "qbind/nested_vector.h"
#include "qbind/tuple.h"

#include "qbind/iterator.h"

#include "qbind/symbol.h"

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
    // std::variant<int, long> v1;
    // std::variant<const int, const long> v2;
    // v2 = v1;



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

struct k1{
    signed char m, a, t;
    C u;
    I r;
    union{
        G g;
        H h;
        I i;
        J j;
        E e;
        F f;
        S s;
        struct k0*k;
        struct {
            J n;
            G *G0;
        };
    };
};

void myK()
{
    k0 *myK0 = ktn(KJ, 1000);

    k1* myK1 = reinterpret_cast<k1*>(myK0);

    // k1 *myK1 = new k1();
    // myK1->t = KJ;
    // myK1->n = 10;

    // unsigned char myarr[40];
    // myarr[0] = 40;

    // std::cout << (int64_t)myarr << std::endl;
    // myK1->G0 = reinterpret_cast<unsigned char*>(myarr);

    // K myK0 = reinterpret_cast<::K>(myK1);
    // 24 bytes.

    std::cout << (int64_t)myK0 << std::endl;
    std::cout << (int64_t)myK1 << std::endl;   

    for (auto i = 0; i < 6; ++i)
    {
        std::cout << reinterpret_cast<int32_t *>(myK1)[i] << " " << reinterpret_cast<int32_t *>(myK0)[i] << std::endl;
    }
    std::cout << std::endl;

    std::cout << "================" << std::endl;
    std::cout << (long)(&(myK0->G0[0])) << std::endl;
    std::cout << (long)(&(myK1->G0)) << std::endl;

    unsigned char *k0begin = &(myK0->G0[0]);
    unsigned char **k1begin = &(myK1->G0);

    std::cout << "================" << std::endl;
    unsigned char *k1beginp = (unsigned char *)(void *)k1begin;

    std::cout << (long)(k0begin) << std::endl;
    std::cout << (long)(k1beginp) << std::endl;

    std::cout << "begin p2" << std::endl;
    unsigned char *k1beginp2 = reinterpret_cast<unsigned char *>(&myK1->G0);

    std::cout << (long)(k1beginp2) << std::endl;

    k1beginp2[100] = 99;

    std::cout << (long)(myK0->G0[100]) << std::endl;
    std::cout << "================" << std::endl;

    auto newbuf = new unsigned char[100];
    newbuf[100] = 200;

    myK1->G0 = newbuf;



    auto myK02 = reinterpret_cast<::K>(myK1);

    std::cout << (long)(myK02->G0[100]) << std::endl;


    //myK1->G0[0] = 55;
    //std::cout << (long)(myK0->G0[0]) << std::endl;

    std::cout << (long)myK0->t << std::endl;
    std::cout << (long)myK1->t << std::endl;

    std::cout << (long)myK0->n << std::endl;
    std::cout << (long)myK1->n << std::endl;
    std::cout << sizeof(k1) << " " << sizeof(k0) << std::endl;

    // std::cout << (long)myK0->G0 << std::endl;
    // std::cout << (long)myK1->G0 << std::endl;

    // std::cout << (int)myK0->G0[0] << std::endl;
    // std::cout << (int)myK1->G0[0] << std::endl;

    // auto begin = (J *)((myK0)->G0);
    // std::cout << (int64_t) (& (begin[5])) << std::endl;
    // std::cout << (int64_t)(&(reinterpret_cast<int64_t *>(myK1->G0)[5])) << std::endl;
    // begin[5] = 999;

    // std::cout << reinterpret_cast<int64_t *>(myK1->G0)[5] << std::endl;
}

void myK2()
{
    k1 *myK1 = new k1();
    myK1->t = KJ;
    myK1->n = 10;

    auto begin = new unsigned char[10];

    myK1->G0 = reinterpret_cast<unsigned char *>(begin[0]);

    std::cout << "begin " << (long)begin << std::endl;
    //std::cout << (long)(&(myK1->G0[0])) << std::endl;

    // for (auto i = 0; i < 10; ++i)
    //     myK1->G0[i] = i + 1;

    auto myK0 = reinterpret_cast<::K>(myK1);
    std::cout << (long)(&(myK0->G0[0])) << std::endl;
    std::cout << (long)(&(myK1->G0)) << std::endl;

    std::cout << (long)(myK0->G0[0]) << std::endl;
}

void print(::K k)
{
    std::cout << "m " << (long)reinterpret_cast<signed char *>(k)[0] << std::endl;
    std::cout << "a " << (long)reinterpret_cast<signed char *>(k)[1] << std::endl;
    std::cout << "t " << (long)reinterpret_cast<signed char *>(k)[2] << std::endl;
    std::cout << "u " << (long)reinterpret_cast<signed *>(k)[3] << std::endl;
    std::cout << "r " << (long)reinterpret_cast<int *>(k)[1] << std::endl;
    std::cout << "atom or n " << (long)reinterpret_cast<int64_t *>(k)[1] << std::endl;
    std::cout << "G0 " << (long)reinterpret_cast<int64_t *>(k)[2] << std::endl;
}

void print(k1* k)
{
    std::cout << "m " << (long)reinterpret_cast<signed char *>(k)[0] << std::endl;
    std::cout << "a " << (long)reinterpret_cast<signed char *>(k)[1] << std::endl;
    std::cout << "t " << (long)reinterpret_cast<signed char *>(k)[2] << std::endl;
    std::cout << "u " << (long)reinterpret_cast<signed *>(k)[3] << std::endl;
    std::cout << "r " << (long)reinterpret_cast<int *>(k)[1] << std::endl;
    std::cout << "atom or n " << (long)reinterpret_cast<int64_t *>(k)[1] << std::endl;
    std::cout << "G0 " << (long)reinterpret_cast<int64_t *>(k)[2] << std::endl;
}

int main()
{
    const char * const x = nullptr;
    //++x;
    // std::string str2("Hello");
    // char *ptr = const_cast<char *>(str2.data());
    // char **pptr;
    // pptr = &ptr;

    // std::cout << *pptr << std::endl;
    return 0;

    // This will allocate memory for a k0 struct and its array contiguously on the heap.
    size_t n = 1000;
    // TODO: Should this be sizeof(k0) - sizeof(k0::G) + (n)*sizeof(T) ??
    k0 *s = (k0*)malloc(sizeof(k0) + n * sizeof(char));

    s->G0[0] = 99;
    s->G0[999] = 128;

    std::cout << "m: " <<  (long)(&(s->m)) << std::endl;
    std::cout << "a: " <<  (long)(&(s->a)) << std::endl;
    std::cout << "t: " <<  (long)(&(s->t)) << std::endl;
    std::cout << "u: " <<  (long)(&(s->u)) << std::endl;
    std::cout << "r: " <<  (long)(&(s->r)) << std::endl;
    std::cout << "g: " <<  (long)(&(s->g)) << std::endl;
    std::cout << "h: " <<  (long)(&(s->h)) << std::endl;
    std::cout << "i: " <<  (long)(&(s->i)) << std::endl;
    std::cout << "j: " <<  (long)(&(s->j)) << std::endl;
    std::cout << "e: " <<  (long)(&(s->e)) << std::endl;
    std::cout << "f: " <<  (long)(&(s->f)) << std::endl;
    std::cout << "s: " <<  (long)(&(s->s)) << std::endl;
    std::cout << "k: " <<  (long)(&(s->k)) << std::endl;
    std::cout << "n: " <<  (long)(&(s->n)) << std::endl;
    std::cout << "G0: " <<  (long)(&(s->G0)) << std::endl;

    auto p = reinterpret_cast<char *>(s);

    std::cout << (long)p[16] << std::endl;
    std::cout << (long)p[1015] << std::endl;

    // auto iatom = ki(5);
    // auto ptr = reinterpret_cast<int64_t *>(iatom);
    // print(iatom);

    // K myK0 = ktn(KJ, 743);
    // ((J *)((myK0)->G0))[0] = 696969;

    // r1(myK0);
    // print(myK0);
    // auto myK1fake = reinterpret_cast<k1*>(myK0);
    // print(myK1fake);
    // std::cout << "===============================" << std::endl;
    // k1 *myK1 = new k1;
    // myK1->t = 7;
    // myK1->r = 1;
    // myK1->n = 1000;
    // auto buf = new int64_t[1000];
    // std::cout << (long)buf << std::endl;
    // buf[0] = 696969;
    // myK1->G0 = reinterpret_cast<unsigned char *>(buf);

    // print(myK1);
    // auto myK0fake = reinterpret_cast<::K>(myK1);
    // print(myK0fake);

// std::cout << "here" << (long)(myK0fake->G0) << std::endl;
// std::cout << "here" << (long)(myK0fake->G0[0]) << std::endl;

//     std::cout << ((J *)((myK0)->G0))[0] << std::endl;
//     buf[0] = 747474;
//     std::cout << ((J *)((myK0)->G0))[0] << std::endl;

//     std::cout << "===============================" << std::endl;
    // ((J *)((myK0fake)->G0))[0] = 747474;
    // ((J *)((myK0fake)->G0))[1] = 654321;

    // print(myK1);
    // print(myK0fake);

    // std::cout << buf[0] << std::endl;
    // myK();
    return 0;

    qbind::MemoryManager::initialise();


    //std::tuple<int,long,short> t;
    

    // Checks
    check_element_access();
    return 0;
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

    std::cout << "here" <<  (qatom.type == qbind::Type::Symbol) << (qatom.structure == qbind::Structure::Atom) << std::endl;

    // This throws if not const
    std::string_view qatomdata = qatom;
    std::cout << (long)qatomdata.data() << std::endl;
    std::cout << qatomdata << std::endl;

    qatom = "Goodbye";
    std::string_view qatomdata2 = qatom;
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
