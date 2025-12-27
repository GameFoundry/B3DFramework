/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_EXTEND_TO_INT16_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_EXTEND_TO_INT16_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/combine.h>
#include <simdpp/detail/not_implemented.h>
#include <simdpp/core/i_shift_r.h>
#include <simdpp/core/move_l.h>
#include <simdpp/core/zip_hi.h>
#include <simdpp/core/zip_lo.h>
#include <simdpp/detail/vector_array_conv_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

SIMDPP_INL uint16<16> i_to_uint16(const uint8<16>& a)
{
#if SIMDPP_USE_NULL
    uint16x16 r;
    for (unsigned i = 0; i < 16; i++) {
        r.vec(i/8).el(i%8) = uint16_t(a.el(i));
    }
    return r;
#elif SIMDPP_USE_AVX2
    return _mm256_cvtepu8_epi16(a.native());
#elif SIMDPP_USE_SSE4_1
    uint16x8 r1, r2;
    r1 = _mm_cvtepu8_epi16(a.native());
    r2 = _mm_cvtepu8_epi16(move16_l<8>(a).eval().native());
    return combine(r1, r2);
#elif SIMDPP_USE_SSE2 || (SIMDPP_USE_ALTIVEC && SIMDPP_LITTLE_ENDIAN) || SIMDPP_USE_MSA
    uint16x8 r1, r2;
    r1 = zip16_lo(a, (uint8x16) make_zero());
    r2 = zip16_hi(a, (uint8x16) make_zero());
    return combine(r1, r2);
#elif SIMDPP_USE_NEON
    uint16x16 r;
    r.vec(0) = vmovl_u8(vget_low_u8(a.native()));
    r.vec(1) = vmovl_u8(vget_high_u8(a.native()));
    return r;
#elif (SIMDPP_USE_ALTIVEC && SIMDPP_BIG_ENDIAN)
    uint16x8 r1, r2;
    r1 = zip16_lo((uint8x16) make_zero(), a);
    r2 = zip16_hi((uint8x16) make_zero(), a);
    return combine(r1, r2);
#endif
}

#if SIMDPP_USE_AVX2
SIMDPP_INL uint16<32> i_to_uint16(const uint8<32>& a)
{
#if SIMDPP_USE_AVX512BW
    return _mm512_cvtepu8_epi16(a.native());
#else
    uint16<16> r0, r1;
    uint8<16> a0, a1;
    split(a, a0, a1);
    r0 = _mm256_cvtepu8_epi16(a0.native());
    r1 = _mm256_cvtepu8_epi16(a1.native());
    return combine(r0, r1);
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint16<64> i_to_uint16(const uint8<64>& a)
{
    uint16<32> r0, r1;
    uint8<32> a0, a1;
    split(a, a0, a1);
    r0 = _mm512_cvtepu8_epi16(a0.native());
    r1 = _mm512_cvtepu8_epi16(a1.native());
    return combine(r0, r1);
}
#endif

template<unsigned N> SIMDPP_INL
uint16<N> i_to_uint16(const uint8<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_INSERT(uint16<N>, i_to_uint16, a)
}

// -----------------------------------------------------------------------------

SIMDPP_INL int16x16 i_to_int16(const int8x16& a)
{
#if SIMDPP_USE_NULL
    int16x16 r;
    for (unsigned i = 0; i < 16; i++) {
        r.vec(i/8).el(i%8) = int16_t(a.el(i));
    }
    return r;
#elif SIMDPP_USE_AVX2
    return _mm256_cvtepi8_epi16(a.native());
#elif SIMDPP_USE_SSE4_1
    int16x8 r1, r2;
    r1 = _mm_cvtepi8_epi16(a.native());
    r2 = _mm_cvtepi8_epi16(move16_l<8>(a).eval().native());
    return combine(r1, r2);
#elif SIMDPP_USE_SSE2
    int16x8 r1, r2;
    r1 = zip16_lo((int8x16) make_zero(), a);
    r1 = shift_r<8>(r1);
    r2 = zip16_hi((int8x16) make_zero(), a);
    r2 = shift_r<8>(r2);
    return combine(r1, r2);
#elif SIMDPP_USE_NEON
    int16x16 r;
    r.vec(0) = vmovl_s8(vget_low_s8(a.native()));
    r.vec(1) = vmovl_s8(vget_high_s8(a.native()));
    return r;
#elif SIMDPP_USE_MSA
    int8x16 sign = shift_r<7>(a);
    int16x8 lo, hi;
    lo = zip16_lo(a, sign);
    hi = zip16_hi(a, sign);
    return combine(lo, hi);
#elif SIMDPP_USE_ALTIVEC
    int16x16 r;
    r.vec(0) = vec_unpackh(a.vec(0).native());
    r.vec(1) = vec_unpackl(a.vec(0).native());
    return r;
#endif
}

#if SIMDPP_USE_AVX2
SIMDPP_INL int16<32> i_to_int16(const int8<32>& a)
{
#if SIMDPP_USE_AVX512BW
    return _mm512_cvtepi8_epi16(a.native());
#else
    int16<16> r0, r1;
    int8<16> a0, a1;
    split(a, a0, a1);
    r0 = _mm256_cvtepi8_epi16(a0.native());
    r1 = _mm256_cvtepi8_epi16(a1.native());
    return combine(r0, r1);
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL int16<64> i_to_int16(const int8<64>& a)
{
    int16<32> r0, r1;
    int8<32> a0, a1;
    split(a, a0, a1);
    r0 = _mm512_cvtepi8_epi16(a0.native());
    r1 = _mm512_cvtepi8_epi16(a1.native());
    return combine(r0, r1);
}
#endif

template<unsigned N> SIMDPP_INL
int16<N> i_to_int16(const int8<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_INSERT(int16<N>, i_to_int16, a)
}


} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif


