/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_SHRINK_TO_INT8_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_SHRINK_TO_INT8_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/make_shuffle_bytes_mask.h>
#include <simdpp/core/permute4.h>
#include <simdpp/core/permute_bytes16.h>
#include <simdpp/core/shuffle4x2.h>
#include <simdpp/core/unzip_lo.h>
#include <simdpp/detail/insn/conv_shrink_to_int16.h>
#include <simdpp/detail/insn/conv_shrink_to_int32.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

// -----------------------------------------------------------------------------

SIMDPP_INL uint8<16> i_to_uint8(const uint16<16>& a)
{
#if SIMDPP_USE_NULL
    uint8<16> r;
    for (unsigned i = 0; i < 16; i++) {
        r.el(i) = uint8_t(a.vec(i/8).el(i%8));
    }
    return r;
#elif SIMDPP_USE_AVX512VL
    return _mm256_cvtepi16_epi8(a.native());
#elif SIMDPP_USE_SSSE3
    uint8<32> perm_mask = make_shuffle_bytes16_mask<0,2,4,6,8,10,12,14,
                                                    0,0,0,0,0,0,0,0>(perm_mask);
    uint8<32> a8;
    uint64<4> a64;
    a8 = a;
    a64 = permute_bytes16(a8, perm_mask);
#if SIMDPP_USE_AVX2
    a64 = permute4<0,2,0,2>(a64);
    return _mm256_castsi256_si128(a64.native());
#else
    return (uint8<16>) zip2_lo(a64.vec(0), a64.vec(1));
#endif
#elif SIMDPP_USE_NEON64
    uint8x8_t low = vmovn_u16(a.vec(0).native());
    return vmovn_high_u16(low, a.vec(1).native());
#elif SIMDPP_USE_NEON
    uint8x8_t low = vmovn_u16(a.vec(0).native());
    uint8x8_t high = vmovn_u16(a.vec(1).native());
    return vcombine_u8(low, high);
#elif SIMDPP_USE_ALTIVEC
    return vec_pack(a.vec(0).native(), a.vec(1).native());
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_MSA
    uint8<16> r1, r2;
    r1 = a.vec(0);
    r2 = a.vec(1);
    return unzip16_lo(r1, r2);
#endif
}

