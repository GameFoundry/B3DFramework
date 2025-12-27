/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_F_RSQRT_RH_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_F_RSQRT_RH_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <cmath>
#include <simdpp/types.h>
#include <simdpp/core/f_mul.h>
#include <simdpp/core/f_sub.h>
#include <simdpp/detail/null/math.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

template<class V> SIMDPP_INL
V v_rsqrt_rh(const V& cx, const V& a)
{
    V x2, r, x = cx;

    x2 = mul(x, x);
    r = mul(a, x2);
    r = sub(3.0, r);
    x = mul(x, 0.5);
    r = mul(x, r);

    return r;
}

static SIMDPP_INL
float32x4 i_rsqrt_rh(const float32x4& cx, const float32x4& a)
{
    // x_n = x*(3-d*x*x)/2
    float32<4> x = cx;
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    float32x4 r;
    for (unsigned i = 0; i < cx.length; i++) {
        float ix = x.el(i);
        float ia = a.el(i);
        r.el(i) = ix * (3.0f - ia*ix*ix) * 0.5f;
    }
    return r;
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_MSA
    return v_rsqrt_rh(x, a);
#elif SIMDPP_USE_NEON_FLT_SP
    float32x4 x2, r;

    x2 = mul(x, x);
    r = vrsqrtsq_f32(a.native(), x2.native());
    x = mul(x, r);

    return x;
#elif SIMDPP_USE_ALTIVEC
    float32x4 x2, r, xp5, c3;

    c3 = make_float(3.0f);

    x2 = mul(x, x);
    // r = (c3 - a*x2)
    r = vec_nmsub(a.native(), x2.native(), c3.native());
    xp5 = mul(x, 0.5);
    r = mul(xp5, r);

    return r;
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float32x8 i_rsqrt_rh(const float32x8& x, const float32x8& a)
{
    return v_rsqrt_rh(x, a);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float32<16> i_rsqrt_rh(const float32<16>& x, const float32<16>& a)
{
    return v_rsqrt_rh(x, a);
}
#endif


template<unsigned N> SIMDPP_INL
float32<N> i_rsqrt_rh(const float32<N>& x, const float32<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL2(float32<N>, i_rsqrt_rh, x, a);
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

