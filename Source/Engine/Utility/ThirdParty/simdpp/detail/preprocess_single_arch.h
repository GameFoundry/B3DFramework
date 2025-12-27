/*  Copyright (C) 2015  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

/*  This file contains macros that set up information for single architecture
    definition. It's here to reduce duplication. Other files supply appropriate
    definitions of certain input macros and collect the output from the macros
    defined in this file.

    Note that in C++ it's not possible to substitute the nested macros in
    replacement list of a #define at the point of definition of said #define.
    All nested macros will be substituted to the current values at each site of
    use. The consequence is that the resulting macros of this file may only be
    evaluated by #if elsewhere, otherwise the last inclusion of the file will
    effectively overwrite the values.

    The input data may be supplied via two ways:
    1)  If SIMDPP_ARCH_PP_LIST is not defined, then the instruction sets should
        be supplied via SIMDPP_ARCH_* macros. If particular macro expands to
        non-zero value then the given instruction set and any subsets are
        enabled. After processing the information supplied via SIMDPP_ARCH_*
        this file undefines all these macros.

    2)  If SIMDPP_ARCH_PP_LIST is defined, then it supplies a list of enabled
        instruction sets. The macro should be defined to a comma-separated list
        of identifiers, each equivalent to one of SIMDPP_ARCH_* macro names.

    Note that if SIMDPP_ARCH_PP_LIST is defined, then all SIMDPP_ARCH_* macros
    themselves must not be defined, as they cause unintended expansions within
    tihs file.

    The output data:

    SIMDPP_ARCH_PP_USE_* - defined depending on whether the particular
        instructions are enabled.

    SIMDPP_ARCH_PP_NS_USE_* - defined depending on whether the particular
        instructions set id should be included into SIMDPP_ARCH_NAMESPACE
        definition. The difference from SIMDPP_ARCH_PP_USE_* is that some
        instruction sets are subsets of other instruction sets, so it makes
        no sense to make the namespace longer. E.g. we use arch_ssse3 instead of
        arch_sse2_sse3_ssse3.
*/

#include <simdpp/detail/preprocessor.h>
#include <simdpp/detail/preprocessor/cat.hpp>
#include <simdpp/detail/preprocessor/facilities/expand.hpp>
#include <simdpp/detail/insn_id.h>

// Undef output macros
#ifdef SIMDPP_ARCH_PP_ARCH_ID
#undef SIMDPP_ARCH_PP_ARCH_ID
#endif

#ifdef SIMDPP_ARCH_PP_USE_NULL
#undef SIMDPP_ARCH_PP_USE_NULL
#endif
#ifdef SIMDPP_ARCH_PP_USE_SSE2
#undef SIMDPP_ARCH_PP_USE_SSE2
#endif
#ifdef SIMDPP_ARCH_PP_USE_SSE3
#undef SIMDPP_ARCH_PP_USE_SSE3
#endif
#ifdef SIMDPP_ARCH_PP_USE_SSSE3
#undef SIMDPP_ARCH_PP_USE_SSSE3
#endif
#ifdef SIMDPP_ARCH_PP_USE_SSE4_1
#undef SIMDPP_ARCH_PP_USE_SSE4_1
#endif
#ifdef SIMDPP_ARCH_PP_USE_X86_POPCNT_INSN
#undef SIMDPP_ARCH_PP_USE_X86_POPCNT_INSN
#endif
#ifdef SIMDPP_ARCH_PP_USE_AVX
#undef SIMDPP_ARCH_PP_USE_AVX
#endif
#ifdef SIMDPP_ARCH_PP_USE_AVX2
#undef SIMDPP_ARCH_PP_USE_AVX2
#endif
#ifdef SIMDPP_ARCH_PP_USE_AVX512F
#undef SIMDPP_ARCH_PP_USE_AVX512F
#endif
#ifdef SIMDPP_ARCH_PP_USE_AVX512BW
#undef SIMDPP_ARCH_PP_USE_AVX512BW
#endif
#ifdef SIMDPP_ARCH_PP_USE_AVX512DQ
#undef SIMDPP_ARCH_PP_USE_AVX512DQ
#endif
#ifdef SIMDPP_ARCH_PP_USE_AVX512VL
#undef SIMDPP_ARCH_PP_USE_AVX512VL
#endif
#ifdef SIMDPP_ARCH_PP_USE_FMA3
#undef SIMDPP_ARCH_PP_USE_FMA3
#endif
#ifdef SIMDPP_ARCH_PP_USE_FMA4
#undef SIMDPP_ARCH_PP_USE_FMA4
#endif
#ifdef SIMDPP_ARCH_PP_USE_XOP
#undef SIMDPP_ARCH_PP_USE_XOP
#endif
#ifdef SIMDPP_ARCH_PP_USE_NEON
#undef SIMDPP_ARCH_PP_USE_NEON
#endif
#ifdef SIMDPP_ARCH_PP_USE_NEON_FLT_SP
#undef SIMDPP_ARCH_PP_USE_NEON_FLT_SP
#endif
#ifdef SIMDPP_ARCH_PP_USE_ALTIVEC
#undef SIMDPP_ARCH_PP_USE_ALTIVEC
#endif
#ifdef SIMDPP_ARCH_PP_USE_VSX_206
#undef SIMDPP_ARCH_PP_USE_VSX_206
#endif
#ifdef SIMDPP_ARCH_PP_USE_VSX_207
#undef SIMDPP_ARCH_PP_USE_VSX_207
#endif
#ifdef SIMDPP_ARCH_PP_USE_MSA
#undef SIMDPP_ARCH_PP_USE_MSA
#endif

