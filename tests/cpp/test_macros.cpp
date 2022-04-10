#include <catch2/catch.hpp>

#include "qbind/function.h"

#define STRi(x) #x
#define STR(x) STRi(x)

/**
 * @brief It does not matter that the strings here are "messy code". It only
 * matters that the code generate is valid. 
 */
TEST_CASE("QBIND_PARAMETERS")
{
    // No arguments
    std::string res = STR(QBIND_PARAMETERS(0));
    REQUIRE("()" == res);
    // One argument
    res = STR(QBIND_PARAMETERS(1));
    REQUIRE("(K karr0)" == res);
    // Two arguments
    res = STR(QBIND_PARAMETERS(2));
    REQUIRE("(K karr0 , K karr1)" == res);
    // N arguments
    res = STR(QBIND_PARAMETERS(8));
    REQUIRE("(K karr0 , K karr1 , K karr2 , K karr3 , K karr4 , K karr5 , K karr6 , K karr7)" == res);
}

/**
 * @brief Again, we are checking here the signature is valid, not clean.
 */
TEST_CASE("QBIND_FN_SIGNATURE")
{
    // No arguments, K return, function named x.
    std::string res = STR(QBIND_FN_SIGNATURE(x, 0));
    REQUIRE("K x ()" == res);

    // One argument, K return, function named x.
    res = STR(QBIND_FN_SIGNATURE(x, 1));
    REQUIRE("K x (K karr0)" == res);

    // Two arguments, K return, function named x.
    res = STR(QBIND_FN_SIGNATURE(x, 2));
    REQUIRE("K x (K karr0 , K karr1)" == res);

    // N arguments, K return, function named x.
    res = STR(QBIND_FN_SIGNATURE(x, 8));
    REQUIRE("K x (K karr0 , K karr1 , K karr2 , K karr3 , K karr4 , K karr5 , K karr6 , K karr7)" == res);
}

TEST_CASE("QBIND_ARGUMENTS")
{
    // No arguments
    std::string res = STR(QBIND_ARGUMENTS(fn, 0));
    REQUIRE("()" == res);
    // One argument
    res = STR(QBIND_ARGUMENTS(fn, 1));
    REQUIRE("(c.to_cpp<qbind::ArgType<decltype(fn) , 0>(karr0))" == res);
    // Two arguments
    res = STR(QBIND_ARGUMENTS(fn, 2));
    REQUIRE("(c.to_cpp<qbind::ArgType<decltype(fn) , 0>(karr0) ,"
            " c.to_cpp<qbind::ArgType<decltype(fn) , 1>(karr1))" == res);
    // N arguments
    res = STR(QBIND_ARGUMENTS(fn, 8));
    REQUIRE("(c.to_cpp<qbind::ArgType<decltype(fn) , 0>(karr0) ,"
            " c.to_cpp<qbind::ArgType<decltype(fn) , 1>(karr1) ,"
            " c.to_cpp<qbind::ArgType<decltype(fn) , 2>(karr2) ,"
            " c.to_cpp<qbind::ArgType<decltype(fn) , 3>(karr3) ,"
            " c.to_cpp<qbind::ArgType<decltype(fn) , 4>(karr4) ,"
            " c.to_cpp<qbind::ArgType<decltype(fn) , 5>(karr5) ,"
            " c.to_cpp<qbind::ArgType<decltype(fn) , 6>(karr6) ,"
            " c.to_cpp<qbind::ArgType<decltype(fn) , 7>(karr7))" == res);
}

TEST_CASE("QBIND_CALL_SIGNATURE")
{
    // No arguments, void return, function named x.
    std::string res = STR(QBIND_CALL_SIGNATURE(0, x, 0));
    REQUIRE("(x ());" == res);
    // No arguments, K return, function named x.
    res = STR(QBIND_CALL_SIGNATURE(1, x, 0));
    REQUIRE("return c.to_q<qbind::ResultType<decltype(x)>> (x "
            "()"
            ");" == res);

    // One argument, void return, function named x.
    res = STR(QBIND_CALL_SIGNATURE(0, x, 1));
    REQUIRE("(x (c.to_cpp<qbind::ArgType<decltype(x) , 0>(karr0)));" == res);
    // One argument, K return, function named x.
    res = STR(QBIND_CALL_SIGNATURE(1, x, 1));
    REQUIRE("return c.to_q<qbind::ResultType<decltype(x)>> (x "
            "(c.to_cpp<qbind::ArgType<decltype(x) , 0>(karr0))"
            ");" == res);

    // Two arguments, void return, function named x.
    res = STR(QBIND_CALL_SIGNATURE(0, x, 2));
    REQUIRE("(x "
            "(c.to_cpp<qbind::ArgType<decltype(x) , 0>(karr0) ,"
            " c.to_cpp<qbind::ArgType<decltype(x) , 1>(karr1))"
            ");" == res);
    // Two arguments, K return, function named x.
    res = STR(QBIND_CALL_SIGNATURE(1, x, 2));
    REQUIRE("return c.to_q<qbind::ResultType<decltype(x)>> (x "
            "(c.to_cpp<qbind::ArgType<decltype(x) , 0>(karr0) ,"
            " c.to_cpp<qbind::ArgType<decltype(x) , 1>(karr1))"
            ");" == res);

    // N arguments, void return, function named x.
    res = STR(QBIND_CALL_SIGNATURE(0, x, 8));
    REQUIRE("(x "
            "(c.to_cpp<qbind::ArgType<decltype(x) , 0>(karr0) ,"
            " c.to_cpp<qbind::ArgType<decltype(x) , 1>(karr1) ,"
            " c.to_cpp<qbind::ArgType<decltype(x) , 2>(karr2) ,"
            " c.to_cpp<qbind::ArgType<decltype(x) , 3>(karr3) ,"
            " c.to_cpp<qbind::ArgType<decltype(x) , 4>(karr4) ,"
            " c.to_cpp<qbind::ArgType<decltype(x) , 5>(karr5) ,"
            " c.to_cpp<qbind::ArgType<decltype(x) , 6>(karr6) ,"
            " c.to_cpp<qbind::ArgType<decltype(x) , 7>(karr7))"
            ");" == res);
    // N arguments, K return, function named x.
    res = STR(QBIND_CALL_SIGNATURE(1, x, 8));
    REQUIRE("return c.to_q<qbind::ResultType<decltype(x)>> (x "
            "(c.to_cpp<qbind::ArgType<decltype(x) , 0>(karr0) ,"
            " c.to_cpp<qbind::ArgType<decltype(x) , 1>(karr1) ,"
            " c.to_cpp<qbind::ArgType<decltype(x) , 2>(karr2) ,"
            " c.to_cpp<qbind::ArgType<decltype(x) , 3>(karr3) ,"
            " c.to_cpp<qbind::ArgType<decltype(x) , 4>(karr4) ,"
            " c.to_cpp<qbind::ArgType<decltype(x) , 5>(karr5) ,"
            " c.to_cpp<qbind::ArgType<decltype(x) , 6>(karr6) ,"
            " c.to_cpp<qbind::ArgType<decltype(x) , 7>(karr7))"
            ");" == res);
}

/**
 * @brief This test shows what the final generated source for an exported function
 * should look like. 
 */
TEST_CASE("QBIND_FN_EXPORT")
{
    // No arguments void function
    std::string res = STR(QBIND_FN_EXPORT(na, qna, 0, 0));
    REQUIRE("{ "
                "extern \"C\" "
                "{ "
                    "K qna () "
                    "{ "
                        "qbind::helpers::NonConstLvalueRefArgChecker<decltype(na)>(); "
                        "qbind::Converter c; "
                        "try { "
                            "(na ()); "
                        "} catch (const std::exception& e) { "
                            "thread_local std::string qna_errmsg; "
                            "qna_errmsg = e.what(); "
                            "return krr(qna_errmsg.c_str()); "
                        "} " 
                        "return knk(0); "
                    "} "
                "} "
            "}" == res);

    // No arguments returning function
    res = STR(QBIND_FN_EXPORT(na, qna, 0, 1));
    REQUIRE("{ "
                "extern \"C\" "
                "{ "
                    "K qna () "
                    "{ "
                        "qbind::helpers::NonConstLvalueRefArgChecker<decltype(na)>(); "
                        "qbind::Converter c; "
                        "try { "
                            "return c.to_q<qbind::ResultType<decltype(na)>> (na ()); "
                        "} catch (const std::exception& e) { "
                            "thread_local std::string qna_errmsg; "
                            "qna_errmsg = e.what(); "
                            "return krr(qna_errmsg.c_str()); "
                        "} " 
                        "return knk(0); "
                    "} "
                "} "
            "}" == res);    

    // One argument void function
    res = STR(QBIND_FN_EXPORT(id, qid, 1, 0));
    REQUIRE("{ "
                "extern \"C\" "
                "{ "
                    "K qid (K karr0) "
                    "{ "
                        "qbind::helpers::NonConstLvalueRefArgChecker<decltype(id)>(); "
                        "qbind::Converter c; "
                        "try { "
                            "(id (c.to_cpp<qbind::ArgType<decltype(id) , 0>(karr0))); "
                        "} catch (const std::exception& e) { "
                            "thread_local std::string qid_errmsg; "
                            "qid_errmsg = e.what(); "
                            "return krr(qid_errmsg.c_str()); "
                        "} " 
                        "return knk(0); "
                    "} "
                "} "
            "}" == res);

    // One argument returning function
    res = STR(QBIND_FN_EXPORT(id, qid, 1, 1));
    REQUIRE("{ "
                "extern \"C\" "
                "{ "
                    "K qid (K karr0) "
                    "{ "
                        "qbind::helpers::NonConstLvalueRefArgChecker<decltype(id)>(); "
                        "qbind::Converter c; "
                        "try { "
                            "return c.to_q<qbind::ResultType<decltype(id)>> (id "
                            "(c.to_cpp<qbind::ArgType<decltype(id) , 0>(karr0))"
                            "); "
                        "} catch (const std::exception& e) { "
                            "thread_local std::string qid_errmsg; "
                            "qid_errmsg = e.what(); "
                            "return krr(qid_errmsg.c_str()); "
                        "} " 
                        "return knk(0); "
                    "} "
                "} "
            "}" == res);  

    // Two arguments void function
    res = STR(QBIND_FN_EXPORT(add, qadd, 2, 0));
    REQUIRE("{ "
                "extern \"C\" "
                "{ "
                    "K qadd (K karr0 , K karr1) "
                    "{ "
                        "qbind::helpers::NonConstLvalueRefArgChecker<decltype(add)>(); "
                        "qbind::Converter c; "
                        "try { "
                            "(add "
                            "(c.to_cpp<qbind::ArgType<decltype(add) , 0>(karr0) ,"
                            " c.to_cpp<qbind::ArgType<decltype(add) , 1>(karr1))"
                            "); "
                        "} catch (const std::exception& e) { "
                            "thread_local std::string qadd_errmsg; "
                            "qadd_errmsg = e.what(); "
                            "return krr(qadd_errmsg.c_str()); "
                        "} " 
                        "return knk(0); "
                    "} "
                "} "
            "}" == res);

    // Two arguments returning function
    res = STR(QBIND_FN_EXPORT(add, qadd, 2, 1));
    REQUIRE("{ "
                "extern \"C\" "
                "{ "
                    "K qadd (K karr0 , K karr1) "
                    "{ "
                        "qbind::helpers::NonConstLvalueRefArgChecker<decltype(add)>(); "
                        "qbind::Converter c; "
                        "try { "
                            "return c.to_q<qbind::ResultType<decltype(add)>> (add "
                            "(c.to_cpp<qbind::ArgType<decltype(add) , 0>(karr0) ,"
                            " c.to_cpp<qbind::ArgType<decltype(add) , 1>(karr1))"
                            "); "
                        "} catch (const std::exception& e) { "
                            "thread_local std::string qadd_errmsg; "
                            "qadd_errmsg = e.what(); "
                            "return krr(qadd_errmsg.c_str()); "
                        "} " 
                        "return knk(0); "
                    "} "
                "} "
            "}" == res);  

    // N arguments void function
    res = STR(QBIND_FN_EXPORT(bigfunc, qbigfunc, 8, 0));
    REQUIRE("{ "
                "extern \"C\" "
                "{ "
                    "K qbigfunc (K karr0 , K karr1 , K karr2 , K karr3 , K karr4 , K karr5 , K karr6 , K karr7) "
                    "{ "
                        "qbind::helpers::NonConstLvalueRefArgChecker<decltype(bigfunc)>(); "
                        "qbind::Converter c; "
                        "try { "
                            "(bigfunc "
                                "(c.to_cpp<qbind::ArgType<decltype(bigfunc) , 0>(karr0) ,"
                                " c.to_cpp<qbind::ArgType<decltype(bigfunc) , 1>(karr1) ,"
                                " c.to_cpp<qbind::ArgType<decltype(bigfunc) , 2>(karr2) ,"
                                " c.to_cpp<qbind::ArgType<decltype(bigfunc) , 3>(karr3) ,"
                                " c.to_cpp<qbind::ArgType<decltype(bigfunc) , 4>(karr4) ,"
                                " c.to_cpp<qbind::ArgType<decltype(bigfunc) , 5>(karr5) ,"
                                " c.to_cpp<qbind::ArgType<decltype(bigfunc) , 6>(karr6) ,"
                                " c.to_cpp<qbind::ArgType<decltype(bigfunc) , 7>(karr7))"
                            "); "
                        "} catch (const std::exception& e) { "
                            "thread_local std::string qbigfunc_errmsg; "
                            "qbigfunc_errmsg = e.what(); "
                            "return krr(qbigfunc_errmsg.c_str()); "
                        "} " 
                        "return knk(0); "
                    "} "
                "} "
            "}" == res);

    // N arguments returning function
    res = STR(QBIND_FN_EXPORT(bigfunc, qbigfunc, 8, 1));
    REQUIRE("{ "
                "extern \"C\" "
                "{ "
                    "K qbigfunc (K karr0 , K karr1 , K karr2 , K karr3 , K karr4 , K karr5 , K karr6 , K karr7) "
                    "{ "
                        "qbind::helpers::NonConstLvalueRefArgChecker<decltype(bigfunc)>(); "
                        "qbind::Converter c; "
                        "try { "
                            "return c.to_q<qbind::ResultType<decltype(bigfunc)>> (bigfunc "
                                "(c.to_cpp<qbind::ArgType<decltype(bigfunc) , 0>(karr0) ,"
                                " c.to_cpp<qbind::ArgType<decltype(bigfunc) , 1>(karr1) ,"
                                " c.to_cpp<qbind::ArgType<decltype(bigfunc) , 2>(karr2) ,"
                                " c.to_cpp<qbind::ArgType<decltype(bigfunc) , 3>(karr3) ,"
                                " c.to_cpp<qbind::ArgType<decltype(bigfunc) , 4>(karr4) ,"
                                " c.to_cpp<qbind::ArgType<decltype(bigfunc) , 5>(karr5) ,"
                                " c.to_cpp<qbind::ArgType<decltype(bigfunc) , 6>(karr6) ,"
                                " c.to_cpp<qbind::ArgType<decltype(bigfunc) , 7>(karr7))"
                            "); "
                        "} catch (const std::exception& e) { "
                            "thread_local std::string qbigfunc_errmsg; "
                            "qbigfunc_errmsg = e.what(); "
                            "return krr(qbigfunc_errmsg.c_str()); "
                        "} " 
                        "return knk(0); "
                    "} "
                "} "
            "}" == res);  
}
