/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_F_FLOOR_H
#define LIBSIMDPP_SIMDPP_CORE_F_FLOOR_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <cmath>
#include <simdpp/types.h>
#include <simdpp/detail/insn/f_floor.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Rounds the values of a vector towards negative infinity

    @code
    r0 = floor(a0)
    ...
    rN = floor(aN)
    @endcode

    @par 128-bit version:
    @icost{SSE2-SSSE3, 12-14}
    @icost{NEON, 10-11}

    @par 256-bit version:
    @icost{SSE2-SSSE3, 24-26}
    @icost{NEON, 20-21}
    @icost{ALTIVEC, 2}
*/
template<unsigned N, class E> SIMDPP_INL
float32<N,expr_empty> floor(const float32<N,E>& a)
{
    return detail::insn::i_floor(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
float64<N,expr_empty> floor(const float64<N,E>& a)
{
    return detail::insn::i_floor(a.eval());
}


} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

