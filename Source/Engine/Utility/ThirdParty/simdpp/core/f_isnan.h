/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_ISNAN_H
#define LIBSIMDPP_SIMDPP_CORE_ISNAN_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/f_isnan.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Checks whether elements in @a a are IEEE754 NaN.

    @code
    r0 = isnan(a0) ? 0xffffffff : 0
    ...
    rN = isnan(aN) ? 0xffffffff : 0
    @endcode

    @par 256-bit version:
    @icost{SSE2-SSE4.1, NEON, ALTIVEC, 2}
*/
template<unsigned N, class E> SIMDPP_INL
mask_float32<N,expr_empty> isnan(const float32<N,E>& a)
{
    return detail::insn::i_isnan(a.eval());
}

/** Checks whether elements in @a a are IEEE754 NaN.

    @code
    r0 = isnan(a0) ? 0xffffffffffffffff : 0
    ...
    rN = isnan(aN) ? 0xffffffffffffffff : 0
    @endcode

    @par 128-bit version:
    @novec{NEON, ALTIVEC}

    @par 256-bit version:
    @novec{NEON, ALTIVEC}
    @icost{SSE2-SSE4.1, 2}
*/
template<unsigned N, class E> SIMDPP_INL
mask_float64<N,expr_empty> isnan(const float64<N,E>& a)
{
    return detail::insn::i_isnan(a.eval());
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