#ifdef SIMDPP_ARCH_PP_NS_USE_NULL
#undef SIMDPP_ARCH_PP_NS_USE_NULL
#endif
#ifdef SIMDPP_ARCH_PP_NS_USE_SSE2
#undef SIMDPP_ARCH_PP_NS_USE_SSE2
#endif
#ifdef SIMDPP_ARCH_PP_NS_USE_SSE3
#undef SIMDPP_ARCH_PP_NS_USE_SSE3
#endif
#ifdef SIMDPP_ARCH_PP_NS_USE_SSSE3
#undef SIMDPP_ARCH_PP_NS_USE_SSSE3
#endif
#ifdef SIMDPP_ARCH_PP_NS_USE_SSE4_1
#undef SIMDPP_ARCH_PP_NS_USE_SSE4_1
#endif
#ifdef SIMDPP_ARCH_PP_NS_USE_POPCNT_INSN
#undef SIMDPP_ARCH_PP_NS_USE_POPCNT_INSN
#endif
#ifdef SIMDPP_ARCH_PP_NS_USE_AVX
#undef SIMDPP_ARCH_PP_NS_USE_AVX
#endif
#ifdef SIMDPP_ARCH_PP_NS_USE_AVX2
#undef SIMDPP_ARCH_PP_NS_USE_AVX2
#endif
#ifdef SIMDPP_ARCH_PP_NS_USE_AVX512F
#undef SIMDPP_ARCH_PP_NS_USE_AVX512F
#endif
#ifdef SIMDPP_ARCH_PP_NS_USE_AVX512BW
#undef SIMDPP_ARCH_PP_NS_USE_AVX512BW
#endif
#ifdef SIMDPP_ARCH_PP_NS_USE_AVX512DQ
#undef SIMDPP_ARCH_PP_NS_USE_AVX512DQ
#endif
#ifdef SIMDPP_ARCH_PP_NS_USE_AVX512VL
#undef SIMDPP_ARCH_PP_NS_USE_AVX512VL
#endif
#ifdef SIMDPP_ARCH_PP_NS_USE_FMA3
#undef SIMDPP_ARCH_PP_NS_USE_FMA3
#endif
#ifdef SIMDPP_ARCH_PP_NS_USE_FMA4
#undef SIMDPP_ARCH_PP_NS_USE_FMA4
#endif
#ifdef SIMDPP_ARCH_PP_NS_USE_XOP
#undef SIMDPP_ARCH_PP_NS_USE_XOP
#endif
#ifdef SIMDPP_ARCH_PP_NS_USE_NEON
#undef SIMDPP_ARCH_PP_NS_USE_NEON
#endif
#ifdef SIMDPP_ARCH_PP_NS_USE_NEON_FLT_SP
#undef SIMDPP_ARCH_PP_NS_USE_NEON_FLT_SP
#endif
#ifdef SIMDPP_ARCH_PP_NS_USE_ALTIVEC
#undef SIMDPP_ARCH_PP_NS_USE_ALTIVEC
#endif
#ifdef SIMDPP_ARCH_PP_NS_USE_VSX_206
#undef SIMDPP_ARCH_PP_NS_USE_VSX_206
#endif
#ifdef SIMDPP_ARCH_PP_NS_USE_VSX_207
#undef SIMDPP_ARCH_PP_NS_USE_VSX_207
#endif
#ifdef SIMDPP_ARCH_PP_NS_USE_MSA
#undef SIMDPP_ARCH_PP_NS_USE_MSA
#endif