#if SIMDPP_USE_AVX2
SIMDPP_INL uint8<32> i_to_uint8(const uint16<32>& a)
{
#if SIMDPP_USE_AVX512BW
    return _mm512_cvtepi16_epi8(a.native());
#else
    uint8<32> perm_mask = make_shuffle_bytes16_mask<0,2,4,6,8,10,12,14,
                                                    0,0,0,0,0,0,0,0>(perm_mask);
    uint8<32> a8_0, a8_1;
    uint64<4> a64_0, a64_1;
    a8_0 = a.vec(0);
    a8_1 = a.vec(1);
    a64_0 = permute_bytes16(a8_0, perm_mask);
    a64_1 = permute_bytes16(a8_1, perm_mask);
    return (uint8<32>) shuffle4x2<0,2,4,6>(a64_0, a64_1);
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint8<64> i_to_uint8(const uint16<64>& a)
{
    uint8<32> r1 = _mm512_cvtepi16_epi8(a.vec(0).native());
    uint8<32> r2 = _mm512_cvtepi16_epi8(a.vec(1).native());
    return combine(r1, r2);
}
#endif

template<unsigned N> SIMDPP_INL
uint8<N> i_to_uint8(const uint16<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_EXTRACT(uint8<N>, i_to_uint8, a)
}

// -----------------------------------------------------------------------------

SIMDPP_INL uint8<16> i_to_uint8(const uint32<16>& a)
{
#if SIMDPP_USE_NULL
    uint8<16> r;
    for (unsigned i = 0; i < 16; i++) {
        r.el(i) = uint8_t(a.vec(i/4).el(i%4));
    }
    return r;
#elif SIMDPP_USE_AVX512F
    return _mm512_cvtepi32_epi8(a.native());
#elif SIMDPP_USE_SSSE3
    uint8<64> perm_mask = make_shuffle_bytes16_mask<0,4,8,12,0,0,0,0,0,0,0,0,0,0,0,0>(perm_mask);
    uint8<64> a8;
    uint32<16> a32;
    a8 = a;
    a32 = permute_bytes16(a8, perm_mask);

    uint32<4> b0, b1, b2, b3;
#if SIMDPP_USE_AVX2
    split(a32.vec(0), b0, b1);
    split(a32.vec(1), b2, b3);
#else
    b0 = a32.vec(0);
    b1 = a32.vec(1);
    b2 = a32.vec(2);
    b3 = a32.vec(3);
#endif
    uint64<2> r0, r1;
    r0 = zip4_lo(b0, b1);
    r1 = zip4_lo(b2, b3);
    return (uint8<16>) zip2_lo(r0, r1);
#else
    uint16<16> a16 = i_to_uint16(a);
    return i_to_uint8(a16);
#endif
}

#if SIMDPP_USE_AVX2
SIMDPP_INL uint8<32> i_to_uint8(const uint32<32>& a)
{
#if SIMDPP_USE_AVX512F
    uint8<16> r0 = _mm512_cvtepi32_epi8(a.vec(0).native());
    uint8<16> r1 = _mm512_cvtepi32_epi8(a.vec(1).native());
    return combine(r0, r1);
#else
    uint8<32> perm_mask = make_shuffle_bytes16_mask<0,4,8,12,0,0,0,0,0,0,0,0,0,0,0,0>(perm_mask);
    uint8<32> a8_0, a8_1, a8_2, a8_3;
    uint32<8> a32_0, a32_1, a32_2, a32_3;
    uint64<4> a64_0, a64_1;
    uint32<4> b32_0, b32_1, c32_0, c32_1;
    a8_0 = a.vec(0);
    a8_1 = a.vec(1);
    a8_2 = a.vec(2);
    a8_3 = a.vec(3);
    a32_0 = permute_bytes16(a8_0, perm_mask);
    a32_1 = permute_bytes16(a8_1, perm_mask);
    a32_2 = permute_bytes16(a8_2, perm_mask);
    a32_3 = permute_bytes16(a8_3, perm_mask);
    a64_0 = zip4_lo(a32_0, a32_1);
    a64_1 = zip4_lo(a32_2, a32_3);
    a32_0 = zip2_lo(a64_0, a64_1);
    split(a32_0, b32_0, b32_1);
    c32_0 = unzip4_lo(b32_0, b32_1);
    c32_1 = unzip4_hi(b32_0, b32_1);
    return (uint8<32>) combine(c32_0, c32_1);
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint8<64> i_to_uint8(const uint32<64>& a)
{
    uint8<16> r0 = _mm512_cvtepi32_epi8(a.vec(0).native());
    uint8<16> r1 = _mm512_cvtepi32_epi8(a.vec(1).native());
    uint8<16> r2 = _mm512_cvtepi32_epi8(a.vec(2).native());
    uint8<16> r3 = _mm512_cvtepi32_epi8(a.vec(3).native());
    return combine(combine(r0, r1), combine(r2, r3));
}
#endif

template<unsigned N> SIMDPP_INL
uint8<N> i_to_uint8(const uint32<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_EXTRACT(uint8<N>, i_to_uint8, a)
}

// -----------------------------------------------------------------------------

SIMDPP_INL uint8<16> i_to_uint8(const uint64<16>& a)
{
#if SIMDPP_USE_NULL
    uint8<16> r;
    for (unsigned i = 0; i < 16; i++) {
        r.el(i) = uint8_t(a.vec(i/2).el(i%2));
    }
    return r;
#elif SIMDPP_USE_AVX512F
    __m128i r0 = _mm512_cvtepi64_epi8(a.vec(0).native());
    __m128i r1 = _mm512_cvtepi64_epi8(a.vec(1).native());
    return _mm_unpacklo_epi64(r0, r1);
#elif SIMDPP_USE_AVX2
    uint8<32> perm_mask = make_shuffle_bytes16_mask<0,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0>(perm_mask);
    uint16<16> a16_0, a16_1, a16_2, a16_3;
    uint32<8> a32_0, a32_1;
    uint64<4> a64_0;
    uint16<8> b16;

    a16_0 = permute_bytes16((uint8<32>) a.vec(0), perm_mask);
    a16_1 = permute_bytes16((uint8<32>) a.vec(1), perm_mask);
    a16_2 = permute_bytes16((uint8<32>) a.vec(2), perm_mask);
    a16_3 = permute_bytes16((uint8<32>) a.vec(3), perm_mask);

    a32_0 = zip8_lo(a16_0, a16_1);
    a32_1 = zip8_lo(a16_2, a16_3);

    a64_0 = zip4_lo(a32_0, a32_1);
    a64_0 = permute4<0,2,0,2>(a64_0);

    b16 = _mm256_castsi256_si128(a64_0.native());

    uint16<8> perm_mask2 = make_shuffle_bytes16_mask<0,4,1,5,2,6,3,7>(perm_mask2);
    b16 = permute_bytes16(b16, perm_mask2);
    return (uint8<16>) b16;
#else
    // TODO: SSSE3
    uint32<16> a32 = i_to_uint32(a);
    return i_to_uint8(a32);
#endif
}

#if SIMDPP_USE_AVX2
SIMDPP_INL uint8<32> i_to_uint8(const uint64<32>& a)
{
#if SIMDPP_USE_AVX512F
    __m128i r0 = _mm512_cvtepi64_epi8(a.vec(0).native());
    __m128i r1 = _mm512_cvtepi64_epi8(a.vec(1).native());
    __m128i r2 = _mm512_cvtepi64_epi8(a.vec(2).native());
    __m128i r3 = _mm512_cvtepi64_epi8(a.vec(3).native());
    uint8<16> r01 = _mm_unpacklo_epi64(r0, r1);
    uint8<16> r23 = _mm_unpacklo_epi64(r2, r3);
    return combine(r01, r23);
#else
    uint8<32> perm_mask = make_shuffle_bytes16_mask<0,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0>(perm_mask);
    uint16<16> a16_0, a16_1, a16_2, a16_3, a16_4, a16_5, a16_6, a16_7;
    uint32<8> a32_0, a32_1, a32_2, a32_3;
    uint64<4> a64_0, a64_1;

    a16_0 = permute_bytes16((uint8<32>) a.vec(0), perm_mask);
    a16_1 = permute_bytes16((uint8<32>) a.vec(1), perm_mask);
    a16_2 = permute_bytes16((uint8<32>) a.vec(2), perm_mask);
    a16_3 = permute_bytes16((uint8<32>) a.vec(3), perm_mask);
    a16_4 = permute_bytes16((uint8<32>) a.vec(4), perm_mask);
    a16_5 = permute_bytes16((uint8<32>) a.vec(5), perm_mask);
    a16_6 = permute_bytes16((uint8<32>) a.vec(6), perm_mask);
    a16_7 = permute_bytes16((uint8<32>) a.vec(7), perm_mask);

    a32_0 = zip8_lo(a16_0, a16_1);
    a32_1 = zip8_lo(a16_2, a16_3);
    a32_2 = zip8_lo(a16_4, a16_5);
    a32_3 = zip8_lo(a16_6, a16_7);

    a64_0 = zip4_lo(a32_0, a32_1);
    a64_1 = zip4_lo(a32_2, a32_3);
    a64_0 = zip2_lo(a64_0, a64_1);
    a16_0 = permute4<0,2,1,3>(a64_0);

    uint16<16> perm_mask2 = make_shuffle_bytes16_mask<0,4,1,5,2,6,3,7>(perm_mask2);
    a16_0 = permute_bytes16(a16_0, perm_mask2);
    return (uint8<32>) a16_0;
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint8<64> i_to_uint8(const uint64<64>& a)
{
    __m128i r0 = _mm512_cvtepi64_epi8(a.vec(0).native());
    __m128i r1 = _mm512_cvtepi64_epi8(a.vec(1).native());
    __m128i r2 = _mm512_cvtepi64_epi8(a.vec(2).native());
    __m128i r3 = _mm512_cvtepi64_epi8(a.vec(3).native());
    __m128i r4 = _mm512_cvtepi64_epi8(a.vec(4).native());
    __m128i r5 = _mm512_cvtepi64_epi8(a.vec(5).native());
    __m128i r6 = _mm512_cvtepi64_epi8(a.vec(6).native());
    __m128i r7 = _mm512_cvtepi64_epi8(a.vec(7).native());
    uint8<16> r01 = _mm_unpacklo_epi64(r0, r1);
    uint8<16> r23 = _mm_unpacklo_epi64(r2, r3);
    uint8<16> r45 = _mm_unpacklo_epi64(r4, r5);
    uint8<16> r67 = _mm_unpacklo_epi64(r6, r7);
    return combine(combine(r01, r23), combine(r45, r67));
}
#endif

template<unsigned N> SIMDPP_INL
uint8<N> i_to_uint8(const uint64<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_EXTRACT(uint8<N>, i_to_uint8, a)
}

// -----------------------------------------------------------------------------

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif


