/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_F_SIGN_H
#define LIBSIMDPP_SIMDPP_CORE_F_SIGN_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/f_sign.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {


/** Extracts sign bits from the values in float32x4 vector

    @code
    r0 = a0 & 0x80000000
    ...
    rN = aN & 0x80000000
    @endcode

    @par 128-bit version:
    @icost{SSE2-SSE4.1, ALTIVEC, NEON, 1-2}

    @par 256-bit version:
    @icost{SSE2-SSE4.1, ALTIVEC, NEON, 2-3}
    @icost{AVX-AVX2, 1-2}
*/
template<unsigned N, class E> SIMDPP_INL
float32<N,expr_empty> sign(const float32<N,E>& a)
{
    return detail::insn::i_sign(a.eval());
}

/** Extracts sigh bit from the values in float64x2 vector.

    @code
    r0 = a0 & 0x8000000000000000
    ...
    r0 = aN & 0x8000000000000000
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
float64<N,expr_empty> sign(const float64<N,E>& a)
{
    return detail::insn::i_sign(a.eval());
}


} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

