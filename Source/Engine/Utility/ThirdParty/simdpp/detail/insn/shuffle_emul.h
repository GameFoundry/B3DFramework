/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_SHUFFLE_EMUL_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_SHUFFLE_EMUL_H
#if SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/permute2.h>
#include <simdpp/core/shuffle1.h>
#include <simdpp/core/combine.h>
#include <simdpp/core/split.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

template<unsigned s0, unsigned s1, class V> SIMDPP_INL
V permute_half(const V& a0, const V& a1)
{
    switch (s0*4+s1) {
    default:
    case 0: /* 0 0 */ return permute2<0,0>(a0);
    case 1: /* 0 1 */ return a0;
    case 2: /* 0 2 */ return shuffle1<0,0>(a0, a1);
    case 3: /* 0 3 */ return shuffle1<0,1>(a0, a1);
    case 4: /* 1 0 */ return permute2<1,0>(a0);
    case 5: /* 1 1 */ return permute2<1,1>(a0);
    case 6: /* 1 2 */ return shuffle1<1,0>(a0, a1);
    case 7: /* 1 3 */ return shuffle1<1,1>(a0, a1);
    case 8: /* 2 0 */ return shuffle1<0,0>(a1, a0);
    case 9: /* 2 1 */ return shuffle1<0,1>(a1, a0);
    case 10: /* 2 2 */ return permute2<0,0>(a1);
    case 11: /* 2 3 */ return a1;
    case 12: /* 3 0 */ return shuffle1<1,0>(a1, a0);
    case 13: /* 3 1 */ return shuffle1<1,1>(a1, a0);
    case 14: /* 3 2 */ return permute2<1,0>(a1);
    case 15: /* 3 3 */ return permute2<1,1>(a1);
    }
}

/** Permutes 4 64-bit elements within 256-bit vector using 2 element shuffling
    functions.
*/
template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
uint64x4 permute_emul(const uint64x4& a)
{
    uint64x2 r0, r1, a0, a1;
    split(a, a0, a1);
    r0 = permute_half<s0,s1>(a0, a1);
    r1 = permute_half<s2,s3>(a0, a1);
    return combine(r0, r1);
}

template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
float64x4 permute_emul(const float64x4& a)
{
    float64x2 r0, r1, a0, a1;
    split(a, a0, a1);
    r0 = permute_half<s0,s1>(a0, a1);
    r1 = permute_half<s2,s3>(a0, a1);
    return combine(r0, r1);
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif // SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC
#endif
