/*  Copyright (C) 2016  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_F_REDUCE_ADD_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_F_REDUCE_ADD_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/extract.h>
#include <simdpp/core/f_add.h>
#include <simdpp/core/permute2.h>
#include <simdpp/detail/extract128.h>
#include <simdpp/detail/workarounds.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {


static SIMDPP_INL
float i_reduce_add(const float32x4& a)
{
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    float r = a.el(0);
    for (unsigned i = 1; i < a.length; i++) {
        r += a.el(i);
    }
    return r;
#elif SIMDPP_USE_SSE3
    float32x4 b = a;
    b = _mm_hadd_ps(b.native(), b.native());
    b = _mm_hadd_ps(b.native(), b.native());
    return _mm_cvtss_f32(b.native());
#elif SIMDPP_USE_SSE2
    float32x4 sum2 = _mm_movehl_ps(a.native(), a.native());
    float32x4 sum = add(a, sum2);
    sum = add(sum, permute2<1,0>(sum));
    return _mm_cvtss_f32(sum.native());
#elif SIMDPP_USE_NEON_FLT_SP
    float32x2_t a2 = vpadd_f32(vget_low_f32(a.native()), vget_high_f32(a.native()));
    a2 = vpadd_f32(a2, a2);
    return vget_lane_f32(a2, 0);
#elif SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    float32x4 b = a;
    b = add(b, move4_l<1>(b));
    b = add(b, move4_l<2>(b));
    return extract<0>(b);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float i_reduce_add(const float32x8& a)
{
    __m128 ah = detail::extract128<1>(a).native();
    __m128 al = detail::extract128<0>(a).native();
    al = _mm_hadd_ps(al, ah);
    al = _mm_hadd_ps(al, al);
    al = _mm_hadd_ps(al, al);
    return _mm_cvtss_f32(al);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float i_reduce_add(const float32<16>& a)
{
    return i_reduce_add(add(extract256<0>(a), extract256<1>(a)));
}
#endif

template<unsigned N>
SIMDPP_INL float i_reduce_add(const float32<N>& a)
{
    float32v r = a.vec(0);
    for (unsigned i = 1; i < a.vec_length; ++i)
        r = add(r, a.vec(i));
    return i_reduce_add(r);
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
double i_reduce_add(const float64x2& a)
{
#if SIMDPP_USE_SSE3
    return _mm_cvtsd_f64(_mm_hadd_pd(a.native(), a.native()));
#elif SIMDPP_USE_SSE2
    float64x2 b = add(a, permute2<1,1>(a));
    return _mm_cvtsd_f64(b.native());
#elif SIMDPP_USE_NEON64
    float64x2_t a2 = vpaddq_f64(a.native(), a.native());
    return vgetq_lane_f64(a2, 0);
#elif SIMDPP_USE_VSX_206 || SIMDPP_USE_MSA
    float64x2 b = add(a, permute2<1,1>(a));
    return extract<0>(b);
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC
    double r = a.el(0);
    for (unsigned i = 1; i < a.length; i++) {
        r += a.el(i);
    }
    return r;
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
double i_reduce_add(const float64x4& a)
{
    __m128d ah = detail::extract128<1>(a).native();
    __m128d al = detail::extract128<0>(a).native();
    al = _mm_hadd_pd(al, ah);
    al = _mm_hadd_pd(al, al);
    return _mm_cvtsd_f64(al);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
double i_reduce_add(const float64<8>& a)
{
    return i_reduce_add(add(extract256<0>(a), extract256<1>(a)));
}
#endif

template<unsigned N>
SIMDPP_INL double i_reduce_add(const float64<N>& a)
{
    float64v r = a.vec(0);
    for (unsigned i = 1; i < a.vec_length; ++i)
        r = add(r, a.vec(i));
    return i_reduce_add(r);
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

