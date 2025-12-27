/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_I_MIN_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_I_MIN_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/blend.h>
#include <simdpp/core/cmp_lt.h>
#include <simdpp/core/bit_xor.h>
#include <simdpp/detail/not_implemented.h>
#include <simdpp/detail/null/math.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {


static SIMDPP_INL
int8x16 i_min(const int8x16& a, const int8x16& b)
{
#if SIMDPP_USE_NULL
    return detail::null::min(a, b);
#elif SIMDPP_USE_SSE4_1
    return _mm_min_epi8(a.native(), b.native());
#elif SIMDPP_USE_SSE2
    int8x16 ca = bit_xor(a, 0x80);
    int8x16 cb = bit_xor(b, 0x80);
    int8x16 r = _mm_min_epu8(ca.native(), cb.native());
    return bit_xor(r, 0x80);
#elif SIMDPP_USE_NEON
    return vminq_s8(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_min(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_min_s_b(a.native(), b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int8x32 i_min(const int8x32& a, const int8x32& b)
{
    return _mm256_min_epi8(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL int8<64> i_min(const int8<64>& a, const int8<64>& b)
{
    return _mm512_min_epi8(a.native(), b.native());
}
#endif

template<unsigned N> SIMDPP_INL
int8<N> i_min(const int8<N>& a, const int8<N>& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(int8<N>, i_min, a, b);
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint8x16 i_min(const uint8x16& a, const uint8x16& b)
{
#if SIMDPP_USE_NULL
    return detail::null::min(a, b);
#elif SIMDPP_USE_SSE2
    return _mm_min_epu8(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vminq_u8(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_min(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_min_u_b(a.native(), b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint8x32 i_min(const uint8x32& a, const uint8x32& b)
{
    return _mm256_min_epu8(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint8<64> i_min(const uint8<64>& a, const uint8<64>& b)
{
    return _mm512_min_epu8(a.native(), b.native());
}
#endif

template<unsigned N> SIMDPP_INL
uint8<N> i_min(const uint8<N>& a, const uint8<N>& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(uint8<N>, i_min, a, b);
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
int16x8 i_min(const int16x8& a, const int16x8& b)
{
#if SIMDPP_USE_NULL
    return detail::null::min(a, b);
#elif SIMDPP_USE_SSE2
    return _mm_min_epi16(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vminq_s16(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_min(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_min_s_h(a.native(), b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int16x16 i_min(const int16x16& a, const int16x16& b)
{
    return _mm256_min_epi16(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL int16<32> i_min(const int16<32>& a, const int16<32>& b)
{
    return _mm512_min_epi16(a.native(), b.native());
}
#endif

template<unsigned N> SIMDPP_INL
int16<N> i_min(const int16<N>& a, const int16<N>& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(int16<N>, i_min, a, b);
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint16x8 i_min(const uint16x8& a, const uint16x8& b)
{
#if SIMDPP_USE_NULL
    return detail::null::min(a, b);
#elif SIMDPP_USE_SSE4_1
    return _mm_min_epu16(a.native(), b.native());
#elif SIMDPP_USE_SSE2
    int16x8 ca = bit_xor(a, 0x8000);
    int16x8 cb = bit_xor(b, 0x8000);
    int16x8 r = _mm_min_epi16(ca.native(), cb.native());
    return bit_xor(r, 0x8000);
#elif SIMDPP_USE_NEON
    return vminq_u16(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_min(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_min_u_h(a.native(), b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint16x16 i_min(const uint16x16& a, const uint16x16& b)
{
    return _mm256_min_epu16(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint16<32> i_min(const uint16<32>& a, const uint16<32>& b)
{
    return _mm512_min_epu16(a.native(), b.native());
}
#endif

template<unsigned N> SIMDPP_INL
uint16<N> i_min(const uint16<N>& a, const uint16<N>& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(uint16<N>, i_min, a, b);
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
int32x4 i_min(const int32x4& a, const int32x4& b)
{
#if SIMDPP_USE_NULL
    return detail::null::min(a, b);
#elif SIMDPP_USE_SSE4_1
    return _mm_min_epi32(a.native(), b.native());
#elif SIMDPP_USE_SSE2
    mask_int32x4 mask = cmp_lt(a, b);
    return blend(a, b, mask);
#elif SIMDPP_USE_NEON
    return vminq_s32(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_min(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_min_s_w(a.native(), b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int32x8 i_min(const int32x8& a, const int32x8& b)
{
    return _mm256_min_epi32(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
int32<16> i_min(const int32<16>& a, const int32<16>& b)
{
    return _mm512_min_epi32(a.native(), b.native());
}
#endif

template<unsigned N> SIMDPP_INL
int32<N> i_min(const int32<N>& a, const int32<N>& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(int32<N>, i_min, a, b);
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint32x4 i_min(const uint32x4& a, const uint32x4& b)
{
#if SIMDPP_USE_NULL
    return detail::null::min(a, b);
#elif SIMDPP_USE_SSE4_1
    return _mm_min_epu32(a.native(), b.native());
#elif SIMDPP_USE_SSE2
    mask_int32x4 mask = cmp_lt(a, b);
    return blend(a, b, mask);
#elif SIMDPP_USE_NEON
    return vminq_u32(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_min(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_min_u_w(a.native(), b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint32x8 i_min(const uint32x8& a, const uint32x8& b)
{
    return _mm256_min_epu32(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint32<16> i_min(const uint32<16>& a, const uint32<16>& b)
{
    return _mm512_min_epu32(a.native(), b.native());
}
#endif

template<unsigned N> SIMDPP_INL
uint32<N> i_min(const uint32<N>& a, const uint32<N>& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(uint32<N>, i_min, a, b);
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
int64x2 i_min(const int64x2& a, const int64x2& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm_min_epi64(a.native(), b.native());
#elif SIMDPP_USE_AVX2 || SIMDPP_USE_NEON64
    mask_int64x2 mask = cmp_lt(a, b);
    return blend(a, b, mask);
#elif SIMDPP_USE_VSX_207
    return vec_min(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_min_s_d(a.native(), b.native());
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC
    return detail::null::min(a, b);
#else
    return SIMDPP_NOT_IMPLEMENTED2(a, b);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int64x4 i_min(const int64x4& a, const int64x4& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_min_epi64(a.native(), b.native());
#else
    mask_int64x4 mask = cmp_lt(a, b);
    return blend(a, b, mask);
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
int64<8> i_min(const int64<8>& a, const int64<8>& b)
{
    return _mm512_min_epi64(a.native(), b.native());
}
#endif

template<unsigned N> SIMDPP_INL
int64<N> i_min(const int64<N>& a, const int64<N>& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(int64<N>, i_min, a, b);
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint64x2 i_min(const uint64x2& a, const uint64x2& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm_min_epu64(a.native(), b.native());
#elif SIMDPP_USE_AVX2 || SIMDPP_USE_NEON64
    mask_int64x2 mask = cmp_lt(a, b);
    return blend(a, b, mask);
#elif SIMDPP_USE_VSX_207
    return vec_min(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_min_u_d(a.native(), b.native());
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC
    return detail::null::min(a, b);
#else
    return SIMDPP_NOT_IMPLEMENTED2(a, b);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint64x4 i_min(const uint64x4& a, const uint64x4& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_min_epu64(a.native(), b.native());
#else
    mask_int64x4 mask = cmp_lt(a, b);
    return blend(a, b, mask);
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint64<8> i_min(const uint64<8>& a, const uint64<8>& b)
{
    return _mm512_min_epu64(a.native(), b.native());
}
#endif

template<unsigned N> SIMDPP_INL
uint64<N> i_min(const uint64<N>& a, const uint64<N>& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(uint64<N>, i_min, a, b);
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

