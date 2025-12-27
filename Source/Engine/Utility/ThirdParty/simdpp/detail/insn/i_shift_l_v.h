/*  Copyright (C) 2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_I_SHIFT_L_V_H
#define LIBSIMDPP_SIMDPP_CORE_I_SHIFT_L_V_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/null/math.h>
#include <simdpp/detail/insn/i_shift.h>
#include <simdpp/detail/shuffle/shuffle_mask.h>
#include <simdpp/core/i_mul.h>
#include <simdpp/core/permute_bytes16.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

// emulates 8-bit variable shift using 16-bit variable shift
template<class U8> SIMDPP_INL
U8 v_emul_shift_l_v8_using_v16(const U8& a, const U8& count)
{
    using U16 = typename same_width<U8>::u16;

    U16 a16; a16 = a;
    U16 c16; c16 = count;

    U16 select_mask = make_uint(0xff00);
    U16 a_lo = a16;
    U16 a_hi = bit_and(a16, select_mask);
    U16 c_lo = bit_andnot(c16, select_mask);
    U16 c_hi = shift_r<8>(c16);
    a_lo = shift_l(a_lo, c_lo);
    a_hi = shift_l(a_hi, c_hi);
    a_lo = bit_andnot(a_lo, select_mask);

    a16 = bit_or(a_lo, a_hi);
    return (U8) a16;
}

// emulates 8-bit variable shift using permute_bytes16 and 16-bit multiplication
template<class U8> SIMDPP_INL
U8 v_emul_shift_l_v8_using_mul(const U8& a, const U8& count)
{
    using U16 = typename same_width<U8>::u16;

    // Variable shift is implemented by obtaining 1 << countN for each element
    // from a and then multiplying each element by that number. Implementation
    // is complicated by the fact, that only 16-bit multiplication is available.
    U8 mulshift_mask = make_uint(0x01, 0x02, 0x04, 0x08,
                                 0x10, 0x20, 0x40, 0x80);
    U16 mulshift = (U16) permute_bytes16(mulshift_mask, count);

    U16 a16; a16 = a;
    U16 a16_lo, a16_hi, mulshift_lo, mulshift_hi;
    U16 select_mask = make_uint(0x00ff);

    // Move the element values to the high byte of the 16-bit elements and the
    // shift values to the low byte. The results will have the low byte clear
    // which will help composing the result back to a single vector.
    a16_lo = shift_l<8>(a16);
    mulshift_lo = bit_and(mulshift, select_mask);
    a16_hi = bit_andnot(a16, select_mask);
    mulshift_hi = shift_r<8>(mulshift);

    a16_lo = mul_lo(a16_lo, mulshift_lo);
    a16_hi = mul_lo(a16_hi, mulshift_hi);

    a16_lo = shift_r<8>(a16_lo);
    a16 = bit_or(a16_lo, a16_hi);
    return (U8) a16;
}

static SIMDPP_INL
uint8<16> i_shift_l_v(const uint8<16>& a, const uint8<16>& count)
{
#if SIMDPP_USE_NULL
    return detail::null::shift_l_v(a, count);
#elif SIMDPP_USE_AVX512BW && SIMDPP_USE_AVX512VL
    return v_emul_shift_l_v8_using_v16(a, count);
#elif SIMDPP_USE_SSSE3
    return v_emul_shift_l_v8_using_mul(a, count);
#elif SIMDPP_USE_NEON
    return vshlq_u8(a.native(), vreinterpretq_s8_u8(count.native()));
#elif SIMDPP_USE_ALTIVEC
    return vec_sl(a.native(), count.native());
#elif SIMDPP_USE_MSA
    return (v16u8) __msa_sll_b((v16i8)a.native(), (v16i8)count.native());
#else
    return SIMDPP_NOT_IMPLEMENTED2(a, count);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint8<32> i_shift_l_v(const uint8<32>& a, const uint8<32>& count)
{
#if SIMDPP_USE_AVX512BW && SIMDPP_USE_AVX512VL
    return v_emul_shift_l_v8_using_v16(a, count);
#else
    return v_emul_shift_l_v8_using_mul(a, count);
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
uint8<64> i_shift_l_v(const uint8<64>& a, const uint8<64>& count)
{
    return v_emul_shift_l_v8_using_v16(a, count);
}
#endif

// -----------------------------------------------------------------------------

// emulates 16-bit variable shift using permute_bytes16 and 16-bit multiplication
template<class U16>
U16 v_emul_shift_l_v16_using_mul(const U16& a, const U16& count)
{
    using U8 = typename same_width<U16>::u8;

    // Variable shift is implemented by obtaining 1 << countN for each element
    // from a and then multiplying each element by that number. The
    // implementation is complicated by the fact that permute_bytes16 permutes
    // 8-bit elements instead of 16 which would be optimal in this case
    U8 mulshift_mask = make_uint(0x01, 0x02, 0x04, 0x08,
                                 0x10, 0x20, 0x40, 0x80,
                                 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00);
    U16 qcount = bit_or(count, shift_l<8>(count));

    // toggle the 4-th bit so that the high byte takes zeros from the mulshift
    // mask when the shift count is higher than 8.
    qcount = bit_xor(qcount, 0x0800);
    U16 mulshift = (U16) permute_bytes16(mulshift_mask, (U8) qcount);
    return mul_lo(a, mulshift);
}

static SIMDPP_INL
uint16<8> i_shift_l_v(const uint16<8>& a, const uint16<8>& count)
{
#if SIMDPP_USE_NULL
    return detail::null::shift_l_v(a, count);
#elif SIMDPP_USE_AVX512BW && SIMDPP_USE_AVX512VL
    return _mm_sllv_epi16(a.native(), count.native());
#elif SIMDPP_USE_SSSE3
    return v_emul_shift_l_v16_using_mul(a, count);
#elif SIMDPP_USE_NEON
    return vshlq_u16(a.native(), vreinterpretq_s16_u16(count.native()));
#elif SIMDPP_USE_ALTIVEC
    return vec_sl(a.native(), count.native());
#elif SIMDPP_USE_MSA
    return (v8u16) __msa_sll_h((v8i16)a.native(), (v8i16)count.native());
#else
    return SIMDPP_NOT_IMPLEMENTED2(a, count);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint16<16> i_shift_l_v(const uint16<16>& a, const uint16<16>& count)
{
#if SIMDPP_USE_AVX512BW && SIMDPP_USE_AVX512VL
    return _mm256_sllv_epi16(a.native(), count.native());
#else
    return v_emul_shift_l_v16_using_mul(a, count);
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint16<32> i_shift_l_v(const uint16<32>& a, const uint16<32>& count)
{
    return _mm512_sllv_epi16(a.native(), count.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint32<4> i_shift_l_v(const uint32<4>& a, const uint32<4>& count)
{
#if SIMDPP_USE_NULL
    return detail::null::shift_l_v(a, count);
#elif SIMDPP_USE_AVX2
    return _mm_sllv_epi32(a.native(), count.native());
#elif SIMDPP_USE_SSE2
    uint32<4> count0 = count;
#if SIMDPP_USE_SSE4_1
    uint32<4> zero = make_zero();
    count0 = _mm_blend_epi16(count0.native(), zero.native(), 0xcc);
#else
    uint32<4> mask = make_uint(0xffffffff, 0, 0xffffffff, 0);
    count0 = bit_and(count0, mask);
#endif
    uint32<4> count1 = _mm_srli_epi64(count.native(), 32);
    uint32<4> count2 = _mm_srli_si128(count0.native(), 8);
    uint32<4> count3 = _mm_srli_si128(count.native(), 12);

    __m128i a0 = _mm_sll_epi32(a.native(), count0.native());
    __m128i a1 = _mm_sll_epi32(a.native(), count1.native());
    __m128i a2 = _mm_sll_epi32(a.native(), count2.native());
    __m128i a3 = _mm_sll_epi32(a.native(), count3.native());
#if SIMDPP_USE_SSE4_1
    a0 = _mm_blend_epi16(a0, a1, 0x0c);
    a2 = _mm_blend_epi16(a2, a3, 0xc0);
    a0 = _mm_blend_epi16(a0, a2, 0xf0);
#else
    __m128 f0 = _mm_shuffle_ps(_mm_castsi128_ps(a0),
                               _mm_castsi128_ps(a1),
                               SIMDPP_SHUFFLE_MASK_4x4(0, 0, 1, 1));
    __m128 f1 = _mm_shuffle_ps(_mm_castsi128_ps(a2),
                               _mm_castsi128_ps(a3),
                               SIMDPP_SHUFFLE_MASK_4x4(2, 2, 3, 3));
    f0 = _mm_shuffle_ps(f0, f1, SIMDPP_SHUFFLE_MASK_4x4(0, 2, 0, 2));
    a0 = _mm_castps_si128(f0);
#endif
    return a0;
#elif SIMDPP_USE_NEON
    return vshlq_u32(a.native(), vreinterpretq_s32_u32(count.native()));
#elif SIMDPP_USE_ALTIVEC
    return vec_sl(a.native(), count.native());
#elif SIMDPP_USE_MSA
    return (v4u32) __msa_sll_w((v4i32)a.native(), (v4i32)count.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint32<8> i_shift_l_v(const uint32<8>& a, const uint32<8>& count)
{
    return _mm256_sllv_epi32(a.native(), count.native());
}
#endif

#if SIMDPP_USE_AVX512F
SIMDPP_INL uint32<16> i_shift_l_v(const uint32<16>& a, const uint32<16>& count)
{
    return _mm512_sllv_epi32(a.native(), count.native());
}
#endif

// -----------------------------------------------------------------------------

template<class V, class U> SIMDPP_INL
V i_shift_l_v(const V& a, const U& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(V, i_shift_l_v, a, b);
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
