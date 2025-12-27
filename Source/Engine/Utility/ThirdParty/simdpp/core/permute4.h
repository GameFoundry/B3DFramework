/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_PERMUTE4_H
#define LIBSIMDPP_SIMDPP_CORE_PERMUTE4_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/permute4.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Permutes the 16-bit values within each 4 consecutive values of the vector.
    The selector values must be in range [0; 3].

    @code
    r0 = a[s0]
    ...
    r3 = a[s3]
    r4 = a[s0+4]
    ...
    r7 = a[s3+4]

    256-bit version:

    r8 = a[s0+8]
    ...
    r11 = a[s3+8]
    r12 = a[s0+12]
    ...
    r15 = a[s3+12]
    @endcode

    @par: 128-bit version:
    @icost{SSE2-AVX2, 2}
    @icost{NEON, 1-5}
    @icost{ALTIVEC, 1-2}

    @par: 256-bit version:
    @icost{SSE2-AVX, 4}
    @icost{AVX2, 2}
    @icost{NEON, 2-10}
    @icost{ALTIVEC, 2-3}
*/
template<unsigned s0, unsigned s1, unsigned s2, unsigned s3,
         unsigned N, class V> SIMDPP_INL
typename detail::get_expr_nomask<V>::empty
        permute4(const any_vec16<N,V>& a)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    typename detail::get_expr_nomask<V>::type ra;
    ra = a.wrapped().eval();
    return detail::insn::i_permute4<s0,s1,s2,s3>(ra);
}

/** Permutes the values of each set of four consecutive 32-bit values. The
    selector values must be in range [0; 3].

    @code
    r0 = a[s0]
    ...
    r3 = a[s3]

    256-bit version:
    r4 = a[s0+4]
    ...
    r7 = a[s3+4]
    @endcode

    @par integer
    @par 128-bit version:
    @icost{NEON, 1-4}
    @icost{ALTIVEC, 1-2}

    @par 256-bit version:
    @icost{SSE2-AVX, 2}
    @icost{NEON, 2-8}
    @icost{ALTIVEC, 2-3}

    @par floating-point
    @par 128-bit version:
    @icost{NEON, 1-4}
    @icost{ALTIVEC, 1-2}

    @par 256-bit version:
    @icost{SSE2-SSE4.1, 2}
    @icost{NEON, 2-8}
    @icost{ALTIVEC, 2-3}
*/
template<unsigned s0, unsigned s1, unsigned s2, unsigned s3,
         unsigned N, class V> SIMDPP_INL
typename detail::get_expr_nomask<V>::empty
        permute4(const any_vec32<N,V>& a)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    typename detail::get_expr_nomask<V>::type ra;
    ra = a.wrapped().eval();
    return detail::insn::i_permute4<s0,s1,s2,s3>(ra);
}

/** Permutes the values of each set of four consecutive 64-bit values. The
    selector values must be in range [0; 3].

    @code
    r0 = a[s0]
    r1 = a[s1]
    r2 = a[s2]
    r3 = a[s3]
    @endcode

    @par integer
    @icost{SSE2-AVX, 2}

    @par floating-point
    @icost{SSE2-AVX, 1-2}
    @icost{NEON, 1-4}
    @icost{ALTIVEC, 1-4}
*/
template<unsigned s0, unsigned s1, unsigned s2, unsigned s3,
         unsigned N, class V> SIMDPP_INL
typename detail::get_expr_nomask<V>::empty
        permute4(const any_vec64<N,V>& a)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    typename detail::get_expr_nomask<V>::type ra;
    ra = a.wrapped().eval();
    return detail::insn::i_permute4<s0,s1,s2,s3>(ra);
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

