/*  Copyright (C) 2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_SHUFFLE2x2_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_SHUFFLE2x2_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/make_shuffle_bytes_mask.h>
#include <simdpp/core/shuffle_bytes16.h>
#include <simdpp/detail/not_implemented.h>
#include <simdpp/detail/shuffle/neon_int32x4.h>
#include <simdpp/detail/shuffle/neon_int64x2.h>
#include <simdpp/detail/shuffle/shuffle_mask.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {


// -----------------------------------------------------------------------------
// float32

template<unsigned s0, unsigned s1> SIMDPP_INL
float32<4> i_shuffle2x2(const float32<4>& a, const float32<4>& b)
{
    static_assert(s0 < 4 && s1 < 4, "Selector out of range");
#if SIMDPP_USE_NULL
    float32<4> r;
    r.el(0) = s0 < 2 ? a.el(s0) : b.el(s0-2);
    r.el(1) = s1 < 2 ? a.el(s1) : b.el(s1-2);
    r.el(2) = s0 < 2 ? a.el(s0+2) : b.el(s0);
    r.el(3) = s1 < 2 ? a.el(s1+2) : b.el(s1);
    return r;
#elif SIMDPP_USE_SSE2
    if (s0 < 2 && s1 < 2) {
        return _mm_shuffle_ps(a.native(), a.native(),
                              SIMDPP_SHUFFLE_MASK_4x4(s0,s1,s0+2,s1+2));
    } else if (s0 >= 2 && s1 >= 2) {
        return _mm_shuffle_ps(b.native(), b.native(),
                              SIMDPP_SHUFFLE_MASK_4x4(s0-2,s1-2,s0,s1));
#if SIMDPP_USE_SSE4_1
    } else if (s0 == 0 && s1 == 3) {
        return _mm_blend_ps(a.native(), b.native(), 0xa);
    } else if (s0 == 2 && s1 == 1) {
        return _mm_blend_ps(b.native(), a.native(), 0xa);
#endif
    } else if (s0 < 2) { // s1 >= 2
        __m128 t = _mm_shuffle_ps(a.native(), b.native(),
                                  SIMDPP_SHUFFLE_MASK_4x4(s0,s0+1,s1-2,s1-1));
        return _mm_shuffle_ps(t, t, _MM_SHUFFLE(3,1,2,0));
    } else { // s0 >= 2, s1 < 2
        __m128 t = _mm_shuffle_ps(b.native(), a.native(),
                                  SIMDPP_SHUFFLE_MASK_4x4(s1,s1+1,s0-2,s0-1));
        return _mm_shuffle_ps(t, t, _MM_SHUFFLE(3,1,2,0));
    }
#elif SIMDPP_USE_NEON
    return (float32<4>) detail::neon_shuffle_int32x4::shuffle2x2<s0,s1>(float32<4>(a), float32<4>(b));
#elif SIMDPP_USE_ALTIVEC
    uint32<4> mask = make_shuffle_bytes16_mask<s0, s1>(mask);
    return shuffle_bytes16(a, b, mask);
#elif SIMDPP_USE_MSA
    const unsigned q0 = s0 < 2 ? s0 : s0 + 2;
    const unsigned q1 = s1 < 2 ? s1 : s1 + 2;
    uint32<4> mask = make_uint(q0,q1,q0+2,q1+2);
    return (v4f32) __msa_vshf_w((v4i32)mask.native(),
                                (v4i32)b.native(),
                                (v4i32)a.native());
#else
    return SIMDPP_NOT_IMPLEMENTED_TEMPLATE2(int64<s0+4>, a, b);
#endif
}

#if SIMDPP_USE_AVX
template<unsigned s0, unsigned s1> SIMDPP_INL
float32<8> i_shuffle2x2(const float32<8>& a, const float32<8>& b)
{
    static_assert(s0 < 4 && s1 < 4, "Selector out of range");
    if (s0 < 2 && s1 < 2) {
        return _mm256_permute_ps(a.native(), SIMDPP_SHUFFLE_MASK_4x4(s0,s1,s0+2,s1+2));
    } else if (s0 >= 2 && s1 >= 2) {
        return _mm256_permute_ps(b.native(), SIMDPP_SHUFFLE_MASK_4x4(s0-2,s1-2,s0,s1));
    } else if (s0 == 0 && s1 == 3) {
        return _mm256_blend_ps(a.native(), b.native(), 0xaa);
    } else if (s0 == 2 && s1 == 1) {
        return _mm256_blend_ps(b.native(), a.native(), 0xaa);
    } else if (s0 < 2) { // s1 >= 2
        __m256 t = _mm256_shuffle_ps(a.native(), b.native(),
                                     SIMDPP_SHUFFLE_MASK_4x4(s0,s0+1,s1-2,s1-1));
        return _mm256_permute_ps(t, _MM_SHUFFLE(3,1,2,0));
    } else { // s0 >= 2, s1 < 2
        __m256 t = _mm256_shuffle_ps(b.native(), a.native(),
                                     SIMDPP_SHUFFLE_MASK_4x4(s1,s1+1,s0-2,s0-1));
        return _mm256_permute_ps(t, _MM_SHUFFLE(3,1,2,0));
    }
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned s0, unsigned s1> SIMDPP_INL
float32<16> i_shuffle2x2(const float32<16>& a, const float32<16>& b)
{
    static_assert(s0 < 4 && s1 < 4, "Selector out of range");
    if (s0 < 2 && s1 < 2) {
        return _mm512_permute_ps(a.native(), SIMDPP_SHUFFLE_MASK_4x4(s0,s1,s0+2,s1+2));
    } else if (s0 >= 2 && s1 >= 2) {
        return _mm512_permute_ps(b.native(), SIMDPP_SHUFFLE_MASK_4x4(s0-2,s1-2,s0,s1));
    } else if (s0 == 0 && s1 == 3) {
        return _mm512_mask_blend_ps(0xaaaa, a.native(), b.native());
    } else if (s0 == 2 && s1 == 1) {
        return _mm512_mask_blend_ps(0xaaaa, b.native(), a.native());
    } else if (s0 < 2) { // s1 >= 2
        __m512 t = _mm512_shuffle_ps(a.native(), b.native(),
                                     SIMDPP_SHUFFLE_MASK_4x4(s0,s0+1,s1-2,s1-1));
        return _mm512_permute_ps(t, _MM_SHUFFLE(3,1,2,0));
    } else { // s0 >= 2, s1 < 2
        __m512 t = _mm512_shuffle_ps(b.native(), a.native(),
                                     SIMDPP_SHUFFLE_MASK_4x4(s1,s1+1,s0-2,s0-1));
        return _mm512_permute_ps(t, _MM_SHUFFLE(3,1,2,0));
    }
}
#endif

template<unsigned s0, unsigned s1, unsigned N> SIMDPP_INL
float32<N> i_shuffle2x2(const float32<N>& a, const float32<N>& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(float32<N>, (i_shuffle2x2<s0,s1>), a, b);
}

// float64


template<unsigned s0, unsigned s1> SIMDPP_INL
float64<2> i_shuffle2x2(const float64<2>& a, const float64<2>& b)
{
    static_assert(s0 < 4 && s1 < 4, "Selector out of range");
#if SIMDPP_USE_SSE2
    if (s0 < 2 && s1 < 2) {
        return _mm_shuffle_pd(a.native(), a.native(),
                              SIMDPP_SHUFFLE_MASK_2x2(s0, s1));
    } else if (s0 >= 2 && s1 >= 2) {
        return _mm_shuffle_pd(b.native(), b.native(),
                              SIMDPP_SHUFFLE_MASK_2x2(s0-2,s1-2));
#if SIMDPP_USE_SSE4_1
    } else if (s0 == 0 && s1 == 3) {
        return _mm_blend_pd(a.native(), b.native(), 0x2);
    } else if (s0 == 2 && s1 == 1) {
        return _mm_blend_pd(b.native(), a.native(), 0x2);
#endif
    } else if (s0 < 2) { // s1 >= 2
        return _mm_shuffle_pd(a.native(), b.native(),
                              SIMDPP_SHUFFLE_MASK_2x2(s0, s1-2));
    } else { // s0 >= 2, s1 < 2
        return _mm_shuffle_pd(b.native(), a.native(),
                              SIMDPP_SHUFFLE_MASK_2x2(s1, s0-2));
    }
#elif SIMDPP_USE_NEON64
    return (float64<2>)detail::neon_shuffle_int64x2::shuffle2x2<s0, s1>(uint64<2>(a), uint64<2>(b));
#elif SIMDPP_USE_VSX_206
    __vector double da = a.native(), db = b.native();
    if (s0 < 2 && s1 < 2) {
        return vec_xxpermdi(da, da, SIMDPP_VSX_SHUFFLE_MASK_2x2(s0, s1));
    } else if (s0 >= 2 && s1 >= 2) {
        return vec_xxpermdi(db, db, SIMDPP_VSX_SHUFFLE_MASK_2x2(s0-2,s1-2));
    } else if (s0 < 2) { // s1 >= 2
        return vec_xxpermdi(da, db, SIMDPP_VSX_SHUFFLE_MASK_2x2(s0, s1-2));
    } else { // s0 >= 2, s1 < 2
        return vec_xxpermdi(db, da, SIMDPP_VSX_SHUFFLE_MASK_2x2(s1, s0-2));
    }
#elif SIMDPP_USE_MSA
    uint64<2> mask = make_uint(s0, s1);
    return (v2f64) __msa_vshf_d((v2i64)mask.native(),
                                (v2i64)b.native(),
                                (v2i64)a.native());
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC
    float64<2> r;
    r.el(0) = s0 < 2 ? a.el(s0) : b.el(s0-2);
    r.el(1) = s1 < 2 ? a.el(s1) : b.el(s1-2);
    return r;
#else
    return SIMDPP_NOT_IMPLEMENTED_TEMPLATE2(int64<s0+4>, a, b);
#endif
}

#if SIMDPP_USE_AVX
template<unsigned s0, unsigned s1> SIMDPP_INL
float64<4> i_shuffle2x2(const float64<4>& a, const float64<4>& b)
{
    static_assert(s0 < 4 && s1 < 4, "Selector out of range");
    if (s0 < 2 && s1 < 2) {
        return _mm256_shuffle_pd(a.native(), a.native(),
                                 SIMDPP_SHUFFLE_MASK_2x2_2(s0,s1));
    } else if (s0 >= 2 && s1 >= 2) {
        return _mm256_shuffle_pd(b.native(), b.native(),
                                 SIMDPP_SHUFFLE_MASK_2x2_2(s0-2,s1-2));
    } else if (s0 == 0 && s1 == 3) {
        return _mm256_blend_pd(a.native(), b.native(), 0xa);
    } else if (s0 == 2 && s1 == 1) {
        return _mm256_blend_pd(b.native(), a.native(), 0xa);
    } else if (s0 < 2) { // s1 >= 2
        return _mm256_shuffle_pd(a.native(), b.native(),
                                 SIMDPP_SHUFFLE_MASK_2x2_2(s0,s1-2));
    } else { // s0 >= 2, s1 < 2
        return _mm256_shuffle_pd(b.native(), a.native(),
                                 SIMDPP_SHUFFLE_MASK_2x2_2(s1,s0-2));
    }
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned s0, unsigned s1> SIMDPP_INL
float64<8> i_shuffle2x2(const float64<8>& a, const float64<8>& b)
{
    static_assert(s0 < 4 && s1 < 4, "Selector out of range");
    if (s0 < 2 && s1 < 2) {
        return _mm512_shuffle_pd(a.native(), a.native(),
                                 SIMDPP_SHUFFLE_MASK_2x2_4(s0,s1));
    } else if (s0 >= 2 && s1 >= 2) {
        return _mm512_shuffle_pd(b.native(), b.native(),
                                 SIMDPP_SHUFFLE_MASK_2x2_4(s0-2,s1-2));
    } else if (s0 == 0 && s1 == 3) {
        return _mm512_mask_blend_pd(0xaa, a.native(), b.native());
    } else if (s0 == 2 && s1 == 1) {
        return _mm512_mask_blend_pd(0xaa, b.native(), a.native());
    } else if (s0 < 2) { // s1 >= 2
        return _mm512_shuffle_pd(a.native(), b.native(),
                                 SIMDPP_SHUFFLE_MASK_2x2_4(s0,s1-2));
    } else { // s0 >= 2, s1 < 2
        return _mm512_shuffle_pd(b.native(), a.native(),
                                 SIMDPP_SHUFFLE_MASK_2x2_4(s1,s0-2));
    }
}
#endif

template<unsigned s0, unsigned s1, unsigned N> SIMDPP_INL
float64<N> i_shuffle2x2(const float64<N>& a, const float64<N>& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(float64<N>, (i_shuffle2x2<s0,s1>), a, b);
}

// int32

template<unsigned s0, unsigned s1> SIMDPP_INL
uint32<4> i_shuffle2x2(const uint32<4>& a, const uint32<4>& b)
{
    static_assert(s0 < 4 && s1 < 4, "Selector out of range");
#if SIMDPP_USE_NULL
    uint32<4> r;
    r.el(0) = s0 < 2 ? a.el(s0) : b.el(s0-2);
    r.el(1) = s1 < 2 ? a.el(s1) : b.el(s1-2);
    r.el(2) = s0 < 2 ? a.el(s0+2) : b.el(s0);
    r.el(3) = s1 < 2 ? a.el(s1+2) : b.el(s1);
    return r;
#elif SIMDPP_USE_SSE2
    if (s0 < 2 && s1 < 2) {
        const unsigned mask = SIMDPP_SHUFFLE_MASK_4x4(s0,s1,s0+2,s1+2);
        return _mm_shuffle_epi32(a.native(), mask);
    } else if (s0 >= 2 && s1 >= 2) {
        const unsigned mask = SIMDPP_SHUFFLE_MASK_4x4(s0-2,s1-2,s0,s1);
        return _mm_shuffle_epi32(b.native(), mask);
#if SIMDPP_USE_SSE4_1
    } else if (s0 == 0 && s1 == 3) {
        return _mm_blend_epi16(a.native(), b.native(), 0xcc);
    } else if (s0 == 2 && s1 == 1) {
        return _mm_blend_epi16(b.native(), a.native(), 0xcc);
#endif
    } else if (s0 < 2) { // s1 >= 2
        float32<4> fa, fb; fa = a, fb = b;
        __m128 t = _mm_shuffle_ps(fa.native(), fb.native(),
                                  SIMDPP_SHUFFLE_MASK_4x4(s0,s0+1,s1-2,s1-1));
        return _mm_castps_si128(_mm_shuffle_ps(t, t, _MM_SHUFFLE(3,1,2,0)));
    } else { // s0 >= 2, s1 < 2
        float32<4> fa, fb; fa = a, fb = b;
        __m128 t = _mm_shuffle_ps(fb.native(), fa.native(),
                                  SIMDPP_SHUFFLE_MASK_4x4(s1,s1+1,s0-2,s0-1));
        return _mm_castps_si128(_mm_shuffle_ps(t, t, _MM_SHUFFLE(3,1,2,0)));
    }
#elif SIMDPP_USE_NEON
    return detail::neon_shuffle_int32x4::shuffle2x2<s0,s1>(a, b);
#elif SIMDPP_USE_ALTIVEC
    uint32<4> mask = make_shuffle_bytes16_mask<s0, s1>(mask);
    return shuffle_bytes16(a, b, mask);
#elif SIMDPP_USE_MSA
    const unsigned q0 = s0 < 2 ? s0 : s0 + 2;
    const unsigned q1 = s1 < 2 ? s1 : s1 + 2;
    uint32<4> mask = make_uint(q0,q1,q0+2,q1+2);

    return (v4u32) __msa_vshf_w((v4i32)(v4u32)mask,
                                (v4i32)(v4u32)b,
                                (v4i32)(v4u32)a);
#else
    return SIMDPP_NOT_IMPLEMENTED_TEMPLATE2(int64<s0+4>, a, b);
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned s0, unsigned s1> SIMDPP_INL
uint32<8> i_shuffle2x2(const uint32<8>& a, const uint32<8>& b)
{
    static_assert(s0 < 4 && s1 < 4, "Selector out of range");
    if (s0 < 2 && s1 < 2) {
        const unsigned mask = SIMDPP_SHUFFLE_MASK_4x4(s0,s1,s0+2,s1+2);
        return _mm256_shuffle_epi32(a.native(), mask);
    } else if (s0 >= 2 && s1 >= 2) {
        const unsigned mask = SIMDPP_SHUFFLE_MASK_4x4(s0-2,s1-2,s0,s1);
        return _mm256_shuffle_epi32(b.native(), mask);
    } else if (s0 == 0 && s1 == 3) {
        return _mm256_blend_epi32(a.native(), b.native(), 0xa);
    } else if (s0 == 2 && s1 == 1) {
        return _mm256_blend_epi32(b.native(), a.native(), 0xa);
    } else if (s0 < 2) { // s1 >= 2
        float32<8> fa, fb; fa = a, fb = b;
        __m256 t = _mm256_shuffle_ps(fa.native(), fb.native(),
                                     SIMDPP_SHUFFLE_MASK_4x4(s0,s0+1,s1-2,s1-1));
        return _mm256_castps_si256(_mm256_permute_ps(t, _MM_SHUFFLE(3,1,2,0)));
    } else { // s0 >= 2, s1 < 2
        float32<8> fa, fb; fa = a, fb = b;
        __m256 t = _mm256_shuffle_ps(fb.native(), fa.native(),
                                     SIMDPP_SHUFFLE_MASK_4x4(s1,s1+1,s0-2,s0-1));
        return _mm256_castps_si256(_mm256_permute_ps(t, _MM_SHUFFLE(3,1,2,0)));
    }
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned s0, unsigned s1> SIMDPP_INL
uint32<16> i_shuffle2x2(const uint32<16>& a, const uint32<16>& b)
{
    static_assert(s0 < 4 && s1 < 4, "Selector out of range");
    if (s0 < 2 && s1 < 2) {
        const unsigned mask = SIMDPP_SHUFFLE_MASK_4x4(s0,s1,s0+2,s1+2);
        return _mm512_shuffle_epi32(a.native(), _MM_PERM_ENUM(mask));
    } else if (s0 >= 2 && s1 >= 2) {
        const unsigned mask = SIMDPP_SHUFFLE_MASK_4x4(s0-2,s1-2,s0,s1);
        return _mm512_shuffle_epi32(b.native(), _MM_PERM_ENUM(mask));
    } else if (s0 == 0 && s1 == 3) {
        return _mm512_mask_blend_epi32(0xaaaa, a.native(), b.native());
    } else if (s0 == 2 && s1 == 1) {
        return _mm512_mask_blend_epi32(0xaaaa, b.native(), a.native());
    } else if (s0 < 2) { // s1 >= 2
        float32<16> fa, fb; fa = a; fb = b;
        __m512 t = _mm512_shuffle_ps(fa.native(), fb.native(),
                                     SIMDPP_SHUFFLE_MASK_4x4(s0,s0+1,s1-2,s1-1));
        return _mm512_castps_si512(_mm512_permute_ps(t, _MM_SHUFFLE(3,1,2,0)));
    } else { // s0 >= 2, s1 < 2
        float32<16> fa, fb; fa = a; fb = b;
        __m512 t = _mm512_shuffle_ps(fb.native(), fa.native(),
                                     SIMDPP_SHUFFLE_MASK_4x4(s1,s1+1,s0-2,s0-1));
        return _mm512_castps_si512(_mm512_permute_ps(t, _MM_SHUFFLE(3,1,2,0)));
    }
}
#endif

template<unsigned s0, unsigned s1, unsigned N> SIMDPP_INL
uint32<N> i_shuffle2x2(const uint32<N>& a, const uint32<N>& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(uint32<N>, (i_shuffle2x2<s0,s1>), a, b);
}

// int64

template<unsigned s0, unsigned s1> SIMDPP_INL
uint64<2> i_shuffle2x2(const uint64<2>& a, const uint64<2>& b)
{
    static_assert(s0 < 4 && s1 < 4, "Selector out of range");
#if SIMDPP_USE_SSE2
    if (s0 < 2 && s1 < 2) {
        const unsigned mask = SIMDPP_SHUFFLE_MASK_4x4(s0*2, s0*2+1, s1*2, s1*2+1);
        return _mm_shuffle_epi32(a.native(), mask);
    } else if (s0 >= 2 && s1 >= 2) {
        const unsigned mask = SIMDPP_SHUFFLE_MASK_4x4(s0*2, s0*2+1, s1*2, s1*2+1);
        return _mm_shuffle_epi32(b.native(), mask);
    } else if (s0 == 0 && s1 == 2) {
        return _mm_unpacklo_epi64(a.native(), b.native());
    } else if (s0 == 2 && s1 == 0) {
        return _mm_unpacklo_epi64(b.native(), a.native());
    } else if (s0 == 1 && s1 == 3) {
        return _mm_unpackhi_epi64(a.native(), b.native());
    } else if (s0 == 3 && s1 == 1) {
        return _mm_unpackhi_epi64(b.native(), a.native());
#if SIMDPP_USE_SSE4_1
    } else if (s0 == 0 && s1 == 3) {
        return _mm_blend_epi16(a.native(), b.native(), 0xf0);
    } else if (s0 == 2 && s1 == 1) {
        return _mm_blend_epi16(b.native(), a.native(), 0xf0);
#endif
#if SIMDPP_USE_SSSE3
    } else if (s0 == 1 && s1 == 2) {
        return _mm_alignr_epi8(b.native(), a.native(), 8);
    } else if (s0 == 3 && s1 == 0) {
        return _mm_alignr_epi8(a.native(), b.native(), 8);
#endif
    } else if (s0 < 2) { // s1 >= 2
        __m128d na = _mm_castsi128_pd(a.native());
        __m128d nb = _mm_castsi128_pd(b.native());
        __m128d res = _mm_shuffle_pd(na, nb, SIMDPP_SHUFFLE_MASK_2x2(s0, s1-2));
        return _mm_castpd_si128(res);
    } else { // s0 >= 2, s1 < 2
        __m128d na = _mm_castsi128_pd(a.native());
        __m128d nb = _mm_castsi128_pd(b.native());
        __m128d res = _mm_shuffle_pd(nb, na, SIMDPP_SHUFFLE_MASK_2x2(s1, s0-2));
        return _mm_castpd_si128(res);
    }
#elif SIMDPP_USE_NEON
    return detail::neon_shuffle_int64x2::shuffle2x2<s0, s1>(a, b);
#elif SIMDPP_USE_VSX_207
    __vector uint64_t da = a.native(), db = b.native();
    if (s0 < 2 && s1 < 2) {
        return vec_xxpermdi(da, da, SIMDPP_VSX_SHUFFLE_MASK_2x2(s0, s1));
    } else if (s0 >= 2 && s1 >= 2) {
        return vec_xxpermdi(db, db, SIMDPP_VSX_SHUFFLE_MASK_2x2(s0-2,s1-2));
    } else if (s0 < 2) { // s1 >= 2
        return vec_xxpermdi(da, db, SIMDPP_VSX_SHUFFLE_MASK_2x2(s0, s1-2));
    } else { // s0 >= 2, s1 < 2
        return vec_xxpermdi(db, da, SIMDPP_VSX_SHUFFLE_MASK_2x2(s1, s0-2));
    }
#elif SIMDPP_USE_MSA
    uint64<2> mask = make_uint(s0, s1);

    return (v2u64) __msa_vshf_d((v2i64)mask.native(),
                                (v2i64)b.native(),
                                (v2i64)a.native());
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC
    uint64<2> r;
    r.el(0) = s0 < 2 ? a.el(s0) : b.el(s0-2);
    r.el(1) = s1 < 2 ? a.el(s1) : b.el(s1-2);
    return r;
#else
    return SIMDPP_NOT_IMPLEMENTED_TEMPLATE2(int64<s0+4>, a, b);
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned s0, unsigned s1> SIMDPP_INL
uint64<4> i_shuffle2x2(const uint64<4>& a, const uint64<4>& b)
{
    static_assert(s0 < 4 && s1 < 4, "Selector out of range");
    if (s0 < 2 && s1 < 2) {
        const unsigned mask = SIMDPP_SHUFFLE_MASK_4x4(s0*2, s0*2+1, s1*2, s1*2+1);
        return _mm256_shuffle_epi32(a.native(), mask);
    } else if (s0 >= 2 && s1 >= 2) {
        const unsigned mask = SIMDPP_SHUFFLE_MASK_4x4(s0*2, s0*2+1, s1*2, s1*2+1);
        return _mm256_shuffle_epi32(b.native(), mask);
    } else if (s0 == 0 && s1 == 2) {
        return _mm256_unpacklo_epi64(a.native(), b.native());
    } else if (s0 == 2 && s1 == 0) {
        return _mm256_unpacklo_epi64(b.native(), a.native());
    } else if (s0 == 1 && s1 == 3) {
        return _mm256_unpackhi_epi64(a.native(), b.native());
    } else if (s0 == 3 && s1 == 1) {
        return _mm256_unpackhi_epi64(b.native(), a.native());
    } else if (s0 == 0 && s1 == 3) {
        return _mm256_blend_epi32(a.native(), b.native(), 0xcc);
    } else if (s0 == 2 && s1 == 1) {
        return _mm256_blend_epi32(b.native(), a.native(), 0xcc);
    } else if (s0 == 1 && s1 == 2) {
        return _mm256_alignr_epi8(b.native(), a.native(), 8);
    } else { // if (s0 == 3 && s1 == 0)
        return _mm256_alignr_epi8(a.native(), b.native(), 8);
    }
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned s0, unsigned s1> SIMDPP_INL
uint64<8> i_shuffle2x2(const uint64<8>& a, const uint64<8>& b)
{
    static_assert(s0 < 4 && s1 < 4, "Selector out of range");
    if (s0 < 2 && s1 < 2) {
        const unsigned mask = SIMDPP_SHUFFLE_MASK_4x4(s0,s1,s0+2,s1+2);
        return _mm512_permutex_epi64(a.native(), mask);
    } else if (s0 >= 2 && s1 >= 2) {
        const unsigned mask = SIMDPP_SHUFFLE_MASK_4x4(s0-2,s1-2,s0,s1);
        return _mm512_permutex_epi64(b.native(), mask);
    } else if (s0 == 0 && s1 == 2) {
        return _mm512_unpacklo_epi64(a.native(), b.native());
    } else if (s0 == 2 && s1 == 0) {
        return _mm512_unpacklo_epi64(b.native(), a.native());
    } else if (s0 == 1 && s1 == 3) {
        return _mm512_unpackhi_epi64(a.native(), b.native());
    } else if (s0 == 3 && s1 == 1) {
        return _mm512_unpackhi_epi64(b.native(), a.native());
    } else if (s0 == 0 && s1 == 3) {
        return _mm512_mask_blend_epi64(0xaa, a.native(), b.native());
    } else if (s0 == 2 && s1 == 1) {
        return _mm512_mask_blend_epi64(0xaa, b.native(), a.native());
    } else if (s0 < 2) { // s1 >= 2
        __m512d na = _mm512_castsi512_pd(a.native());
        __m512d nb = _mm512_castsi512_pd(b.native());
        __m512d res = _mm512_shuffle_pd(na, nb, SIMDPP_SHUFFLE_MASK_2x2_4(s0, s1-2));
        return _mm512_castpd_si512(res);
    } else { // s0 >= 2, s1 < 2
        __m512d na = _mm512_castsi512_pd(a.native());
        __m512d nb = _mm512_castsi512_pd(b.native());
        __m512d res = _mm512_shuffle_pd(nb, na, SIMDPP_SHUFFLE_MASK_2x2_4(s1, s0-2));
        return _mm512_castpd_si512(res);
    }
    /* GCC BUG
    } else if (s0 == 1 && s1 == 2) {
        return _mm512_alignr_epi8(b.native(), a.native(), 8);
    } else if (s0 == 3 && s1 == 0) {
        return _mm512_alignr_epi8(a.native(), b.native(), 8);
    }*/
}
#endif

template<unsigned s0, unsigned s1, unsigned N> SIMDPP_INL
uint64<N> i_shuffle2x2(const uint64<N>& a, const uint64<N>& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(uint64<N>, (i_shuffle2x2<s0,s1>), a, b);
}


} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

