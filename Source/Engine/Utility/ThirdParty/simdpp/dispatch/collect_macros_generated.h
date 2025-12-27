/*  Copyright (C) 2015  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/


// This file is generated automatically. See tools/gen_dispatcher_collect_macros.py

#ifndef LIBSIMDPP_DISPATCH_COLLECT_MACROS_GENERATED_H
#define LIBSIMDPP_DISPATCH_COLLECT_MACROS_GENERATED_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

// We rely on setup_arch.h being included before this file to undefine
// all SIMDPP_ARCH_* macros
#include <simdpp/setup_arch.h>

#if SIMDPP_EMIT_DISPATCHER
#include <simdpp/detail/preprocessor/punctuation/remove_parens.hpp>

#define SIMDPP_DISPATCH_MAX_ARCHS 15


#ifdef SIMDPP_DISPATCH_ARCH1
    #define SIMDPP_ARCH_PP_LIST SIMDPP_DISPATCH_ARCH1
    #include <simdpp/detail/preprocess_single_arch.h>

    // Use the results of preprocess_single_arch.h to define
    // SIMDPP_DISPATCH_1_NAMESPACE

    #if SIMDPP_ARCH_PP_NS_USE_NULL
    #define SIMDPP_DISPATCH_1_NS_ID_NULL SIMDPP_INSN_ID_NULL
    #else
    #define SIMDPP_DISPATCH_1_NS_ID_NULL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE2
    #define SIMDPP_DISPATCH_1_NS_ID_SSE2 SIMDPP_INSN_ID_SSE2
    #else
    #define SIMDPP_DISPATCH_1_NS_ID_SSE2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE3
    #define SIMDPP_DISPATCH_1_NS_ID_SSE3 SIMDPP_INSN_ID_SSE3
    #else
    #define SIMDPP_DISPATCH_1_NS_ID_SSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSSE3
    #define SIMDPP_DISPATCH_1_NS_ID_SSSE3 SIMDPP_INSN_ID_SSSE3
    #else
    #define SIMDPP_DISPATCH_1_NS_ID_SSSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE4_1
    #define SIMDPP_DISPATCH_1_NS_ID_SSE4_1 SIMDPP_INSN_ID_SSE4_1
    #else
    #define SIMDPP_DISPATCH_1_NS_ID_SSE4_1
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_POPCNT_INSN
    #define SIMDPP_DISPATCH_1_NS_ID_POPCNT_INSN SIMDPP_INSN_ID_POPCNT_INSN
    #else
    #define SIMDPP_DISPATCH_1_NS_ID_POPCNT_INSN
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX
    #define SIMDPP_DISPATCH_1_NS_ID_AVX SIMDPP_INSN_ID_AVX
    #else
    #define SIMDPP_DISPATCH_1_NS_ID_AVX
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX2
    #define SIMDPP_DISPATCH_1_NS_ID_AVX2 SIMDPP_INSN_ID_AVX2
    #else
    #define SIMDPP_DISPATCH_1_NS_ID_AVX2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA3
    #define SIMDPP_DISPATCH_1_NS_ID_FMA3 SIMDPP_INSN_ID_FMA3
    #else
    #define SIMDPP_DISPATCH_1_NS_ID_FMA3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA4
    #define SIMDPP_DISPATCH_1_NS_ID_FMA4 SIMDPP_INSN_ID_FMA4
    #else
    #define SIMDPP_DISPATCH_1_NS_ID_FMA4
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_XOP
    #define SIMDPP_DISPATCH_1_NS_ID_XOP SIMDPP_INSN_ID_XOP
    #else
    #define SIMDPP_DISPATCH_1_NS_ID_XOP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512F
    #define SIMDPP_DISPATCH_1_NS_ID_AVX512F SIMDPP_INSN_ID_AVX512F
    #else
    #define SIMDPP_DISPATCH_1_NS_ID_AVX512F
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512BW
    #define SIMDPP_DISPATCH_1_NS_ID_AVX512BW SIMDPP_INSN_ID_AVX512BW
    #else
    #define SIMDPP_DISPATCH_1_NS_ID_AVX512BW
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512DQ
    #define SIMDPP_DISPATCH_1_NS_ID_AVX512DQ SIMDPP_INSN_ID_AVX512DQ
    #else
    #define SIMDPP_DISPATCH_1_NS_ID_AVX512DQ
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512VL
    #define SIMDPP_DISPATCH_1_NS_ID_AVX512VL SIMDPP_INSN_ID_AVX512VL
    #else
    #define SIMDPP_DISPATCH_1_NS_ID_AVX512VL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON
    #define SIMDPP_DISPATCH_1_NS_ID_NEON SIMDPP_INSN_ID_NEON
    #else
    #define SIMDPP_DISPATCH_1_NS_ID_NEON
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON_FLT_SP
    #define SIMDPP_DISPATCH_1_NS_ID_NEON_FLT_SP SIMDPP_INSN_ID_NEON_FLT_SP
    #else
    #define SIMDPP_DISPATCH_1_NS_ID_NEON_FLT_SP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_MSA
    #define SIMDPP_DISPATCH_1_NS_ID_MSA SIMDPP_INSN_ID_MSA
    #else
    #define SIMDPP_DISPATCH_1_NS_ID_MSA
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_ALTIVEC
    #define SIMDPP_DISPATCH_1_NS_ID_ALTIVEC SIMDPP_INSN_ID_ALTIVEC
    #else
    #define SIMDPP_DISPATCH_1_NS_ID_ALTIVEC
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_206
    #define SIMDPP_DISPATCH_1_NS_ID_VSX_206 SIMDPP_INSN_ID_VSX_206
    #else
    #define SIMDPP_DISPATCH_1_NS_ID_VSX_206
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_207
    #define SIMDPP_DISPATCH_1_NS_ID_VSX_207 SIMDPP_INSN_ID_VSX_207
    #else
    #define SIMDPP_DISPATCH_1_NS_ID_VSX_207
    #endif

    #define SIMDPP_DISPATCH_1_NAMESPACE SIMDPP_PP_PASTE22(arch,               \
        SIMDPP_DISPATCH_1_NS_ID_NULL,                                         \
        SIMDPP_DISPATCH_1_NS_ID_SSE2,                                         \
        SIMDPP_DISPATCH_1_NS_ID_SSE3,                                         \
        SIMDPP_DISPATCH_1_NS_ID_SSSE3,                                        \
        SIMDPP_DISPATCH_1_NS_ID_SSE4_1,                                       \
        SIMDPP_DISPATCH_1_NS_ID_POPCNT_INSN,                                  \
        SIMDPP_DISPATCH_1_NS_ID_AVX,                                          \
        SIMDPP_DISPATCH_1_NS_ID_AVX2,                                         \
        SIMDPP_DISPATCH_1_NS_ID_AVX512F,                                      \
        SIMDPP_DISPATCH_1_NS_ID_AVX512BW,                                     \
        SIMDPP_DISPATCH_1_NS_ID_AVX512DQ,                                     \
        SIMDPP_DISPATCH_1_NS_ID_AVX512VL,                                     \
        SIMDPP_DISPATCH_1_NS_ID_FMA3,                                         \
        SIMDPP_DISPATCH_1_NS_ID_FMA4,                                         \
        SIMDPP_DISPATCH_1_NS_ID_XOP,                                          \
        SIMDPP_DISPATCH_1_NS_ID_NEON,                                         \
        SIMDPP_DISPATCH_1_NS_ID_NEON_FLT_SP,                                  \
        SIMDPP_DISPATCH_1_NS_ID_MSA,                                          \
        SIMDPP_DISPATCH_1_NS_ID_ALTIVEC,                                      \
        SIMDPP_DISPATCH_1_NS_ID_VSX_206,                                      \
        SIMDPP_DISPATCH_1_NS_ID_VSX_207)

    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE SIMDPP_DISPATCH_1_NAMESPACE
    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH 1
    #include <simdpp/dispatch/preprocess_single_compile_arch.h>
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE

    #define SIMDPP_DISPATCH_1_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                \
        {   /* the following will fail if the overload is not available */    \
            FUN_TYPE fun_ptr = &SIMDPP_DISPATCH_1_NAMESPACE::SIMDPP_PP_REMOVE_PARENS(NAME);\
            ARRAY[1-1] = ::simdpp::SIMDPP_DISPATCH_1_NAMESPACE::detail::create_fn_version(fun_ptr);\
        }

    #define SIMDPP_DISPATCH_1_FN_DECLARE(SIGNATURE)                           \
        namespace SIMDPP_DISPATCH_1_NAMESPACE {                               \
            SIMDPP_PP_REMOVE_PARENS(SIGNATURE);                               \
        }
    #undef SIMDPP_ARCH_PP_LIST
#else
    #define SIMDPP_DISPATCH_1_FN_REGISTER(ARRAY,NAME,FUN_TYPE)
    #define SIMDPP_DISPATCH_1_FN_DECLARE(SIGNATURE)
#endif

#ifdef SIMDPP_DISPATCH_ARCH2
    #define SIMDPP_ARCH_PP_LIST SIMDPP_DISPATCH_ARCH2
    #include <simdpp/detail/preprocess_single_arch.h>

    // Use the results of preprocess_single_arch.h to define
    // SIMDPP_DISPATCH_2_NAMESPACE

    #if SIMDPP_ARCH_PP_NS_USE_NULL
    #define SIMDPP_DISPATCH_2_NS_ID_NULL SIMDPP_INSN_ID_NULL
    #else
    #define SIMDPP_DISPATCH_2_NS_ID_NULL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE2
    #define SIMDPP_DISPATCH_2_NS_ID_SSE2 SIMDPP_INSN_ID_SSE2
    #else
    #define SIMDPP_DISPATCH_2_NS_ID_SSE2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE3
    #define SIMDPP_DISPATCH_2_NS_ID_SSE3 SIMDPP_INSN_ID_SSE3
    #else
    #define SIMDPP_DISPATCH_2_NS_ID_SSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSSE3
    #define SIMDPP_DISPATCH_2_NS_ID_SSSE3 SIMDPP_INSN_ID_SSSE3
    #else
    #define SIMDPP_DISPATCH_2_NS_ID_SSSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE4_1
    #define SIMDPP_DISPATCH_2_NS_ID_SSE4_1 SIMDPP_INSN_ID_SSE4_1
    #else
    #define SIMDPP_DISPATCH_2_NS_ID_SSE4_1
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_POPCNT_INSN
    #define SIMDPP_DISPATCH_2_NS_ID_POPCNT_INSN SIMDPP_INSN_ID_POPCNT_INSN
    #else
    #define SIMDPP_DISPATCH_2_NS_ID_POPCNT_INSN
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX
    #define SIMDPP_DISPATCH_2_NS_ID_AVX SIMDPP_INSN_ID_AVX
    #else
    #define SIMDPP_DISPATCH_2_NS_ID_AVX
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX2
    #define SIMDPP_DISPATCH_2_NS_ID_AVX2 SIMDPP_INSN_ID_AVX2
    #else
    #define SIMDPP_DISPATCH_2_NS_ID_AVX2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA3
    #define SIMDPP_DISPATCH_2_NS_ID_FMA3 SIMDPP_INSN_ID_FMA3
    #else
    #define SIMDPP_DISPATCH_2_NS_ID_FMA3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA4
    #define SIMDPP_DISPATCH_2_NS_ID_FMA4 SIMDPP_INSN_ID_FMA4
    #else
    #define SIMDPP_DISPATCH_2_NS_ID_FMA4
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_XOP
    #define SIMDPP_DISPATCH_2_NS_ID_XOP SIMDPP_INSN_ID_XOP
    #else
    #define SIMDPP_DISPATCH_2_NS_ID_XOP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512F
    #define SIMDPP_DISPATCH_2_NS_ID_AVX512F SIMDPP_INSN_ID_AVX512F
    #else
    #define SIMDPP_DISPATCH_2_NS_ID_AVX512F
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512BW
    #define SIMDPP_DISPATCH_2_NS_ID_AVX512BW SIMDPP_INSN_ID_AVX512BW
    #else
    #define SIMDPP_DISPATCH_2_NS_ID_AVX512BW
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512DQ
    #define SIMDPP_DISPATCH_2_NS_ID_AVX512DQ SIMDPP_INSN_ID_AVX512DQ
    #else
    #define SIMDPP_DISPATCH_2_NS_ID_AVX512DQ
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512VL
    #define SIMDPP_DISPATCH_2_NS_ID_AVX512VL SIMDPP_INSN_ID_AVX512VL
    #else
    #define SIMDPP_DISPATCH_2_NS_ID_AVX512VL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON
    #define SIMDPP_DISPATCH_2_NS_ID_NEON SIMDPP_INSN_ID_NEON
    #else
    #define SIMDPP_DISPATCH_2_NS_ID_NEON
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON_FLT_SP
    #define SIMDPP_DISPATCH_2_NS_ID_NEON_FLT_SP SIMDPP_INSN_ID_NEON_FLT_SP
    #else
    #define SIMDPP_DISPATCH_2_NS_ID_NEON_FLT_SP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_MSA
    #define SIMDPP_DISPATCH_2_NS_ID_MSA SIMDPP_INSN_ID_MSA
    #else
    #define SIMDPP_DISPATCH_2_NS_ID_MSA
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_ALTIVEC
    #define SIMDPP_DISPATCH_2_NS_ID_ALTIVEC SIMDPP_INSN_ID_ALTIVEC
    #else
    #define SIMDPP_DISPATCH_2_NS_ID_ALTIVEC
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_206
    #define SIMDPP_DISPATCH_2_NS_ID_VSX_206 SIMDPP_INSN_ID_VSX_206
    #else
    #define SIMDPP_DISPATCH_2_NS_ID_VSX_206
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_207
    #define SIMDPP_DISPATCH_2_NS_ID_VSX_207 SIMDPP_INSN_ID_VSX_207
    #else
    #define SIMDPP_DISPATCH_2_NS_ID_VSX_207
    #endif

    #define SIMDPP_DISPATCH_2_NAMESPACE SIMDPP_PP_PASTE22(arch,               \
        SIMDPP_DISPATCH_2_NS_ID_NULL,                                         \
        SIMDPP_DISPATCH_2_NS_ID_SSE2,                                         \
        SIMDPP_DISPATCH_2_NS_ID_SSE3,                                         \
        SIMDPP_DISPATCH_2_NS_ID_SSSE3,                                        \
        SIMDPP_DISPATCH_2_NS_ID_SSE4_1,                                       \
        SIMDPP_DISPATCH_2_NS_ID_POPCNT_INSN,                                  \
        SIMDPP_DISPATCH_2_NS_ID_AVX,                                          \
        SIMDPP_DISPATCH_2_NS_ID_AVX2,                                         \
        SIMDPP_DISPATCH_2_NS_ID_AVX512F,                                      \
        SIMDPP_DISPATCH_2_NS_ID_AVX512BW,                                     \
        SIMDPP_DISPATCH_2_NS_ID_AVX512DQ,                                     \
        SIMDPP_DISPATCH_2_NS_ID_AVX512VL,                                     \
        SIMDPP_DISPATCH_2_NS_ID_FMA3,                                         \
        SIMDPP_DISPATCH_2_NS_ID_FMA4,                                         \
        SIMDPP_DISPATCH_2_NS_ID_XOP,                                          \
        SIMDPP_DISPATCH_2_NS_ID_NEON,                                         \
        SIMDPP_DISPATCH_2_NS_ID_NEON_FLT_SP,                                  \
        SIMDPP_DISPATCH_2_NS_ID_MSA,                                          \
        SIMDPP_DISPATCH_2_NS_ID_ALTIVEC,                                      \
        SIMDPP_DISPATCH_2_NS_ID_VSX_206,                                      \
        SIMDPP_DISPATCH_2_NS_ID_VSX_207)

    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE SIMDPP_DISPATCH_2_NAMESPACE
    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH 1
    #include <simdpp/dispatch/preprocess_single_compile_arch.h>
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE

    #define SIMDPP_DISPATCH_2_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                \
        {   /* the following will fail if the overload is not available */    \
            FUN_TYPE fun_ptr = &SIMDPP_DISPATCH_2_NAMESPACE::SIMDPP_PP_REMOVE_PARENS(NAME);\
            ARRAY[2-1] = ::simdpp::SIMDPP_DISPATCH_2_NAMESPACE::detail::create_fn_version(fun_ptr);\
        }

    #define SIMDPP_DISPATCH_2_FN_DECLARE(SIGNATURE)                           \
        namespace SIMDPP_DISPATCH_2_NAMESPACE {                               \
            SIMDPP_PP_REMOVE_PARENS(SIGNATURE);                               \
        }
    #undef SIMDPP_ARCH_PP_LIST
