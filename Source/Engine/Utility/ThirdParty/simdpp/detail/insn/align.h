/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_ALIGN_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_ALIGN_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/bit_or.h>
#include <simdpp/core/move_l.h>
#include <simdpp/core/move_r.h>
#include <simdpp/core/permute4.h>
#include <simdpp/core/shuffle2x2.h>
#include <simdpp/core/shuffle4x2.h>
#include <simdpp/detail/shuffle/shuffle_mask.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

// base 8x16 implementation
template<unsigned shift> SIMDPP_INL
uint8x16 i_align16(const uint8x16& clower, const uint8x16& cupper)
{
    uint8x16 lower = clower, upper = cupper;
#if SIMDPP_USE_NULL
    uint8x16 r;
    //use int to disable warnings wrt. comparison result always being true/false
    for (int i = 0; i < (int)(16-shift); i++) {
        r.el(i) = lower.el(i + shift);
    }
    for (unsigned i = 16-shift; i < 16; i++) {
        r.el(i) = upper.el(i - 16 + shift);
    }
    return r;
#elif SIMDPP_USE_SSSE3
    return _mm_alignr_epi8(upper.native(), lower.native(), shift);
#elif SIMDPP_USE_SSE2
    uint8x16 a;
    lower = move16_l<shift>(lower);
    upper = move16_r<16-shift>(upper);
    a = bit_or(upper, lower);
    return a;
#elif SIMDPP_USE_NEON
    if (shift == 0)
        return lower;
    if (shift == 16)
        return upper;
    return vextq_u8(lower.native(), upper.native(), shift % 16);
#elif SIMDPP_USE_ALTIVEC
    return vec_sld_biendian<shift>(lower, upper);
#elif SIMDPP_USE_MSA
    return (v16u8) __msa_sld_b((v16i8)upper.native(),
                               (v16i8)lower.native(), shift);
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned shift> SIMDPP_INL
uint8x32 i_align16(const uint8x32& lower, const uint8x32& upper)
{
    return _mm256_alignr_epi8(upper.native(), lower.native(), shift);
}
#endif

#if SIMDPP_USE_AVX512BW
template<unsigned shift> SIMDPP_INL
uint8<64> i_align16(const uint8<64>& lower, const uint8<64>& upper)
{
    return _mm512_alignr_epi8(upper.native(), lower.native(), shift);
}
#endif

template<unsigned shift, unsigned N> SIMDPP_INL
uint8<N> i_align16(const uint8<N>& lower, const uint8<N>& upper)
{
    SIMDPP_VEC_ARRAY_IMPL2(uint8<N>, i_align16<shift>, lower, upper);
}

// -----------------------------------------------------------------------------

template<unsigned shift> SIMDPP_INL
uint16<8> i_align8(const uint16<8>& lower, const uint16<8>& upper)
{
#if SIMDPP_USE_NULL
    uint16<8> r;
    //use int to disable warnings wrt. comparison result always being true/false
    for (int i = 0; i < (int)(8-shift); i++) {
        r.el(i) = lower.el(i + shift);
    }
    for (unsigned i = 8-shift; i < 8; i++) {
        r.el(i) = upper.el(i - 8 + shift);
    }
    return r;
#else
    return uint16<8>(i_align16<shift*2>(uint8<16>(lower),
                                        uint8<16>(upper)));
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned shift> SIMDPP_INL
uint16<16> i_align8(const uint16<16>& lower, const uint16<16>& upper)
{
    return _mm256_alignr_epi8(upper.native(), lower.native(), shift*2);
}
#endif

#if SIMDPP_USE_AVX512BW
template<unsigned shift> SIMDPP_INL
uint16<32> i_align8(const uint16<32>& lower, const uint16<32>& upper)
{
    return _mm512_alignr_epi8(upper.native(), lower.native(), shift*2);
}
#endif

template<unsigned shift, unsigned N> SIMDPP_INL
uint16<N> i_align8(const uint16<N>& lower, const uint16<N>& upper)
{
    SIMDPP_VEC_ARRAY_IMPL2(uint16<N>, i_align8<shift>, lower, upper);
}

// -----------------------------------------------------------------------------

template<unsigned shift> SIMDPP_INL
uint32x4 i_align4(const uint32x4& lower, const uint32x4& upper)
{
#if SIMDPP_USE_NULL
    uint32x4 r;
    //use int to disable warnings wrt. comparison result always being true/false
    for (int i = 0; i < (int)(4-shift); i++) {
        r.el(i) = lower.el(i + shift);
    }
    for (unsigned i = 4-shift; i < 4; i++) {
        r.el(i) = upper.el(i - 4 + shift);
    }
    return r;
#elif SIMDPP_USE_SSE2
    switch (shift) {
    default:
    case 0: return lower;
#if SIMDPP_USE_SSSE3
    case 1:
    case 2:
    case 3: return _mm_alignr_epi8(upper.native(), lower.native(), shift*4);
#else
    case 2: return _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(lower.native()),
                                                   _mm_castsi128_ps(upper.native()),
                                                   SIMDPP_SHUFFLE_MASK_4x4(2,3,0,1)));
    case 1:
    case 3: return bit_or(move4_l<shift>(lower),
                          move4_r<4-shift>(upper));
#endif
    case 4: return upper;
    }
#elif SIMDPP_USE_NEON
    if (shift == 0)
        return lower;
    if (shift == 4)
        return upper;
    return vextq_u32(lower.native(), upper.native(), shift);
#elif SIMDPP_USE_ALTIVEC
    return (uint32<4>) vec_sld_biendian<shift*4>((uint8<16>)lower, (uint8<16>)upper);
#elif SIMDPP_USE_MSA
    return (v4u32) __msa_sld_b((v16i8)upper.native(),
                               (v16i8)lower.native(), shift*4);
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned shift> SIMDPP_INL
uint32<8> i_align4(const uint32<8>& lower, const uint32<8>& upper)
{
    return _mm256_alignr_epi8(upper.native(), lower.native(), shift*4);
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned shift> SIMDPP_INL
uint32<16> i_align4(const uint32<16>& lower, const uint32<16>& upper)
{
    // note that _mm512_alignr_epi32 operates on entire vector
    switch (shift) {
    default:
    case 0: return lower;
    case 1: return shuffle4x2<1,2,3,4>(lower, upper);
    case 2: return shuffle4x2<2,3,4,5>(lower, upper);
    case 3: return shuffle4x2<3,4,5,6>(lower, upper);
    case 4: return upper;
    }
}
#endif

template<unsigned shift, unsigned N> SIMDPP_INL
uint32<N> i_align4(const uint32<N>& lower, const uint32<N>& upper)
{
    SIMDPP_VEC_ARRAY_IMPL2(uint32<N>, i_align4<shift>, lower, upper);
}

// -----------------------------------------------------------------------------

template<unsigned shift> SIMDPP_INL
uint64x2 i_align2(const uint64x2& lower, const uint64x2& upper)
{
#if SIMDPP_USE_SSE2
    switch (shift) {
    default:
    case 0: return lower;
    case 1: return _mm_castpd_si128(_mm_shuffle_pd(_mm_castsi128_pd(lower.native()),
                                                   _mm_castsi128_pd(upper.native()),
                                                   SIMDPP_SHUFFLE_MASK_2x2(1,0)));
    case 2: return upper;
    }
#elif SIMDPP_USE_NEON
    if (shift == 0)
        return lower;
    if (shift == 2)
        return upper;
    return vextq_u64(lower.native(), upper.native(), shift % 2);
#elif SIMDPP_USE_VSX_207
    return (uint64<2>) vec_sld_biendian<shift*8>((uint8<16>) lower,
                                                 (uint8<16>) upper);
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC
    uint64x2 r;
    //use int to disable warnings wrt. comparison result always being true/false
    for (int i = 0; i < (int)(2-shift); i++) {
        r.el(i) = lower.el(i + shift);
    }
    for (unsigned i = 2-shift; i < 2; i++) {
        r.el(i) = upper.el(i - 2 + shift);
    }
    return r;
#elif SIMDPP_USE_MSA
    return (v2u64) __msa_sld_b((v16i8) upper.native(),
                               (v16i8) lower.native(), shift*8);
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned shift> SIMDPP_INL
uint64<4> i_align2(const uint64<4>& lower, const uint64<4>& upper)
{
    return _mm256_alignr_epi8(upper.native(), lower.native(), shift*8);
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned shift> SIMDPP_INL
uint64<8> i_align2(const uint64<8>& lower, const uint64<8>& upper)
{
    switch (shift) {
    default:
    case 0: return lower;
    case 1: return shuffle2x2<1,2>(lower, upper);
    case 2: return upper;
    }
}
#endif

template<unsigned shift, unsigned N> SIMDPP_INL
uint64<N> i_align2(const uint64<N>& lower, const uint64<N>& upper)
{
    SIMDPP_VEC_ARRAY_IMPL2(uint64<N>, i_align2<shift>, lower, upper);
}

// -----------------------------------------------------------------------------

template<unsigned shift> SIMDPP_INL
float32x4 i_align4(const float32x4& lower, const float32x4& upper)
{
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    float32x4 r;
    //use int to disable warnings wrt. comparison result always being true/false
    for (int i = 0; i < (int)(4-shift); i++) {
        r.el(i) = lower.el(i + shift);
    }
    for (unsigned i = 4-shift; i < 4; i++) {
        r.el(i) = upper.el(i - 4 + shift);
    }
    return r;
#elif SIMDPP_USE_SSE2
    switch (shift) {
    default:
    case 0: return lower;
#if SIMDPP_USE_SSSE3
    case 1:
    case 3: {
        __m128i res = _mm_alignr_epi8(_mm_castps_si128(upper.native()),
                                      _mm_castps_si128(lower.native()), shift*4);
        return _mm_castsi128_ps(res);
    }
#else
    case 1:
    case 3: return bit_or(move4_l<shift>(lower),
                          move4_r<4-shift>(upper));
#endif
    case 2: return _mm_shuffle_ps(lower.native(), upper.native(),
                                  SIMDPP_SHUFFLE_MASK_4x4(2,3,0,1));
    case 4: return upper;
    }
#elif SIMDPP_USE_NEON_FLT_SP
    if (shift == 0)
        return lower;
    if (shift == 4)
        return upper;
    return vextq_f32(lower.native(), upper.native(), shift);
#elif SIMDPP_USE_ALTIVEC
    return (float32<4>) vec_sld_biendian<shift*4>((uint8<16>)lower, (uint8<16>)upper);
#elif SIMDPP_USE_MSA
    return (v4f32) __msa_sld_b((v16i8)upper.native(),
                               (v16i8)lower.native(), shift*4);
#endif
}

#if SIMDPP_USE_AVX
template<unsigned shift> SIMDPP_INL
float32<8> i_align4(const float32<8>& lower, const float32<8>& upper)
{
    switch (shift) {
    default:
    case 0: return lower;
#if SIMDPP_USE_AVX2
    case 1:
    case 3: {
        __m256i res = _mm256_alignr_epi8(_mm256_castps_si256(upper.native()),
                                         _mm256_castps_si256(lower.native()), shift*4);
        return _mm256_castsi256_ps(res);
    }
#else
    case 1: return shuffle4x2<1,2,3,4>(lower, upper);
    case 3: return shuffle4x2<3,4,5,6>(lower, upper);
#endif
    case 2: return _mm256_shuffle_ps(lower.native(), upper.native(),
                                     SIMDPP_SHUFFLE_MASK_4x4(2,3,0,1));
    case 4: return upper;
    }
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned shift> SIMDPP_INL
float32<16> i_align4(const float32<16>& lower, const float32<16>& upper)
{
    switch (shift) {
    default:
    case 0: return lower;
    case 1: return shuffle4x2<1,2,3,4>(lower, upper);
    case 2: return _mm512_shuffle_ps(lower.native(), upper.native(),
                                     SIMDPP_SHUFFLE_MASK_4x4(2,3,0,1));
    case 3: return shuffle4x2<3,4,5,6>(lower, upper);
    case 4: return upper;
    }
}
#endif

template<unsigned shift, unsigned N> SIMDPP_INL
float32<N> i_align4(const float32<N>& lower, const float32<N>& upper)
{
    SIMDPP_VEC_ARRAY_IMPL2(float32<N>, i_align4<shift>, lower, upper);
}

// -----------------------------------------------------------------------------

template<unsigned shift> SIMDPP_INL
float64x2 i_align2(const float64x2& lower, const float64x2& upper)
{
#if SIMDPP_USE_SSE2
    switch (shift) {
    default:
    case 0: return lower;
    case 1: return _mm_shuffle_pd(lower.native(), upper.native(),
                                  SIMDPP_SHUFFLE_MASK_2x2(1, 0));
    case 2: return upper;
    }
#elif SIMDPP_USE_NEON64
    if (shift == 0)
        return lower;
    if (shift == 2)
        return upper;
    return vextq_f64(lower.native(), upper.native(), shift);
#elif SIMDPP_USE_VSX_206
    return (float64<2>) vec_sld_biendian<shift*8>((uint8<16>)lower,
                                                  (uint8<16>)upper);
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC || SIMDPP_USE_NEON32
    float64x2 r;
    //use int to disable warnings wrt. comparison result always being true/false
    for (int i = 0; i < (int)(2-shift); i++) {
        r.el(i) = lower.el(i + shift);
    }
    for (unsigned i = 2-shift; i < 2; i++) {
        r.el(i) = upper.el(i - 2 + shift);
    }
    return r;
#elif SIMDPP_USE_MSA
    return (v2f64) __msa_sld_b((v16i8) upper.native(),
                               (v16i8) lower.native(), shift*8);
#else
    return SIMDPP_NOT_IMPLEMENTED_TEMPLATE2(float64<shift+4>, lower, upper);
#endif
}

#if SIMDPP_USE_AVX
template<unsigned shift> SIMDPP_INL
float64<4> i_align2(const float64<4>& lower, const float64<4>& upper)
{
    switch (shift) {
    default:
    case 0: return lower;
    case 1: return _mm256_shuffle_pd(lower.native(), upper.native(),
                                     SIMDPP_SHUFFLE_MASK_2x2_2(1, 0));
    case 2: return upper;
    }
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned shift> SIMDPP_INL
float64<8> i_align2(const float64<8>& lower, const float64<8>& upper)
{
    switch (shift) {
    default:
    case 0: return lower;
    case 1: return _mm512_shuffle_pd(lower.native(), upper.native(),
                                     SIMDPP_SHUFFLE_MASK_2x2_4(1, 0));
    case 2: return upper;
    }
}
#endif

template<unsigned shift, unsigned N> SIMDPP_INL
float64<N> i_align2(const float64<N>& lower, const float64<N>& upper)
{
    SIMDPP_VEC_ARRAY_IMPL2(float64<N>, i_align2<shift>, lower, upper);
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

