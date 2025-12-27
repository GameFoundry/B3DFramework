/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_ISNAN_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_ISNAN_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/null/math.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {


static SIMDPP_INL
mask_float32x4 i_isnan(const float32x4& a)
{
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    return detail::null::isnan(a);
#elif SIMDPP_USE_AVX512VL
    return _mm_cmp_ps_mask(a.native(), a.native(), _CMP_UNORD_Q);
#elif SIMDPP_USE_AVX
    return _mm_cmp_ps(a.native(), a.native(), _CMP_UNORD_Q);
#elif SIMDPP_USE_SSE2
    return (mask_float32x4) _mm_cmpunord_ps(a.native(), a.native());
#elif SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC
    return cmp_neq(a, a);
#elif SIMDPP_USE_MSA
    return (v4f32) __msa_fcun_w(a.native(), a.native());
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
mask_float32x8 i_isnan(const float32x8& a)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_cmp_ps_mask(a.native(), a.native(), _CMP_UNORD_Q);
#else
    return _mm256_cmp_ps(a.native(), a.native(), _CMP_UNORD_Q);
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
mask_float32<16> i_isnan(const float32<16>& a)
{
    return _mm512_cmp_ps_mask(a.native(), a.native(), _CMP_UNORD_Q);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
mask_float64x2 i_isnan(const float64x2& a)
{
#if SIMDPP_USE_AVX512VL
    return _mm_cmp_pd_mask(a.native(), a.native(), _CMP_UNORD_Q);
#elif SIMDPP_USE_AVX
    return _mm_cmp_pd(a.native(), a.native(), _CMP_UNORD_Q);
#elif SIMDPP_USE_SSE2
    return _mm_cmpunord_pd(a.native(), a.native());
#elif SIMDPP_USE_NEON64 || SIMDPP_USE_VSX_206
    return cmp_neq(a, a);
#elif SIMDPP_USE_MSA
    return (v2f64) __msa_fcun_d(a.native(), a.native());
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC
    return detail::null::isnan(a);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
mask_float64x4 i_isnan(const float64x4& a)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_cmp_pd_mask(a.native(), a.native(), _CMP_UNORD_Q);
#else
    return _mm256_cmp_pd(a.native(), a.native(), _CMP_UNORD_Q);
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
mask_float64<8> i_isnan(const float64<8>& a)
{
    return _mm512_cmp_pd_mask(a.native(), a.native(), _CMP_UNORD_Q);
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
typename V::mask_vector_type i_isnan(const V& a)
{
    SIMDPP_VEC_ARRAY_IMPL1(typename V::mask_vector_type, i_isnan, a);
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