#else
    #define SIMDPP_DISPATCH_2_FN_REGISTER(ARRAY,NAME,FUN_TYPE)
    #define SIMDPP_DISPATCH_2_FN_DECLARE(SIGNATURE)
#endif

#ifdef SIMDPP_DISPATCH_ARCH3
    #define SIMDPP_ARCH_PP_LIST SIMDPP_DISPATCH_ARCH3
    #include <simdpp/detail/preprocess_single_arch.h>

    // Use the results of preprocess_single_arch.h to define
    // SIMDPP_DISPATCH_3_NAMESPACE

    #if SIMDPP_ARCH_PP_NS_USE_NULL
    #define SIMDPP_DISPATCH_3_NS_ID_NULL SIMDPP_INSN_ID_NULL
    #else
    #define SIMDPP_DISPATCH_3_NS_ID_NULL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE2
    #define SIMDPP_DISPATCH_3_NS_ID_SSE2 SIMDPP_INSN_ID_SSE2
    #else
    #define SIMDPP_DISPATCH_3_NS_ID_SSE2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE3
    #define SIMDPP_DISPATCH_3_NS_ID_SSE3 SIMDPP_INSN_ID_SSE3
    #else
    #define SIMDPP_DISPATCH_3_NS_ID_SSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSSE3
    #define SIMDPP_DISPATCH_3_NS_ID_SSSE3 SIMDPP_INSN_ID_SSSE3
    #else
    #define SIMDPP_DISPATCH_3_NS_ID_SSSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE4_1
    #define SIMDPP_DISPATCH_3_NS_ID_SSE4_1 SIMDPP_INSN_ID_SSE4_1
    #else
    #define SIMDPP_DISPATCH_3_NS_ID_SSE4_1
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_POPCNT_INSN
    #define SIMDPP_DISPATCH_3_NS_ID_POPCNT_INSN SIMDPP_INSN_ID_POPCNT_INSN
    #else
    #define SIMDPP_DISPATCH_3_NS_ID_POPCNT_INSN
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX
    #define SIMDPP_DISPATCH_3_NS_ID_AVX SIMDPP_INSN_ID_AVX
    #else
    #define SIMDPP_DISPATCH_3_NS_ID_AVX
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX2
    #define SIMDPP_DISPATCH_3_NS_ID_AVX2 SIMDPP_INSN_ID_AVX2
    #else
    #define SIMDPP_DISPATCH_3_NS_ID_AVX2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA3
    #define SIMDPP_DISPATCH_3_NS_ID_FMA3 SIMDPP_INSN_ID_FMA3
    #else
    #define SIMDPP_DISPATCH_3_NS_ID_FMA3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA4
    #define SIMDPP_DISPATCH_3_NS_ID_FMA4 SIMDPP_INSN_ID_FMA4
    #else
    #define SIMDPP_DISPATCH_3_NS_ID_FMA4
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_XOP
    #define SIMDPP_DISPATCH_3_NS_ID_XOP SIMDPP_INSN_ID_XOP
    #else
    #define SIMDPP_DISPATCH_3_NS_ID_XOP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512F
    #define SIMDPP_DISPATCH_3_NS_ID_AVX512F SIMDPP_INSN_ID_AVX512F
    #else
    #define SIMDPP_DISPATCH_3_NS_ID_AVX512F
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512BW
    #define SIMDPP_DISPATCH_3_NS_ID_AVX512BW SIMDPP_INSN_ID_AVX512BW
    #else
    #define SIMDPP_DISPATCH_3_NS_ID_AVX512BW
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512DQ
    #define SIMDPP_DISPATCH_3_NS_ID_AVX512DQ SIMDPP_INSN_ID_AVX512DQ
    #else
    #define SIMDPP_DISPATCH_3_NS_ID_AVX512DQ
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512VL
    #define SIMDPP_DISPATCH_3_NS_ID_AVX512VL SIMDPP_INSN_ID_AVX512VL
    #else
    #define SIMDPP_DISPATCH_3_NS_ID_AVX512VL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON
    #define SIMDPP_DISPATCH_3_NS_ID_NEON SIMDPP_INSN_ID_NEON
    #else
    #define SIMDPP_DISPATCH_3_NS_ID_NEON
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON_FLT_SP
    #define SIMDPP_DISPATCH_3_NS_ID_NEON_FLT_SP SIMDPP_INSN_ID_NEON_FLT_SP
    #else
    #define SIMDPP_DISPATCH_3_NS_ID_NEON_FLT_SP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_MSA
    #define SIMDPP_DISPATCH_3_NS_ID_MSA SIMDPP_INSN_ID_MSA
    #else
    #define SIMDPP_DISPATCH_3_NS_ID_MSA
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_ALTIVEC
    #define SIMDPP_DISPATCH_3_NS_ID_ALTIVEC SIMDPP_INSN_ID_ALTIVEC
    #else
    #define SIMDPP_DISPATCH_3_NS_ID_ALTIVEC
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_206
    #define SIMDPP_DISPATCH_3_NS_ID_VSX_206 SIMDPP_INSN_ID_VSX_206
    #else
    #define SIMDPP_DISPATCH_3_NS_ID_VSX_206
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_207
    #define SIMDPP_DISPATCH_3_NS_ID_VSX_207 SIMDPP_INSN_ID_VSX_207
    #else
    #define SIMDPP_DISPATCH_3_NS_ID_VSX_207
    #endif

    #define SIMDPP_DISPATCH_3_NAMESPACE SIMDPP_PP_PASTE22(arch,               \
        SIMDPP_DISPATCH_3_NS_ID_NULL,                                         \
        SIMDPP_DISPATCH_3_NS_ID_SSE2,                                         \
        SIMDPP_DISPATCH_3_NS_ID_SSE3,                                         \
        SIMDPP_DISPATCH_3_NS_ID_SSSE3,                                        \
        SIMDPP_DISPATCH_3_NS_ID_SSE4_1,                                       \
        SIMDPP_DISPATCH_3_NS_ID_POPCNT_INSN,                                  \
        SIMDPP_DISPATCH_3_NS_ID_AVX,                                          \
        SIMDPP_DISPATCH_3_NS_ID_AVX2,                                         \
        SIMDPP_DISPATCH_3_NS_ID_AVX512F,                                      \
        SIMDPP_DISPATCH_3_NS_ID_AVX512BW,                                     \
        SIMDPP_DISPATCH_3_NS_ID_AVX512DQ,                                     \
        SIMDPP_DISPATCH_3_NS_ID_AVX512VL,                                     \
        SIMDPP_DISPATCH_3_NS_ID_FMA3,                                         \
        SIMDPP_DISPATCH_3_NS_ID_FMA4,                                         \
        SIMDPP_DISPATCH_3_NS_ID_XOP,                                          \
        SIMDPP_DISPATCH_3_NS_ID_NEON,                                         \
        SIMDPP_DISPATCH_3_NS_ID_NEON_FLT_SP,                                  \
        SIMDPP_DISPATCH_3_NS_ID_MSA,                                          \
        SIMDPP_DISPATCH_3_NS_ID_ALTIVEC,                                      \
        SIMDPP_DISPATCH_3_NS_ID_VSX_206,                                      \
        SIMDPP_DISPATCH_3_NS_ID_VSX_207)

    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE SIMDPP_DISPATCH_3_NAMESPACE
    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH 1
    #include <simdpp/dispatch/preprocess_single_compile_arch.h>
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE

    #define SIMDPP_DISPATCH_3_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                \
        {   /* the following will fail if the overload is not available */    \
            FUN_TYPE fun_ptr = &SIMDPP_DISPATCH_3_NAMESPACE::SIMDPP_PP_REMOVE_PARENS(NAME);\
            ARRAY[3-1] = ::simdpp::SIMDPP_DISPATCH_3_NAMESPACE::detail::create_fn_version(fun_ptr);\
        }

    #define SIMDPP_DISPATCH_3_FN_DECLARE(SIGNATURE)                           \
        namespace SIMDPP_DISPATCH_3_NAMESPACE {                               \
            SIMDPP_PP_REMOVE_PARENS(SIGNATURE);                               \
        }
    #undef SIMDPP_ARCH_PP_LIST
#else
    #define SIMDPP_DISPATCH_3_FN_REGISTER(ARRAY,NAME,FUN_TYPE)
    #define SIMDPP_DISPATCH_3_FN_DECLARE(SIGNATURE)
#endif

#ifdef SIMDPP_DISPATCH_ARCH4
    #define SIMDPP_ARCH_PP_LIST SIMDPP_DISPATCH_ARCH4
    #include <simdpp/detail/preprocess_single_arch.h>

    // Use the results of preprocess_single_arch.h to define
    // SIMDPP_DISPATCH_4_NAMESPACE

    #if SIMDPP_ARCH_PP_NS_USE_NULL
    #define SIMDPP_DISPATCH_4_NS_ID_NULL SIMDPP_INSN_ID_NULL
    #else
    #define SIMDPP_DISPATCH_4_NS_ID_NULL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE2
    #define SIMDPP_DISPATCH_4_NS_ID_SSE2 SIMDPP_INSN_ID_SSE2
    #else
    #define SIMDPP_DISPATCH_4_NS_ID_SSE2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE3
    #define SIMDPP_DISPATCH_4_NS_ID_SSE3 SIMDPP_INSN_ID_SSE3
    #else
    #define SIMDPP_DISPATCH_4_NS_ID_SSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSSE3
    #define SIMDPP_DISPATCH_4_NS_ID_SSSE3 SIMDPP_INSN_ID_SSSE3
    #else
    #define SIMDPP_DISPATCH_4_NS_ID_SSSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE4_1
    #define SIMDPP_DISPATCH_4_NS_ID_SSE4_1 SIMDPP_INSN_ID_SSE4_1
    #else
    #define SIMDPP_DISPATCH_4_NS_ID_SSE4_1
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_POPCNT_INSN
    #define SIMDPP_DISPATCH_4_NS_ID_POPCNT_INSN SIMDPP_INSN_ID_POPCNT_INSN
    #else
    #define SIMDPP_DISPATCH_4_NS_ID_POPCNT_INSN
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX
    #define SIMDPP_DISPATCH_4_NS_ID_AVX SIMDPP_INSN_ID_AVX
    #else
    #define SIMDPP_DISPATCH_4_NS_ID_AVX
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX2
    #define SIMDPP_DISPATCH_4_NS_ID_AVX2 SIMDPP_INSN_ID_AVX2
    #else
    #define SIMDPP_DISPATCH_4_NS_ID_AVX2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA3
    #define SIMDPP_DISPATCH_4_NS_ID_FMA3 SIMDPP_INSN_ID_FMA3
    #else
    #define SIMDPP_DISPATCH_4_NS_ID_FMA3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA4
    #define SIMDPP_DISPATCH_4_NS_ID_FMA4 SIMDPP_INSN_ID_FMA4
    #else
    #define SIMDPP_DISPATCH_4_NS_ID_FMA4
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_XOP
    #define SIMDPP_DISPATCH_4_NS_ID_XOP SIMDPP_INSN_ID_XOP
    #else
    #define SIMDPP_DISPATCH_4_NS_ID_XOP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512F
    #define SIMDPP_DISPATCH_4_NS_ID_AVX512F SIMDPP_INSN_ID_AVX512F
    #else
    #define SIMDPP_DISPATCH_4_NS_ID_AVX512F
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512BW
    #define SIMDPP_DISPATCH_4_NS_ID_AVX512BW SIMDPP_INSN_ID_AVX512BW
    #else
    #define SIMDPP_DISPATCH_4_NS_ID_AVX512BW
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512DQ
    #define SIMDPP_DISPATCH_4_NS_ID_AVX512DQ SIMDPP_INSN_ID_AVX512DQ
    #else
    #define SIMDPP_DISPATCH_4_NS_ID_AVX512DQ
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512VL
    #define SIMDPP_DISPATCH_4_NS_ID_AVX512VL SIMDPP_INSN_ID_AVX512VL
    #else
    #define SIMDPP_DISPATCH_4_NS_ID_AVX512VL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON
    #define SIMDPP_DISPATCH_4_NS_ID_NEON SIMDPP_INSN_ID_NEON
    #else
    #define SIMDPP_DISPATCH_4_NS_ID_NEON
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON_FLT_SP
    #define SIMDPP_DISPATCH_4_NS_ID_NEON_FLT_SP SIMDPP_INSN_ID_NEON_FLT_SP
    #else
    #define SIMDPP_DISPATCH_4_NS_ID_NEON_FLT_SP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_MSA
    #define SIMDPP_DISPATCH_4_NS_ID_MSA SIMDPP_INSN_ID_MSA
    #else
    #define SIMDPP_DISPATCH_4_NS_ID_MSA
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_ALTIVEC
    #define SIMDPP_DISPATCH_4_NS_ID_ALTIVEC SIMDPP_INSN_ID_ALTIVEC
    #else
    #define SIMDPP_DISPATCH_4_NS_ID_ALTIVEC
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_206
    #define SIMDPP_DISPATCH_4_NS_ID_VSX_206 SIMDPP_INSN_ID_VSX_206
    #else
    #define SIMDPP_DISPATCH_4_NS_ID_VSX_206
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_207
    #define SIMDPP_DISPATCH_4_NS_ID_VSX_207 SIMDPP_INSN_ID_VSX_207
    #else
    #define SIMDPP_DISPATCH_4_NS_ID_VSX_207
    #endif

    #define SIMDPP_DISPATCH_4_NAMESPACE SIMDPP_PP_PASTE22(arch,               \
        SIMDPP_DISPATCH_4_NS_ID_NULL,                                         \
        SIMDPP_DISPATCH_4_NS_ID_SSE2,                                         \
        SIMDPP_DISPATCH_4_NS_ID_SSE3,                                         \
        SIMDPP_DISPATCH_4_NS_ID_SSSE3,                                        \
        SIMDPP_DISPATCH_4_NS_ID_SSE4_1,                                       \
        SIMDPP_DISPATCH_4_NS_ID_POPCNT_INSN,                                  \
        SIMDPP_DISPATCH_4_NS_ID_AVX,                                          \
        SIMDPP_DISPATCH_4_NS_ID_AVX2,                                         \
        SIMDPP_DISPATCH_4_NS_ID_AVX512F,                                      \
        SIMDPP_DISPATCH_4_NS_ID_AVX512BW,                                     \
        SIMDPP_DISPATCH_4_NS_ID_AVX512DQ,                                     \
        SIMDPP_DISPATCH_4_NS_ID_AVX512VL,                                     \
        SIMDPP_DISPATCH_4_NS_ID_FMA3,                                         \
        SIMDPP_DISPATCH_4_NS_ID_FMA4,                                         \
        SIMDPP_DISPATCH_4_NS_ID_XOP,                                          \
        SIMDPP_DISPATCH_4_NS_ID_NEON,                                         \
        SIMDPP_DISPATCH_4_NS_ID_NEON_FLT_SP,                                  \
        SIMDPP_DISPATCH_4_NS_ID_MSA,                                          \
        SIMDPP_DISPATCH_4_NS_ID_ALTIVEC,                                      \
        SIMDPP_DISPATCH_4_NS_ID_VSX_206,                                      \
        SIMDPP_DISPATCH_4_NS_ID_VSX_207)

    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE SIMDPP_DISPATCH_4_NAMESPACE
    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH 1
    #include <simdpp/dispatch/preprocess_single_compile_arch.h>
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE

    #define SIMDPP_DISPATCH_4_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                \
        {   /* the following will fail if the overload is not available */    \
            FUN_TYPE fun_ptr = &SIMDPP_DISPATCH_4_NAMESPACE::SIMDPP_PP_REMOVE_PARENS(NAME);\
            ARRAY[4-1] = ::simdpp::SIMDPP_DISPATCH_4_NAMESPACE::detail::create_fn_version(fun_ptr);\
        }

    #define SIMDPP_DISPATCH_4_FN_DECLARE(SIGNATURE)                           \
        namespace SIMDPP_DISPATCH_4_NAMESPACE {                               \
            SIMDPP_PP_REMOVE_PARENS(SIGNATURE);                               \
        }
    #undef SIMDPP_ARCH_PP_LIST
