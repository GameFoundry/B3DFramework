/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_F_ADD_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_F_ADD_H

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
float32<4> i_fadd(const float32<4>& a, const float32<4>& b)
{
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    return detail::null::add(a, b);
#elif SIMDPP_USE_SSE2
    return _mm_add_ps(a.native(), b.native());
#elif SIMDPP_USE_NEON_FLT_SP
    return vaddq_f32(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_add(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_fadd_w(a.native(), b.native());
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float32<8> i_fadd(const float32<8>& a, const float32<8>& b)
{
    return _mm256_add_ps(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float32<16> i_fadd(const float32<16>& a, const float32<16>& b)
{
    return _mm512_add_ps(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
float64<2> i_fadd(const float64<2>& a, const float64<2>& b)
{
#if SIMDPP_USE_SSE2
    return _mm_add_pd(a.native(), b.native());
#elif SIMDPP_USE_NEON64
    return vaddq_f64(a.native(), b.native());
#elif SIMDPP_USE_VSX_206
    return vec_add(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_fadd_d(a.native(), b.native());
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC
    return detail::null::add(a, b);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float64<4> i_fadd(const float64<4>& a, const float64<4>& b)
{
    return _mm256_add_pd(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float64<8> i_fadd(const float64<8>& a, const float64<8>& b)
{
    return _mm512_add_pd(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
V i_fadd(const V& a, const V& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(V, i_fadd, a, b)
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

