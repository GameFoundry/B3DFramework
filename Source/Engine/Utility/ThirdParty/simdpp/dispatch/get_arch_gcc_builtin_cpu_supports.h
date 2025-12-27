/*  Copyright (C) 2013  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_DISPATCH_GET_ARCH_GCC_BUILTIN_CPU_SUPPORTS_H
#define LIBSIMDPP_DISPATCH_GET_ARCH_GCC_BUILTIN_CPU_SUPPORTS_H

#if ((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 8)) && (__i386__ || __amd64__)
#define SIMDPP_HAS_GET_ARCH_GCC_BUILTIN_CPU_SUPPORTS 1

#include <simdpp/dispatch/arch.h>

namespace simdpp {

/** Retrieves supported architecture using GCC __builtin_cpu_supports function.
    Works only on x86.
*/
inline Arch get_arch_gcc_builtin_cpu_supports()
{
    Arch arch_info;
#if (__GNUC__ > 4)
    if (__builtin_cpu_supports("avx512f")) { // since 5.0
        arch_info |= Arch::X86_SSE2;
        arch_info |= Arch::X86_SSE3;
        arch_info |= Arch::X86_SSSE3;
        arch_info |= Arch::X86_SSE4_1;
        arch_info |= Arch::X86_AVX;
        arch_info |= Arch::X86_AVX2;
        arch_info |= Arch::X86_FMA3;
        arch_info |= Arch::X86_AVX512F;
    } else
#endif
    if (__builtin_cpu_supports("avx2")) {
        arch_info |= Arch::X86_SSE2;
        arch_info |= Arch::X86_SSE3;
        arch_info |= Arch::X86_SSSE3;
        arch_info |= Arch::X86_SSE4_1;
        arch_info |= Arch::X86_AVX;
        arch_info |= Arch::X86_AVX2;
    } else if (__builtin_cpu_supports("avx")) {
        arch_info |= Arch::X86_SSE2;
        arch_info |= Arch::X86_SSE3;
        arch_info |= Arch::X86_SSSE3;
        arch_info |= Arch::X86_SSE4_1;
        arch_info |= Arch::X86_AVX;
    } else if (__builtin_cpu_supports("sse4.1")) {
        arch_info |= Arch::X86_SSE2;
        arch_info |= Arch::X86_SSE3;
        arch_info |= Arch::X86_SSSE3;
        arch_info |= Arch::X86_SSE4_1;
    } else if (__builtin_cpu_supports("ssse3")) {
        arch_info |= Arch::X86_SSE2;
        arch_info |= Arch::X86_SSE3;
        arch_info |= Arch::X86_SSSE3;
    } else if (__builtin_cpu_supports("sse3")) {
        arch_info |= Arch::X86_SSE3;
        arch_info |= Arch::X86_SSE2;
    } else if (__builtin_cpu_supports("sse2")) {
        arch_info |= Arch::X86_SSE2;
    }
    if (__builtin_cpu_supports("popcnt"))
        arch_info |= Arch::X86_POPCNT_INSN;

    return arch_info;
}
} // namespace simdpp

#endif
#endif
