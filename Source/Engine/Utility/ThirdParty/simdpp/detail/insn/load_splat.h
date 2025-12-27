/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_LOAD_SPLAT_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_LOAD_SPLAT_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/load_u.h>
#include <simdpp/detail/insn/set_splat.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

static SIMDPP_INL
void i_load_splat(uint8x16& v, const char* p0)
{
    const uint8_t* v0 = reinterpret_cast<const uint8_t*>(p0);
#if SIMDPP_USE_NULL
    v = detail::null::make_vec<uint8x16>(*v0);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_MSA
    i_set_splat(v, *v0);
#elif SIMDPP_USE_NEON
    v = vld1q_dup_u8(v0);
#elif SIMDPP_USE_ALTIVEC
    v = altivec::load1_u(v, v0);
    v = splat<0>(v);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_load_splat(uint8x32& v, const char* p0)
{
    i_set_splat(v, *reinterpret_cast<const uint8_t*>(p0));
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL void i_load_splat(uint8<64>& v, const char* p0)
{
    i_set_splat(v, *reinterpret_cast<const uint8_t*>(p0));
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_load_splat(uint16x8& v, const char* p0)
{
    const uint16_t* v0 = reinterpret_cast<const uint16_t*>(p0);
#if SIMDPP_USE_NULL
    v = detail::null::make_vec<uint16x8>(*v0);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_MSA
    i_set_splat(v, *v0);
#elif SIMDPP_USE_NEON
    v = vld1q_dup_u16(v0);
#elif SIMDPP_USE_ALTIVEC
    v = altivec::load1_u(v, v0);
    v = splat<0>(v);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_load_splat(uint16x16& v, const char* p0)
{
    i_set_splat(v, *reinterpret_cast<const uint16_t*>(p0));
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL void i_load_splat(uint16<32>& v, const char* p0)
{
    i_set_splat(v, *reinterpret_cast<const uint16_t*>(p0));
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_load_splat(uint32x4& v, const char* p0)
{
    const uint32_t* v0 = reinterpret_cast<const uint32_t*>(p0);
#if SIMDPP_USE_NULL
    v = detail::null::make_vec<uint32x4>(*v0);
#elif SIMDPP_USE_SSE2
    v = _mm_cvtsi32_si128(*v0);
    v = permute4<0,0,0,0>(v);
#elif SIMDPP_USE_NEON
    v =  vld1q_dup_u32(v0);
#elif SIMDPP_USE_ALTIVEC
    v = altivec::load1_u(v, v0);
    v = splat<0>(v);
#elif SIMDPP_USE_MSA
    i_set_splat(v, *v0);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_load_splat(uint32x8& v, const char* p0)
{
    i_set_splat(v, *reinterpret_cast<const uint32_t*>(p0));
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_load_splat(uint32<16>& v, const char* p0)
{
    __m128 x = _mm_load_ss(reinterpret_cast<const float*>(p0));
    v = _mm512_broadcastd_epi32(_mm_castps_si128(x));
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_load_splat(uint64x2& v, const char* p0)
{
    const uint64_t* v0 = reinterpret_cast<const uint64_t*>(p0);
#if SIMDPP_USE_SSE2
    v = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(v0));
    v = permute2<0,0>(v);
#elif SIMDPP_USE_NEON
#if (__GNUC__ == 4) && (__GNUC_MINOR__ == 7)
    // BUG: GCC 4.7 loads only the first element
    uint64x1_t x = vld1_dup_u64(v0);
    v = vdupq_lane_u64(x, 0);
#else
    v = vld1q_dup_u64(v0);
#endif
#elif SIMDPP_USE_VSX_207
    v = load_u(v0);
    v = splat<0>(v);
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC
    v = detail::null::make_vec<uint64x2>(*v0);
#elif SIMDPP_USE_MSA
    i_set_splat(v, *v0);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_load_splat(uint64x4& v, const char* p0)
{
    __m128i x = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(p0));
    v = _mm256_broadcastq_epi64(x);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_load_splat(uint64<8>& v, const char* p0)
{
    __m128i x = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(p0));
    v = _mm512_broadcastq_epi64(x);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_load_splat(float32x4& v, const char* p0)
{
    const float* v0 = reinterpret_cast<const float*>(p0);
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    v = detail::null::make_vec<float32x4>(*v0);
#elif SIMDPP_USE_AVX
    v = _mm_broadcast_ss(v0);
#elif SIMDPP_USE_SSE2
    v = _mm_load_ss(v0);
    v = permute4<0,0,0,0>(v);
#elif SIMDPP_USE_NEON
    v = vld1q_dup_f32(v0);
#elif SIMDPP_USE_ALTIVEC
    v = altivec::load1_u(v, v0);
    v = splat<0>(v);
#elif SIMDPP_USE_MSA
    i_set_splat(v, *v0);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
void i_load_splat(float32x8& v, const char* p0)
{
    v = _mm256_broadcast_ss(reinterpret_cast<const float*>(p0));
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_load_splat(float32<16>& v, const char* p0)
{
    __m128 x = _mm_load_ss(reinterpret_cast<const float*>(p0));
    v = _mm512_broadcastss_ps(x);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_load_splat(float64x2& v, const char* p0)
{
    const double* v0 = reinterpret_cast<const double*>(p0);

#if SIMDPP_USE_SSE3
    v = _mm_loaddup_pd(v0);
#elif SIMDPP_USE_SSE2
    v = _mm_load_sd(v0);
    v = permute2<0,0>(v);
#elif SIMDPP_USE_NEON64
    v = vld1q_dup_f64(v0);
#elif SIMDPP_USE_VSX_206
    v = load_u(v0);
    v = splat<0>(v);
#elif SIMDPP_USE_MSA
    i_set_splat(v, *v0);
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC
    v = detail::null::make_vec<float64x2>(*v0);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
void i_load_splat(float64x4& v, const char* p0)
{
    v = _mm256_broadcast_sd(reinterpret_cast<const double*>(p0));
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_load_splat(float64<8>& v, const char* p0)
{
    __m128d x = _mm_load_sd(reinterpret_cast<const double*>(p0));
    v = _mm512_broadcastsd_pd(x);
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
void i_load_splat(V& v, const char* p0)
{
    typename V::base_vector_type tv;
    i_load_splat(tv, p0);
    for (unsigned i = 0; i < v.vec_length; ++i) {
        v.vec(i) = tv;
    }
}

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
V i_load_splat_any(const char* p)
{
    typename detail::remove_sign<V>::type r;
    i_load_splat(r, p);
    return V(r);
}

// -----------------------------------------------------------------------------
} // namespace insn

template<class V> SIMDPP_INL
void construct_eval(V& v, const expr_vec_load_splat& e)
{
    v = insn::i_load_splat_any<V>(e.a);
}

} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