#else
    #define SIMDPP_DISPATCH_4_FN_REGISTER(ARRAY,NAME,FUN_TYPE)
    #define SIMDPP_DISPATCH_4_FN_DECLARE(SIGNATURE)
#endif

#ifdef SIMDPP_DISPATCH_ARCH5
    #define SIMDPP_ARCH_PP_LIST SIMDPP_DISPATCH_ARCH5
    #include <simdpp/detail/preprocess_single_arch.h>

    // Use the results of preprocess_single_arch.h to define
    // SIMDPP_DISPATCH_5_NAMESPACE

    #if SIMDPP_ARCH_PP_NS_USE_NULL
    #define SIMDPP_DISPATCH_5_NS_ID_NULL SIMDPP_INSN_ID_NULL
    #else
    #define SIMDPP_DISPATCH_5_NS_ID_NULL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE2
    #define SIMDPP_DISPATCH_5_NS_ID_SSE2 SIMDPP_INSN_ID_SSE2
    #else
    #define SIMDPP_DISPATCH_5_NS_ID_SSE2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE3
    #define SIMDPP_DISPATCH_5_NS_ID_SSE3 SIMDPP_INSN_ID_SSE3
    #else
    #define SIMDPP_DISPATCH_5_NS_ID_SSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSSE3
    #define SIMDPP_DISPATCH_5_NS_ID_SSSE3 SIMDPP_INSN_ID_SSSE3
    #else
    #define SIMDPP_DISPATCH_5_NS_ID_SSSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE4_1
    #define SIMDPP_DISPATCH_5_NS_ID_SSE4_1 SIMDPP_INSN_ID_SSE4_1
    #else
    #define SIMDPP_DISPATCH_5_NS_ID_SSE4_1
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_POPCNT_INSN
    #define SIMDPP_DISPATCH_5_NS_ID_POPCNT_INSN SIMDPP_INSN_ID_POPCNT_INSN
    #else
    #define SIMDPP_DISPATCH_5_NS_ID_POPCNT_INSN
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX
    #define SIMDPP_DISPATCH_5_NS_ID_AVX SIMDPP_INSN_ID_AVX
    #else
    #define SIMDPP_DISPATCH_5_NS_ID_AVX
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX2
    #define SIMDPP_DISPATCH_5_NS_ID_AVX2 SIMDPP_INSN_ID_AVX2
    #else
    #define SIMDPP_DISPATCH_5_NS_ID_AVX2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA3
    #define SIMDPP_DISPATCH_5_NS_ID_FMA3 SIMDPP_INSN_ID_FMA3
    #else
    #define SIMDPP_DISPATCH_5_NS_ID_FMA3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA4
    #define SIMDPP_DISPATCH_5_NS_ID_FMA4 SIMDPP_INSN_ID_FMA4
    #else
    #define SIMDPP_DISPATCH_5_NS_ID_FMA4
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_XOP
    #define SIMDPP_DISPATCH_5_NS_ID_XOP SIMDPP_INSN_ID_XOP
    #else
    #define SIMDPP_DISPATCH_5_NS_ID_XOP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512F
    #define SIMDPP_DISPATCH_5_NS_ID_AVX512F SIMDPP_INSN_ID_AVX512F
    #else
    #define SIMDPP_DISPATCH_5_NS_ID_AVX512F
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512BW
    #define SIMDPP_DISPATCH_5_NS_ID_AVX512BW SIMDPP_INSN_ID_AVX512BW
    #else
    #define SIMDPP_DISPATCH_5_NS_ID_AVX512BW
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512DQ
    #define SIMDPP_DISPATCH_5_NS_ID_AVX512DQ SIMDPP_INSN_ID_AVX512DQ
    #else
    #define SIMDPP_DISPATCH_5_NS_ID_AVX512DQ
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512VL
    #define SIMDPP_DISPATCH_5_NS_ID_AVX512VL SIMDPP_INSN_ID_AVX512VL
    #else
    #define SIMDPP_DISPATCH_5_NS_ID_AVX512VL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON
    #define SIMDPP_DISPATCH_5_NS_ID_NEON SIMDPP_INSN_ID_NEON
    #else
    #define SIMDPP_DISPATCH_5_NS_ID_NEON
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON_FLT_SP
    #define SIMDPP_DISPATCH_5_NS_ID_NEON_FLT_SP SIMDPP_INSN_ID_NEON_FLT_SP
    #else
    #define SIMDPP_DISPATCH_5_NS_ID_NEON_FLT_SP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_MSA
    #define SIMDPP_DISPATCH_5_NS_ID_MSA SIMDPP_INSN_ID_MSA
    #else
    #define SIMDPP_DISPATCH_5_NS_ID_MSA
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_ALTIVEC
    #define SIMDPP_DISPATCH_5_NS_ID_ALTIVEC SIMDPP_INSN_ID_ALTIVEC
    #else
    #define SIMDPP_DISPATCH_5_NS_ID_ALTIVEC
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_206
    #define SIMDPP_DISPATCH_5_NS_ID_VSX_206 SIMDPP_INSN_ID_VSX_206
    #else
    #define SIMDPP_DISPATCH_5_NS_ID_VSX_206
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_207
    #define SIMDPP_DISPATCH_5_NS_ID_VSX_207 SIMDPP_INSN_ID_VSX_207
    #else
    #define SIMDPP_DISPATCH_5_NS_ID_VSX_207
    #endif

    #define SIMDPP_DISPATCH_5_NAMESPACE SIMDPP_PP_PASTE22(arch,               \
        SIMDPP_DISPATCH_5_NS_ID_NULL,                                         \
        SIMDPP_DISPATCH_5_NS_ID_SSE2,                                         \
        SIMDPP_DISPATCH_5_NS_ID_SSE3,                                         \
        SIMDPP_DISPATCH_5_NS_ID_SSSE3,                                        \
        SIMDPP_DISPATCH_5_NS_ID_SSE4_1,                                       \
        SIMDPP_DISPATCH_5_NS_ID_POPCNT_INSN,                                  \
        SIMDPP_DISPATCH_5_NS_ID_AVX,                                          \
        SIMDPP_DISPATCH_5_NS_ID_AVX2,                                         \
        SIMDPP_DISPATCH_5_NS_ID_AVX512F,                                      \
        SIMDPP_DISPATCH_5_NS_ID_AVX512BW,                                     \
        SIMDPP_DISPATCH_5_NS_ID_AVX512DQ,                                     \
        SIMDPP_DISPATCH_5_NS_ID_AVX512VL,                                     \
        SIMDPP_DISPATCH_5_NS_ID_FMA3,                                         \
        SIMDPP_DISPATCH_5_NS_ID_FMA4,                                         \
        SIMDPP_DISPATCH_5_NS_ID_XOP,                                          \
        SIMDPP_DISPATCH_5_NS_ID_NEON,                                         \
        SIMDPP_DISPATCH_5_NS_ID_NEON_FLT_SP,                                  \
        SIMDPP_DISPATCH_5_NS_ID_MSA,                                          \
        SIMDPP_DISPATCH_5_NS_ID_ALTIVEC,                                      \
        SIMDPP_DISPATCH_5_NS_ID_VSX_206,                                      \
        SIMDPP_DISPATCH_5_NS_ID_VSX_207)

    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE SIMDPP_DISPATCH_5_NAMESPACE
    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH 1
    #include <simdpp/dispatch/preprocess_single_compile_arch.h>
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE

    #define SIMDPP_DISPATCH_5_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                \
        {   /* the following will fail if the overload is not available */    \
            FUN_TYPE fun_ptr = &SIMDPP_DISPATCH_5_NAMESPACE::SIMDPP_PP_REMOVE_PARENS(NAME);\
            ARRAY[5-1] = ::simdpp::SIMDPP_DISPATCH_5_NAMESPACE::detail::create_fn_version(fun_ptr);\
        }

    #define SIMDPP_DISPATCH_5_FN_DECLARE(SIGNATURE)                           \
        namespace SIMDPP_DISPATCH_5_NAMESPACE {                               \
            SIMDPP_PP_REMOVE_PARENS(SIGNATURE);                               \
        }
    #undef SIMDPP_ARCH_PP_LIST
#else
    #define SIMDPP_DISPATCH_5_FN_REGISTER(ARRAY,NAME,FUN_TYPE)
    #define SIMDPP_DISPATCH_5_FN_DECLARE(SIGNATURE)
#endif

#ifdef SIMDPP_DISPATCH_ARCH6
    #define SIMDPP_ARCH_PP_LIST SIMDPP_DISPATCH_ARCH6
    #include <simdpp/detail/preprocess_single_arch.h>

    // Use the results of preprocess_single_arch.h to define
    // SIMDPP_DISPATCH_6_NAMESPACE

    #if SIMDPP_ARCH_PP_NS_USE_NULL
    #define SIMDPP_DISPATCH_6_NS_ID_NULL SIMDPP_INSN_ID_NULL
    #else
    #define SIMDPP_DISPATCH_6_NS_ID_NULL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE2
    #define SIMDPP_DISPATCH_6_NS_ID_SSE2 SIMDPP_INSN_ID_SSE2
    #else
    #define SIMDPP_DISPATCH_6_NS_ID_SSE2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE3
    #define SIMDPP_DISPATCH_6_NS_ID_SSE3 SIMDPP_INSN_ID_SSE3
    #else
    #define SIMDPP_DISPATCH_6_NS_ID_SSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSSE3
    #define SIMDPP_DISPATCH_6_NS_ID_SSSE3 SIMDPP_INSN_ID_SSSE3
    #else
    #define SIMDPP_DISPATCH_6_NS_ID_SSSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE4_1
    #define SIMDPP_DISPATCH_6_NS_ID_SSE4_1 SIMDPP_INSN_ID_SSE4_1
    #else
    #define SIMDPP_DISPATCH_6_NS_ID_SSE4_1
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_POPCNT_INSN
    #define SIMDPP_DISPATCH_6_NS_ID_POPCNT_INSN SIMDPP_INSN_ID_POPCNT_INSN
    #else
    #define SIMDPP_DISPATCH_6_NS_ID_POPCNT_INSN
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX
    #define SIMDPP_DISPATCH_6_NS_ID_AVX SIMDPP_INSN_ID_AVX
    #else
    #define SIMDPP_DISPATCH_6_NS_ID_AVX
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX2
    #define SIMDPP_DISPATCH_6_NS_ID_AVX2 SIMDPP_INSN_ID_AVX2
    #else
    #define SIMDPP_DISPATCH_6_NS_ID_AVX2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA3
    #define SIMDPP_DISPATCH_6_NS_ID_FMA3 SIMDPP_INSN_ID_FMA3
    #else
    #define SIMDPP_DISPATCH_6_NS_ID_FMA3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA4
    #define SIMDPP_DISPATCH_6_NS_ID_FMA4 SIMDPP_INSN_ID_FMA4
    #else
    #define SIMDPP_DISPATCH_6_NS_ID_FMA4
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_XOP
    #define SIMDPP_DISPATCH_6_NS_ID_XOP SIMDPP_INSN_ID_XOP
    #else
    #define SIMDPP_DISPATCH_6_NS_ID_XOP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512F
    #define SIMDPP_DISPATCH_6_NS_ID_AVX512F SIMDPP_INSN_ID_AVX512F
    #else
    #define SIMDPP_DISPATCH_6_NS_ID_AVX512F
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512BW
    #define SIMDPP_DISPATCH_6_NS_ID_AVX512BW SIMDPP_INSN_ID_AVX512BW
    #else
    #define SIMDPP_DISPATCH_6_NS_ID_AVX512BW
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512DQ
    #define SIMDPP_DISPATCH_6_NS_ID_AVX512DQ SIMDPP_INSN_ID_AVX512DQ
    #else
    #define SIMDPP_DISPATCH_6_NS_ID_AVX512DQ
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512VL
    #define SIMDPP_DISPATCH_6_NS_ID_AVX512VL SIMDPP_INSN_ID_AVX512VL
    #else
    #define SIMDPP_DISPATCH_6_NS_ID_AVX512VL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON
    #define SIMDPP_DISPATCH_6_NS_ID_NEON SIMDPP_INSN_ID_NEON
    #else
    #define SIMDPP_DISPATCH_6_NS_ID_NEON
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON_FLT_SP
    #define SIMDPP_DISPATCH_6_NS_ID_NEON_FLT_SP SIMDPP_INSN_ID_NEON_FLT_SP
    #else
    #define SIMDPP_DISPATCH_6_NS_ID_NEON_FLT_SP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_MSA
    #define SIMDPP_DISPATCH_6_NS_ID_MSA SIMDPP_INSN_ID_MSA
    #else
    #define SIMDPP_DISPATCH_6_NS_ID_MSA
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_ALTIVEC
    #define SIMDPP_DISPATCH_6_NS_ID_ALTIVEC SIMDPP_INSN_ID_ALTIVEC
    #else
    #define SIMDPP_DISPATCH_6_NS_ID_ALTIVEC
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_206
    #define SIMDPP_DISPATCH_6_NS_ID_VSX_206 SIMDPP_INSN_ID_VSX_206
    #else
    #define SIMDPP_DISPATCH_6_NS_ID_VSX_206
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_207
    #define SIMDPP_DISPATCH_6_NS_ID_VSX_207 SIMDPP_INSN_ID_VSX_207
    #else
    #define SIMDPP_DISPATCH_6_NS_ID_VSX_207
    #endif

    #define SIMDPP_DISPATCH_6_NAMESPACE SIMDPP_PP_PASTE22(arch,               \
        SIMDPP_DISPATCH_6_NS_ID_NULL,                                         \
        SIMDPP_DISPATCH_6_NS_ID_SSE2,                                         \
        SIMDPP_DISPATCH_6_NS_ID_SSE3,                                         \
        SIMDPP_DISPATCH_6_NS_ID_SSSE3,                                        \
        SIMDPP_DISPATCH_6_NS_ID_SSE4_1,                                       \
        SIMDPP_DISPATCH_6_NS_ID_POPCNT_INSN,                                  \
        SIMDPP_DISPATCH_6_NS_ID_AVX,                                          \
        SIMDPP_DISPATCH_6_NS_ID_AVX2,                                         \
        SIMDPP_DISPATCH_6_NS_ID_AVX512F,                                      \
        SIMDPP_DISPATCH_6_NS_ID_AVX512BW,                                     \
        SIMDPP_DISPATCH_6_NS_ID_AVX512DQ,                                     \
        SIMDPP_DISPATCH_6_NS_ID_AVX512VL,                                     \
        SIMDPP_DISPATCH_6_NS_ID_FMA3,                                         \
        SIMDPP_DISPATCH_6_NS_ID_FMA4,                                         \
        SIMDPP_DISPATCH_6_NS_ID_XOP,                                          \
        SIMDPP_DISPATCH_6_NS_ID_NEON,                                         \
        SIMDPP_DISPATCH_6_NS_ID_NEON_FLT_SP,                                  \
        SIMDPP_DISPATCH_6_NS_ID_MSA,                                          \
        SIMDPP_DISPATCH_6_NS_ID_ALTIVEC,                                      \
        SIMDPP_DISPATCH_6_NS_ID_VSX_206,                                      \
        SIMDPP_DISPATCH_6_NS_ID_VSX_207)

    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE SIMDPP_DISPATCH_6_NAMESPACE
    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH 1
    #include <simdpp/dispatch/preprocess_single_compile_arch.h>
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE

    #define SIMDPP_DISPATCH_6_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                \
        {   /* the following will fail if the overload is not available */    \
            FUN_TYPE fun_ptr = &SIMDPP_DISPATCH_6_NAMESPACE::SIMDPP_PP_REMOVE_PARENS(NAME);\
            ARRAY[6-1] = ::simdpp::SIMDPP_DISPATCH_6_NAMESPACE::detail::create_fn_version(fun_ptr);\
        }

    #define SIMDPP_DISPATCH_6_FN_DECLARE(SIGNATURE)                           \
        namespace SIMDPP_DISPATCH_6_NAMESPACE {                               \
            SIMDPP_PP_REMOVE_PARENS(SIGNATURE);                               \
        }
    #undef SIMDPP_ARCH_PP_LIST
