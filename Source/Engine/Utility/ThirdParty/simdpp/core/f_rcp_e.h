/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_F_RCP_E_H
#define LIBSIMDPP_SIMDPP_CORE_F_RCP_E_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/f_rcp_e.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Computes approximate reciprocal.

    Relative error is as follows:
     - 1/2 ULP for NULL and NEON
     - ~1/2730 for SSE2
     - 1/16376 for AVX512
     - 1/4096 for ALTIVEC
     - 1/256 for NEON_FLT_SP

    @code
    r0 = approx(1.0f / a0)
    ...
    rN = approx(1.0f / aN)
    @endcode

    @par 256-bit version:
    @icost{SSE2-SSE4.1, NEON, ALTIVEC, 2}
*/
template<unsigned N, class E> SIMDPP_INL
float32<N,expr_empty> rcp_e(const float32<N,E>& a)
{
    return detail::insn::i_rcp_e(a.eval());
}


} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

