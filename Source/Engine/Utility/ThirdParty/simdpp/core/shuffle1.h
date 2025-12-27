/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_SHUFFLV1_H
#define LIBSIMDPP_SIMDPP_CORE_SHUFFLV1_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/shuffle2x2.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Selects 64-bit values from two vectors. The first value in
    each pair of values must come from @a a, the second - from @a b. The
    selector values must be in range [0; 1].

    @code
    r0 = a[s0]
    r1 = b[s1]

    256-bit version:
    r2 = a[s0+2]
    r3 = b[s1+2]
    @endcode

    @par floating-point
    @par 128-bit version:
    @novec{NEON, ALTIVEC}

    @par 256-bit version:
    @icost{SSE2-SSE4.1, 2}
    @novec{NEON, ALTIVEC}

    @par integer
    @par 128-bit version:
    @icost{ALTIVEC, 1-2}

    @par 256-bit version:
    @icost{SSE2-AVX, 2}
    @icost{NEON, 1-2}
    @icost{ALTIVEC, 2-3}
*/
template<unsigned s0, unsigned s1, unsigned N, class V1, class V2> SIMDPP_INL
typename detail::get_expr2_nomask<V1, V2>::empty
        shuffle1(const any_vec64<N,V1>& a, const any_vec64<N,V2>& b)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
    typename detail::get_expr2_nomask<V1, V2>::type ra = a.wrapped().eval(),
                                                    rb = b.wrapped().eval();
    return detail::insn::i_shuffle2x2<s0,s1+2>(ra, rb);
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

