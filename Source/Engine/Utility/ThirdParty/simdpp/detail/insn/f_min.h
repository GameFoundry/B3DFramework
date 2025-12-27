/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_F_MIN_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_F_MIN_H

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
float32x4 i_min(const float32x4& a, const float32x4& b)
{
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    return detail::null::min(a, b);
#elif SIMDPP_USE_SSE2
    return _mm_min_ps(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vminq_f32(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_min(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_fmin_w(a.native(), b.native());
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float32x8 i_min(const float32x8& a, const float32x8& b)
{
    return _mm256_min_ps(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float32<16> i_min(const float32<16>& a, const float32<16>& b)
{
    return _mm512_min_ps(a.native(), b.native());
}
#endif

template<unsigned N> SIMDPP_INL
float32<N> i_min(const float32<N>& a, const float32<N>& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(float32<N>, i_min, a, b);
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
float64x2 i_min(const float64x2& a, const float64x2& b)
{
#if SIMDPP_USE_SSE2
    return _mm_min_pd(a.native(), b.native());
#elif SIMDPP_USE_NEON64
    return vminq_f64(a.native(), b.native());
#elif SIMDPP_USE_VSX_206
    return vec_min(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_fmin_d(a.native(), b.native());
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC
    return detail::null::min(a, b);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float64x4 i_min(const float64x4& a, const float64x4& b)
{
    return _mm256_min_pd(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float64<8> i_min(const float64<8>& a, const float64<8>& b)
{
    return _mm512_min_pd(a.native(), b.native());
}
#endif

template<unsigned N> SIMDPP_INL
float64<N> i_min(const float64<N>& a, const float64<N>& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(float64<N>, i_min, a, b);
}


} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

