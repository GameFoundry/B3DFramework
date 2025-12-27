/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_F_TRUNC_H
#define LIBSIMDPP_SIMDPP_CORE_F_TRUNC_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <cmath>
#include <simdpp/detail/insn/f_trunc.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {


/** Rounds the values of a vector towards zero
    @code
    r0 = trunc(a0)
    ...
    rN = trunc(aN)
    @endcode

    @par 128-bit version:
    @icost{SSE2, SSE3, SSSE3, 7-9}
    @icost{NEON, 5-6}

    @par 256-bit version:
    @icost{SSE2, SSE3, SSSE3, 14-16}
    @icost{NEON, 10-11}
    @icost{SSE4.1, ALTIVEC, 2}
*/
template<unsigned N, class E> SIMDPP_INL
float32<N,expr_empty> trunc(const float32<N,E>& a)
{
    return detail::insn::i_trunc(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
float64<N,expr_empty> trunc(const float64<N,E>& a)
{
    return detail::insn::i_trunc(a.eval());
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

