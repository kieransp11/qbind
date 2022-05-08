#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>

#include <boost/function_types/function_arity.hpp> // boost::function_types::function_arity;
#include <boost/function_types/result_type.hpp>    // boost::function_types::result_type;
#include <boost/function_types/parameter_types.hpp> // boost::function_types::parameter_types;
#include <boost/mpl/at.hpp>                         // boost::mpl::at_c;

#include <type_traits>
#include <utility>

#include <kx/kx.h>                                  // used in QBIND_FN_EXPORT

namespace qbind {

template <typename FuncType>
using Arity = boost::function_types::function_arity<FuncType>;

template <typename FuncType>
using ResultType = typename boost::function_types::result_type<FuncType>::type;

template <typename FuncType, size_t ArgIndex>
using ArgType = typename boost::mpl::at_c<boost::function_types::parameter_types<FuncType>, ArgIndex>::type;

namespace helpers {

namespace internal {

template<typename T>
constexpr void IsNonConstLvalueRef()
{
    static_assert(!std::is_lvalue_reference_v<T> || std::is_const_v<std::remove_reference_t<T>>,
        "QBind does not support non-const lvalue references arguments. Take a non-owning view by value instead.");
}

template <typename Func, typename IndexSeq>
struct NonConstLvalueRefArgCheckerHelper;

template <typename Func, size_t... Inds>
struct NonConstLvalueRefArgCheckerHelper<Func, std::integer_sequence<size_t, Inds...> >
{
    static void check()
    {
        (IsNonConstLvalueRef<ArgType<Func, Inds>>(), ...);
    }
};

} // internal

template <typename Func>
void NonConstLvalueRefArgChecker()
{
    internal::NonConstLvalueRefArgCheckerHelper<
        Func, 
        std::make_index_sequence<Arity<Func>::value> 
    >::check();
}

} // helpers

} // qbind
/**
 * @brief Empty macro
 */
#define QBIND_EMPTY

/**
 * @brief Macro for comma replacement
 */
#define QBIND_COMMA() ,

/**
 * @brief The identity macro. Returns its own argument.
 *
 * This macro is useful to delay functional macros, i.e. QBIND_ID(COMMA)().
 * Alternatively it can be used to immediately evaluate the an argument.
 */
#define QBIND_ID(id) id

#define QBIND_JOINi(a, b) a##b
/**
 * @brief Join a and b after both are evaluated.
 */
#define QBIND_JOIN(a, b) QBIND_JOINi(a, b)

// Making a function signature

/**
 * @brief Q function parameters are always of the form "K karr[n]"
 * 
 * @param n: Position in parameter list (0 indexed)
 */
#define QBIND_PARAMETER(n) QBIND_JOIN(K karr, n)

/**
 * @brief Fold function for BOOST_PP_REPEAT for a parameter pack
 */
#define QBIND_PARAMETER_FOLD(z, n, str) QBIND_PARAMETER(n) QBIND_ID(QBIND_COMMA)()

/**
 * @brief Make an parameter pack. 
 * 
 * This will have the correct form for varying arities:
 *  - Arity 0:     ()
 *  - Arity 1:     (K karr0)
 *  - Otherwise:   (K karr0, ... K karr[n])
 * 
 * @param n: Number of parameters required
 */
#define QBIND_PARAMETERS(n) BOOST_PP_IF(n,                                                                  \
    BOOST_PP_IF( BOOST_PP_SUB(n, 1),                                                                        \
        (BOOST_PP_REPEAT(BOOST_PP_SUB(n, 1), QBIND_PARAMETER_FOLD,) QBIND_PARAMETER(BOOST_PP_SUB(n, 1))),   \
        (QBIND_PARAMETER(0)))                                                                               \
    ,())

/**
 * @brief Make a function signature
 *
 * @param name: Name of the function to export
 * @param nargs: Number or arguments
 */
#define QBIND_FN_SIGNATURE(name, nargs) K name QBIND_PARAMETERS(nargs)

// Making a function call

/**
 * @brief Bound function argument always convert the nth parameter of the q function.
 * 
 * @param n: Position in argument list (0 indexed)
 * @param fn: Which function this argument will be passed to
 */
#define QBIND_ARGUMENT(fn, n) c.to_cpp<qbind::ArgType<decltype(fn) QBIND_ID(QBIND_COMMA)() n>>(QBIND_JOIN(karr, n))

/**
 * @brief Fold function for BOOST_PP_REPEAT for an argument list
 * 
 */
#define QBIND_ARGUMENT_FOLD(z, n, fn) QBIND_ARGUMENT(fn, n) QBIND_ID(QBIND_COMMA)()

/**
 * @brief Make an argument pack.
 * 
 * This will have the correct form for varying arities.
 */
#define QBIND_ARGUMENTS(fn, n) BOOST_PP_IF(n,                                                                   \
    BOOST_PP_IF( BOOST_PP_SUB(n, 1),                                                                            \
        (BOOST_PP_REPEAT(BOOST_PP_SUB(n, 1), QBIND_ARGUMENT_FOLD, fn) QBIND_ARGUMENT(fn, BOOST_PP_SUB(n, 1))),  \
        (QBIND_ARGUMENT(fn, 0)))                                                                                \
    ,())  

/**
 * @brief Convert returned value to Q. 
 * 
 * Note the call site of the function needs to be enclosed in brackets to its passed to the
 * converter as an argument.
 * 
 * @param returns: 0 to discard the result, otherwise convert to a K object
 */
#define QBIND_RETURN_CONVERSION(returns, fn) BOOST_PP_IF(returns, return c.to_q<qbind::ResultType<decltype(fn)>>, QBIND_EMPTY)

#define QBIND_CALL_SIGNATURE(returns, name, nargs) \
    QBIND_RETURN_CONVERSION(returns, name)         \
    (name QBIND_ARGUMENTS(name, nargs));

// TODO: Make sure we get a good error message that we can't support non-const lvalue references.
// Suggest a non-owning view by value instead.
// TODO: krr and orr are not working properly
/**
 * @brief Export a function to Q.
 * 
 * Error handling documented here: 
 * https://code.kx.com/q/interfaces/capiref/#krr-signal-c-error
 * 
 * In the case the function returns void this will return the Q generic null.
 * This is represented by a zero length mixed array.
 * 
 * @param fn: Function to export
 * @param name: Name to export function as (must be unique as its extern C)
 * @param nargs: Number of arguments of function
 * @param returns: 0 if doesn't return
 */
#define QBIND_FN_EXPORT(fn, name, nargs, returns)                        \
    extern "C"                                                           \
    {                                                                    \
        QBIND_FN_SIGNATURE(name, nargs)                                  \
        {                                                                \
            qbind::helpers::NonConstLvalueRefArgChecker<decltype(fn)>(); \
            qbind::Converter c;                                          \
            try                                                          \
            {                                                            \
                QBIND_CALL_SIGNATURE(returns, fn, nargs)                 \
            }                                                            \
            catch (const std::exception& e)                              \
            {                                                            \
                thread_local std::string errmsg;                         \
                errmsg = e.what();                                       \
                std::cerr << errmsg << std::endl;                        \
                return krr(errmsg.data());                               \
            }                                                            \
            return knk(0);                                               \
        }                                                                \
    }                                                                    