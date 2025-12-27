/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#include <simdpp/detail/preprocessor/stringize.hpp>
#include <simdpp/dispatch/arch.h>
#include <simdpp/dispatch/dispatcher.h>

/*  This file setups this_compile_arch() and create_fn_version() functions
    for a particular architecture.

    The file may be included into other files multiple times, it has been
    extracted to reduce code duplication.

    The file depends on preprocess_single_arch.h being included beforehand and
    its output macros not being overridden yet. Additionally, it depends on
    SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE being defined to appropriate
    namespace for the architecture.

    The file also depends on the definition of
    SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH. If it is defined to 0, only
    this_compile_arch() is available as
    simdpp::SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE::this_compile_arch().
    If the macro is defined to nonzero, then both this_compile_arch() and
    create_fn_version() are available at
    simdpp::SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE::this_compile_arch() and
    simdpp::SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE::create_fn_version().

    This separation is required because the current architecture and the
    set of dispatched architectures (if enabled) are defined separately, and
    it's hard to figure out whether there's any overlap. If it is, then we need
    to make sure that all functions are defined only once in each namespace.
*/
namespace simdpp {
namespace SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE {

#if SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH
namespace detail {
#endif

/** Returns the instruction set flags that will be required by the currently
    compiled code
*/
static inline Arch this_compile_arch()
{
    Arch res = Arch::NONE_NULL;
#if SIMDPP_ARCH_PP_USE_SSE2
    res |= Arch::X86_SSE2;
#endif
#if SIMDPP_ARCH_PP_USE_SSE3
    res |= Arch::X86_SSE3;
#endif
#if SIMDPP_ARCH_PP_USE_SSSE3
    res |= Arch::X86_SSSE3;
#endif
#if SIMDPP_ARCH_PP_USE_SSE4_1
    res |= Arch::X86_SSE4_1;
#endif
#if SIMDPP_ARCH_PP_USE_X86_POPCNT_INSN
    res |= Arch::X86_POPCNT_INSN;
#endif
#if SIMDPP_ARCH_PP_USE_AVX
    res |= Arch::X86_AVX;
#endif
#if SIMDPP_ARCH_PP_USE_AVX2
    res |= Arch::X86_AVX2;
#endif
#if SIMDPP_ARCH_PP_USE_FMA3
    res |= Arch::X86_FMA3;
#endif
#if SIMDPP_ARCH_PP_USE_FMA4
    res |= Arch::X86_FMA4;
#endif
#if SIMDPP_ARCH_PP_USE_XOP
    res |= Arch::X86_XOP;
#endif
#if SIMDPP_ARCH_PP_USE_AVX512F
    res |= Arch::X86_AVX512F;
#endif
#if SIMDPP_ARCH_PP_USE_AVX512BW
    res |= Arch::X86_AVX512BW;
#endif
#if SIMDPP_ARCH_PP_USE_AVX512DQ
    res |= Arch::X86_AVX512DQ;
#endif
#if SIMDPP_ARCH_PP_USE_AVX512VL
    res |= Arch::X86_AVX512VL;
#endif
#if SIMDPP_ARCH_PP_USE_NEON
    res |= Arch::ARM_NEON;
#endif
#if SIMDPP_ARCH_PP_USE_NEON_FLT_SP
    res |= Arch::ARM_NEON_FLT_SP;
#endif
#if SIMDPP_ARCH_PP_USE_ALTIVEC
    res |= Arch::POWER_ALTIVEC;
#endif
#if SIMDPP_ARCH_PP_USE_VSX_206
    res |= Arch::POWER_VSX_206;
#endif
#if SIMDPP_ARCH_PP_USE_VSX_207
    res |= Arch::POWER_VSX_207;
#endif
#if SIMDPP_ARCH_PP_USE_MSA
    res |= Arch::MIPS_MSA;
#endif
    return res;
}

#if SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_FOR_DISPATCH

template<class FunPtr>
static inline simdpp::detail::FnVersion create_fn_version(FunPtr fun_ptr)
{
    simdpp::detail::FnVersion res;
    res.needed_arch = simdpp::SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE::detail::this_compile_arch();
    res.fun_ptr = reinterpret_cast<simdpp::detail::VoidFunPtr>(fun_ptr);
    res.arch_name = SIMDPP_PP_STRINGIZE(SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE);
    return res;
}

} // namespace detail
#endif

} // namespace SIMDPP_ARCH_PP_THIS_COMPILE_ARCH_NAMESPACE
} // namespace simdpp

// this file may be included several times; thus no include guard present
