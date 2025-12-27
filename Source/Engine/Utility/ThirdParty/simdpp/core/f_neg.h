/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_F_NEG_H
#define LIBSIMDPP_SIMDPP_CORE_F_NEG_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/expr/f_neg.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Negates the values of a float32x4 vector

    @code
    r0 = -a0
    ...
    rN = -aN
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, ALTIVEC, 1-2}

    @par 256-bit version:
    @icost{SSE2-SSE4.1, ALTIVEC, 2-3}
    @icost{AVX-AVX2, NEON, 2}
*/
template<unsigned N, class E> SIMDPP_INL
float32<N, expr_fneg<float32<N,E>>> neg(const float32<N,E>& a)
{
    return { { a } };
}

/** Negates the values of a vector

    @code
    r0 = -a0
    ...
    rN = -aN
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 1-2}
    @novec{NEON, ALTIVEC}

    @par 256-bit version:
    @icost{SSE2-SSE4.1, 2-3}
    @icost{AVX-AVX2, 1-2}
    @novec{NEON, ALTIVEC}
*/
template<unsigned N, class E> SIMDPP_INL
float64<N, expr_fneg<float64<N,E>>> neg(const float64<N,E>& a)
{
    return { { a } };
}


} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