#else
    #define SIMDPP_DISPATCH_6_FN_REGISTER(ARRAY,NAME,FUN_TYPE)
    #define SIMDPP_DISPATCH_6_FN_DECLARE(SIGNATURE)
#endif

#ifdef SIMDPP_DISPATCH_ARCH7
    #define SIMDPP_ARCH_PP_LIST SIMDPP_DISPATCH_ARCH7
    #include <simdpp/detail/preprocess_single_arch.h>

    // Use the results of preprocess_single_arch.h to define
    // SIMDPP_DISPATCH_7_NAMESPACE

    #if SIMDPP_ARCH_PP_NS_USE_NULL
    #define SIMDPP_DISPATCH_7_NS_ID_NULL SIMDPP_INSN_ID_NULL
    #else
    #define SIMDPP_DISPATCH_7_NS_ID_NULL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE2
    #define SIMDPP_DISPATCH_7_NS_ID_SSE2 SIMDPP_INSN_ID_SSE2
    #else
    #define SIMDPP_DISPATCH_7_NS_ID_SSE2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE3
    #define SIMDPP_DISPATCH_7_NS_ID_SSE3 SIMDPP_INSN_ID_SSE3
    #else
    #define SIMDPP_DISPATCH_7_NS_ID_SSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSSE3
    #define SIMDPP_DISPATCH_7_NS_ID_SSSE3 SIMDPP_INSN_ID_SSSE3
    #else
    #define SIMDPP_DISPATCH_7_NS_ID_SSSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE4_1
    #define SIMDPP_DISPATCH_7_NS_ID_SSE4_1 SIMDPP_INSN_ID_SSE4_1
    #else
    #define SIMDPP_DISPATCH_7_NS_ID_SSE4_1
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_POPCNT_INSN
    #define SIMDPP_DISPATCH_7_NS_ID_POPCNT_INSN SIMDPP_INSN_ID_POPCNT_INSN
    #else
    #define SIMDPP_DISPATCH_7_NS_ID_POPCNT_INSN
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX
    #define SIMDPP_DISPATCH_7_NS_ID_AVX SIMDPP_INSN_ID_AVX
    #else
    #define SIMDPP_DISPATCH_7_NS_ID_AVX
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX2
    #define SIMDPP_DISPATCH_7_NS_ID_AVX2 SIMDPP_INSN_ID_AVX2
    #else
    #define SIMDPP_DISPATCH_7_NS_ID_AVX2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA3
    #define SIMDPP_DISPATCH_7_NS_ID_FMA3 SIMDPP_INSN_ID_FMA3
    #else
    #define SIMDPP_DISPATCH_7_NS_ID_FMA3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA4
    #define SIMDPP_DISPATCH_7_NS_ID_FMA4 SIMDPP_INSN_ID_FMA4
    #else
    #define SIMDPP_DISPATCH_7_NS_ID_FMA4
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_XOP
    #define SIMDPP_DISPATCH_7_NS_ID_XOP SIMDPP_INSN_ID_XOP
    #else
    #define SIMDPP_DISPATCH_7_NS_ID_XOP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512F
    #define SIMDPP_DISPATCH_7_NS_ID_AVX512F SIMDPP_INSN_ID_AVX512F
    #else
    #define SIMDPP_DISPATCH_7_NS_ID_AVX512F
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512BW
    #define SIMDPP_DISPATCH_7_NS_ID_AVX512BW SIMDPP_INSN_ID_AVX512BW
    #else
    #define SIMDPP_DISPATCH_7_NS_ID_AVX512BW
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512DQ
    #define SIMDPP_DISPATCH_7_NS_ID_AVX512DQ SIMDPP_INSN_ID_AVX512DQ
    #else
    #define SIMDPP_DISPATCH_7_NS_ID_AVX512DQ
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512VL
    #define SIMDPP_DISPATCH_7_NS_ID_AVX512VL SIMDPP_INSN_ID_AVX512VL
    #else
    #define SIMDPP_DISPATCH_7_NS_ID_AVX512VL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON
    #define SIMDPP_DISPATCH_7_NS_ID_NEON SIMDPP_INSN_ID_NEON
    #else
    #define SIMDPP_DISPATCH_7_NS_ID_NEON
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON_FLT_SP
    #define SIMDPP_DISPATCH_7_NS_ID_NEON_FLT_SP SIMDPP_INSN_ID_NEON_FLT_SP
    #else
    #define SIMDPP_DISPATCH_7_NS_ID_NEON_FLT_SP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_MSA
    #define SIMDPP_DISPATCH_7_NS_ID_MSA SIMDPP_INSN_ID_MSA
    #else
    #define SIMDPP_DISPATCH_7_NS_ID_MSA
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_ALTIVEC
    #define SIMDPP_DISPATCH_7_NS_ID_ALTIVEC SIMDPP_INSN_ID_ALTIVEC
    #else
    #define SIMDPP_DISPATCH_7_NS_ID_ALTIVEC
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_206
    #define SIMDPP_DISPATCH_7_NS_ID_VSX_206 SIMDPP_INSN_ID_VSX_206
    #else
    #define SIMDPP_DISPATCH_7_NS_ID_VSX_206
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_207
    #define SIMDPP_DISPATCH_7_NS_ID_VSX_207 SIMDPP_INSN_ID_VSX_207
    #else
    #define SIMDPP_DISPATCH_7_NS_ID_VSX_207
    #endif

    #define SIMDPP_DISPATCH_7_NAMESPACE SIMDPP_PP_PASTE22(arch,               \
        SIMDPP_DISPATCH_7_NS_ID_NULL,                                         \
        SIMDPP_DISPATCH_7_NS_ID_SSE2,                                         \
        SIMDPP_DISPATCH_7_NS_ID_SSE3,                                         \
        SIMDPP_DISPATCH_7_NS_ID_SSSE3,                                        \
        SIMDPP_DISPATCH_7_NS_ID_SSE4_1,                                       \
        SIMDPP_DISPATCH_7_NS_ID_POPCNT_INSN,                                  \
        SIMDPP_DISPATCH_7_NS_ID_AVX,                                          \
        SIMDPP_DISPATCH_7_NS_ID_AVX2,                                         \
        SIMDPP_DISPATCH_7_NS_ID_AVX512F,                                      \
        SIMDPP_DISPATCH_7_NS_ID_AVX512BW,                                     \
        SIMDPP_DISPATCH_7_NS_ID_AVX512DQ,                                     \
        SIMDPP_DISPATCH_7_NS_ID_AVX512VL,                                     \
        SIMDPP_DISPATCH_7_NS_ID_FMA3,                                         \
        SIMDPP_DISPATCH_7_NS_ID_FMA4,                                         \
        SIMDPP_DISPATCH_7_NS_ID_XOP,                                          \
        SIMDPP_DISPATCH_7_NS_ID_NEON,                                         \
        SIMDPP_DISPATCH_7_NS_ID_NEON_FLT_SP,                                  \
        SIMDPP_DISPATCH_7_NS_ID_MSA,                                          \
        SIMDPP_DISPATCH_7_NS_ID_ALTIVEC,                                      \
        SIMDPP_DISPATCH_7_NS_ID_VSX_206,                                      \
        SIMDPP_DISPATCH_7_NS_ID_VSX_207)

    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE SIMDPP_DISPATCH_7_NAMESPACE
    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH 1
    #include <simdpp/dispatch/preprocess_single_compile_arch.h>
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE

    #define SIMDPP_DISPATCH_7_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                \
        {   /* the following will fail if the overload is not available */    \
            FUN_TYPE fun_ptr = &SIMDPP_DISPATCH_7_NAMESPACE::SIMDPP_PP_REMOVE_PARENS(NAME);\
            ARRAY[7-1] = ::simdpp::SIMDPP_DISPATCH_7_NAMESPACE::detail::create_fn_version(fun_ptr);\
        }

    #define SIMDPP_DISPATCH_7_FN_DECLARE(SIGNATURE)                           \
        namespace SIMDPP_DISPATCH_7_NAMESPACE {                               \
            SIMDPP_PP_REMOVE_PARENS(SIGNATURE);                               \
        }
    #undef SIMDPP_ARCH_PP_LIST
#else
    #define SIMDPP_DISPATCH_7_FN_REGISTER(ARRAY,NAME,FUN_TYPE)
    #define SIMDPP_DISPATCH_7_FN_DECLARE(SIGNATURE)
#endif

#ifdef SIMDPP_DISPATCH_ARCH8
    #define SIMDPP_ARCH_PP_LIST SIMDPP_DISPATCH_ARCH8
    #include <simdpp/detail/preprocess_single_arch.h>

    // Use the results of preprocess_single_arch.h to define
    // SIMDPP_DISPATCH_8_NAMESPACE

    #if SIMDPP_ARCH_PP_NS_USE_NULL
    #define SIMDPP_DISPATCH_8_NS_ID_NULL SIMDPP_INSN_ID_NULL
    #else
    #define SIMDPP_DISPATCH_8_NS_ID_NULL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE2
    #define SIMDPP_DISPATCH_8_NS_ID_SSE2 SIMDPP_INSN_ID_SSE2
    #else
    #define SIMDPP_DISPATCH_8_NS_ID_SSE2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE3
    #define SIMDPP_DISPATCH_8_NS_ID_SSE3 SIMDPP_INSN_ID_SSE3
    #else
    #define SIMDPP_DISPATCH_8_NS_ID_SSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSSE3
    #define SIMDPP_DISPATCH_8_NS_ID_SSSE3 SIMDPP_INSN_ID_SSSE3
    #else
    #define SIMDPP_DISPATCH_8_NS_ID_SSSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE4_1
    #define SIMDPP_DISPATCH_8_NS_ID_SSE4_1 SIMDPP_INSN_ID_SSE4_1
    #else
    #define SIMDPP_DISPATCH_8_NS_ID_SSE4_1
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_POPCNT_INSN
    #define SIMDPP_DISPATCH_8_NS_ID_POPCNT_INSN SIMDPP_INSN_ID_POPCNT_INSN
    #else
    #define SIMDPP_DISPATCH_8_NS_ID_POPCNT_INSN
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX
    #define SIMDPP_DISPATCH_8_NS_ID_AVX SIMDPP_INSN_ID_AVX
    #else
    #define SIMDPP_DISPATCH_8_NS_ID_AVX
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX2
    #define SIMDPP_DISPATCH_8_NS_ID_AVX2 SIMDPP_INSN_ID_AVX2
    #else
    #define SIMDPP_DISPATCH_8_NS_ID_AVX2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA3
    #define SIMDPP_DISPATCH_8_NS_ID_FMA3 SIMDPP_INSN_ID_FMA3
    #else
    #define SIMDPP_DISPATCH_8_NS_ID_FMA3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA4
    #define SIMDPP_DISPATCH_8_NS_ID_FMA4 SIMDPP_INSN_ID_FMA4
    #else
    #define SIMDPP_DISPATCH_8_NS_ID_FMA4
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_XOP
    #define SIMDPP_DISPATCH_8_NS_ID_XOP SIMDPP_INSN_ID_XOP
    #else
    #define SIMDPP_DISPATCH_8_NS_ID_XOP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512F
    #define SIMDPP_DISPATCH_8_NS_ID_AVX512F SIMDPP_INSN_ID_AVX512F
    #else
    #define SIMDPP_DISPATCH_8_NS_ID_AVX512F
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512BW
    #define SIMDPP_DISPATCH_8_NS_ID_AVX512BW SIMDPP_INSN_ID_AVX512BW
    #else
    #define SIMDPP_DISPATCH_8_NS_ID_AVX512BW
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512DQ
    #define SIMDPP_DISPATCH_8_NS_ID_AVX512DQ SIMDPP_INSN_ID_AVX512DQ
    #else
    #define SIMDPP_DISPATCH_8_NS_ID_AVX512DQ
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512VL
    #define SIMDPP_DISPATCH_8_NS_ID_AVX512VL SIMDPP_INSN_ID_AVX512VL
    #else
    #define SIMDPP_DISPATCH_8_NS_ID_AVX512VL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON
    #define SIMDPP_DISPATCH_8_NS_ID_NEON SIMDPP_INSN_ID_NEON
    #else
    #define SIMDPP_DISPATCH_8_NS_ID_NEON
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON_FLT_SP
    #define SIMDPP_DISPATCH_8_NS_ID_NEON_FLT_SP SIMDPP_INSN_ID_NEON_FLT_SP
    #else
    #define SIMDPP_DISPATCH_8_NS_ID_NEON_FLT_SP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_MSA
    #define SIMDPP_DISPATCH_8_NS_ID_MSA SIMDPP_INSN_ID_MSA
    #else
    #define SIMDPP_DISPATCH_8_NS_ID_MSA
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_ALTIVEC
    #define SIMDPP_DISPATCH_8_NS_ID_ALTIVEC SIMDPP_INSN_ID_ALTIVEC
    #else
    #define SIMDPP_DISPATCH_8_NS_ID_ALTIVEC
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_206
    #define SIMDPP_DISPATCH_8_NS_ID_VSX_206 SIMDPP_INSN_ID_VSX_206
    #else
    #define SIMDPP_DISPATCH_8_NS_ID_VSX_206
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_207
    #define SIMDPP_DISPATCH_8_NS_ID_VSX_207 SIMDPP_INSN_ID_VSX_207
    #else
    #define SIMDPP_DISPATCH_8_NS_ID_VSX_207
    #endif

    #define SIMDPP_DISPATCH_8_NAMESPACE SIMDPP_PP_PASTE22(arch,               \
        SIMDPP_DISPATCH_8_NS_ID_NULL,                                         \
        SIMDPP_DISPATCH_8_NS_ID_SSE2,                                         \
        SIMDPP_DISPATCH_8_NS_ID_SSE3,                                         \
        SIMDPP_DISPATCH_8_NS_ID_SSSE3,                                        \
        SIMDPP_DISPATCH_8_NS_ID_SSE4_1,                                       \
        SIMDPP_DISPATCH_8_NS_ID_POPCNT_INSN,                                  \
        SIMDPP_DISPATCH_8_NS_ID_AVX,                                          \
        SIMDPP_DISPATCH_8_NS_ID_AVX2,                                         \
        SIMDPP_DISPATCH_8_NS_ID_AVX512F,                                      \
        SIMDPP_DISPATCH_8_NS_ID_AVX512BW,                                     \
        SIMDPP_DISPATCH_8_NS_ID_AVX512DQ,                                     \
        SIMDPP_DISPATCH_8_NS_ID_AVX512VL,                                     \
        SIMDPP_DISPATCH_8_NS_ID_FMA3,                                         \
        SIMDPP_DISPATCH_8_NS_ID_FMA4,                                         \
        SIMDPP_DISPATCH_8_NS_ID_XOP,                                          \
        SIMDPP_DISPATCH_8_NS_ID_NEON,                                         \
        SIMDPP_DISPATCH_8_NS_ID_NEON_FLT_SP,                                  \
        SIMDPP_DISPATCH_8_NS_ID_MSA,                                          \
        SIMDPP_DISPATCH_8_NS_ID_ALTIVEC,                                      \
        SIMDPP_DISPATCH_8_NS_ID_VSX_206,                                      \
        SIMDPP_DISPATCH_8_NS_ID_VSX_207)

    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE SIMDPP_DISPATCH_8_NAMESPACE
    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH 1
    #include <simdpp/dispatch/preprocess_single_compile_arch.h>
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE

    #define SIMDPP_DISPATCH_8_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                \
        {   /* the following will fail if the overload is not available */    \
            FUN_TYPE fun_ptr = &SIMDPP_DISPATCH_8_NAMESPACE::SIMDPP_PP_REMOVE_PARENS(NAME);\
            ARRAY[8-1] = ::simdpp::SIMDPP_DISPATCH_8_NAMESPACE::detail::create_fn_version(fun_ptr);\
        }

    #define SIMDPP_DISPATCH_8_FN_DECLARE(SIGNATURE)                           \
        namespace SIMDPP_DISPATCH_8_NAMESPACE {                               \
            SIMDPP_PP_REMOVE_PARENS(SIGNATURE);                               \
        }
    #undef SIMDPP_ARCH_PP_LIST
