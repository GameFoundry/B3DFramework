/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_F_MUL_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_F_MUL_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON
    #include <simdpp/detail/null/math.h>
#endif
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

static SIMDPP_INL
float32<4> i_fmul(const float32<4>& a, const float32<4>& b)
{
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    return detail::null::mul(a, b);
#elif SIMDPP_USE_SSE2
    return _mm_mul_ps(a.native(), b.native());
#elif SIMDPP_USE_NEON_FLT_SP
    return vmulq_f32(a.native(), b.native());
#elif SIMDPP_USE_VSX_206
    return vec_mul(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    float32<4> zero = make_zero();
    return vec_madd(a.native(), b.native(), zero.native());
#elif SIMDPP_USE_MSA
    return __msa_fmul_w(a.native(), b.native());
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float32<8> i_fmul(const float32<8>& a, const float32<8>& b)
{
    return _mm256_mul_ps(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float32<16> i_fmul(const float32<16>& a, const float32<16>& b)
{
    return _mm512_mul_ps(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
float64<2> i_fmul(const float64<2>& a, const float64<2>& b)
{
#if SIMDPP_USE_SSE2
    return _mm_mul_pd(a.native(), b.native());
#elif SIMDPP_USE_NEON64
    return vmulq_f64(a.native(), b.native());
#elif SIMDPP_USE_VSX_206
    return vec_mul(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_fmul_d(a.native(), b.native());
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC
    return detail::null::mul(a, b);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float64<4> i_fmul(const float64<4>& a, const float64<4>& b)
{
    return _mm256_mul_pd(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float64<8> i_fmul(const float64<8>& a, const float64<8>& b)
{
    return _mm512_mul_pd(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
V i_fmul(const V& a, const V& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(V, i_fmul, a, b)
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

