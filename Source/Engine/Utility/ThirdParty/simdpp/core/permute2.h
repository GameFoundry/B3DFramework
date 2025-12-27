/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_PERMUTE2_H
#define LIBSIMDPP_SIMDPP_CORE_PERMUTE2_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/permute2.h>
#include <simdpp/detail/get_expr.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Permutes the 16-bit values within sets of two consecutive elements of the
    vector. The selector values must be in range [0; 1].

    @code
    r0 = a[s0]
    r1 = a[s1]
    r2 = a[s0+2]
    r3 = a[s1+2]
    r4 = a[s0+4]
    r5 = a[s1+4]
    ...
    @endcode

    @par: 128-bit version:
    @icost{SSE2-AVX2, 2}
    @icost{NEON, ALTIVEC, 1-2}

    @par: 256-bit version:
    @icost{SSE2-AVX, 4}
    @icost{AVX2, 2}
    @icost{NEON, 2-4}
    @icost{ALTIVEC, 2-3}
*/
template<unsigned s0, unsigned s1, unsigned N, class V> SIMDPP_INL
typename detail::get_expr_nomask<V>::empty
        permute2(const any_vec16<N,V>& a)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
    typename detail::get_expr_nomask<V>::type ra;
    ra = a.wrapped().eval();
    return detail::insn::i_permute2<s0,s1>(ra);
}

/** Permutes the values of each set of four consecutive 32-bit values. The
    selector values must be in range [0; 1].

    @code
    r0 = a[s0]
    r1 = a[s1]
    r2 = a[s0+2]
    r3 = a[s1+2]
    256-bit version:
    r4 = a[s0+4]
    r5 = a[s1+4]
    r6 = a[s0+6]
    r7 = a[s1+6]
    @endcode

    @par integer
    @par 128-bit version:
    @icost{NEON, 2-4}
    @icost{ALTIVEC, 1-2}

    @par 256-bit version:
    @icost{SSE2-AVX, 2}
    @icost{NEON, 4-8}
    @icost{ALTIVEC, 2-3}

    @par floating-point
    @par 128-bit version:
    @icost{NEON, 2-4}
    @icost{ALTIVEC, 1-2}

    @par 256-bit version:
    @icost{SSE2-AVX, 2}
    @icost{NEON, 4-8}
    @icost{ALTIVEC, 2-3}
*/
template<unsigned s0, unsigned s1, unsigned N, class V> SIMDPP_INL
typename detail::get_expr_nomask<V>::empty
        permute2(const any_vec32<N,V>& a)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
    typename detail::get_expr_nomask<V>::type ra;
    ra = a.wrapped().eval();
    return detail::insn::i_permute2<s0,s1>(ra);
}

/** Permutes the values of each set of four consecutive 32-bit values. The
    selector values must be in range [0; 1].

    @code
    r0 = a[s0]
    r1 = a[s1]

    256-bit version:
    r2 = a[s0+2]
    r3 = a[s1+2]
    @endcode

    @par 128-bit version:
    @icost{NEON, 1-2}
    @icost{ALTIVEC, 1-2}

    @par 256-bit version:
    @icost{SSE2-AVX, 2}
    @icost{NEON, 2-4}
    @icost{ALTIVEC, 2-4}
*/
template<unsigned s0, unsigned s1, unsigned N, class V> SIMDPP_INL
typename detail::get_expr_nomask<V>::empty
        permute2(const any_vec64<N,V>& a)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
    typename detail::get_expr_nomask<V>::type ra;
    ra = a.wrapped().eval();
    return detail::insn::i_permute2<s0,s1>(ra);
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

