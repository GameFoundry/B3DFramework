/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_F_RCP_RH_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_F_RCP_RH_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/f_sub.h>
#include <simdpp/core/f_mul.h>
#include <simdpp/core/make_float.h>
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC
    #include <cmath>
    #include <simdpp/detail/null/math.h>
#endif
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {


static SIMDPP_INL
float32x4 i_rcp_rh(const float32x4& cx, const float32x4& a)
{
    float32<4> x = cx;
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    float32x4 r;
    for (unsigned i = 0; i < a.length; i++) {
        float ix = x.el(i);
        float ia = a.el(i);
        r.el(i) = ix*(2.0f - ix*ia);
    }
    return r;
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_MSA
    float32x4 r;

    r = mul(a, x);
    r = sub(2.0, r);
    x = mul(x, r);

    return x;
#elif SIMDPP_USE_NEON_FLT_SP
    float32x4 r;
    r = vrecpsq_f32(a.native(), x.native());
    x = mul(x, r);

    return x;
#elif SIMDPP_USE_ALTIVEC
    float32x4 r, c2;
    c2 = make_float(2.0f);
    // -(x*a-c2)
    r = vec_nmsub(x.native(), a.native(), c2.native());
    x = mul(x, r);
    return x;
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float32x8 i_rcp_rh(const float32x8& cx, const float32x8& a)
{
    float32x8 r, x = cx;

    r = mul(a, x);
    r = sub(2.0, r);
    x = mul(x, r);

    return x;
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float32<16> i_rcp_rh(const float32<16>& cx, const float32<16>& a)
{
    float32<16> r, x = cx;

    r = mul(a, x);
    r = sub(2.0, r);
    x = mul(x, r);

    return x;
}
#endif

template<unsigned N> SIMDPP_INL
float32<N> i_rcp_rh(const float32<N>& x, const float32<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL2(float32<N>, i_rcp_rh, x, a);
}


} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

