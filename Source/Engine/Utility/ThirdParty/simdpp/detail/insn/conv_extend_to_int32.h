/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_EXTEND_TO_INT32_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_EXTEND_TO_INT32_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/combine.h>
#include <simdpp/detail/insn/conv_extend_to_int16.h>
#include <simdpp/core/i_shift_r.h>
#include <simdpp/core/move_l.h>
#include <simdpp/core/zip_hi.h>
#include <simdpp/core/zip_lo.h>
#include <simdpp/core/unzip_lo.h>
#include <simdpp/detail/vector_array_conv_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint32<8> i_to_uint32(const uint16<8>& a)
{
#if SIMDPP_USE_NULL
    uint32<8> r;
    for (unsigned i = 0; i < r.length; i++) {
        r.vec(i/4).el(i%4) = uint32_t(a.vec(0).el(i));
    }
    return r;
#elif SIMDPP_USE_AVX2
    return  _mm256_cvtepu16_epi32(a.native());
#elif SIMDPP_USE_SSE4_1
    uint32<8> r;
    r.vec(0) = _mm_cvtepu16_epi32(a.native());
    r.vec(1) = _mm_cvtepu16_epi32(move8_l<4>(a).eval().native());
    return r;
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_MSA || (SIMDPP_USE_ALTIVEC && SIMDPP_LITTLE_ENDIAN)
    uint16<8> zero = make_zero();
    uint32<8> r;
    r.vec(0) = zip8_lo(a, zero);
    r.vec(1) = zip8_hi(a, zero);
    return r;
#elif (SIMDPP_USE_ALTIVEC && SIMDPP_BIG_ENDIAN)
    uint16<8> zero = make_zero();
    uint32<8> r;
    r.vec(0) = zip8_lo(zero, a);
    r.vec(1) = zip8_hi(zero, a);
    return r;
#elif SIMDPP_USE_NEON
    uint32<8> r;
    r.vec(0) = vmovl_u16(vget_low_u16(a.vec(0).native()));
    r.vec(1) = vmovl_u16(vget_high_u16(a.vec(1).native()));
    return r;
#endif
}

