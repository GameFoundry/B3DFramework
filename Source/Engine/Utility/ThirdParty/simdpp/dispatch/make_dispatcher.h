/*  Copyright (C) 2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_DISPATCH_MAKE_DISPATCHER_H
#define LIBSIMDPP_DISPATCH_MAKE_DISPATCHER_H

#include <simdpp/dispatch/macros_generated.h>
#include <simdpp/detail/preprocessor/punctuation/remove_parens.hpp>
#include <simdpp/detail/preprocessor/seq/for_each.hpp>
#include <simdpp/detail/preprocessor/variadic/to_seq.hpp>

#if SIMDPP_EMIT_DISPATCHER
#include <simdpp/detail/preprocessor/punctuation/comma_if.hpp>
#include <simdpp/detail/preprocessor/seq/elem.hpp>
#include <simdpp/detail/preprocessor/seq/for_each_i.hpp>
#include <simdpp/detail/preprocessor/tuple/rem.hpp>
#include <simdpp/dispatch/collect_macros_generated.h>

// When debugging this code, it's a good idea to familiarize yourself with
// advanced preprocessor techniques first. Several resources follow:
// http://jhnet.co.uk/articles/cpp_magic
// https://github.com/pfultz2/Cloak/wiki/C-Preprocessor-tricks,-tips,-and-idioms
// https://stackoverflow.com/questions/11632219/c-preprocessor-macro-specialisation-based-on-an-argument
// https://stackoverflow.com/questions/44758329/c-or-macro-magic-to-generate-method-and-forward-arguments
// https://stackoverflow.com/questions/11031062/c-preprocessor-avoid-code-repetition-of-member-variable-list
// https://stackoverflow.com/questions/11729168/how-to-get-function-signature-via-preprocessor-define-written-before-it
// Some ideas have been taken from these pages.

// The libsimdpp library includes a partial copy of boost.preprocessor library
// with BOOST_ prefix replaced with SIMDPP_ and variadics support enabled
// unconditionally. If implementing new features, feel free to add any headers
// that are needed but are missing. In case a header becomes no longer used,
// please remove it from the repository.

// The implementation has been affected by the following MSVC preprocessor
// bugs / non-standard behavior:
//
// - MSVC treats expanded variadic arguments as single token in other function
// argument lists. SIMDPP_DETAIL_MSVC_DEFER_MACRO_ARGS has been used as a
// workaround in these cases.
//
// - MSVC does not accept empty token as valid argument in function macros. In
// cases when this matter, SIMDPP_PP_EMPTY() was passed after the real
// arguments.
//
// - MSVC is eager to expand function macros before its arguments are fully
// expanded. This has been worked around with placing one or more
// SIMDPP_PP_EMPTY between the function macro and its argument list to defer
// the expansion.
//
// - MSVC sometimes stops the expansion of macros even when the expansion should
// continue. This is worked around by wrapping the code in SIMDPP_PP_EXPAND

#define SIMDPP_DETAIL_MSVC_DEFER_MACRO_ARGS(macro, args) macro args

#if ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MSVC()
    #define SIMDPP_DETAIL_IGNORE_PARENS(x) SIMDPP_PP_EAT x
#else
    #define SIMDPP_DETAIL_IGNORE_PARENS_DEFER(x) SIMDPP_PP_EAT x
    #define SIMDPP_DETAIL_IGNORE_PARENS(x) SIMDPP_DETAIL_IGNORE_PARENS_DEFER(x)
#endif

#define SIMDPP_DETAIL_EXTRACT_PARENS_IGNORE_REST(x)                             \
    SIMDPP_DETAIL_EXTRACT_PARENS_IGNORE_REST_STRIP_REST_DEFER(SIMDPP_DETAIL_EXTRACT_PARENS_IGNORE_REST_ADD_COMMA x,)

#if ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MSVC()
    #define SIMDPP_DETAIL_EXTRACT_PARENS_IGNORE_REST_STRIP_REST_DEFER(...)      \
        SIMDPP_DETAIL_EXTRACT_PARENS_IGNORE_REST_STRIP_REST(__VA_ARGS__)

    #define SIMDPP_DETAIL_EXTRACT_PARENS_IGNORE_REST_STRIP_REST(x, ...) SIMDPP_PP_REM x
#else
    #define SIMDPP_DETAIL_EXTRACT_PARENS_IGNORE_REST_STRIP_REST_DEFER(...)      \
        SIMDPP_DETAIL_MSVC_DEFER_MACRO_ARGS(SIMDPP_DETAIL_EXTRACT_PARENS_IGNORE_REST_STRIP_REST, (__VA_ARGS__))

    #define SIMDPP_DETAIL_EXTRACT_PARENS_IGNORE_REST_STRIP_REST(x, ...)         \
        SIMDPP_DETAIL_MSVC_DEFER_MACRO_ARGS(SIMDPP_PP_REM, x)
#endif

#define SIMDPP_DETAIL_EXTRACT_PARENS_IGNORE_REST_ADD_COMMA(...) (__VA_ARGS__),

// We can't just use SIMDPP_DETAIL_EXTRACT_PARENS_IGNORE_REST or
// SIMDPP_DETAIL_EXTRACT_ARG_IMPL directly, because the argument list may be
// empty, so SIMDPP_DETAIL_EXTRACT_* may be invoked with the argument being
// empty token. The argument otherwise always starts with a parenthesis, so
// let's check that.
#define SIMDPP_DETAIL_EXTRACT_TYPE(T)                                           \
    SIMDPP_PP_IIF(SIMDPP_PP_IS_BEGIN_PARENS(T),                                 \
                  SIMDPP_DETAIL_EXTRACT_PARENS_IGNORE_REST,                     \
                  SIMDPP_PP_EAT                                                 \
                 )(T)

#define SIMDPP_DETAIL_EXTRACT_ARG_IMPL(T) SIMDPP_PP_REM T
#define SIMDPP_DETAIL_EXTRACT_ARG(T)                                            \
    SIMDPP_PP_IIF(SIMDPP_PP_IS_BEGIN_PARENS(T),                                 \
                  SIMDPP_DETAIL_EXTRACT_ARG_IMPL,                               \
                  SIMDPP_PP_EAT                                                 \
                 )(T)

#define SIMDPP_DETAIL_EXTRACT_FORWARD_IMPL(T) SIMDPP_PP_EAT T
#define SIMDPP_DETAIL_EXTRACT_FORWARD(T)                                        \
    SIMDPP_PP_IIF(SIMDPP_PP_IS_BEGIN_PARENS(T),                                 \
                  SIMDPP_DETAIL_EXTRACT_FORWARD_IMPL,                           \
                  SIMDPP_PP_EAT                                                 \
                 )(T)

#define SIMDPP_DETAIL_TYPES_EACH(r, data, i, x)                                 \
    SIMDPP_PP_COMMA_IF(i) SIMDPP_DETAIL_EXTRACT_TYPE(x)

#define SIMDPP_DETAIL_ARGS_EACH(r, data, i, x)                                  \
    SIMDPP_PP_COMMA_IF(i) SIMDPP_DETAIL_EXTRACT_ARG(x)

#define SIMDPP_DETAIL_FORWARD_EACH(r, data, i, x)                               \
    SIMDPP_PP_COMMA_IF(i) SIMDPP_DETAIL_EXTRACT_FORWARD(x)

// The following 3 macros expand a given function argument list with the
// following format '(A) a, (B) b, (C) c' into different results.

// Will expand to 'A, B, C'
#define SIMDPP_DETAIL_TYPES(args)                                               \
    SIMDPP_PP_SEQ_FOR_EACH_I(SIMDPP_DETAIL_TYPES_EACH, data, SIMDPP_PP_VARIADIC_TO_SEQ args)

// Will expand to 'A a, B b, C c'
#define SIMDPP_DETAIL_ARGS(args)                                                \
    SIMDPP_PP_SEQ_FOR_EACH_I(SIMDPP_DETAIL_ARGS_EACH, data, SIMDPP_PP_VARIADIC_TO_SEQ args)

// Will expand to 'a, b, c'
#define SIMDPP_DETAIL_FORWARD(args)                                             \
    SIMDPP_PP_SEQ_FOR_EACH_I(SIMDPP_DETAIL_FORWARD_EACH, data, SIMDPP_PP_VARIADIC_TO_SEQ args)

// Will expand to 1 if argument contains SIMDPP_PP_PROBE macro anywhere, 0 otherwise
#if ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MSVC()
    #define SIMDPP_PP_PROBE_TO_BOOL(...)                                        \
        SIMDPP_PP_PROBE_TO_BOOL_I(__VA_ARGS__, 0)

#else
    // Note that we can't use SIMDPP_DETAIL_MSVC_DEFER_MACRO_ARGS because this
    // would interfere with the expansion of that macro itself

    #define SIMDPP_PP_PROBE_TO_BOOL(...)                                        \
        SIMDPP_PP_PROBE_TO_BOOL_MSVC_DEFER(SIMDPP_PP_PROBE_TO_BOOL_I, (__VA_ARGS__, 0))
    #define SIMDPP_PP_PROBE_TO_BOOL_MSVC_DEFER(macro, args) macro args
#endif

#define SIMDPP_PP_PROBE_TO_BOOL_I(n1, n2, ...) n2
#define SIMDPP_PP_PROBE() ~, 1, 0

// Given a single argument R, potentially consisting of multiple tokens,
// expands to SIMDPP_PP_PROBE() if R is empty, otherwise expands to nothing.
#if ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MSVC()
    #define SIMDPP_DETAIL_PROBE_IF_VOID_EMPTY(R)                                \
        SIMDPP_PP_IIF(                                                          \
            SIMDPP_PP_PROBE_TO_BOOL(SIMDPP_DETAIL_IS_EMPTY_PROBE R ()),         \
            SIMDPP_PP_PROBE,                                                    \
            SIMDPP_PP_EMPTY                                                     \
        )()
#else
    #define SIMDPP_DETAIL_PROBE_IF_VOID_EMPTY(R)                                \
        SIMDPP_PP_IIF SIMDPP_PP_EMPTY()(                                        \
            SIMDPP_PP_PROBE_TO_BOOL(SIMDPP_DETAIL_IS_EMPTY_PROBE R ()),         \
            SIMDPP_PP_PROBE,                                                    \
            SIMDPP_PP_EMPTY                                                     \
        )()
#endif

#define SIMDPP_DETAIL_IS_EMPTY_PROBE(...) SIMDPP_PP_PROBE()

// Given a single argument R, potentially consisting of multiple tokens or
// parenthesized token groups, expands to nothing if R starts with a parenthesis.
// Otherwise expands to SIMDPP_DETAIL_PROBE_IF_VOID_EMPTY with R
// passed as an argument.
#if ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MSVC()
    #define SIMDPP_DETAIL_PROBE_IF_VOID_1PAREN(R)                               \
        SIMDPP_PP_IIF(SIMDPP_PP_IS_BEGIN_PARENS(R),                             \
                      SIMDPP_PP_EAT,                                            \
                      SIMDPP_DETAIL_PROBE_IF_VOID_EMPTY                         \
                     )(R)
#else
    // R might be empty token, thus workarounds are needed (see top of the file)

    #define SIMDPP_DETAIL_PROBE_IF_VOID_1PAREN(R)                               \
        SIMDPP_PP_IIF(SIMDPP_PP_IS_BEGIN_PARENS(R),                             \
                      SIMDPP_PP_EAT,                                            \
                      SIMDPP_DETAIL_PROBE_IF_VOID_EMPTY                         \
                     )(R SIMDPP_PP_EMPTY())
#endif

// Given a single argument R, potentially consisting of multiple tokens or
// parenthesized token groups, expands to nothing if R starts with a 'void'
// token. Otherwise expands to SIMDPP_DETAIL_PROBE_IF_VOID_1PAREN
// passing R with the first 'void' token removed as an argument.

#if ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MSVC()
    #define SIMDPP_DETAIL_PROBE_IF_VOID_1PARAM(R)                               \
        SIMDPP_PP_IIF(                                                          \
            SIMDPP_PP_PROBE_TO_BOOL(SIMDPP_DETAIL_STARTS_WITH_VOID_PROBE(R)),   \
            SIMDPP_DETAIL_PROBE_IF_VOID_1PAREN,                                 \
            SIMDPP_PP_EAT                                                       \
        )(SIMDPP_PP_CAT(SIMDPP_DETAIL_STARTS_WITH_VOID_STRIP_, R))
#else
    // SIMDPP_PP_IIF is expanded one expansion too early and R might be empty
    // token, thus workarounds are needed (see top of the file)

    #define SIMDPP_DETAIL_PROBE_IF_VOID_1PARAM(R)                               \
        SIMDPP_PP_IIF SIMDPP_PP_EMPTY()(                                        \
            SIMDPP_PP_PROBE_TO_BOOL(SIMDPP_DETAIL_STARTS_WITH_VOID_PROBE(R SIMDPP_PP_EMPTY())), \
            SIMDPP_DETAIL_PROBE_IF_VOID_1PAREN,                                 \
            SIMDPP_PP_EAT                                                       \
        )(SIMDPP_PP_CAT(SIMDPP_DETAIL_STARTS_WITH_VOID_STRIP_, R) SIMDPP_PP_EMPTY SIMDPP_PP_EMPTY()())
#endif

#define SIMDPP_DETAIL_STARTS_WITH_VOID_CHECK_void SIMDPP_PP_PROBE()
#define SIMDPP_DETAIL_STARTS_WITH_VOID_STRIP_void
#define SIMDPP_DETAIL_STARTS_WITH_VOID_PROBE(R)                                 \
    SIMDPP_DETAIL_STARTS_WITH_VOID_PROBE_I(SIMDPP_DETAIL_STARTS_WITH_VOID_CHECK_ ## R)
#define SIMDPP_DETAIL_STARTS_WITH_VOID_PROBE_I(R) R

// Given a parenthesized list of arguments R, expands to nothing if R contains
// more than one argument. Otherwise expands SIMDPP_DETAIL_PROBE_IF_VOID_1PARAM
// passing R with removed outer parentheses
#define SIMDPP_DETAIL_PROBE_IF_VOID(R)                               \
    SIMDPP_PP_IF(SIMDPP_PP_DEC(SIMDPP_PP_VARIADIC_SIZE(SIMDPP_PP_REM R)),       \
                 SIMDPP_PP_EAT,                                                 \
                 SIMDPP_DETAIL_PROBE_IF_VOID_1PARAM                             \
                )(SIMDPP_PP_REMOVE_PARENS(R))

// Given a parenthesized list of arguments R, expands to nothing if R is
// parenthesized void token, otherwise expands to return token
#if ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MSVC()

    #define SIMDPP_DETAIL_RETURN_IF_NOT_VOID(R)                                 \
        SIMDPP_PP_IIF(SIMDPP_PP_PROBE_TO_BOOL(SIMDPP_DETAIL_PROBE_IF_VOID(R)),  \
                      SIMDPP_PP_EMPTY,                                          \
                      SIMDPP_DETAIL_RETURN_TOKEN                                \
                     )()
#else
    // The following problems have been worked around on MSVC (see top of the
    // file):
    //  - SIMDPP_PP_IIF is expanded two expansions too early
    //  - the result of SIMDPP_DETAIL_PROBE_IF_VOID is not passed back to
    // SIMDPP_PP_PROBE_TO_BOOL properly
    //  - the final SIMDPP_PP_IIF is not expanded for some reason.

    #define SIMDPP_DETAIL_RETURN_IF_NOT_VOID(R)                                 \
        SIMDPP_PP_EXPAND(                                                       \
            SIMDPP_PP_IIF SIMDPP_PP_EMPTY SIMDPP_PP_EMPTY()()(                  \
                SIMDPP_DETAIL_MSVC_DEFER_MACRO_ARGS(                            \
                    SIMDPP_PP_PROBE_TO_BOOL, (SIMDPP_DETAIL_PROBE_IF_VOID(R))   \
                ),                                                              \
                SIMDPP_PP_EMPTY,                                                \
                SIMDPP_DETAIL_RETURN_TOKEN                                      \
            )()                                                                 \
        )
#endif

#define SIMDPP_DETAIL_RETURN_TOKEN() return

#define SIMDPP_DETAIL_MAKE_DISPATCHER_IMPL(TEMPLATE_PREFIX, TEMPLATE_ARGS, R, NAME, ARGS) \
                                                                                \
SIMDPP_DISPATCH_DECLARE_FUNCTIONS(                                              \
    (SIMDPP_PP_REMOVE_PARENS(TEMPLATE_PREFIX)                                   \
     SIMDPP_PP_REMOVE_PARENS(R) NAME (SIMDPP_DETAIL_TYPES(ARGS))))              \
                                                                                \
SIMDPP_PP_REMOVE_PARENS(TEMPLATE_PREFIX)                                        \
SIMDPP_PP_REMOVE_PARENS(R) NAME(SIMDPP_DETAIL_ARGS(ARGS))                       \
{                                                                               \
    using FunPtr = SIMDPP_PP_REMOVE_PARENS(R)(*)(SIMDPP_DETAIL_TYPES(ARGS));    \
    static FunPtr selected = nullptr;                                           \
    if (selected == nullptr) {                                                  \
        ::simdpp::detail::FnVersion versions[SIMDPP_DISPATCH_MAX_ARCHS] = {};   \
        SIMDPP_DISPATCH_COLLECT_FUNCTIONS(versions,                             \
            (NAME SIMDPP_PP_REMOVE_PARENS(TEMPLATE_ARGS)), FunPtr)              \
        ::simdpp::detail::FnVersion version =                                   \
            ::simdpp::detail::select_version_any(versions,                      \
                SIMDPP_DISPATCH_MAX_ARCHS, SIMDPP_USER_ARCH_INFO);              \
        selected = reinterpret_cast<FunPtr>(version.fun_ptr);                   \
    }                                                                           \
    SIMDPP_DETAIL_RETURN_IF_NOT_VOID(R) selected(SIMDPP_DETAIL_FORWARD(ARGS));  \
}

#define SIMDPP_DETAIL_IGNORE_PARENS2(x)                                         \
    SIMDPP_DETAIL_IGNORE_PARENS(SIMDPP_DETAIL_IGNORE_PARENS(x))

#define SIMDPP_DETAIL_IGNORE_PARENS3(x)                                         \
    SIMDPP_DETAIL_IGNORE_PARENS(SIMDPP_DETAIL_IGNORE_PARENS(                    \
        SIMDPP_DETAIL_IGNORE_PARENS(x)                                          \
    ))

#define SIMDPP_DETAIL_IGNORE_PARENS4(x)                                         \
    SIMDPP_DETAIL_IGNORE_PARENS(SIMDPP_DETAIL_IGNORE_PARENS(                    \
        SIMDPP_DETAIL_IGNORE_PARENS(SIMDPP_DETAIL_IGNORE_PARENS(x))             \
    ))

// SIMDPP_PP_SEQ_ELEM does not work with sequence elements containing commas,
// so we use a workaround
#define SIMDPP_DETAIL_MAKE_DISPATCHER1(DESC) SIMDPP_ERROR_INCORRECT_NUMBER_OF_ARGUMENTS
#define SIMDPP_DETAIL_MAKE_DISPATCHER2(DESC) SIMDPP_ERROR_INCORRECT_NUMBER_OF_ARGUMENTS
#define SIMDPP_DETAIL_MAKE_DISPATCHER3(DESC)                                    \
    SIMDPP_DETAIL_MAKE_DISPATCHER_IMPL(                                         \
        (),                                                                     \
        (),                                                                     \
        (SIMDPP_DETAIL_EXTRACT_PARENS_IGNORE_REST(DESC)),                       \
        SIMDPP_DETAIL_EXTRACT_PARENS_IGNORE_REST(SIMDPP_DETAIL_IGNORE_PARENS(DESC)), \
        (SIMDPP_DETAIL_EXTRACT_PARENS_IGNORE_REST(SIMDPP_DETAIL_IGNORE_PARENS2(DESC))))

#define SIMDPP_DETAIL_MAKE_DISPATCHER4(DESC) SIMDPP_ERROR_INCORRECT_NUMBER_OF_ARGUMENTS
#define SIMDPP_DETAIL_MAKE_DISPATCHER5(DESC)                                    \
    SIMDPP_DETAIL_MAKE_DISPATCHER_IMPL(                                         \
        (SIMDPP_DETAIL_EXTRACT_PARENS_IGNORE_REST(DESC)),                       \
        (SIMDPP_DETAIL_EXTRACT_PARENS_IGNORE_REST(SIMDPP_DETAIL_IGNORE_PARENS(DESC))), \
        (SIMDPP_DETAIL_EXTRACT_PARENS_IGNORE_REST(SIMDPP_DETAIL_IGNORE_PARENS2(DESC))), \
        SIMDPP_DETAIL_EXTRACT_PARENS_IGNORE_REST(SIMDPP_DETAIL_IGNORE_PARENS3(DESC)), \
        (SIMDPP_DETAIL_EXTRACT_PARENS_IGNORE_REST(SIMDPP_DETAIL_IGNORE_PARENS4(DESC))))


#define SIMDPP_DETAIL_MAKE_DISPATCHER6(DESC) SIMDPP_ERROR_INCORRECT_NUMBER_OF_ARGUMENTS
#define SIMDPP_DETAIL_MAKE_DISPATCHER7(DESC) SIMDPP_ERROR_INCORRECT_NUMBER_OF_ARGUMENTS

/** Builds a dispatcher for a specific non-member function. The same macro is
    used for functions with or without return value, with different parameter
    counts and for template functions.

    The function accepts a sequence of 3 or 5 parenthesized token groups. Each
    group conveys the following information:
     - (optional) the full template prefix, e.g. (template<class T>)
     - (optional) the template argument list enclosed in brackets, e.g. (<T,U>)
     - the return type, e.g. (void), or (float)
     - the function name, e.g. (my_function)
     - comma separated list of function arguments. Each argument is specified as
        a parenthesized type name and argument name which follows immediately.
        For example ((float) x, (int) y, (std::pair<int, int>) z).

    The following examples show several ways to invoke the
    SIMDPP_MAKE_DISPATCHER macro:

    SIMDPP_MAKE_DISPATCHER((void)(my_function1)())
    SIMDPP_MAKE_DISPATCHER((void)(my_function1)((int) x, (float)z))
    SIMDPP_MAKE_DISPATCHER((int)(my_function1)((int) x, (float)z))
    SIMDPP_MAKE_DISPATCHER((template<class A, class B>)(<A,B>)(A)(my_function1)())
    SIMDPP_MAKE_DISPATCHER((template<class A, class B>)(<A,B>)(A)(my_function1)((B) x, (B)z))

    SIMDPP_ARCH_NAMESPACE::NAME (where NAME refers to the name of the function
    supplied to the SIMDPP_MAKE_DISPATCHER macro) must refer to the function to
    be disptached relative to the namespace in which the SIMDPP_MAKE_DISPATCHER
    macro is used in. That is, the macro must be used in a namespace one level
    up than the dispatched function, and that namespace must be
    SIMDPP_ARCH_NAMESPACE.

    The return and parameter types must be exactly the same as those of the
    function to be dispatched. The dispatched function may be overloaded.

    When dispatching function templates, each used template must be explicitly
    dispatched in all architecture-specific compilation units. For example,
    when using simdpp_multiarch (see cmake/SimdppMultiarch.cmake), these
    instantiations must be defined in SRC_FILE source file.

    The macro defines a function with the same signature as the dispatched
    function in the namespace the macro is used. The body of that function
    implements the dispatch mechanism.

    The dispatch functions check the enabled instruction set and select the
    best function on first call. The initialization does not introduce race
    conditions when done concurrently.

    The generated dispatching code links to all versions of the dispatched
    function statically, so techniques to prevent linkers from stripping
    unreferenced object files are not needed.
*/
#define SIMDPP_MAKE_DISPATCHER(DESC)                                            \
    SIMDPP_PP_CAT(SIMDPP_DETAIL_MAKE_DISPATCHER, SIMDPP_PP_SEQ_SIZE(DESC))(DESC)