#else
    #define SIMDPP_DISPATCH_8_FN_REGISTER(ARRAY,NAME,FUN_TYPE)
    #define SIMDPP_DISPATCH_8_FN_DECLARE(SIGNATURE)
#endif

#ifdef SIMDPP_DISPATCH_ARCH9
    #define SIMDPP_ARCH_PP_LIST SIMDPP_DISPATCH_ARCH9
    #include <simdpp/detail/preprocess_single_arch.h>

    // Use the results of preprocess_single_arch.h to define
    // SIMDPP_DISPATCH_9_NAMESPACE

    #if SIMDPP_ARCH_PP_NS_USE_NULL
    #define SIMDPP_DISPATCH_9_NS_ID_NULL SIMDPP_INSN_ID_NULL
    #else
    #define SIMDPP_DISPATCH_9_NS_ID_NULL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE2
    #define SIMDPP_DISPATCH_9_NS_ID_SSE2 SIMDPP_INSN_ID_SSE2
    #else
    #define SIMDPP_DISPATCH_9_NS_ID_SSE2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE3
    #define SIMDPP_DISPATCH_9_NS_ID_SSE3 SIMDPP_INSN_ID_SSE3
    #else
    #define SIMDPP_DISPATCH_9_NS_ID_SSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSSE3
    #define SIMDPP_DISPATCH_9_NS_ID_SSSE3 SIMDPP_INSN_ID_SSSE3
    #else
    #define SIMDPP_DISPATCH_9_NS_ID_SSSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE4_1
    #define SIMDPP_DISPATCH_9_NS_ID_SSE4_1 SIMDPP_INSN_ID_SSE4_1
    #else
    #define SIMDPP_DISPATCH_9_NS_ID_SSE4_1
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_POPCNT_INSN
    #define SIMDPP_DISPATCH_9_NS_ID_POPCNT_INSN SIMDPP_INSN_ID_POPCNT_INSN
    #else
    #define SIMDPP_DISPATCH_9_NS_ID_POPCNT_INSN
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX
    #define SIMDPP_DISPATCH_9_NS_ID_AVX SIMDPP_INSN_ID_AVX
    #else
    #define SIMDPP_DISPATCH_9_NS_ID_AVX
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX2
    #define SIMDPP_DISPATCH_9_NS_ID_AVX2 SIMDPP_INSN_ID_AVX2
    #else
    #define SIMDPP_DISPATCH_9_NS_ID_AVX2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA3
    #define SIMDPP_DISPATCH_9_NS_ID_FMA3 SIMDPP_INSN_ID_FMA3
    #else
    #define SIMDPP_DISPATCH_9_NS_ID_FMA3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA4
    #define SIMDPP_DISPATCH_9_NS_ID_FMA4 SIMDPP_INSN_ID_FMA4
    #else
    #define SIMDPP_DISPATCH_9_NS_ID_FMA4
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_XOP
    #define SIMDPP_DISPATCH_9_NS_ID_XOP SIMDPP_INSN_ID_XOP
    #else
    #define SIMDPP_DISPATCH_9_NS_ID_XOP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512F
    #define SIMDPP_DISPATCH_9_NS_ID_AVX512F SIMDPP_INSN_ID_AVX512F
    #else
    #define SIMDPP_DISPATCH_9_NS_ID_AVX512F
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512BW
    #define SIMDPP_DISPATCH_9_NS_ID_AVX512BW SIMDPP_INSN_ID_AVX512BW
    #else
    #define SIMDPP_DISPATCH_9_NS_ID_AVX512BW
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512DQ
    #define SIMDPP_DISPATCH_9_NS_ID_AVX512DQ SIMDPP_INSN_ID_AVX512DQ
    #else
    #define SIMDPP_DISPATCH_9_NS_ID_AVX512DQ
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512VL
    #define SIMDPP_DISPATCH_9_NS_ID_AVX512VL SIMDPP_INSN_ID_AVX512VL
    #else
    #define SIMDPP_DISPATCH_9_NS_ID_AVX512VL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON
    #define SIMDPP_DISPATCH_9_NS_ID_NEON SIMDPP_INSN_ID_NEON
    #else
    #define SIMDPP_DISPATCH_9_NS_ID_NEON
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON_FLT_SP
    #define SIMDPP_DISPATCH_9_NS_ID_NEON_FLT_SP SIMDPP_INSN_ID_NEON_FLT_SP
    #else
    #define SIMDPP_DISPATCH_9_NS_ID_NEON_FLT_SP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_MSA
    #define SIMDPP_DISPATCH_9_NS_ID_MSA SIMDPP_INSN_ID_MSA
    #else
    #define SIMDPP_DISPATCH_9_NS_ID_MSA
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_ALTIVEC
    #define SIMDPP_DISPATCH_9_NS_ID_ALTIVEC SIMDPP_INSN_ID_ALTIVEC
    #else
    #define SIMDPP_DISPATCH_9_NS_ID_ALTIVEC
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_206
    #define SIMDPP_DISPATCH_9_NS_ID_VSX_206 SIMDPP_INSN_ID_VSX_206
    #else
    #define SIMDPP_DISPATCH_9_NS_ID_VSX_206
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_207
    #define SIMDPP_DISPATCH_9_NS_ID_VSX_207 SIMDPP_INSN_ID_VSX_207
    #else
    #define SIMDPP_DISPATCH_9_NS_ID_VSX_207
    #endif

    #define SIMDPP_DISPATCH_9_NAMESPACE SIMDPP_PP_PASTE22(arch,               \
        SIMDPP_DISPATCH_9_NS_ID_NULL,                                         \
        SIMDPP_DISPATCH_9_NS_ID_SSE2,                                         \
        SIMDPP_DISPATCH_9_NS_ID_SSE3,                                         \
        SIMDPP_DISPATCH_9_NS_ID_SSSE3,                                        \
        SIMDPP_DISPATCH_9_NS_ID_SSE4_1,                                       \
        SIMDPP_DISPATCH_9_NS_ID_POPCNT_INSN,                                  \
        SIMDPP_DISPATCH_9_NS_ID_AVX,                                          \
        SIMDPP_DISPATCH_9_NS_ID_AVX2,                                         \
        SIMDPP_DISPATCH_9_NS_ID_AVX512F,                                      \
        SIMDPP_DISPATCH_9_NS_ID_AVX512BW,                                     \
        SIMDPP_DISPATCH_9_NS_ID_AVX512DQ,                                     \
        SIMDPP_DISPATCH_9_NS_ID_AVX512VL,                                     \
        SIMDPP_DISPATCH_9_NS_ID_FMA3,                                         \
        SIMDPP_DISPATCH_9_NS_ID_FMA4,                                         \
        SIMDPP_DISPATCH_9_NS_ID_XOP,                                          \
        SIMDPP_DISPATCH_9_NS_ID_NEON,                                         \
        SIMDPP_DISPATCH_9_NS_ID_NEON_FLT_SP,                                  \
        SIMDPP_DISPATCH_9_NS_ID_MSA,                                          \
        SIMDPP_DISPATCH_9_NS_ID_ALTIVEC,                                      \
        SIMDPP_DISPATCH_9_NS_ID_VSX_206,                                      \
        SIMDPP_DISPATCH_9_NS_ID_VSX_207)

    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE SIMDPP_DISPATCH_9_NAMESPACE
    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH 1
    #include <simdpp/dispatch/preprocess_single_compile_arch.h>
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE

    #define SIMDPP_DISPATCH_9_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                \
        {   /* the following will fail if the overload is not available */    \
            FUN_TYPE fun_ptr = &SIMDPP_DISPATCH_9_NAMESPACE::SIMDPP_PP_REMOVE_PARENS(NAME);\
            ARRAY[9-1] = ::simdpp::SIMDPP_DISPATCH_9_NAMESPACE::detail::create_fn_version(fun_ptr);\
        }

    #define SIMDPP_DISPATCH_9_FN_DECLARE(SIGNATURE)                           \
        namespace SIMDPP_DISPATCH_9_NAMESPACE {                               \
            SIMDPP_PP_REMOVE_PARENS(SIGNATURE);                               \
        }
    #undef SIMDPP_ARCH_PP_LIST
#else
    #define SIMDPP_DISPATCH_9_FN_REGISTER(ARRAY,NAME,FUN_TYPE)
    #define SIMDPP_DISPATCH_9_FN_DECLARE(SIGNATURE)
#endif

#ifdef SIMDPP_DISPATCH_ARCH10
    #define SIMDPP_ARCH_PP_LIST SIMDPP_DISPATCH_ARCH10
    #include <simdpp/detail/preprocess_single_arch.h>

    // Use the results of preprocess_single_arch.h to define
    // SIMDPP_DISPATCH_10_NAMESPACE

    #if SIMDPP_ARCH_PP_NS_USE_NULL
    #define SIMDPP_DISPATCH_10_NS_ID_NULL SIMDPP_INSN_ID_NULL
    #else
    #define SIMDPP_DISPATCH_10_NS_ID_NULL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE2
    #define SIMDPP_DISPATCH_10_NS_ID_SSE2 SIMDPP_INSN_ID_SSE2
    #else
    #define SIMDPP_DISPATCH_10_NS_ID_SSE2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE3
    #define SIMDPP_DISPATCH_10_NS_ID_SSE3 SIMDPP_INSN_ID_SSE3
    #else
    #define SIMDPP_DISPATCH_10_NS_ID_SSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSSE3
    #define SIMDPP_DISPATCH_10_NS_ID_SSSE3 SIMDPP_INSN_ID_SSSE3
    #else
    #define SIMDPP_DISPATCH_10_NS_ID_SSSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE4_1
    #define SIMDPP_DISPATCH_10_NS_ID_SSE4_1 SIMDPP_INSN_ID_SSE4_1
    #else
    #define SIMDPP_DISPATCH_10_NS_ID_SSE4_1
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_POPCNT_INSN
    #define SIMDPP_DISPATCH_10_NS_ID_POPCNT_INSN SIMDPP_INSN_ID_POPCNT_INSN
    #else
    #define SIMDPP_DISPATCH_10_NS_ID_POPCNT_INSN
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX
    #define SIMDPP_DISPATCH_10_NS_ID_AVX SIMDPP_INSN_ID_AVX
    #else
    #define SIMDPP_DISPATCH_10_NS_ID_AVX
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX2
    #define SIMDPP_DISPATCH_10_NS_ID_AVX2 SIMDPP_INSN_ID_AVX2
    #else
    #define SIMDPP_DISPATCH_10_NS_ID_AVX2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA3
    #define SIMDPP_DISPATCH_10_NS_ID_FMA3 SIMDPP_INSN_ID_FMA3
    #else
    #define SIMDPP_DISPATCH_10_NS_ID_FMA3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA4
    #define SIMDPP_DISPATCH_10_NS_ID_FMA4 SIMDPP_INSN_ID_FMA4
    #else
    #define SIMDPP_DISPATCH_10_NS_ID_FMA4
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_XOP
    #define SIMDPP_DISPATCH_10_NS_ID_XOP SIMDPP_INSN_ID_XOP
    #else
    #define SIMDPP_DISPATCH_10_NS_ID_XOP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512F
    #define SIMDPP_DISPATCH_10_NS_ID_AVX512F SIMDPP_INSN_ID_AVX512F
    #else
    #define SIMDPP_DISPATCH_10_NS_ID_AVX512F
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512BW
    #define SIMDPP_DISPATCH_10_NS_ID_AVX512BW SIMDPP_INSN_ID_AVX512BW
    #else
    #define SIMDPP_DISPATCH_10_NS_ID_AVX512BW
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512DQ
    #define SIMDPP_DISPATCH_10_NS_ID_AVX512DQ SIMDPP_INSN_ID_AVX512DQ
    #else
    #define SIMDPP_DISPATCH_10_NS_ID_AVX512DQ
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512VL
    #define SIMDPP_DISPATCH_10_NS_ID_AVX512VL SIMDPP_INSN_ID_AVX512VL
    #else
    #define SIMDPP_DISPATCH_10_NS_ID_AVX512VL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON
    #define SIMDPP_DISPATCH_10_NS_ID_NEON SIMDPP_INSN_ID_NEON
    #else
    #define SIMDPP_DISPATCH_10_NS_ID_NEON
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON_FLT_SP
    #define SIMDPP_DISPATCH_10_NS_ID_NEON_FLT_SP SIMDPP_INSN_ID_NEON_FLT_SP
    #else
    #define SIMDPP_DISPATCH_10_NS_ID_NEON_FLT_SP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_MSA
    #define SIMDPP_DISPATCH_10_NS_ID_MSA SIMDPP_INSN_ID_MSA
    #else
    #define SIMDPP_DISPATCH_10_NS_ID_MSA
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_ALTIVEC
    #define SIMDPP_DISPATCH_10_NS_ID_ALTIVEC SIMDPP_INSN_ID_ALTIVEC
    #else
    #define SIMDPP_DISPATCH_10_NS_ID_ALTIVEC
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_206
    #define SIMDPP_DISPATCH_10_NS_ID_VSX_206 SIMDPP_INSN_ID_VSX_206
    #else
    #define SIMDPP_DISPATCH_10_NS_ID_VSX_206
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_207
    #define SIMDPP_DISPATCH_10_NS_ID_VSX_207 SIMDPP_INSN_ID_VSX_207
    #else
    #define SIMDPP_DISPATCH_10_NS_ID_VSX_207
    #endif

    #define SIMDPP_DISPATCH_10_NAMESPACE SIMDPP_PP_PASTE22(arch,              \
        SIMDPP_DISPATCH_10_NS_ID_NULL,                                        \
        SIMDPP_DISPATCH_10_NS_ID_SSE2,                                        \
        SIMDPP_DISPATCH_10_NS_ID_SSE3,                                        \
        SIMDPP_DISPATCH_10_NS_ID_SSSE3,                                       \
        SIMDPP_DISPATCH_10_NS_ID_SSE4_1,                                      \
        SIMDPP_DISPATCH_10_NS_ID_POPCNT_INSN,                                 \
        SIMDPP_DISPATCH_10_NS_ID_AVX,                                         \
        SIMDPP_DISPATCH_10_NS_ID_AVX2,                                        \
        SIMDPP_DISPATCH_10_NS_ID_AVX512F,                                     \
        SIMDPP_DISPATCH_10_NS_ID_AVX512BW,                                    \
        SIMDPP_DISPATCH_10_NS_ID_AVX512DQ,                                    \
        SIMDPP_DISPATCH_10_NS_ID_AVX512VL,                                    \
        SIMDPP_DISPATCH_10_NS_ID_FMA3,                                        \
        SIMDPP_DISPATCH_10_NS_ID_FMA4,                                        \
        SIMDPP_DISPATCH_10_NS_ID_XOP,                                         \
        SIMDPP_DISPATCH_10_NS_ID_NEON,                                        \
        SIMDPP_DISPATCH_10_NS_ID_NEON_FLT_SP,                                 \
        SIMDPP_DISPATCH_10_NS_ID_MSA,                                         \
        SIMDPP_DISPATCH_10_NS_ID_ALTIVEC,                                     \
        SIMDPP_DISPATCH_10_NS_ID_VSX_206,                                     \
        SIMDPP_DISPATCH_10_NS_ID_VSX_207)

    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE SIMDPP_DISPATCH_10_NAMESPACE
    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH 1
    #include <simdpp/dispatch/preprocess_single_compile_arch.h>
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE

    #define SIMDPP_DISPATCH_10_FN_REGISTER(ARRAY,NAME,FUN_TYPE)               \
        {   /* the following will fail if the overload is not available */    \
            FUN_TYPE fun_ptr = &SIMDPP_DISPATCH_10_NAMESPACE::SIMDPP_PP_REMOVE_PARENS(NAME);\
            ARRAY[10-1] = ::simdpp::SIMDPP_DISPATCH_10_NAMESPACE::detail::create_fn_version(fun_ptr);\
        }

    #define SIMDPP_DISPATCH_10_FN_DECLARE(SIGNATURE)                          \
        namespace SIMDPP_DISPATCH_10_NAMESPACE {                              \
            SIMDPP_PP_REMOVE_PARENS(SIGNATURE);                               \
        }
    #undef SIMDPP_ARCH_PP_LIST
