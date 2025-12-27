/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_SPLAT_H
#define LIBSIMDPP_SIMDPP_CORE_SPLAT_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/splat.h>
#include <simdpp/detail/get_expr.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Broadcasts the specified element to all elements.

    @code
    r0 = a[s]
    r1 = a[s]
    ...
    rN = a[s]
    @endcode

    @par int8

    @par 128-bit version:
    @icost{SSE2-AVX, 5}
    @icost{AVX2, 2}

    @par 256-bit version:
    @icost{SSE2-AVX, 6}
    @icost{NEON, ALTIVEC, 2}

    @par int16

    @par 128-bit version:
    @icost{SSE2-AVX, 5}
    @icost{AVX2, 2}

    @par 256-bit version:
    @icost{SSE2-AVX, 6}
    @icost{NEON, ALTIVEC, 2}

    @par int32

    @par 256-bit version:
    @icost{NEON, ALTIVEC, 2}

    @par int64

    @par 128-bit version:
    @icost{ALTIVEC, 1-2}

    @par 256-bit version:
    @icost{SSE2-AVX, NEON, 2}
    @icost{ALTIVEC, 1-2}

    @par float32

    @par 256-bit version:
    @icost{SSE2-AVX, NEON, ALTIVEC, 2}

    @par float64

    @par 128-bit version:
    @novec{NEON, ALTIVEC}

    @par 256-bit version:
    @icost{SSE2-AVX, 2}
    @novec{NEON, ALTIVEC}
*/
template<unsigned s, unsigned N, class V> SIMDPP_INL
typename detail::get_expr_nomask<V>::empty
        splat(const any_vec<N,V>& a)
{
    static_assert(s < V::length, "Access out of bounds");
    typename detail::get_expr_nomask<V>::type ra = a.wrapped().eval();
    return detail::insn::i_splat<s>(ra);
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