#if SIMDPP_USE_AVX2
SIMDPP_INL uint32<16> i_to_uint32(const uint16<16>& a)
{
#if SIMDPP_USE_AVX512F
    return _mm512_cvtepu16_epi32(a.native());
#else
    uint32<16> r;
    uint16<8> a0, a1;
    split(a, a0, a1);
    r.vec(0) = _mm256_cvtepu16_epi32(a0.native());
    r.vec(1) = _mm256_cvtepu16_epi32(a1.native());
    return r;
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint32<32> i_to_uint32(const uint16<32>& a)
{
    uint32<32> r;
    uint16<16> a0, a1;
    split(a, a0, a1);
    r.vec(0) = _mm512_cvtepu16_epi32(a0.native());
    r.vec(1) = _mm512_cvtepu16_epi32(a1.native());
    return r;
}
#endif

template<unsigned N> SIMDPP_INL
uint32<N> i_to_uint32(const uint16<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_INSERT(uint32<N>, i_to_uint32, a)
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint32<16> i_to_uint32(const uint8<16>& a)
{
#if SIMDPP_USE_NULL
    uint32<16> r;
    for (unsigned i = 0; i < r.length; i++) {
        r.vec(i/4).el(i%4) = uint32_t(a.vec(0).el(i));
    }
    return r;
#elif SIMDPP_USE_AVX512F
    return _mm512_cvtepu8_epi32(a.native());
#elif SIMDPP_USE_AVX2
    uint32<16> r;
    r.vec(0) =  _mm256_cvtepu8_epi32(a.native());
    r.vec(1) =  _mm256_cvtepu8_epi32(move16_l<8>(a).eval().native());
    return r;
#elif SIMDPP_USE_SSE4_1
    uint32<16> r;
    r.vec(0) = _mm_cvtepu8_epi32(a.native());
    r.vec(1) = _mm_cvtepu8_epi32(move16_l<4>(a).eval().native());
    r.vec(2) = _mm_cvtepu8_epi32(move16_l<8>(a).eval().native());
    r.vec(3) = _mm_cvtepu8_epi32(move16_l<12>(a).eval().native());
    return r;
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    return i_to_uint32(i_to_uint16(a));
#endif
}

#if SIMDPP_USE_AVX2
SIMDPP_INL uint32<32> i_to_uint32(const uint8<32>& a)
{
#if SIMDPP_USE_AVX512F
    uint32<32> r;
    uint8<16> a0, a1;
    split(a, a0, a1);
    r.vec(0) = _mm512_cvtepu8_epi32(a0.native());
    r.vec(1) = _mm512_cvtepu8_epi32(a1.native());
    return r;
#else
    uint32<32> r;
    uint8<16> a0, a1;
    split(a, a0, a1);
    r.vec(0) =  _mm256_cvtepu8_epi32(a0.native());
    r.vec(1) =  _mm256_cvtepu8_epi32(move16_l<8>(a0).eval().native());
    r.vec(2) =  _mm256_cvtepu8_epi32(a1.native());
    r.vec(3) =  _mm256_cvtepu8_epi32(move16_l<8>(a1).eval().native());
    return r;
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint32<64> i_to_uint32(const uint8<64>& a)
{
    uint32<64> r;
    uint8<32> a01, a23;
    uint8<16> a0, a1, a2, a3;
    split(a, a01, a23);
    split(a01, a0, a1);
    split(a23, a2, a3);

    r.vec(0) =  _mm512_cvtepu8_epi32(a0.native());
    r.vec(1) =  _mm512_cvtepu8_epi32(a1.native());
    r.vec(2) =  _mm512_cvtepu8_epi32(a2.native());
    r.vec(3) =  _mm512_cvtepu8_epi32(a3.native());
    return r;
}
#endif

template<unsigned N> SIMDPP_INL
uint32<N> i_to_uint32(const uint8<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_INSERT(uint32<N>, i_to_uint32, a)
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
int32<8> i_to_int32(const int16<8>& a)
{
#if SIMDPP_USE_NULL
    int32<8> r;
    for (unsigned i = 0; i < r.length; i++) {
        r.vec(i/4).el(i%4) = int32_t(a.vec(0).el(i));
    }
    return r;
#elif SIMDPP_USE_AVX2
    return  _mm256_cvtepi16_epi32(a.native());
#elif SIMDPP_USE_SSE4_1
    int32x8 r;
    r.vec(0) = _mm_cvtepi16_epi32(a.native());
    r.vec(1) = _mm_cvtepi16_epi32(move8_l<4>(a).eval().native());
    return r;
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_MSA
    int16x8 sign = shift_r<15>(a);
    int32x4 lo, hi;
    lo = zip8_lo(a, sign);
    hi = zip8_hi(a, sign);
    return combine(lo, hi);
#elif SIMDPP_USE_NEON
    int32x8 r;
    r.vec(0) = vmovl_s16(vget_low_s16(a.vec(0).native()));
    r.vec(1) = vmovl_s16(vget_high_s16(a.vec(1).native()));
    return r;
#elif SIMDPP_USE_ALTIVEC
    int32x4 b0, b1;
    b0 = vec_unpackh((__vector int16_t)a.vec(0).native());
    b1 = vec_unpackl((__vector int16_t)a.vec(0).native());
    return combine(b0, b1);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int32<16> i_to_int32(const int16<16>& a)
{
#if SIMDPP_USE_AVX512F
    return _mm512_cvtepi16_epi32(a.native());
#else
    int32<8> r0, r1;
    int16<8> a0, a1;
    split(a, a0, a1);
    r0 = _mm256_cvtepi16_epi32(a0.native());
    r1 = _mm256_cvtepi16_epi32(a1.native());
    return combine(r0, r1);
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL int32<32> i_to_int32(const int16<32>& a)
{
    int32<16> r0, r1;
    int16<16> a0, a1;
    split(a, a0, a1);
    r0 = _mm512_cvtepi16_epi32(a0.native());
    r1 = _mm512_cvtepi16_epi32(a1.native());
    return combine(r0, r1);
}
#endif

template<unsigned N> SIMDPP_INL
int32<N> i_to_int32(const int16<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_INSERT(int32<N>, i_to_int32, a)
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
int32<16> i_to_int32(const int8<16>& a)
{
#if SIMDPP_USE_NULL
    int32<16> r;
    for (unsigned i = 0; i < r.length; i++) {
        r.vec(i/4).el(i%4) = int32_t(a.vec(0).el(i));
    }
    return r;
#elif SIMDPP_USE_AVX512F
    return _mm512_cvtepi8_epi32(a.native());
#elif SIMDPP_USE_AVX2
    int32<16> r;
    r.vec(0) = _mm256_cvtepi8_epi32(a.native());
    r.vec(1) = _mm256_cvtepi8_epi32(move16_l<8>(a).eval().native());
    return r;
#elif SIMDPP_USE_SSE4_1
    int32<16> r;
    r.vec(0) = _mm_cvtepi8_epi32(a.native());
    r.vec(1) = _mm_cvtepi8_epi32(move16_l<4>(a).eval().native());
    r.vec(2) = _mm_cvtepi8_epi32(move16_l<8>(a).eval().native());
    r.vec(3) = _mm_cvtepi8_epi32(move16_l<12>(a).eval().native());
    return r;
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    return i_to_int32(i_to_int16(a));
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int32<32> i_to_int32(const int8<32>& a)
{
#if SIMDPP_USE_AVX512F
    int32<32> r;
    int8<16> a0, a1;
    split(a, a0, a1);
    r.vec(0) = _mm512_cvtepi8_epi32(a0.native());
    r.vec(1) = _mm512_cvtepi8_epi32(a1.native());
    return r;
#else
    int32<32> r;
    int8<16> a0, a1;
    split(a, a0, a1);
    r.vec(0) =  _mm256_cvtepi8_epi32(a0.native());
    r.vec(1) =  _mm256_cvtepi8_epi32(move16_l<8>(a0).eval().native());
    r.vec(2) =  _mm256_cvtepi8_epi32(a1.native());
    r.vec(3) =  _mm256_cvtepi8_epi32(move16_l<8>(a1).eval().native());
    return r;
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL int32<64> i_to_int32(const int8<64>& a)
{
    int32<64> r;
    int8<32> a01, a23;
    int8<16> a0, a1, a2, a3;
    split(a, a01, a23);
    split(a01, a0, a1);
    split(a23, a2, a3);

    r.vec(0) =  _mm512_cvtepi8_epi32(a0.native());
    r.vec(1) =  _mm512_cvtepi8_epi32(a1.native());
    r.vec(2) =  _mm512_cvtepi8_epi32(a2.native());
    r.vec(3) =  _mm512_cvtepi8_epi32(a3.native());
    return r;
}
#endif

template<unsigned N> SIMDPP_INL
int32<N> i_to_int32(const int8<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_INSERT(int32<N>, i_to_int32, a)
}


} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif


