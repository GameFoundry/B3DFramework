/*  Copyright (C) 2011-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_INSERT_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_INSERT_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/cast.h>
#include <simdpp/core/move_l.h>
#include <simdpp/core/i_shift_l.h>
#include <simdpp/core/i_sub.h>
#include <simdpp/core/make_int.h>
#include <simdpp/detail/insn/split.h>
#include <simdpp/detail/mem_block.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

template<unsigned id> SIMDPP_INL
uint8x16 i_insert(const uint8x16& ca, uint8_t x)
{
    uint8<16> a = ca;
#if SIMDPP_USE_NULL
    a.el(id) = x;
    return a;
#elif SIMDPP_USE_SSE4_1
    return _mm_insert_epi8(a.native(), x, id);
#elif SIMDPP_USE_SSE2
    uint16_t r = _mm_extract_epi16(a.native(), id/2);
    if (id % 2 == 1) {
        r = (r & 0x00ff) | (x << 8);
    } else {
        r = (r & 0xff00) | x;
    }
    a = _mm_insert_epi16(a.native(), r, id/2);
    return a;
#elif SIMDPP_USE_NEON
    return vsetq_lane_u8(x, a.native(), id);
#elif SIMDPP_USE_ALTIVEC
    detail::mem_block<uint8x16> ax(a);
    ax[id] = x;
    a = ax;
    return a;
#elif SIMDPP_USE_MSA
    return (v16u8) __msa_insert_b((v16i8) a.native(), id, x);
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned id> SIMDPP_INL
uint8<32> i_insert(const uint8<32>& a, uint8_t x)
{
    __m256i val = a.native();
    __m128i val128 = _mm256_extracti128_si256(val, id / 16);
    val128 = _mm_insert_epi8(val128, x, id % 16);
    return _mm256_inserti128_si256(val, val128, id / 16);
}
#endif

#if SIMDPP_USE_AVX512BW
template<unsigned id> SIMDPP_INL
uint8<64> i_insert(const uint8<64>& a, uint8_t x)
{
    __m512i val = a.native();
    __m128i val128 = _mm512_extracti32x4_epi32(val, id / 16);
    val128 = _mm_insert_epi8(val128, x, id % 16);
    return _mm512_inserti32x4(val, val128, id / 16);
}
#endif

// -----------------------------------------------------------------------------

template<unsigned id> SIMDPP_INL
uint16x8 i_insert(const uint16x8& ca, uint16_t x)
{
    uint16<8> a = ca;
#if SIMDPP_USE_NULL
    a.el(id) = x;
    return a;
#elif SIMDPP_USE_SSE2
    return _mm_insert_epi16(a.native(), x, id);
#elif SIMDPP_USE_NEON
    return vsetq_lane_u16(x, a.native(), id);
#elif SIMDPP_USE_ALTIVEC
    detail::mem_block<uint16x8> ax(a);
    ax[id] = x;
    a = ax;
    return a;
#elif SIMDPP_USE_MSA
    return (v8u16) __msa_insert_h((v8i16) a.native(), id, x);
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned id> SIMDPP_INL
uint16<16> i_insert(const uint16<16>& a, uint16_t x)
{
    __m256i val = a.native();
    __m128i val128 = _mm256_extracti128_si256(val, id / 8);
    val128 = _mm_insert_epi16(val128, x, id % 8);
    return _mm256_inserti128_si256(val, val128, id / 8);
}
#endif

#if SIMDPP_USE_AVX512BW
template<unsigned id> SIMDPP_INL
uint16<32> i_insert(const uint16<32>& a, uint16_t x)
{
    __m512i val = a.native();
    __m128i val128 = _mm512_extracti32x4_epi32(val, id / 8);
    val128 = _mm_insert_epi16(val128, x, id % 8);
    return _mm512_inserti32x4(val, val128, id / 8);
}
#endif

// -----------------------------------------------------------------------------

template<unsigned id> SIMDPP_INL
uint32x4 i_insert(const uint32x4& ca, uint32_t x)
{
    uint32<4> a = ca;
#if SIMDPP_USE_NULL
    a.el(id) = x;
    return a;
#elif SIMDPP_USE_SSE4_1
    return _mm_insert_epi32(a.native(), x, id);
#elif SIMDPP_USE_SSE2
    uint16_t lo = x & 0xffff;
    uint16_t hi = x >> 16;
    uint16x8 a1 = uint16<8>(a);
    a1 = i_insert<id*2>(a1, lo);
    a1 = i_insert<id*2+1>(a1, hi);
    return uint32<4>(a1);
#elif SIMDPP_USE_NEON
    return vsetq_lane_u32(x, a.native(), id);
#elif SIMDPP_USE_ALTIVEC
    detail::mem_block<uint32x4> ax(a);
    ax[id] = x;
    a = ax;
    return a;
#elif SIMDPP_USE_MSA
    return (v4u32) __msa_insert_w((v4i32) a.native(), id, x);
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned id> SIMDPP_INL
uint32<8> i_insert(const uint32<8>& a, uint32_t x)
{
    __m256i val = a.native();
    __m128i val128 = _mm256_extracti128_si256(val, id / 4);
    val128 = _mm_insert_epi32(val128, x, id % 4);
    return _mm256_inserti128_si256(val, val128, id / 4);
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned id> SIMDPP_INL
uint32<16> i_insert(const uint32<16>& a, uint32_t x)
{
    __m512i val = a.native();
    __m128i val128 = _mm512_extracti32x4_epi32(val, id / 4);
    val128 = _mm_insert_epi32(val128, x, id % 4);
    return _mm512_inserti32x4(val, val128, id / 4);
}
#endif

// -----------------------------------------------------------------------------

template<unsigned id> SIMDPP_INL
uint64x2 i_insert(const uint64x2& ca, uint64_t x)
{
    uint64<2> a = ca;
#if SIMDPP_USE_NULL
    a.el(id) = x;
    return a;
#elif SIMDPP_USE_SSE4_1
#if SIMDPP_32_BITS
    uint32x4 a0 = (uint32x4) a;
    a0 = i_insert<id*2>(a0, uint32_t(x));
    a0 = i_insert<id*2+1>(a0, uint32_t(x >> 32));
    return (uint64x2) a0;
#else
    return _mm_insert_epi64(a.native(), x, id);
#endif
#elif SIMDPP_USE_SSE2
#if SIMDPP_32_BITS
    int32x4 va = _mm_cvtsi32_si128(uint32_t(x));
    int32x4 vb = _mm_cvtsi32_si128(uint32_t(x >> 32));
    int64x2 vx = (int64x2) zip4_lo(va, vb);
    if (id == 0) {
        a = shuffle1<0,1>(vx, a);
    } else {
        a = shuffle1<0,0>(a, vx);
    }
    return a;
#else
    int64x2 vx = _mm_cvtsi64_si128(x);
    if (id == 0) {
        a = shuffle1<0,1>(vx, a);
    } else {
        a = shuffle1<0,0>(a, vx);
    }
    return a;
#endif
#elif SIMDPP_USE_NEON
    return vsetq_lane_u64(x, a.native(), id);
#elif SIMDPP_USE_ALTIVEC
    detail::mem_block<uint64x2> ax(a);
    ax[id] = x;
    a = ax;
    return a;
#elif SIMDPP_USE_MSA
#if SIMDPP_64_BITS
    return (v2u64) __msa_insert_d((v2i64) a.native(), id, x);
#else
    int32<4> a32;
    a32 = a;
    a32 = __msa_insert_w(a32.native(), id*2, x);
    a32 = __msa_insert_w(a32.native(), id*2+1, x >> 32);
    return (uint64<2>) a32;
#endif
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned id> SIMDPP_INL
uint64<4> i_insert(const uint64<4>& a, uint64_t x)
{
    __m256i val = a.native();
    uint64<2> val128 = _mm256_extracti128_si256(val, id / 2);
    val128 = i_insert<id % 2>(val128, x);
    return _mm256_inserti128_si256(val, val128.native(), id / 2);
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned id> SIMDPP_INL
uint64<8> i_insert(const uint64<8>& a, uint64_t x)
{
    __m512i val = a.native();
    uint64<2> val128 = _mm512_extracti32x4_epi32(val, id / 2);
    val128 = i_insert<id % 2>(val128, x);
    return _mm512_inserti32x4(val, val128.native(), id / 2);
}
#endif

// -----------------------------------------------------------------------------

template<unsigned id> SIMDPP_INL
float32x4 i_insert(const float32x4& a, float x)
{
#if SIMDPP_USE_NEON_FLT_SP
    return vsetq_lane_f32(x, a.native(), id);
#else
    return float32<4>(i_insert<id>(uint32<4>(a), bit_cast<uint32_t>(x)));
#endif
}

#if SIMDPP_USE_AVX
template<unsigned id> SIMDPP_INL
float32<8> i_insert(const float32<8>& a, float x)
{
    __m256 val = a.native();
    float32<4> val128 = _mm256_extractf128_ps(val, id / 4);
    val128 = i_insert<id % 4>(val128, x);
    return _mm256_insertf128_ps(val, val128.native(), id / 4);
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned id> SIMDPP_INL
float32<16> i_insert(const float32<16>& a, float x)
{
    __m512 val = a.native();
    float32<4> val128 = _mm512_extractf32x4_ps(val, id / 4);
    val128 = i_insert<id % 4>(val128, x);
    return _mm512_insertf32x4(val, val128.native(), id / 4);
}
#endif

// -----------------------------------------------------------------------------

template<unsigned id> SIMDPP_INL
float64x2 i_insert(const float64x2& a, double x)
{
    return float64<2>(i_insert<id>(uint64<2>(a), bit_cast<int64_t>(x)));
}

#if SIMDPP_USE_AVX
template<unsigned id> SIMDPP_INL
float64<4> i_insert(const float64<4>& a, double x)
{
    __m256d val = a.native();
    float64<2> val128 = _mm256_extractf128_pd(val, id / 2);
    val128 = i_insert<id % 2>(val128, x);
    return _mm256_insertf128_pd(val, val128.native(), id / 2);
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned id> SIMDPP_INL
float64<8> i_insert(const float64<8>& a, double x)
{
    __m512 val = _mm512_castpd_ps(a.native());
    float64<2> val128 = _mm_castps_pd(_mm512_extractf32x4_ps(val, id / 2));
    val128 = i_insert<id % 2>(val128, x);
    return _mm512_castps_pd(_mm512_insertf32x4(val, _mm_castpd_ps(val128.native()), id / 2));
}
#endif

// -----------------------------------------------------------------------------

template<unsigned id, class V, class E> SIMDPP_INL
V i_insert(const V& ca, E el)
{
    V a = ca;
    typename V::base_vector_type base = a.vec(id / V::base_length);
    base = i_insert<id % V::base_length>(base, (typename V::element_type) el);
    a.vec(id / V::base_length) = base;
    return a;
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
