/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_F_RSQRT_E_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_F_RSQRT_E_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
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
float32x4 i_rsqrt_e(const float32x4& a)
{
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    float32x4 r;
    for (unsigned i = 0; i < a.length; i++) {
        r.el(i) = 1.0f / std::sqrt(a.el(i));
    }
    return r;
#elif SIMDPP_USE_SSE2
    return _mm_rsqrt_ps(a.native());
#elif SIMDPP_USE_NEON_FLT_SP
    return vrsqrteq_f32(a.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_rsqrte(a.native());
#elif SIMDPP_USE_MSA
    return __msa_frsqrt_w(a.native());
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float32x8 i_rsqrt_e(const float32x8& a)
{
    return _mm256_rsqrt_ps(a.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float32<16> i_rsqrt_e(const float32<16>& a)
{
    return _mm512_rsqrt14_ps(a.native());
}
#endif

template<unsigned N> SIMDPP_INL
float32<N> i_rsqrt_e(const float32<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL1(float32<N>, i_rsqrt_e, a);
}


} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

