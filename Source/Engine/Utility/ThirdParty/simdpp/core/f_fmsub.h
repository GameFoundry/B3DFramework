/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_F_FMSUB_H
#define LIBSIMDPP_SIMDPP_CORE_F_FMSUB_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/expr/f_fmsub.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Performs a fused multiply-sutract operation

    @code
    r0 = a0 * b0 - c0
    ...
    rN = aN * bN - cN
    @endcode

    Implemented only on architectures with either @c X86_FMA3 or @c X86_FMA4
    support.
*/
template<unsigned N, class E1, class E2, class E3> SIMDPP_INL
float32<N, expr_fmsub<float32<N,E1>,
                      float32<N,E2>,
                      float32<N,E3>>> fmsub(const float32<N,E1>& a,
                                            const float32<N,E2>& b,
                                            const float32<N,E3>& c)
{
    return { { a, b, c } };
}

template<unsigned N, class E1, class E2, class E3> SIMDPP_INL
float64<N, expr_fmsub<float64<N,E1>,
                      float64<N,E2>,
                      float64<N,E3>>> fmsub(const float64<N,E1>& a,
                                            const float64<N,E2>& b,
                                            const float64<N,E3>& c)
{
    return { { a, b, c } };
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

