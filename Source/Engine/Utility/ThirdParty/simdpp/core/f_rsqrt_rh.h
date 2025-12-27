/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_F_RSQRT_RH_H
#define LIBSIMDPP_SIMDPP_CORE_F_RSQRT_RH_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/f_rsqrt_rh.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {


/** Computes one Newton-Rhapson iteration for inverse of square root. @a x is
    the current estimate, @a a are the values to estimate the inverse square
    root for.

    @code
    r0 = x0 * (3 - a0*x0*x0) * 0.5
    ...
    rN = xN * (3 - aN*xN*xN) * 0.5
    @endcode

    @par 128-bit version:
    @icost{SSE2, SSE3, SSSE3, SSE4.1, 5-7}
    @icost{NEON, 3}
    @icost{ALTIVEC, 4-6}

    @par 256-bit version:
    @icost{AVX-AVX2, 7}
    @icost{SSE2, SSE3, SSSE3, SSE4.1, 10-12}
    @icost{NEON, 6}
    @icost{ALTIVEC, 8-10}
*/
template<unsigned N, class E> SIMDPP_INL
float32<N,expr_empty> rsqrt_rh(const float32<N,E>& x, const float32<N,E>& a)
{
    return detail::insn::i_rsqrt_rh(x.eval(), a.eval());
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

