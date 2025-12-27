/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_F_SQRT_H
#define LIBSIMDPP_SIMDPP_CORE_F_SQRT_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/f_sqrt.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {


/** Computes square root.

    @code
    r0 = sqrt(a0)
    ...
    rN = sqrt(aN)
    @endcode

    @par 128-bit version:
    @icost{NEON, 5}
    @icost{ALTIVEC, 5-7}

    @par 256-bit version:
    @icost{SSE2-SSE4.1, 2}
    @icost{NEON, 10}
    @icost{ALTIVEC, 10-12}
*/
template<unsigned N, class E1> SIMDPP_INL
float32<N,expr_empty> sqrt(const float32<N,E1>& a)
{
    return detail::insn::i_sqrt(a.eval());
}

/** Computes square root.

    @code
    r0 = sqrt(a0)
    ...
    rN = sqrt(aN)
    @endcode

    @par 128-bit version:
    @novec{NEON, ALTIVEC}

    @par 256-bit version:
    @icost{SSE2-SSE4.1, 2}
    @novec{NEON, ALTIVEC}
*/
template<unsigned N, class E1> SIMDPP_INL
float64<N,expr_empty> sqrt(const float64<N,E1>& a)
{
    return detail::insn::i_sqrt(a.eval());
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

