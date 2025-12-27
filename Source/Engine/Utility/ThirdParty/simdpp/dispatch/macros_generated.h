/*  Copyright (C) 2013  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/


// This file is generated automatically. See tools/gen_dispatcher_macros.py

#ifndef LIBSIMDPP_DISPATCH_MACROS_GENERATED_H
#define LIBSIMDPP_DISPATCH_MACROS_GENERATED_H

#include <simdpp/dispatch/collect_macros_generated.h>
#include <simdpp/dispatch/make_dispatcher.h>

/** Note: the following macros are deprecated and provided only for backwards
    compatibility. See SIMDPP_MAKE_DISPATCHER for a more versatile replacement.

    Builds a dispatcher for a specific non-member function. Different macros
    are provided for functions with or without return value and for each
    parameter count.

    Macros in the form @a SIMDPP_MAKE_DISPATCHER_VOID# are to be used for
    functions with no return value. It accepts the following macro parameters:

    * NAME: the unqualified name of the function to dispatch.

    * T1, T2, ..., T#: the types of the parameters of the function that is
      dispatched.

    Macros in the form @a SIMDPP_MAKE_DISPATCHER_RET# are to be used for
    functions that return a value. It accepts the following macro parameters:

    * NAME: the unqualified name of the function to dispatch.

    * R: the type of the return value

    * T1, T2, ..., T#: the types of the parameters of the function that is
      dispatcher.


    @c SIMDPP_ARCH_NAMESPACE::NAME must refer to the function to be disptached
    relative to the namespace in which the @a MAKE_DISPATCHER* macro is used in.
    That is, the macro must be used in a namespace one level up than the
    dispatched function, and that namespace must be @c SIMDPP_ARCH_NAMESPACE

    @a R, T1, T2, ..., T# must be exactly the same as those of the function
    to be dispatched. The dispatched function may be overloaded.

    The macro defines a function with the same signature as the dispatched
    function in the namespace the macro is used. The body of that function
    implements the dispatch mechanism.

    The dispatch functions check the enabled instruction set and select the
    best function on first call. The initialization does not introduce race
    conditions when done concurrently.
*/


#define SIMDPP_MAKE_DISPATCHER_VOID0(NAME)                                    \
    SIMDPP_MAKE_DISPATCHER((void)(NAME)())

#define SIMDPP_MAKE_DISPATCHER_VOID1(NAME,T1)                                 \
    SIMDPP_MAKE_DISPATCHER((void)(NAME)((T1) a1))

#define SIMDPP_MAKE_DISPATCHER_VOID2(NAME,T1,T2)                              \
    SIMDPP_MAKE_DISPATCHER((void)(NAME)((T1) a1,(T2) a2))

#define SIMDPP_MAKE_DISPATCHER_VOID3(NAME,T1,T2,T3)                           \
    SIMDPP_MAKE_DISPATCHER((void)(NAME)((T1) a1,(T2) a2,(T3) a3))

#define SIMDPP_MAKE_DISPATCHER_VOID4(NAME,T1,T2,T3,T4)                        \
    SIMDPP_MAKE_DISPATCHER((void)(NAME)((T1) a1,(T2) a2,(T3) a3,(T4) a4))

#define SIMDPP_MAKE_DISPATCHER_VOID5(NAME,T1,T2,T3,T4,T5)                     \
    SIMDPP_MAKE_DISPATCHER((void)(NAME)((T1) a1,(T2) a2,(T3) a3,(T4) a4,(T5) a5))

#define SIMDPP_MAKE_DISPATCHER_VOID6(NAME,T1,T2,T3,T4,T5,T6)                  \
    SIMDPP_MAKE_DISPATCHER((void)(NAME)((T1) a1,(T2) a2,(T3) a3,(T4) a4,(T5) a5,(T6) a6))

#define SIMDPP_MAKE_DISPATCHER_VOID7(NAME,T1,T2,T3,T4,T5,T6,T7)               \
    SIMDPP_MAKE_DISPATCHER((void)(NAME)((T1) a1,(T2) a2,(T3) a3,(T4) a4,(T5) a5,(T6) a6,(T7) a7))

#define SIMDPP_MAKE_DISPATCHER_VOID8(NAME,T1,T2,T3,T4,T5,T6,T7,T8)            \
    SIMDPP_MAKE_DISPATCHER((void)(NAME)((T1) a1,(T2) a2,(T3) a3,(T4) a4,(T5) a5,(T6) a6,(T7) a7,(T8) a8))

#define SIMDPP_MAKE_DISPATCHER_VOID9(NAME,T1,T2,T3,T4,T5,T6,T7,T8,T9)         \
    SIMDPP_MAKE_DISPATCHER((void)(NAME)((T1) a1,(T2) a2,(T3) a3,(T4) a4,(T5) a5,(T6) a6,(T7) a7,(T8) a8,(T9) a9))

#define SIMDPP_MAKE_DISPATCHER_RET0(NAME,R)                                   \
    SIMDPP_MAKE_DISPATCHER((R)(NAME)())

#define SIMDPP_MAKE_DISPATCHER_RET1(NAME,R,T1)                                \
    SIMDPP_MAKE_DISPATCHER((R)(NAME)((T1) a1))

#define SIMDPP_MAKE_DISPATCHER_RET2(NAME,R,T1,T2)                             \
    SIMDPP_MAKE_DISPATCHER((R)(NAME)((T1) a1,(T2) a2))

#define SIMDPP_MAKE_DISPATCHER_RET3(NAME,R,T1,T2,T3)                          \
    SIMDPP_MAKE_DISPATCHER((R)(NAME)((T1) a1,(T2) a2,(T3) a3))

#define SIMDPP_MAKE_DISPATCHER_RET4(NAME,R,T1,T2,T3,T4)                       \
    SIMDPP_MAKE_DISPATCHER((R)(NAME)((T1) a1,(T2) a2,(T3) a3,(T4) a4))

#define SIMDPP_MAKE_DISPATCHER_RET5(NAME,R,T1,T2,T3,T4,T5)                    \
    SIMDPP_MAKE_DISPATCHER((R)(NAME)((T1) a1,(T2) a2,(T3) a3,(T4) a4,(T5) a5))

#define SIMDPP_MAKE_DISPATCHER_RET6(NAME,R,T1,T2,T3,T4,T5,T6)                 \
    SIMDPP_MAKE_DISPATCHER((R)(NAME)((T1) a1,(T2) a2,(T3) a3,(T4) a4,(T5) a5,(T6) a6))

#define SIMDPP_MAKE_DISPATCHER_RET7(NAME,R,T1,T2,T3,T4,T5,T6,T7)              \
    SIMDPP_MAKE_DISPATCHER((R)(NAME)((T1) a1,(T2) a2,(T3) a3,(T4) a4,(T5) a5,(T6) a6,(T7) a7))

#define SIMDPP_MAKE_DISPATCHER_RET8(NAME,R,T1,T2,T3,T4,T5,T6,T7,T8)           \
    SIMDPP_MAKE_DISPATCHER((R)(NAME)((T1) a1,(T2) a2,(T3) a3,(T4) a4,(T5) a5,(T6) a6,(T7) a7,(T8) a8))

#define SIMDPP_MAKE_DISPATCHER_RET9(NAME,R,T1,T2,T3,T4,T5,T6,T7,T8,T9)        \
    SIMDPP_MAKE_DISPATCHER((R)(NAME)((T1) a1,(T2) a2,(T3) a3,(T4) a4,(T5) a5,(T6) a6,(T7) a7,(T8) a8,(T9) a9))


#endif