#else // #if SIMDPP_EMIT_DISPATCHER
#define SIMDPP_MAKE_DISPATCHER(DESC)
#endif

#define SIMDPP_DETAIL_SIGNATURE_EACH(r, data, x) SIMDPP_PP_REMOVE_PARENS(x) ;
#define SIMDPP_DETAIL_SIGNATURES(signatures)                                    \
    SIMDPP_PP_SEQ_FOR_EACH(SIMDPP_DETAIL_SIGNATURE_EACH, data,                  \
                           SIMDPP_PP_VARIADIC_TO_SEQ signatures)

#if SIMDPP_EMIT_DISPATCHER
/** Defines a one or more template instantiations for a dispatcher. Accepts one
    or more parenthesized token groups separated by commas defining one or more
    full template instantiations. For example:
    SIMDPP_INSTANTIATE_DISPATCHER((template void foo<int>(int x))

    or

    SIMDPP_INSTANTIATE_DISPATCHER(
        (template void foo<int>(int x)),
        (template void foo<char>(char x))
    )

    The macro forces instantiation of the dispatch function defined by the
    SIMDPP_MAKE_DISPATCHER macro and also of the functions referenced by the
    dispatcher function. The latter is necessary because the dispatcher is
    compiled into a only single object file out of the set of multiversioned
    object files. The referenced functions will be instantiated in all of them.
*/
// Implementation note: we can't accept the signatures as a sequence, e.g.
// (a)(b)(c), because SIMDPP_PP_SEQ_FOR_EACH does not support sequence elements
// containing commas at the top level
#define SIMDPP_INSTANTIATE_DISPATCHER(...)                                      \
SIMDPP_DETAIL_SIGNATURES((__VA_ARGS__))                                         \
namespace SIMDPP_ARCH_NAMESPACE {                                               \
    SIMDPP_DETAIL_SIGNATURES((__VA_ARGS__))                                     \
}
#else // SIMDPP_EMIT_DISPATCHER
#define SIMDPP_INSTANTIATE_DISPATCHER(...)                                      \
namespace SIMDPP_ARCH_NAMESPACE {                                               \
    SIMDPP_DETAIL_SIGNATURES((__VA_ARGS__))                                     \
}
#endif

#endif // LIBSIMDPP_DISPATCH_MAKE_DISPATCHER_H