#else
    #define SIMDPP_DISPATCH_10_FN_REGISTER(ARRAY,NAME,FUN_TYPE)
    #define SIMDPP_DISPATCH_10_FN_DECLARE(SIGNATURE)
#endif

#ifdef SIMDPP_DISPATCH_ARCH11
    #define SIMDPP_ARCH_PP_LIST SIMDPP_DISPATCH_ARCH11
    #include <simdpp/detail/preprocess_single_arch.h>

    // Use the results of preprocess_single_arch.h to define
    // SIMDPP_DISPATCH_11_NAMESPACE

    #if SIMDPP_ARCH_PP_NS_USE_NULL
    #define SIMDPP_DISPATCH_11_NS_ID_NULL SIMDPP_INSN_ID_NULL
    #else
    #define SIMDPP_DISPATCH_11_NS_ID_NULL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE2
    #define SIMDPP_DISPATCH_11_NS_ID_SSE2 SIMDPP_INSN_ID_SSE2
    #else
    #define SIMDPP_DISPATCH_11_NS_ID_SSE2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE3
    #define SIMDPP_DISPATCH_11_NS_ID_SSE3 SIMDPP_INSN_ID_SSE3
    #else
    #define SIMDPP_DISPATCH_11_NS_ID_SSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSSE3
    #define SIMDPP_DISPATCH_11_NS_ID_SSSE3 SIMDPP_INSN_ID_SSSE3
    #else
    #define SIMDPP_DISPATCH_11_NS_ID_SSSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE4_1
    #define SIMDPP_DISPATCH_11_NS_ID_SSE4_1 SIMDPP_INSN_ID_SSE4_1
    #else
    #define SIMDPP_DISPATCH_11_NS_ID_SSE4_1
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_POPCNT_INSN
    #define SIMDPP_DISPATCH_11_NS_ID_POPCNT_INSN SIMDPP_INSN_ID_POPCNT_INSN
    #else
    #define SIMDPP_DISPATCH_11_NS_ID_POPCNT_INSN
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX
    #define SIMDPP_DISPATCH_11_NS_ID_AVX SIMDPP_INSN_ID_AVX
    #else
    #define SIMDPP_DISPATCH_11_NS_ID_AVX
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX2
    #define SIMDPP_DISPATCH_11_NS_ID_AVX2 SIMDPP_INSN_ID_AVX2
    #else
    #define SIMDPP_DISPATCH_11_NS_ID_AVX2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA3
    #define SIMDPP_DISPATCH_11_NS_ID_FMA3 SIMDPP_INSN_ID_FMA3
    #else
    #define SIMDPP_DISPATCH_11_NS_ID_FMA3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA4
    #define SIMDPP_DISPATCH_11_NS_ID_FMA4 SIMDPP_INSN_ID_FMA4
    #else
    #define SIMDPP_DISPATCH_11_NS_ID_FMA4
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_XOP
    #define SIMDPP_DISPATCH_11_NS_ID_XOP SIMDPP_INSN_ID_XOP
    #else
    #define SIMDPP_DISPATCH_11_NS_ID_XOP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512F
    #define SIMDPP_DISPATCH_11_NS_ID_AVX512F SIMDPP_INSN_ID_AVX512F
    #else
    #define SIMDPP_DISPATCH_11_NS_ID_AVX512F
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512BW
    #define SIMDPP_DISPATCH_11_NS_ID_AVX512BW SIMDPP_INSN_ID_AVX512BW
    #else
    #define SIMDPP_DISPATCH_11_NS_ID_AVX512BW
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512DQ
    #define SIMDPP_DISPATCH_11_NS_ID_AVX512DQ SIMDPP_INSN_ID_AVX512DQ
    #else
    #define SIMDPP_DISPATCH_11_NS_ID_AVX512DQ
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512VL
    #define SIMDPP_DISPATCH_11_NS_ID_AVX512VL SIMDPP_INSN_ID_AVX512VL
    #else
    #define SIMDPP_DISPATCH_11_NS_ID_AVX512VL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON
    #define SIMDPP_DISPATCH_11_NS_ID_NEON SIMDPP_INSN_ID_NEON
    #else
    #define SIMDPP_DISPATCH_11_NS_ID_NEON
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON_FLT_SP
    #define SIMDPP_DISPATCH_11_NS_ID_NEON_FLT_SP SIMDPP_INSN_ID_NEON_FLT_SP
    #else
    #define SIMDPP_DISPATCH_11_NS_ID_NEON_FLT_SP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_MSA
    #define SIMDPP_DISPATCH_11_NS_ID_MSA SIMDPP_INSN_ID_MSA
    #else
    #define SIMDPP_DISPATCH_11_NS_ID_MSA
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_ALTIVEC
    #define SIMDPP_DISPATCH_11_NS_ID_ALTIVEC SIMDPP_INSN_ID_ALTIVEC
    #else
    #define SIMDPP_DISPATCH_11_NS_ID_ALTIVEC
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_206
    #define SIMDPP_DISPATCH_11_NS_ID_VSX_206 SIMDPP_INSN_ID_VSX_206
    #else
    #define SIMDPP_DISPATCH_11_NS_ID_VSX_206
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_207
    #define SIMDPP_DISPATCH_11_NS_ID_VSX_207 SIMDPP_INSN_ID_VSX_207
    #else
    #define SIMDPP_DISPATCH_11_NS_ID_VSX_207
    #endif

    #define SIMDPP_DISPATCH_11_NAMESPACE SIMDPP_PP_PASTE22(arch,              \
        SIMDPP_DISPATCH_11_NS_ID_NULL,                                        \
        SIMDPP_DISPATCH_11_NS_ID_SSE2,                                        \
        SIMDPP_DISPATCH_11_NS_ID_SSE3,                                        \
        SIMDPP_DISPATCH_11_NS_ID_SSSE3,                                       \
        SIMDPP_DISPATCH_11_NS_ID_SSE4_1,                                      \
        SIMDPP_DISPATCH_11_NS_ID_POPCNT_INSN,                                 \
        SIMDPP_DISPATCH_11_NS_ID_AVX,                                         \
        SIMDPP_DISPATCH_11_NS_ID_AVX2,                                        \
        SIMDPP_DISPATCH_11_NS_ID_AVX512F,                                     \
        SIMDPP_DISPATCH_11_NS_ID_AVX512BW,                                    \
        SIMDPP_DISPATCH_11_NS_ID_AVX512DQ,                                    \
        SIMDPP_DISPATCH_11_NS_ID_AVX512VL,                                    \
        SIMDPP_DISPATCH_11_NS_ID_FMA3,                                        \
        SIMDPP_DISPATCH_11_NS_ID_FMA4,                                        \
        SIMDPP_DISPATCH_11_NS_ID_XOP,                                         \
        SIMDPP_DISPATCH_11_NS_ID_NEON,                                        \
        SIMDPP_DISPATCH_11_NS_ID_NEON_FLT_SP,                                 \
        SIMDPP_DISPATCH_11_NS_ID_MSA,                                         \
        SIMDPP_DISPATCH_11_NS_ID_ALTIVEC,                                     \
        SIMDPP_DISPATCH_11_NS_ID_VSX_206,                                     \
        SIMDPP_DISPATCH_11_NS_ID_VSX_207)

    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE SIMDPP_DISPATCH_11_NAMESPACE
    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH 1
    #include <simdpp/dispatch/preprocess_single_compile_arch.h>
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE

    #define SIMDPP_DISPATCH_11_FN_REGISTER(ARRAY,NAME,FUN_TYPE)               \
        {   /* the following will fail if the overload is not available */    \
            FUN_TYPE fun_ptr = &SIMDPP_DISPATCH_11_NAMESPACE::SIMDPP_PP_REMOVE_PARENS(NAME);\
            ARRAY[11-1] = ::simdpp::SIMDPP_DISPATCH_11_NAMESPACE::detail::create_fn_version(fun_ptr);\
        }

    #define SIMDPP_DISPATCH_11_FN_DECLARE(SIGNATURE)                          \
        namespace SIMDPP_DISPATCH_11_NAMESPACE {                              \
            SIMDPP_PP_REMOVE_PARENS(SIGNATURE);                               \
        }
    #undef SIMDPP_ARCH_PP_LIST
#else
    #define SIMDPP_DISPATCH_11_FN_REGISTER(ARRAY,NAME,FUN_TYPE)
    #define SIMDPP_DISPATCH_11_FN_DECLARE(SIGNATURE)
#endif

#ifdef SIMDPP_DISPATCH_ARCH12
    #define SIMDPP_ARCH_PP_LIST SIMDPP_DISPATCH_ARCH12
    #include <simdpp/detail/preprocess_single_arch.h>

    // Use the results of preprocess_single_arch.h to define
    // SIMDPP_DISPATCH_12_NAMESPACE

    #if SIMDPP_ARCH_PP_NS_USE_NULL
    #define SIMDPP_DISPATCH_12_NS_ID_NULL SIMDPP_INSN_ID_NULL
    #else
    #define SIMDPP_DISPATCH_12_NS_ID_NULL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE2
    #define SIMDPP_DISPATCH_12_NS_ID_SSE2 SIMDPP_INSN_ID_SSE2
    #else
    #define SIMDPP_DISPATCH_12_NS_ID_SSE2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE3
    #define SIMDPP_DISPATCH_12_NS_ID_SSE3 SIMDPP_INSN_ID_SSE3
    #else
    #define SIMDPP_DISPATCH_12_NS_ID_SSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSSE3
    #define SIMDPP_DISPATCH_12_NS_ID_SSSE3 SIMDPP_INSN_ID_SSSE3
    #else
    #define SIMDPP_DISPATCH_12_NS_ID_SSSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE4_1
    #define SIMDPP_DISPATCH_12_NS_ID_SSE4_1 SIMDPP_INSN_ID_SSE4_1
    #else
    #define SIMDPP_DISPATCH_12_NS_ID_SSE4_1
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_POPCNT_INSN
    #define SIMDPP_DISPATCH_12_NS_ID_POPCNT_INSN SIMDPP_INSN_ID_POPCNT_INSN
    #else
    #define SIMDPP_DISPATCH_12_NS_ID_POPCNT_INSN
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX
    #define SIMDPP_DISPATCH_12_NS_ID_AVX SIMDPP_INSN_ID_AVX
    #else
    #define SIMDPP_DISPATCH_12_NS_ID_AVX
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX2
    #define SIMDPP_DISPATCH_12_NS_ID_AVX2 SIMDPP_INSN_ID_AVX2
    #else
    #define SIMDPP_DISPATCH_12_NS_ID_AVX2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA3
    #define SIMDPP_DISPATCH_12_NS_ID_FMA3 SIMDPP_INSN_ID_FMA3
    #else
    #define SIMDPP_DISPATCH_12_NS_ID_FMA3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA4
    #define SIMDPP_DISPATCH_12_NS_ID_FMA4 SIMDPP_INSN_ID_FMA4
    #else
    #define SIMDPP_DISPATCH_12_NS_ID_FMA4
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_XOP
    #define SIMDPP_DISPATCH_12_NS_ID_XOP SIMDPP_INSN_ID_XOP
    #else
    #define SIMDPP_DISPATCH_12_NS_ID_XOP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512F
    #define SIMDPP_DISPATCH_12_NS_ID_AVX512F SIMDPP_INSN_ID_AVX512F
    #else
    #define SIMDPP_DISPATCH_12_NS_ID_AVX512F
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512BW
    #define SIMDPP_DISPATCH_12_NS_ID_AVX512BW SIMDPP_INSN_ID_AVX512BW
    #else
    #define SIMDPP_DISPATCH_12_NS_ID_AVX512BW
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512DQ
    #define SIMDPP_DISPATCH_12_NS_ID_AVX512DQ SIMDPP_INSN_ID_AVX512DQ
    #else
    #define SIMDPP_DISPATCH_12_NS_ID_AVX512DQ
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512VL
    #define SIMDPP_DISPATCH_12_NS_ID_AVX512VL SIMDPP_INSN_ID_AVX512VL
    #else
    #define SIMDPP_DISPATCH_12_NS_ID_AVX512VL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON
    #define SIMDPP_DISPATCH_12_NS_ID_NEON SIMDPP_INSN_ID_NEON
    #else
    #define SIMDPP_DISPATCH_12_NS_ID_NEON
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON_FLT_SP
    #define SIMDPP_DISPATCH_12_NS_ID_NEON_FLT_SP SIMDPP_INSN_ID_NEON_FLT_SP
    #else
    #define SIMDPP_DISPATCH_12_NS_ID_NEON_FLT_SP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_MSA
    #define SIMDPP_DISPATCH_12_NS_ID_MSA SIMDPP_INSN_ID_MSA
    #else
    #define SIMDPP_DISPATCH_12_NS_ID_MSA
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_ALTIVEC
    #define SIMDPP_DISPATCH_12_NS_ID_ALTIVEC SIMDPP_INSN_ID_ALTIVEC
    #else
    #define SIMDPP_DISPATCH_12_NS_ID_ALTIVEC
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_206
    #define SIMDPP_DISPATCH_12_NS_ID_VSX_206 SIMDPP_INSN_ID_VSX_206
    #else
    #define SIMDPP_DISPATCH_12_NS_ID_VSX_206
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_207
    #define SIMDPP_DISPATCH_12_NS_ID_VSX_207 SIMDPP_INSN_ID_VSX_207
    #else
    #define SIMDPP_DISPATCH_12_NS_ID_VSX_207
    #endif

    #define SIMDPP_DISPATCH_12_NAMESPACE SIMDPP_PP_PASTE22(arch,              \
        SIMDPP_DISPATCH_12_NS_ID_NULL,                                        \
        SIMDPP_DISPATCH_12_NS_ID_SSE2,                                        \
        SIMDPP_DISPATCH_12_NS_ID_SSE3,                                        \
        SIMDPP_DISPATCH_12_NS_ID_SSSE3,                                       \
        SIMDPP_DISPATCH_12_NS_ID_SSE4_1,                                      \
        SIMDPP_DISPATCH_12_NS_ID_POPCNT_INSN,                                 \
        SIMDPP_DISPATCH_12_NS_ID_AVX,                                         \
        SIMDPP_DISPATCH_12_NS_ID_AVX2,                                        \
        SIMDPP_DISPATCH_12_NS_ID_AVX512F,                                     \
        SIMDPP_DISPATCH_12_NS_ID_AVX512BW,                                    \
        SIMDPP_DISPATCH_12_NS_ID_AVX512DQ,                                    \
        SIMDPP_DISPATCH_12_NS_ID_AVX512VL,                                    \
        SIMDPP_DISPATCH_12_NS_ID_FMA3,                                        \
        SIMDPP_DISPATCH_12_NS_ID_FMA4,                                        \
        SIMDPP_DISPATCH_12_NS_ID_XOP,                                         \
        SIMDPP_DISPATCH_12_NS_ID_NEON,                                        \
        SIMDPP_DISPATCH_12_NS_ID_NEON_FLT_SP,                                 \
        SIMDPP_DISPATCH_12_NS_ID_MSA,                                         \
        SIMDPP_DISPATCH_12_NS_ID_ALTIVEC,                                     \
        SIMDPP_DISPATCH_12_NS_ID_VSX_206,                                     \
        SIMDPP_DISPATCH_12_NS_ID_VSX_207)

    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE SIMDPP_DISPATCH_12_NAMESPACE
    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH 1
    #include <simdpp/dispatch/preprocess_single_compile_arch.h>
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE

    #define SIMDPP_DISPATCH_12_FN_REGISTER(ARRAY,NAME,FUN_TYPE)               \
        {   /* the following will fail if the overload is not available */    \
            FUN_TYPE fun_ptr = &SIMDPP_DISPATCH_12_NAMESPACE::SIMDPP_PP_REMOVE_PARENS(NAME);\
            ARRAY[12-1] = ::simdpp::SIMDPP_DISPATCH_12_NAMESPACE::detail::create_fn_version(fun_ptr);\
        }

    #define SIMDPP_DISPATCH_12_FN_DECLARE(SIGNATURE)                          \
        namespace SIMDPP_DISPATCH_12_NAMESPACE {                              \
            SIMDPP_PP_REMOVE_PARENS(SIGNATURE);                               \
        }
    #undef SIMDPP_ARCH_PP_LIST
