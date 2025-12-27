/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_F_MUL_H
#define LIBSIMDPP_SIMDPP_CORE_F_MUL_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/expr/f_mul.h>
#include <simdpp/core/detail/scalar_arg_impl.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {


/** Multiplies the values of two vectors

    @code
    r0 = a0 * b0
    ...
    rN = aN * bN
    @endcode

    @par 256-bit version:
    @icost{SSE2-SSE4.1, NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
float32<N, expr_fmul<float32<N,E1>,
                     float32<N,E2>>> mul(const float32<N,E1>& a,
                                         const float32<N,E2>& b)
{
    return { { a, b } };
}

SIMDPP_SCALAR_ARG_IMPL_EXPR(mul, expr_fmul, float32, float32)

/** Multiplies the values of two vectors

    @code
    r0 = a0 * b0
    ...
    rN = aN * bN
    @endcode

    @par 128-bit version:
    @novec{NEON, ALTIVEC}

    @par 256-bit version:
    @novec{NEON, ALTIVEC}
    @icost{SSE2-SSE4.1, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
float64<N, expr_fmul<float64<N,E1>,
                     float64<N,E2>>> mul(const float64<N,E1>& a,
                                         const float64<N,E2>& b)
{
    return { { a, b } };
}

SIMDPP_SCALAR_ARG_IMPL_EXPR(mul, expr_fmul, float64, float64)

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

