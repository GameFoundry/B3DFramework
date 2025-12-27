/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_F_RCP_RH_H
#define LIBSIMDPP_SIMDPP_CORE_F_RCP_RH_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/f_rcp_rh.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {


/** Computes one Newton-Rhapson iterations for reciprocal. @a x is the current
    estimate, @a a are the values to estimate reciprocal for.

    @code
    r0 = x0 * (2 - x0*a0)
    ...
    rN = xN * (2 - xN*aN)
    @endcode

    Using this function, one can the division can be implemented as follows:
    @code
    // a/b
    float32x4 x;
    x = rcp_e(b);
    x = rcp_rh(x, b);
    x = rcp_rh(x, b);
    return mul(a, x);
    @endcode

    Precision can be controlled by selecting the number of @c rcp_rh steps.

    @par 128-bit version:
    @icost{SSE2-AVX2, 3-4}
    @icost{NEON, 2}
    @icost{ALTIVEC, 2-3}

    @par 256-bit version:
    @icost{AVX-AVX2, 3-4}
    @icost{SSE2-SSE4.1, 6-7}
    @icost{NEON, 4}
    @icost{ALTIVEC, 4-5}
*/
template<unsigned N, class E> SIMDPP_INL
float32<N,expr_empty> rcp_rh(const float32<N,E>& x, const float32<N,E>& a)
{
    return detail::insn::i_rcp_rh(x.eval(), a.eval());
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