#else
    #define SIMDPP_DISPATCH_12_FN_REGISTER(ARRAY,NAME,FUN_TYPE)
    #define SIMDPP_DISPATCH_12_FN_DECLARE(SIGNATURE)
#endif

#ifdef SIMDPP_DISPATCH_ARCH13
    #define SIMDPP_ARCH_PP_LIST SIMDPP_DISPATCH_ARCH13
    #include <simdpp/detail/preprocess_single_arch.h>

    // Use the results of preprocess_single_arch.h to define
    // SIMDPP_DISPATCH_13_NAMESPACE

    #if SIMDPP_ARCH_PP_NS_USE_NULL
    #define SIMDPP_DISPATCH_13_NS_ID_NULL SIMDPP_INSN_ID_NULL
    #else
    #define SIMDPP_DISPATCH_13_NS_ID_NULL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE2
    #define SIMDPP_DISPATCH_13_NS_ID_SSE2 SIMDPP_INSN_ID_SSE2
    #else
    #define SIMDPP_DISPATCH_13_NS_ID_SSE2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE3
    #define SIMDPP_DISPATCH_13_NS_ID_SSE3 SIMDPP_INSN_ID_SSE3
    #else
    #define SIMDPP_DISPATCH_13_NS_ID_SSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSSE3
    #define SIMDPP_DISPATCH_13_NS_ID_SSSE3 SIMDPP_INSN_ID_SSSE3
    #else
    #define SIMDPP_DISPATCH_13_NS_ID_SSSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE4_1
    #define SIMDPP_DISPATCH_13_NS_ID_SSE4_1 SIMDPP_INSN_ID_SSE4_1
    #else
    #define SIMDPP_DISPATCH_13_NS_ID_SSE4_1
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_POPCNT_INSN
    #define SIMDPP_DISPATCH_13_NS_ID_POPCNT_INSN SIMDPP_INSN_ID_POPCNT_INSN
    #else
    #define SIMDPP_DISPATCH_13_NS_ID_POPCNT_INSN
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX
    #define SIMDPP_DISPATCH_13_NS_ID_AVX SIMDPP_INSN_ID_AVX
    #else
    #define SIMDPP_DISPATCH_13_NS_ID_AVX
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX2
    #define SIMDPP_DISPATCH_13_NS_ID_AVX2 SIMDPP_INSN_ID_AVX2
    #else
    #define SIMDPP_DISPATCH_13_NS_ID_AVX2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA3
    #define SIMDPP_DISPATCH_13_NS_ID_FMA3 SIMDPP_INSN_ID_FMA3
    #else
    #define SIMDPP_DISPATCH_13_NS_ID_FMA3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA4
    #define SIMDPP_DISPATCH_13_NS_ID_FMA4 SIMDPP_INSN_ID_FMA4
    #else
    #define SIMDPP_DISPATCH_13_NS_ID_FMA4
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_XOP
    #define SIMDPP_DISPATCH_13_NS_ID_XOP SIMDPP_INSN_ID_XOP
    #else
    #define SIMDPP_DISPATCH_13_NS_ID_XOP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512F
    #define SIMDPP_DISPATCH_13_NS_ID_AVX512F SIMDPP_INSN_ID_AVX512F
    #else
    #define SIMDPP_DISPATCH_13_NS_ID_AVX512F
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512BW
    #define SIMDPP_DISPATCH_13_NS_ID_AVX512BW SIMDPP_INSN_ID_AVX512BW
    #else
    #define SIMDPP_DISPATCH_13_NS_ID_AVX512BW
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512DQ
    #define SIMDPP_DISPATCH_13_NS_ID_AVX512DQ SIMDPP_INSN_ID_AVX512DQ
    #else
    #define SIMDPP_DISPATCH_13_NS_ID_AVX512DQ
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512VL
    #define SIMDPP_DISPATCH_13_NS_ID_AVX512VL SIMDPP_INSN_ID_AVX512VL
    #else
    #define SIMDPP_DISPATCH_13_NS_ID_AVX512VL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON
    #define SIMDPP_DISPATCH_13_NS_ID_NEON SIMDPP_INSN_ID_NEON
    #else
    #define SIMDPP_DISPATCH_13_NS_ID_NEON
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON_FLT_SP
    #define SIMDPP_DISPATCH_13_NS_ID_NEON_FLT_SP SIMDPP_INSN_ID_NEON_FLT_SP
    #else
    #define SIMDPP_DISPATCH_13_NS_ID_NEON_FLT_SP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_MSA
    #define SIMDPP_DISPATCH_13_NS_ID_MSA SIMDPP_INSN_ID_MSA
    #else
    #define SIMDPP_DISPATCH_13_NS_ID_MSA
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_ALTIVEC
    #define SIMDPP_DISPATCH_13_NS_ID_ALTIVEC SIMDPP_INSN_ID_ALTIVEC
    #else
    #define SIMDPP_DISPATCH_13_NS_ID_ALTIVEC
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_206
    #define SIMDPP_DISPATCH_13_NS_ID_VSX_206 SIMDPP_INSN_ID_VSX_206
    #else
    #define SIMDPP_DISPATCH_13_NS_ID_VSX_206
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_207
    #define SIMDPP_DISPATCH_13_NS_ID_VSX_207 SIMDPP_INSN_ID_VSX_207
    #else
    #define SIMDPP_DISPATCH_13_NS_ID_VSX_207
    #endif

    #define SIMDPP_DISPATCH_13_NAMESPACE SIMDPP_PP_PASTE22(arch,              \
        SIMDPP_DISPATCH_13_NS_ID_NULL,                                        \
        SIMDPP_DISPATCH_13_NS_ID_SSE2,                                        \
        SIMDPP_DISPATCH_13_NS_ID_SSE3,                                        \
        SIMDPP_DISPATCH_13_NS_ID_SSSE3,                                       \
        SIMDPP_DISPATCH_13_NS_ID_SSE4_1,                                      \
        SIMDPP_DISPATCH_13_NS_ID_POPCNT_INSN,                                 \
        SIMDPP_DISPATCH_13_NS_ID_AVX,                                         \
        SIMDPP_DISPATCH_13_NS_ID_AVX2,                                        \
        SIMDPP_DISPATCH_13_NS_ID_AVX512F,                                     \
        SIMDPP_DISPATCH_13_NS_ID_AVX512BW,                                    \
        SIMDPP_DISPATCH_13_NS_ID_AVX512DQ,                                    \
        SIMDPP_DISPATCH_13_NS_ID_AVX512VL,                                    \
        SIMDPP_DISPATCH_13_NS_ID_FMA3,                                        \
        SIMDPP_DISPATCH_13_NS_ID_FMA4,                                        \
        SIMDPP_DISPATCH_13_NS_ID_XOP,                                         \
        SIMDPP_DISPATCH_13_NS_ID_NEON,                                        \
        SIMDPP_DISPATCH_13_NS_ID_NEON_FLT_SP,                                 \
        SIMDPP_DISPATCH_13_NS_ID_MSA,                                         \
        SIMDPP_DISPATCH_13_NS_ID_ALTIVEC,                                     \
        SIMDPP_DISPATCH_13_NS_ID_VSX_206,                                     \
        SIMDPP_DISPATCH_13_NS_ID_VSX_207)

    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE SIMDPP_DISPATCH_13_NAMESPACE
    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH 1
    #include <simdpp/dispatch/preprocess_single_compile_arch.h>
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE

    #define SIMDPP_DISPATCH_13_FN_REGISTER(ARRAY,NAME,FUN_TYPE)               \
        {   /* the following will fail if the overload is not available */    \
            FUN_TYPE fun_ptr = &SIMDPP_DISPATCH_13_NAMESPACE::SIMDPP_PP_REMOVE_PARENS(NAME);\
            ARRAY[13-1] = ::simdpp::SIMDPP_DISPATCH_13_NAMESPACE::detail::create_fn_version(fun_ptr);\
        }

    #define SIMDPP_DISPATCH_13_FN_DECLARE(SIGNATURE)                          \
        namespace SIMDPP_DISPATCH_13_NAMESPACE {                              \
            SIMDPP_PP_REMOVE_PARENS(SIGNATURE);                               \
        }
    #undef SIMDPP_ARCH_PP_LIST
#else
    #define SIMDPP_DISPATCH_13_FN_REGISTER(ARRAY,NAME,FUN_TYPE)
    #define SIMDPP_DISPATCH_13_FN_DECLARE(SIGNATURE)
#endif