#ifdef SIMDPP_ARCH_PP_LIST

    #define SIMDPP_ARCH_CREATE_MASK2(x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,...)    \
        SIMDPP_PP_CAT(SIMDPP_PREFIX_, x0) | \
        SIMDPP_PP_CAT(SIMDPP_PREFIX_, x1) | \
        SIMDPP_PP_CAT(SIMDPP_PREFIX_, x2) | \
        SIMDPP_PP_CAT(SIMDPP_PREFIX_, x3) | \
        SIMDPP_PP_CAT(SIMDPP_PREFIX_, x4) | \
        SIMDPP_PP_CAT(SIMDPP_PREFIX_, x5) | \
        SIMDPP_PP_CAT(SIMDPP_PREFIX_, x6) | \
        SIMDPP_PP_CAT(SIMDPP_PREFIX_, x7) | \
        SIMDPP_PP_CAT(SIMDPP_PREFIX_, x8) | \
        SIMDPP_PP_CAT(SIMDPP_PREFIX_, x9) | \
        SIMDPP_PP_CAT(SIMDPP_PREFIX_, x10) | \
        SIMDPP_PP_CAT(SIMDPP_PREFIX_, x11) | \
        SIMDPP_PP_CAT(SIMDPP_PREFIX_, x12) | \
        SIMDPP_PP_CAT(SIMDPP_PREFIX_, x13)

    #define SIMDPP_ARCH_PP_CREATE_MASK(...) SIMDPP_PP_EXPAND( SIMDPP_ARCH_CREATE_MASK2(__VA_ARGS__,0,0,0,0,0,0,0,0,0,0,0,0,0,0) )
    #define SIMDPP_ARCH_PP_MASK SIMDPP_ARCH_PP_CREATE_MASK(SIMDPP_ARCH_PP_LIST)

    #if ((SIMDPP_ARCH_PP_MASK) & SIMDPP_INSN_MASK_SSE2) == SIMDPP_INSN_MASK_SSE2
        #define SIMDPP_ARCH_PP_USE_SSE2 1
    #endif
    #if ((SIMDPP_ARCH_PP_MASK) & SIMDPP_INSN_MASK_SSE3) == SIMDPP_INSN_MASK_SSE3
        #define SIMDPP_ARCH_PP_USE_SSE3 1
    #endif
    #if ((SIMDPP_ARCH_PP_MASK) & SIMDPP_INSN_MASK_SSSE3) == SIMDPP_INSN_MASK_SSSE3
        #define SIMDPP_ARCH_PP_USE_SSSE3 1
    #endif
    #if ((SIMDPP_ARCH_PP_MASK) & SIMDPP_INSN_MASK_SSE4_1) == SIMDPP_INSN_MASK_SSE4_1
        #define SIMDPP_ARCH_PP_USE_SSE4_1 1
    #endif
    #if ((SIMDPP_ARCH_PP_MASK) & SIMDPP_INSN_MASK_POPCNT_INSN) == SIMDPP_INSN_MASK_POPCNT_INSN
        #define SIMDPP_ARCH_PP_USE_X86_POPCNT_INSN 1
    #endif
    #if ((SIMDPP_ARCH_PP_MASK) & SIMDPP_INSN_MASK_AVX) == SIMDPP_INSN_MASK_AVX
        #define SIMDPP_ARCH_PP_USE_AVX 1
    #endif
    #if ((SIMDPP_ARCH_PP_MASK) & SIMDPP_INSN_MASK_AVX2) == SIMDPP_INSN_MASK_AVX2
        #define SIMDPP_ARCH_PP_USE_AVX2 1
    #endif
    #if ((SIMDPP_ARCH_PP_MASK) & SIMDPP_INSN_MASK_FMA3) == SIMDPP_INSN_MASK_FMA3
        #define SIMDPP_ARCH_PP_USE_FMA3 1
    #endif
    #if ((SIMDPP_ARCH_PP_MASK) & SIMDPP_INSN_MASK_FMA4) == SIMDPP_INSN_MASK_FMA4
        #define SIMDPP_ARCH_PP_USE_FMA4 1
    #endif
    #if ((SIMDPP_ARCH_PP_MASK) & SIMDPP_INSN_MASK_XOP) == SIMDPP_INSN_MASK_XOP
        #define SIMDPP_ARCH_PP_USE_XOP 1
    #endif
    #if ((SIMDPP_ARCH_PP_MASK) & SIMDPP_INSN_MASK_AVX512F) == SIMDPP_INSN_MASK_AVX512F
        #define SIMDPP_ARCH_PP_USE_AVX512F 1
    #endif
    #if ((SIMDPP_ARCH_PP_MASK) & SIMDPP_INSN_MASK_AVX512BW) == SIMDPP_INSN_MASK_AVX512BW
        #define SIMDPP_ARCH_PP_USE_AVX512BW 1
    #endif
    #if ((SIMDPP_ARCH_PP_MASK) & SIMDPP_INSN_MASK_AVX512DQ) == SIMDPP_INSN_MASK_AVX512DQ
        #define SIMDPP_ARCH_PP_USE_AVX512DQ 1
    #endif
    #if ((SIMDPP_ARCH_PP_MASK) & SIMDPP_INSN_MASK_AVX512VL) == SIMDPP_INSN_MASK_AVX512VL
        #define SIMDPP_ARCH_PP_USE_AVX512VL 1
    #endif
    #if ((SIMDPP_ARCH_PP_MASK) & SIMDPP_INSN_MASK_NEON) == SIMDPP_INSN_MASK_NEON
        #define SIMDPP_ARCH_PP_USE_NEON 1
    #endif
    #if ((SIMDPP_ARCH_PP_MASK) & SIMDPP_INSN_MASK_NEON_FLT_SP) == SIMDPP_INSN_MASK_NEON_FLT_SP
        #define SIMDPP_ARCH_PP_USE_NEON_FLT_SP 1
    #endif
    #if ((SIMDPP_ARCH_PP_MASK) & SIMDPP_INSN_MASK_ALTIVEC) == SIMDPP_INSN_MASK_ALTIVEC
        #define SIMDPP_ARCH_PP_USE_ALTIVEC 1
    #endif
    #if ((SIMDPP_ARCH_PP_MASK) & SIMDPP_INSN_MASK_VSX_206) == SIMDPP_INSN_MASK_VSX_206
        #define SIMDPP_ARCH_PP_USE_VSX_206 1
    #endif
    #if ((SIMDPP_ARCH_PP_MASK) & SIMDPP_INSN_MASK_VSX_207) == SIMDPP_INSN_MASK_VSX_207
        #define SIMDPP_ARCH_PP_USE_VSX_207 1
    #endif
    #if ((SIMDPP_ARCH_PP_MASK) & SIMDPP_INSN_MASK_MSA) == SIMDPP_INSN_MASK_MSA
        #define SIMDPP_ARCH_PP_USE_MSA 1
    #endif
    #undef SIMDPP_ARCH_PP_MASK
    #undef SIMDPP_ARCH_CREATE_MASK2
    #undef SIMDPP_ARCH_CREATE_MASK
#else

    #ifdef SIMDPP_ARCH_X86_SSE2
        #define SIMDPP_ARCH_PP_USE_SSE2 1
        #undef SIMDPP_ARCH_X86_SSE2
    #endif
    #ifdef SIMDPP_ARCH_X86_SSE3
        #define SIMDPP_ARCH_PP_USE_SSE3 1
        #undef SIMDPP_ARCH_X86_SSE3
    #endif
    #ifdef SIMDPP_ARCH_X86_SSSE3
        #define SIMDPP_ARCH_PP_USE_SSSE3 1
        #undef SIMDPP_ARCH_X86_SSSE3
    #endif
    #ifdef SIMDPP_ARCH_X86_SSE4_1
        #define SIMDPP_ARCH_PP_USE_SSE4_1 1
        #undef SIMDPP_ARCH_X86_SSE4_1
    #endif
    #ifdef SIMDPP_ARCH_X86_POPCNT_INSN
        #define SIMDPP_ARCH_PP_USE_X86_POPCNT_INSN 1
        #undef SIMDPP_ARCH_X86_POPCNT_INSN
    #endif
    #ifdef SIMDPP_ARCH_X86_AVX
        #define SIMDPP_ARCH_PP_USE_AVX 1
        #undef SIMDPP_ARCH_X86_AVX
    #endif
    #ifdef SIMDPP_ARCH_X86_AVX2
        #define SIMDPP_ARCH_PP_USE_AVX2 1
        #undef SIMDPP_ARCH_X86_AVX2
    #endif
    #ifdef SIMDPP_ARCH_X86_FMA3
        #define SIMDPP_ARCH_PP_USE_FMA3 1
        #undef SIMDPP_ARCH_X86_FMA3
    #endif
    #ifdef SIMDPP_ARCH_X86_FMA4
        #define SIMDPP_ARCH_PP_USE_FMA4 1
        #undef SIMDPP_ARCH_X86_FMA4
    #endif
    #ifdef SIMDPP_ARCH_X86_XOP
        #define SIMDPP_ARCH_PP_USE_XOP 1
        #undef SIMDPP_ARCH_X86_XOP
    #endif
    #ifdef SIMDPP_ARCH_X86_AVX512F
        #define SIMDPP_ARCH_PP_USE_AVX512F 1
        #undef SIMDPP_ARCH_X86_AVX512F
    #endif
    #ifdef SIMDPP_ARCH_X86_AVX512BW
        #define SIMDPP_ARCH_PP_USE_AVX512BW 1
        #undef SIMDPP_ARCH_X86_AVX512BW
    #endif
    #ifdef SIMDPP_ARCH_X86_AVX512DQ
        #define SIMDPP_ARCH_PP_USE_AVX512DQ 1
        #undef SIMDPP_ARCH_X86_AVX512DQ
    #endif
    #ifdef SIMDPP_ARCH_X86_AVX512VL
        #define SIMDPP_ARCH_PP_USE_AVX512VL 1
        #undef SIMDPP_ARCH_X86_AVX512VL
    #endif
    #ifdef SIMDPP_ARCH_ARM_NEON
        #define SIMDPP_ARCH_PP_USE_NEON 1
        #undef SIMDPP_ARCH_ARM_NEON
    #endif
    #ifdef SIMDPP_ARCH_ARM_NEON_FLT_SP
        #define SIMDPP_ARCH_PP_USE_NEON_FLT_SP 1
        #undef SIMDPP_ARCH_ARM_NEON_FLT_SP
    #endif
    #ifdef SIMDPP_ARCH_POWER_ALTIVEC
        #define SIMDPP_ARCH_PP_USE_ALTIVEC 1
        #undef SIMDPP_ARCH_POWER_ALTIVEC
    #endif
    #ifdef SIMDPP_ARCH_POWER_VSX_206
        #define SIMDPP_ARCH_PP_USE_VSX_206 1
        #undef SIMDPP_ARCH_POWER_VSX_206
    #endif
    #ifdef SIMDPP_ARCH_POWER_VSX_207
        #define SIMDPP_ARCH_PP_USE_VSX_207 1
        #undef SIMDPP_ARCH_POWER_VSX_207
    #endif
    #ifdef SIMDPP_ARCH_MIPS_MSA
        #define SIMDPP_ARCH_PP_USE_MSA 1
        #undef SIMDPP_ARCH_MIPS_MSA
    #endif
#endif

// Define support of instruction sets that are implicitly available when another
// instruction set is available
#if SIMDPP_ARCH_PP_USE_AVX512VL
    #ifndef SIMDPP_ARCH_PP_USE_AVX512F
        #define SIMDPP_ARCH_PP_USE_AVX512F 1
    #endif
#endif

#if SIMDPP_ARCH_PP_USE_AVX512DQ
    #ifndef SIMDPP_ARCH_PP_USE_AVX512F
        #define SIMDPP_ARCH_PP_USE_AVX512F 1
    #endif
#endif

#if SIMDPP_ARCH_PP_USE_AVX512BW
    #ifndef SIMDPP_ARCH_PP_USE_AVX512F
        #define SIMDPP_ARCH_PP_USE_AVX512F 1
    #endif
#endif

#if SIMDPP_ARCH_PP_USE_AVX512F
    #ifndef SIMDPP_ARCH_PP_USE_AVX2
        #define SIMDPP_ARCH_PP_USE_AVX2 1
    #endif
#endif

#if SIMDPP_ARCH_PP_USE_AVX2
    #ifndef SIMDPP_ARCH_PP_USE_AVX
        #define SIMDPP_ARCH_PP_USE_AVX 1
    #endif
#endif

#if SIMDPP_ARCH_PP_USE_AVX
    #ifndef SIMDPP_ARCH_PP_USE_SSE4_1
        #define SIMDPP_ARCH_PP_USE_SSE4_1 1
    #endif
#endif

#if SIMDPP_ARCH_PP_USE_FMA3
    #ifndef SIMDPP_ARCH_PP_USE_SSE3
        #define SIMDPP_ARCH_PP_USE_SSE3 1
    #endif
#endif

#if SIMDPP_ARCH_PP_USE_FMA4
    #ifndef SIMDPP_ARCH_PP_USE_SSE3
        #define SIMDPP_ARCH_PP_USE_SSE3 1
    #endif
#endif

#if SIMDPP_ARCH_PP_USE_XOP
    #ifndef SIMDPP_ARCH_PP_USE_SSE3
        #define SIMDPP_ARCH_PP_USE_SSE3 1
    #endif
#endif

#if SIMDPP_ARCH_PP_USE_SSE4_1
    #ifndef SIMDPP_ARCH_PP_USE_SSSE3
        #define SIMDPP_ARCH_PP_USE_SSSE3 1
    #endif
#endif

#if SIMDPP_ARCH_PP_USE_X86_POPCNT_INSN
    #ifndef SIMDPP_ARCH_PP_USE_SSSE3
        #define SIMDPP_ARCH_PP_USE_SSSE3 1
    #endif
#endif

#if SIMDPP_ARCH_PP_USE_SSSE3
    #ifndef SIMDPP_ARCH_PP_USE_SSE3
        #define SIMDPP_ARCH_PP_USE_SSE3 1
    #endif
#endif

#if SIMDPP_ARCH_PP_USE_SSE3
    #ifndef SIMDPP_ARCH_PP_USE_SSE2
        #define SIMDPP_ARCH_PP_USE_SSE2 1
    #endif
#endif

#if SIMDPP_ARCH_PP_USE_NEON_FLT_SP
    #ifndef SIMDPP_ARCH_PP_USE_NEON
        #define SIMDPP_ARCH_PP_USE_NEON 1
    #endif
#endif

#if SIMDPP_ARCH_PP_USE_VSX_207
    #ifndef SIMDPP_ARCH_PP_USE_VSX_206
        #define SIMDPP_ARCH_PP_USE_VSX_206 1
    #endif
#endif

#if SIMDPP_ARCH_PP_USE_VSX_206
    #ifndef SIMDPP_ARCH_PP_USE_ALTIVEC
        #define SIMDPP_ARCH_PP_USE_ALTIVEC 1
    #endif
#endif

#if !(SIMDPP_ARCH_PP_USE_SSE2 || SIMDPP_ARCH_PP_USE_NEON || \
      SIMDPP_ARCH_PP_USE_NEON_FLT_SP || SIMDPP_ARCH_PP_USE_ALTIVEC || SIMDPP_ARCH_PP_USE_MSA)
#define SIMDPP_ARCH_PP_USE_NULL 1
#endif

#if SIMDPP_ARCH_PP_USE_FMA3 && SIMDPP_ARCH_PP_USE_FMA4
    #error "FMA3 and FMA4 can't be used together"
#endif

// To limit the length of the SIMDPP_ARCH_NAMESPACE and friends, we omit
// instruction sets that are subsets of other enabled instruction sets. For
// example, for SSE3 we generate arch_sse3 instead of arch_sse2_sse3.

