/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_ZIP_HI_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_ZIP_HI_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/neon/shuffle.h>
#include <simdpp/detail/null/shuffle.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {


static SIMDPP_INL
uint8x16 i_zip16_hi(const uint8x16& a, const uint8x16& b)
{
#if SIMDPP_USE_NULL
    return detail::null::zip16_hi(a, b);
#elif SIMDPP_USE_SSE2
    return _mm_unpackhi_epi8(a.native(), b.native());
#elif SIMDPP_USE_NEON
    // the compiler will optimize multiple vzip instructions if both zip_hi
    // and zip_hi are used on the same arguments
    return vzipq_u8(a.native(), b.native()).val[1];
#elif SIMDPP_USE_ALTIVEC
    return vec_mergel(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return (v16u8) __msa_ilvl_b((v16i8) b.native(), (v16i8) a.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint8x32 i_zip16_hi(const uint8x32& a, const uint8x32& b)
{
    return _mm256_unpackhi_epi8(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint8<64> i_zip16_hi(const uint8<64>& a, const uint8<64>& b)
{
    return _mm512_unpackhi_epi8(a.native(), b.native());
}
#endif

template<unsigned N> SIMDPP_INL
uint8<N> i_zip16_hi(const uint8<N>& a, const uint8<N>& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(uint8<N>, i_zip16_hi, a, b)
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint16x8 i_zip8_hi(const uint16x8& a, const uint16x8& b)
{
#if SIMDPP_USE_NULL
    return detail::null::zip8_hi(a, b);
#elif SIMDPP_USE_SSE2
    return _mm_unpackhi_epi16(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vzipq_u16(a.native(), b.native()).val[1];
#elif SIMDPP_USE_ALTIVEC
    return vec_mergel(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return (v8u16) __msa_ilvl_h((v8i16) b.native(), (v8i16) a.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint16x16 i_zip8_hi(const uint16x16& a, const uint16x16& b)
{
    return _mm256_unpackhi_epi16(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint16<32> i_zip8_hi(const uint16<32>& a, const uint16<32>& b)
{
    return _mm512_unpackhi_epi16(a.native(), b.native());
}
#endif

template<unsigned N> SIMDPP_INL
uint16<N> i_zip8_hi(const uint16<N>& a, const uint16<N>& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(uint16<N>, i_zip8_hi, a, b)
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint32x4 i_zip4_hi(const uint32x4& a, const uint32x4& b)
{
#if SIMDPP_USE_NULL
    return detail::null::zip4_hi(a, b);
#elif SIMDPP_USE_SSE2
    return _mm_unpackhi_epi32(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vzipq_u32(a.native(), b.native()).val[1];
#elif SIMDPP_USE_ALTIVEC
    return vec_mergel(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return (v4u32) __msa_ilvl_w((v4i32) b.native(), (v4i32) a.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint32x8 i_zip4_hi(const uint32x8& a, const uint32x8& b)
{
    return _mm256_unpackhi_epi32(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint32<16> i_zip4_hi(const uint32<16>& a, const uint32<16>& b)
{
    return _mm512_unpackhi_epi32(a.native(), b.native());
}
#endif

template<unsigned N> SIMDPP_INL
uint32<N> i_zip4_hi(const uint32<N>& a, const uint32<N>& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(uint32<N>, i_zip4_hi, a, b)
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint64x2 i_zip2_hi(const uint64x2& a, const uint64x2& b)
{
#if SIMDPP_USE_SSE2
    return _mm_unpackhi_epi64(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return neon::zip2_hi(a, b);
#elif SIMDPP_USE_VSX_207
    return vec_mergel(a.native(), b.native());
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC
    return detail::null::zip2_hi(a, b);
#elif SIMDPP_USE_MSA
    return (v2u64) __msa_ilvl_d((v2i64) b.native(), (v2i64) a.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint64x4 i_zip2_hi(const uint64x4& a, const uint64x4& b)
{
    return _mm256_unpackhi_epi64(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint64<8> i_zip2_hi(const uint64<8>& a, const uint64<8>& b)
{
    return _mm512_unpackhi_epi64(a.native(), b.native());
}
#endif

template<unsigned N> SIMDPP_INL
uint64<N> i_zip2_hi(const uint64<N>& a, const uint64<N>& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(uint64<N>, i_zip2_hi, a, b)
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
float32x4 i_zip4_hi(const float32x4& a, const float32x4& b)
{
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    return detail::null::zip4_hi(a, b);
#elif SIMDPP_USE_SSE2
    return _mm_unpackhi_ps(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vzipq_f32(a.native(), b.native()).val[1];
#elif SIMDPP_USE_ALTIVEC
    return vec_mergel(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return (v4f32) __msa_ilvl_w((v4i32) b.native(), (v4i32) a.native());
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float32x8 i_zip4_hi(const float32x8& a, const float32x8& b)
{
    return _mm256_unpackhi_ps(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float32<16> i_zip4_hi(const float32<16>& a, const float32<16>& b)
{
    return _mm512_unpackhi_ps(a.native(), b.native());
}
#endif

template<unsigned N> SIMDPP_INL
float32<N> i_zip4_hi(const float32<N>& a, const float32<N>& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(float32<N>, i_zip4_hi, a, b)
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
float64x2 i_zip2_hi(const float64x2& a, const float64x2& b)
{
#if SIMDPP_USE_SSE2
    return _mm_castps_pd(_mm_movehl_ps(_mm_castpd_ps(b.native()),
                                       _mm_castpd_ps(a.native())));
#elif SIMDPP_USE_NEON64
    return vtrn2q_f64(a.native(), b.native());
#elif SIMDPP_USE_VSX_206
    return (__vector double) vec_mergel((__vector uint64_t)a.native(),
                                        (__vector uint64_t)b.native());
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC || SIMDPP_USE_NEON
    return detail::null::zip2_hi(a, b);
#elif SIMDPP_USE_MSA
    return (v2f64) __msa_ilvl_d((v2i64) b.native(), (v2i64) a.native());
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float64x4 i_zip2_hi(const float64x4& a, const float64x4& b)
{
    return _mm256_unpackhi_pd(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float64<8> i_zip2_hi(const float64<8>& a, const float64<8>& b)
{
    return _mm512_unpackhi_pd(a.native(), b.native());
}
#endif

template<unsigned N> SIMDPP_INL
float64<N> i_zip2_hi(const float64<N>& a, const float64<N>& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(float64<N>, i_zip2_hi, a, b)
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