#ifdef SIMDPP_DISPATCH_ARCH14
    #define SIMDPP_ARCH_PP_LIST SIMDPP_DISPATCH_ARCH14
    #include <simdpp/detail/preprocess_single_arch.h>

    // Use the results of preprocess_single_arch.h to define
    // SIMDPP_DISPATCH_14_NAMESPACE

    #if SIMDPP_ARCH_PP_NS_USE_NULL
    #define SIMDPP_DISPATCH_14_NS_ID_NULL SIMDPP_INSN_ID_NULL
    #else
    #define SIMDPP_DISPATCH_14_NS_ID_NULL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE2
    #define SIMDPP_DISPATCH_14_NS_ID_SSE2 SIMDPP_INSN_ID_SSE2
    #else
    #define SIMDPP_DISPATCH_14_NS_ID_SSE2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE3
    #define SIMDPP_DISPATCH_14_NS_ID_SSE3 SIMDPP_INSN_ID_SSE3
    #else
    #define SIMDPP_DISPATCH_14_NS_ID_SSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSSE3
    #define SIMDPP_DISPATCH_14_NS_ID_SSSE3 SIMDPP_INSN_ID_SSSE3
    #else
    #define SIMDPP_DISPATCH_14_NS_ID_SSSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE4_1
    #define SIMDPP_DISPATCH_14_NS_ID_SSE4_1 SIMDPP_INSN_ID_SSE4_1
    #else
    #define SIMDPP_DISPATCH_14_NS_ID_SSE4_1
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_POPCNT_INSN
    #define SIMDPP_DISPATCH_14_NS_ID_POPCNT_INSN SIMDPP_INSN_ID_POPCNT_INSN
    #else
    #define SIMDPP_DISPATCH_14_NS_ID_POPCNT_INSN
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX
    #define SIMDPP_DISPATCH_14_NS_ID_AVX SIMDPP_INSN_ID_AVX
    #else
    #define SIMDPP_DISPATCH_14_NS_ID_AVX
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX2
    #define SIMDPP_DISPATCH_14_NS_ID_AVX2 SIMDPP_INSN_ID_AVX2
    #else
    #define SIMDPP_DISPATCH_14_NS_ID_AVX2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA3
    #define SIMDPP_DISPATCH_14_NS_ID_FMA3 SIMDPP_INSN_ID_FMA3
    #else
    #define SIMDPP_DISPATCH_14_NS_ID_FMA3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA4
    #define SIMDPP_DISPATCH_14_NS_ID_FMA4 SIMDPP_INSN_ID_FMA4
    #else
    #define SIMDPP_DISPATCH_14_NS_ID_FMA4
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_XOP
    #define SIMDPP_DISPATCH_14_NS_ID_XOP SIMDPP_INSN_ID_XOP
    #else
    #define SIMDPP_DISPATCH_14_NS_ID_XOP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512F
    #define SIMDPP_DISPATCH_14_NS_ID_AVX512F SIMDPP_INSN_ID_AVX512F
    #else
    #define SIMDPP_DISPATCH_14_NS_ID_AVX512F
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512BW
    #define SIMDPP_DISPATCH_14_NS_ID_AVX512BW SIMDPP_INSN_ID_AVX512BW
    #else
    #define SIMDPP_DISPATCH_14_NS_ID_AVX512BW
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512DQ
    #define SIMDPP_DISPATCH_14_NS_ID_AVX512DQ SIMDPP_INSN_ID_AVX512DQ
    #else
    #define SIMDPP_DISPATCH_14_NS_ID_AVX512DQ
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512VL
    #define SIMDPP_DISPATCH_14_NS_ID_AVX512VL SIMDPP_INSN_ID_AVX512VL
    #else
    #define SIMDPP_DISPATCH_14_NS_ID_AVX512VL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON
    #define SIMDPP_DISPATCH_14_NS_ID_NEON SIMDPP_INSN_ID_NEON
    #else
    #define SIMDPP_DISPATCH_14_NS_ID_NEON
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON_FLT_SP
    #define SIMDPP_DISPATCH_14_NS_ID_NEON_FLT_SP SIMDPP_INSN_ID_NEON_FLT_SP
    #else
    #define SIMDPP_DISPATCH_14_NS_ID_NEON_FLT_SP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_MSA
    #define SIMDPP_DISPATCH_14_NS_ID_MSA SIMDPP_INSN_ID_MSA
    #else
    #define SIMDPP_DISPATCH_14_NS_ID_MSA
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_ALTIVEC
    #define SIMDPP_DISPATCH_14_NS_ID_ALTIVEC SIMDPP_INSN_ID_ALTIVEC
    #else
    #define SIMDPP_DISPATCH_14_NS_ID_ALTIVEC
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_206
    #define SIMDPP_DISPATCH_14_NS_ID_VSX_206 SIMDPP_INSN_ID_VSX_206
    #else
    #define SIMDPP_DISPATCH_14_NS_ID_VSX_206
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_207
    #define SIMDPP_DISPATCH_14_NS_ID_VSX_207 SIMDPP_INSN_ID_VSX_207
    #else
    #define SIMDPP_DISPATCH_14_NS_ID_VSX_207
    #endif

    #define SIMDPP_DISPATCH_14_NAMESPACE SIMDPP_PP_PASTE22(arch,              \
        SIMDPP_DISPATCH_14_NS_ID_NULL,                                        \
        SIMDPP_DISPATCH_14_NS_ID_SSE2,                                        \
        SIMDPP_DISPATCH_14_NS_ID_SSE3,                                        \
        SIMDPP_DISPATCH_14_NS_ID_SSSE3,                                       \
        SIMDPP_DISPATCH_14_NS_ID_SSE4_1,                                      \
        SIMDPP_DISPATCH_14_NS_ID_POPCNT_INSN,                                 \
        SIMDPP_DISPATCH_14_NS_ID_AVX,                                         \
        SIMDPP_DISPATCH_14_NS_ID_AVX2,                                        \
        SIMDPP_DISPATCH_14_NS_ID_AVX512F,                                     \
        SIMDPP_DISPATCH_14_NS_ID_AVX512BW,                                    \
        SIMDPP_DISPATCH_14_NS_ID_AVX512DQ,                                    \
        SIMDPP_DISPATCH_14_NS_ID_AVX512VL,                                    \
        SIMDPP_DISPATCH_14_NS_ID_FMA3,                                        \
        SIMDPP_DISPATCH_14_NS_ID_FMA4,                                        \
        SIMDPP_DISPATCH_14_NS_ID_XOP,                                         \
        SIMDPP_DISPATCH_14_NS_ID_NEON,                                        \
        SIMDPP_DISPATCH_14_NS_ID_NEON_FLT_SP,                                 \
        SIMDPP_DISPATCH_14_NS_ID_MSA,                                         \
        SIMDPP_DISPATCH_14_NS_ID_ALTIVEC,                                     \
        SIMDPP_DISPATCH_14_NS_ID_VSX_206,                                     \
        SIMDPP_DISPATCH_14_NS_ID_VSX_207)

    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE SIMDPP_DISPATCH_14_NAMESPACE
    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH 1
    #include <simdpp/dispatch/preprocess_single_compile_arch.h>
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE

    #define SIMDPP_DISPATCH_14_FN_REGISTER(ARRAY,NAME,FUN_TYPE)               \
        {   /* the following will fail if the overload is not available */    \
            FUN_TYPE fun_ptr = &SIMDPP_DISPATCH_14_NAMESPACE::SIMDPP_PP_REMOVE_PARENS(NAME);\
            ARRAY[14-1] = ::simdpp::SIMDPP_DISPATCH_14_NAMESPACE::detail::create_fn_version(fun_ptr);\
        }

    #define SIMDPP_DISPATCH_14_FN_DECLARE(SIGNATURE)                          \
        namespace SIMDPP_DISPATCH_14_NAMESPACE {                              \
            SIMDPP_PP_REMOVE_PARENS(SIGNATURE);                               \
        }
    #undef SIMDPP_ARCH_PP_LIST
#else
    #define SIMDPP_DISPATCH_14_FN_REGISTER(ARRAY,NAME,FUN_TYPE)
    #define SIMDPP_DISPATCH_14_FN_DECLARE(SIGNATURE)
#endif

#ifdef SIMDPP_DISPATCH_ARCH15
    #define SIMDPP_ARCH_PP_LIST SIMDPP_DISPATCH_ARCH15
    #include <simdpp/detail/preprocess_single_arch.h>

    // Use the results of preprocess_single_arch.h to define
    // SIMDPP_DISPATCH_15_NAMESPACE

    #if SIMDPP_ARCH_PP_NS_USE_NULL
    #define SIMDPP_DISPATCH_15_NS_ID_NULL SIMDPP_INSN_ID_NULL
    #else
    #define SIMDPP_DISPATCH_15_NS_ID_NULL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE2
    #define SIMDPP_DISPATCH_15_NS_ID_SSE2 SIMDPP_INSN_ID_SSE2
    #else
    #define SIMDPP_DISPATCH_15_NS_ID_SSE2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE3
    #define SIMDPP_DISPATCH_15_NS_ID_SSE3 SIMDPP_INSN_ID_SSE3
    #else
    #define SIMDPP_DISPATCH_15_NS_ID_SSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSSE3
    #define SIMDPP_DISPATCH_15_NS_ID_SSSE3 SIMDPP_INSN_ID_SSSE3
    #else
    #define SIMDPP_DISPATCH_15_NS_ID_SSSE3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_SSE4_1
    #define SIMDPP_DISPATCH_15_NS_ID_SSE4_1 SIMDPP_INSN_ID_SSE4_1
    #else
    #define SIMDPP_DISPATCH_15_NS_ID_SSE4_1
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_POPCNT_INSN
    #define SIMDPP_DISPATCH_15_NS_ID_POPCNT_INSN SIMDPP_INSN_ID_POPCNT_INSN
    #else
    #define SIMDPP_DISPATCH_15_NS_ID_POPCNT_INSN
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX
    #define SIMDPP_DISPATCH_15_NS_ID_AVX SIMDPP_INSN_ID_AVX
    #else
    #define SIMDPP_DISPATCH_15_NS_ID_AVX
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX2
    #define SIMDPP_DISPATCH_15_NS_ID_AVX2 SIMDPP_INSN_ID_AVX2
    #else
    #define SIMDPP_DISPATCH_15_NS_ID_AVX2
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA3
    #define SIMDPP_DISPATCH_15_NS_ID_FMA3 SIMDPP_INSN_ID_FMA3
    #else
    #define SIMDPP_DISPATCH_15_NS_ID_FMA3
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_FMA4
    #define SIMDPP_DISPATCH_15_NS_ID_FMA4 SIMDPP_INSN_ID_FMA4
    #else
    #define SIMDPP_DISPATCH_15_NS_ID_FMA4
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_XOP
    #define SIMDPP_DISPATCH_15_NS_ID_XOP SIMDPP_INSN_ID_XOP
    #else
    #define SIMDPP_DISPATCH_15_NS_ID_XOP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512F
    #define SIMDPP_DISPATCH_15_NS_ID_AVX512F SIMDPP_INSN_ID_AVX512F
    #else
    #define SIMDPP_DISPATCH_15_NS_ID_AVX512F
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512BW
    #define SIMDPP_DISPATCH_15_NS_ID_AVX512BW SIMDPP_INSN_ID_AVX512BW
    #else
    #define SIMDPP_DISPATCH_15_NS_ID_AVX512BW
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512DQ
    #define SIMDPP_DISPATCH_15_NS_ID_AVX512DQ SIMDPP_INSN_ID_AVX512DQ
    #else
    #define SIMDPP_DISPATCH_15_NS_ID_AVX512DQ
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_AVX512VL
    #define SIMDPP_DISPATCH_15_NS_ID_AVX512VL SIMDPP_INSN_ID_AVX512VL
    #else
    #define SIMDPP_DISPATCH_15_NS_ID_AVX512VL
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON
    #define SIMDPP_DISPATCH_15_NS_ID_NEON SIMDPP_INSN_ID_NEON
    #else
    #define SIMDPP_DISPATCH_15_NS_ID_NEON
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_NEON_FLT_SP
    #define SIMDPP_DISPATCH_15_NS_ID_NEON_FLT_SP SIMDPP_INSN_ID_NEON_FLT_SP
    #else
    #define SIMDPP_DISPATCH_15_NS_ID_NEON_FLT_SP
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_MSA
    #define SIMDPP_DISPATCH_15_NS_ID_MSA SIMDPP_INSN_ID_MSA
    #else
    #define SIMDPP_DISPATCH_15_NS_ID_MSA
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_ALTIVEC
    #define SIMDPP_DISPATCH_15_NS_ID_ALTIVEC SIMDPP_INSN_ID_ALTIVEC
    #else
    #define SIMDPP_DISPATCH_15_NS_ID_ALTIVEC
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_206
    #define SIMDPP_DISPATCH_15_NS_ID_VSX_206 SIMDPP_INSN_ID_VSX_206
    #else
    #define SIMDPP_DISPATCH_15_NS_ID_VSX_206
    #endif
    #if SIMDPP_ARCH_PP_NS_USE_VSX_207
    #define SIMDPP_DISPATCH_15_NS_ID_VSX_207 SIMDPP_INSN_ID_VSX_207
    #else
    #define SIMDPP_DISPATCH_15_NS_ID_VSX_207
    #endif

    #define SIMDPP_DISPATCH_15_NAMESPACE SIMDPP_PP_PASTE22(arch,              \
        SIMDPP_DISPATCH_15_NS_ID_NULL,                                        \
        SIMDPP_DISPATCH_15_NS_ID_SSE2,                                        \
        SIMDPP_DISPATCH_15_NS_ID_SSE3,                                        \
        SIMDPP_DISPATCH_15_NS_ID_SSSE3,                                       \
        SIMDPP_DISPATCH_15_NS_ID_SSE4_1,                                      \
        SIMDPP_DISPATCH_15_NS_ID_POPCNT_INSN,                                 \
        SIMDPP_DISPATCH_15_NS_ID_AVX,                                         \
        SIMDPP_DISPATCH_15_NS_ID_AVX2,                                        \
        SIMDPP_DISPATCH_15_NS_ID_AVX512F,                                     \
        SIMDPP_DISPATCH_15_NS_ID_AVX512BW,                                    \
        SIMDPP_DISPATCH_15_NS_ID_AVX512DQ,                                    \
        SIMDPP_DISPATCH_15_NS_ID_AVX512VL,                                    \
        SIMDPP_DISPATCH_15_NS_ID_FMA3,                                        \
        SIMDPP_DISPATCH_15_NS_ID_FMA4,                                        \
        SIMDPP_DISPATCH_15_NS_ID_XOP,                                         \
        SIMDPP_DISPATCH_15_NS_ID_NEON,                                        \
        SIMDPP_DISPATCH_15_NS_ID_NEON_FLT_SP,                                 \
        SIMDPP_DISPATCH_15_NS_ID_MSA,                                         \
        SIMDPP_DISPATCH_15_NS_ID_ALTIVEC,                                     \
        SIMDPP_DISPATCH_15_NS_ID_VSX_206,                                     \
        SIMDPP_DISPATCH_15_NS_ID_VSX_207)

    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE SIMDPP_DISPATCH_15_NAMESPACE
    #define SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH 1
    #include <simdpp/dispatch/preprocess_single_compile_arch.h>
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH
    #undef SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE

    #define SIMDPP_DISPATCH_15_FN_REGISTER(ARRAY,NAME,FUN_TYPE)               \
        {   /* the following will fail if the overload is not available */    \
            FUN_TYPE fun_ptr = &SIMDPP_DISPATCH_15_NAMESPACE::SIMDPP_PP_REMOVE_PARENS(NAME);\
            ARRAY[15-1] = ::simdpp::SIMDPP_DISPATCH_15_NAMESPACE::detail::create_fn_version(fun_ptr);\
        }

    #define SIMDPP_DISPATCH_15_FN_DECLARE(SIGNATURE)                          \
        namespace SIMDPP_DISPATCH_15_NAMESPACE {                              \
            SIMDPP_PP_REMOVE_PARENS(SIGNATURE);                               \
        }
    #undef SIMDPP_ARCH_PP_LIST
#else
    #define SIMDPP_DISPATCH_15_FN_REGISTER(ARRAY,NAME,FUN_TYPE)
    #define SIMDPP_DISPATCH_15_FN_DECLARE(SIGNATURE)
#endif


#define SIMDPP_DISPATCH_DECLARE_FUNCTIONS(SIGNATURE)                        \
        SIMDPP_DISPATCH_1_FN_DECLARE(SIGNATURE)                               \
        SIMDPP_DISPATCH_2_FN_DECLARE(SIGNATURE)                               \
        SIMDPP_DISPATCH_3_FN_DECLARE(SIGNATURE)                               \
        SIMDPP_DISPATCH_4_FN_DECLARE(SIGNATURE)                               \
        SIMDPP_DISPATCH_5_FN_DECLARE(SIGNATURE)                               \
        SIMDPP_DISPATCH_6_FN_DECLARE(SIGNATURE)                               \
        SIMDPP_DISPATCH_7_FN_DECLARE(SIGNATURE)                               \
        SIMDPP_DISPATCH_8_FN_DECLARE(SIGNATURE)                               \
        SIMDPP_DISPATCH_9_FN_DECLARE(SIGNATURE)                               \
        SIMDPP_DISPATCH_10_FN_DECLARE(SIGNATURE)                              \
        SIMDPP_DISPATCH_11_FN_DECLARE(SIGNATURE)                              \
        SIMDPP_DISPATCH_12_FN_DECLARE(SIGNATURE)                              \
        SIMDPP_DISPATCH_13_FN_DECLARE(SIGNATURE)                              \
        SIMDPP_DISPATCH_14_FN_DECLARE(SIGNATURE)                              \
        SIMDPP_DISPATCH_15_FN_DECLARE(SIGNATURE)                              \


#define SIMDPP_DISPATCH_COLLECT_FUNCTIONS(ARRAY,NAME,FUN_TYPE)              \
    SIMDPP_DISPATCH_1_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                        \
    SIMDPP_DISPATCH_2_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                        \
    SIMDPP_DISPATCH_3_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                        \
    SIMDPP_DISPATCH_4_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                        \
    SIMDPP_DISPATCH_5_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                        \
    SIMDPP_DISPATCH_6_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                        \
    SIMDPP_DISPATCH_7_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                        \
    SIMDPP_DISPATCH_8_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                        \
    SIMDPP_DISPATCH_9_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                        \
    SIMDPP_DISPATCH_10_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                       \
    SIMDPP_DISPATCH_11_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                       \
    SIMDPP_DISPATCH_12_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                       \
    SIMDPP_DISPATCH_13_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                       \
    SIMDPP_DISPATCH_14_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                       \
    SIMDPP_DISPATCH_15_FN_REGISTER(ARRAY,NAME,FUN_TYPE)                       \


#endif // SIMDPP_EMIT_DISPATCHER
#endif