#if SIMDPP_ARCH_PP_USE_NULL
#define SIMDPP_ARCH_PP_NS_USE_NULL 1
#endif
#if SIMDPP_ARCH_PP_USE_SSE2 && !SIMDPP_ARCH_PP_USE_SSE3
#define SIMDPP_ARCH_PP_NS_USE_SSE2 1
#endif
#if SIMDPP_ARCH_PP_USE_SSE3 && !SIMDPP_ARCH_PP_USE_SSSE3 && !SIMDPP_ARCH_PP_USE_XOP && !SIMDPP_ARCH_PP_USE_FMA3 && !SIMDPP_ARCH_PP_USE_FMA4
#define SIMDPP_ARCH_PP_NS_USE_SSE3 1
#endif
#if SIMDPP_ARCH_PP_USE_SSSE3 && !SIMDPP_ARCH_PP_USE_SSE4_1
#define SIMDPP_ARCH_PP_NS_USE_SSSE3 1
#endif
#if SIMDPP_ARCH_PP_USE_SSE4_1 && !SIMDPP_ARCH_PP_USE_AVX
#define SIMDPP_ARCH_PP_NS_USE_SSE4_1 1
#endif
#if SIMDPP_ARCH_PP_USE_X86_POPCNT_INSN
#define SIMDPP_ARCH_PP_NS_USE_POPCNT_INSN 1
#endif
#if SIMDPP_ARCH_PP_USE_AVX && !SIMDPP_ARCH_PP_USE_AVX2
#define SIMDPP_ARCH_PP_NS_USE_AVX 1
#endif
#if SIMDPP_ARCH_PP_USE_AVX2 && !SIMDPP_ARCH_PP_USE_AVX512F
#define SIMDPP_ARCH_PP_NS_USE_AVX2 1
#endif
#if SIMDPP_ARCH_PP_USE_AVX512F && !SIMDPP_ARCH_PP_USE_AVX512BW && !SIMDPP_ARCH_PP_USE_AVX512DQ && !SIMDPP_ARCH_PP_USE_AVX512VL
#define SIMDPP_ARCH_PP_NS_USE_AVX512F 1
#endif
#if SIMDPP_ARCH_PP_USE_AVX512BW
#define SIMDPP_ARCH_PP_NS_USE_AVX512BW 1
#endif
#if SIMDPP_ARCH_PP_USE_AVX512DQ
#define SIMDPP_ARCH_PP_NS_USE_AVX512DQ 1
#endif
#if SIMDPP_ARCH_PP_USE_AVX512VL
#define SIMDPP_ARCH_PP_NS_USE_AVX512VL 1
#endif
#if SIMDPP_ARCH_PP_USE_FMA3
#define SIMDPP_ARCH_PP_NS_USE_FMA3 1
#endif
#if SIMDPP_ARCH_PP_USE_FMA4
#define SIMDPP_ARCH_PP_NS_USE_FMA4 1
#endif
#if SIMDPP_ARCH_PP_USE_XOP
#define SIMDPP_ARCH_PP_NS_USE_XOP 1
#endif
#if SIMDPP_ARCH_PP_USE_NEON && !SIMDPP_ARCH_PP_USE_NEON_FLT_SP
#define SIMDPP_ARCH_PP_NS_USE_NEON 1
#endif
#if SIMDPP_ARCH_PP_USE_NEON_FLT_SP
#define SIMDPP_ARCH_PP_NS_USE_NEON_FLT_SP 1
#endif
#if SIMDPP_ARCH_PP_USE_ALTIVEC && !SIMDPP_ARCH_PP_USE_VSX_206
#define SIMDPP_ARCH_PP_NS_USE_ALTIVEC 1
#endif
#if SIMDPP_ARCH_PP_USE_VSX_206 && !SIMDPP_ARCH_PP_USE_VSX_207
#define SIMDPP_ARCH_PP_NS_USE_VSX_206 1
#endif
#if SIMDPP_ARCH_PP_USE_VSX_207
#define SIMDPP_ARCH_PP_NS_USE_VSX_207 1
#endif
#if SIMDPP_ARCH_PP_USE_MSA
#define SIMDPP_ARCH_PP_NS_USE_MSA 1
#endif
