/*  Copyright (C) 2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_I_SHIFT_R_V_H
#define LIBSIMDPP_SIMDPP_CORE_I_SHIFT_R_V_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/null/math.h>
#include <simdpp/detail/insn/i_shift.h>
#include <simdpp/detail/shuffle/shuffle_mask.h>
#include <simdpp/core/i_neg.h>
#include <simdpp/core/i_mul.h>
#include <simdpp/core/permute_bytes16.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

// emulates 8-bit variable shift using 16-bit variable shift
template<class U8> SIMDPP_INL
U8 v_emul_shift_r_u8_using_v16(const U8& a, const U8& count)
{
    using U16 = typename same_width<U8>::u16;

    U16 a16; a16 = a;
    U16 c16; c16 = count;

    U16 select_mask = make_uint(0x00ff);
    U16 a_lo = bit_and(a16, select_mask);
    U16 a_hi = a16;
    U16 c_lo = bit_and(c16, select_mask);
    U16 c_hi = shift_r<8>(c16);
    a_lo = shift_r(a_lo, c_lo);
    a_hi = shift_r(a_hi, c_hi);
    a_hi = bit_andnot(a_hi, select_mask);

    a16 = bit_or(a_lo, a_hi);
    return (U8) a16;
}

// emulates 8-bit variable shift using permute_bytes16 and 16-bit multiplication
template<class U8> SIMDPP_INL
U8 v_emul_shift_r_u8_using_mul(const U8& a, const U8& count)
{
    using U16 = typename same_width<U8>::u16;

    // Variable shift is implemented by reusing shifter in 16-bit unsigned
    // multiplication. The result is obtained by computing 1 << (8-countN)
    // for each element from a, multiplying each element by that number and
    // selecting the high half of the result.
    U8 mulshift_mask = make_uint(0x80, 0x40, 0x20, 0x10,
                                 0x08, 0x04, 0x02, 0x01,
                                 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00);
    U16 mulshift = (U16) permute_bytes16(mulshift_mask, count);
    U16 a16; a16 = a;
    U16 a16_lo, a16_hi, mulshift_lo, mulshift_hi;
    U16 select_mask = make_uint(0x00ff);

    // Move the element values to the high byte of the 16-bit elements and the
    // shift values to the low 9 bits. The 9-th bit is needed because in order
    // to shift by 0 the element values need to be multiplied by 0x100.
    // The results will have the high byte clear which will help composing the
    // result back to a single vector.
    a16_lo = shift_l<8>(a16);
    mulshift_lo = bit_and(mulshift, select_mask);
    mulshift_lo = shift_l<1>(mulshift_lo);
    a16_hi = bit_andnot(a16, select_mask);
    mulshift_hi = shift_l<1>(shift_r<8>(mulshift));

    a16_lo = mul_hi(a16_lo, mulshift_lo);
    a16_hi = mul_hi(a16_hi, mulshift_hi);

    a16_hi = shift_l<8>(a16_hi);
    a16 = bit_or(a16_lo, a16_hi);
    return (U8) a16;
}

static SIMDPP_INL
uint8<16> i_shift_r_v(const uint8<16>& a, const uint8<16>& count)
{
#if SIMDPP_USE_NULL
    return detail::null::shift_r_v(a, count);
#elif SIMDPP_USE_AVX512BW && SIMDPP_USE_AVX512VL
    return v_emul_shift_r_u8_using_v16(a, count);
#elif SIMDPP_USE_SSSE3
    return v_emul_shift_r_u8_using_mul(a, count);
#elif SIMDPP_USE_NEON
    int8<16> qcount = neg((int8<16>)count);
    return vshlq_u8(a.native(), qcount.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_sr(a.native(), count.native());
#elif SIMDPP_USE_MSA
    return (v16u8) __msa_srl_b((v16i8)a.native(), (v16i8)count.native());
#else
    return SIMDPP_NOT_IMPLEMENTED2(a, count);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint8<32> i_shift_r_v(const uint8<32>& a, const uint8<32>& count)
{
#if SIMDPP_USE_AVX512BW && SIMDPP_USE_AVX512VL
    return v_emul_shift_r_u8_using_v16(a, count);
#else
    return v_emul_shift_r_u8_using_mul(a, count);
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
uint8<64> i_shift_r_v(const uint8<64>& a, const uint8<64>& count)
{
    return v_emul_shift_r_u8_using_v16(a, count);
}
#endif

// -----------------------------------------------------------------------------

// emulates 8-bit variable shift using 16-bit variable shift
template<class I8, class U8> SIMDPP_INL
I8 v_emul_shift_r_i8_using_v16(const I8& a, const U8& count)
{
    using I16 = typename same_width<I8>::i16;
    using U16 = typename same_width<I8>::u16;

    U16 a16; a16 = a;
    U16 c16; c16 = count;

    U16 select_mask = make_uint(0x00ff);
    U16 a_lo = shift_l<8>(a16);
    U16 a_hi = a16;
    U16 c_lo = bit_and(c16, select_mask);
    U16 c_hi = shift_r<8>(c16);
    a_lo = shift_r((I16)a_lo, c_lo);
    a_hi = shift_r((I16)a_hi, c_hi);

    a_lo = shift_r<8>(a_lo);
    a_hi = bit_andnot(a_hi, select_mask);
    a16 = bit_or(a_lo, a_hi);

    return (I8) a16;
}

template<class I8, class U8> SIMDPP_INL
I8 v_emul_shift_r_i8_using_mul(const I8& a, const U8& count)
{
    using U16 = typename same_width<U8>::u16;
    using I16 = typename same_width<U8>::i16;

    // Variable shift is implemented by reusing shifter in 16-bit signed
    // multiplication. The result is obtained by computing 1 << (8-countN)
    // for each element from a, multiplying each element by that number and
    // selecting the high half of the result.
    U8 mulshift_mask = make_uint(0x80, 0x40, 0x20, 0x10,
                                 0x08, 0x04, 0x02, 0x01,
                                 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00);
    U16 mulshift = (U16) permute_bytes16(mulshift_mask, count);
    U16 a16; a16 = a;
    U16 a16_lo, a16_hi, mulshift_lo, mulshift_hi;
    U16 select_mask = make_uint(0x00ff);

    // Move the element values to the high byte of the 16-bit elements and the
    // shift values to the low 9 bits. The 9-th bit is needed because in order
    // to shift by 0 the element values need to be multiplied by 0x100.
    // Note that the results may have nonzero high byte because this is signed
    // multiplication.
    a16_lo = shift_l<8>(a16);
    mulshift_lo = bit_and(mulshift, select_mask);
    mulshift_lo = shift_l<1>(mulshift_lo);
    a16_hi = bit_andnot(a16, select_mask);
    mulshift_hi = shift_l<1>(shift_r<8>(mulshift));

    a16_lo = mul_hi((I16)a16_lo, (I16)mulshift_lo);
    a16_hi = mul_hi((I16)a16_hi, (I16)mulshift_hi);

    a16_hi = shift_l<8>(a16_hi);
    a16_lo = bit_and(a16_lo, select_mask);
    a16 = bit_or(a16_lo, a16_hi);
    return (U8) a16;
}

static SIMDPP_INL
int8<16> i_shift_r_v(const int8<16>& a, const uint8<16>& count)
{
#if SIMDPP_USE_NULL
    return detail::null::shift_r_v(a, count);
#elif SIMDPP_USE_AVX512BW && SIMDPP_USE_AVX512VL
    return v_emul_shift_r_i8_using_v16(a, count);
#elif SIMDPP_USE_SSSE3
    return v_emul_shift_r_i8_using_mul(a, count);
#elif SIMDPP_USE_NEON
    int8<16> qcount = neg((int8<16>)count);
    return vshlq_s8(a.native(), qcount.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_sra(a.native(), count.native());
#elif SIMDPP_USE_MSA
    return __msa_sra_b(a.native(), (v16i8) count.native());
#else
    return SIMDPP_NOT_IMPLEMENTED2(a, count);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int8<32> i_shift_r_v(const int8<32>& a, const uint8<32>& count)
{
#if SIMDPP_USE_AVX512BW && SIMDPP_USE_AVX512VL
    return v_emul_shift_r_i8_using_v16(a, count);
#else
    return v_emul_shift_r_i8_using_mul(a, count);
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
int8<64> i_shift_r_v(const int8<64>& a, const uint8<64>& count)
{
    return v_emul_shift_r_i8_using_v16(a, count);
}
#endif

// -----------------------------------------------------------------------------

// emulates 16-bit variable shift using permute_bytes16 and 16-bit multiplication
template<class U16>
U16 v_emul_shift_r_u16_using_mul(const U16& a, const U16& count)
{
    using U8 = typename same_width<U16>::u8;
    using M16 = typename U16::mask_vector_type;
    // Variable shift is implemented by reusing shifter in 16-bit unsigned
    // multiplication. The result is obtained by computing 1 << (16-countN-1)
    // for each element from a, multiplying each element by that number and
    // selecting the high half of the result. Note that the highest shift
    // available when using 16-bit multiplication is 15, which needs to be
    // worked around by extra instructions.
    M16 is_same = cmp_eq(count, 0);
    M16 is_zero = cmp_gt(count, 15);

    U8 mulshift_mask = make_uint(0x00, 0x80, 0x40, 0x20,
                                 0x10, 0x08, 0x04, 0x02,
                                 0x01, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00);

    // permute_bytes16 permutes 8-bit elements instead of 16 which would be
    // optimal in this case. We need to construct the selector in special way
    // for 8-bit permutation.
    // The 4-th is toggled bit so that the high byte takes zeros from the
    // mulshift mask when the shift count is higher than 8.
    U16 qcount = bit_or(count, shift_l<8>(count));
    qcount = bit_xor(qcount, 0x0008);

    U16 mulshift = (U16) permute_bytes16(mulshift_mask, (U8) qcount);
    U16 res = mul_hi(a, mulshift);
    res = blend(a, res, is_same);
    res = bit_andnot(res, is_zero);
    return res;
}

static SIMDPP_INL
uint16<8> i_shift_r_v(const uint16<8>& a, const uint16<8>& count)
{
#if SIMDPP_USE_NULL
    return detail::null::shift_r_v(a, count);
#elif SIMDPP_USE_AVX512BW && SIMDPP_USE_AVX512VL
    return _mm_srlv_epi16(a.native(), count.native());
#elif SIMDPP_USE_SSSE3
    return v_emul_shift_r_u16_using_mul(a, count);
#elif SIMDPP_USE_NEON
    int16<8> qcount = neg((int16<8>)count);
    return vshlq_u16(a.native(), qcount.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_sr(a.native(), count.native());
#elif SIMDPP_USE_MSA
    return (v8u16) __msa_srl_h((v8i16)a.native(), (v8i16)count.native());
#else
    return SIMDPP_NOT_IMPLEMENTED2(a, count);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint16<16> i_shift_r_v(const uint16<16>& a, const uint16<16>& count)
{
#if SIMDPP_USE_AVX512BW && SIMDPP_USE_AVX512VL
    return _mm256_srlv_epi16(a.native(), count.native());
#else
    return v_emul_shift_r_u16_using_mul(a, count);
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint16<32> i_shift_r_v(const uint16<32>& a, const uint16<32>& count)
{
    return _mm512_srlv_epi16(a.native(), count.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
int16<8> i_shift_r_v(const int16<8>& a, const uint16<8>& count)
{
#if SIMDPP_USE_NULL
    return detail::null::shift_r_v(a, count);
#elif SIMDPP_USE_AVX512BW && SIMDPP_USE_AVX512VL
    return _mm_srav_epi16(a.native(), count.native());
#elif SIMDPP_USE_AVX512BW
    __m512i a512 = _mm512_castsi128_si512(a.native());
    __m512i count512 = _mm512_castsi128_si512(count.native());
    return _mm512_castsi512_si128(_mm512_srav_epi16(a512, count512));
#elif SIMDPP_USE_NEON
    int16<8> qcount = neg((int16<8>)count);
    return vshlq_s16(a.native(), qcount.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_sra(a.native(), count.native());
#elif SIMDPP_USE_MSA
    return __msa_sra_h(a.native(), (v8i16) count.native());
#else
    return SIMDPP_NOT_IMPLEMENTED2(a, count);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int16<16> i_shift_r_v(const int16<16>& a, const uint16<16>& count)
{
#if SIMDPP_USE_AVX512BW && SIMDPP_USE_AVX512VL
    return _mm256_srav_epi16(a.native(), count.native());
#elif SIMDPP_USE_AVX512BW
    __m512i a512 = _mm512_castsi256_si512(a.native());
    __m512i count512 = _mm512_castsi256_si512(count.native());
    return _mm512_castsi512_si256(_mm512_srav_epi16(a512, count512));
#else
    return SIMDPP_NOT_IMPLEMENTED2(a, count);
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL int16<32> i_shift_r_v(const int16<32>& a, const uint16<32>& count)
{
    return _mm512_srav_epi16(a.native(), count.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint32<4> i_shift_r_v(const uint32<4>& a, const uint32<4>& count)
{
#if SIMDPP_USE_NULL
    return detail::null::shift_r_v(a, count);
#elif SIMDPP_USE_AVX2
    return _mm_srlv_epi32(a.native(), count.native());
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

    __m128i a0 = _mm_srl_epi32(a.native(), count0.native());
    __m128i a1 = _mm_srl_epi32(a.native(), count1.native());
    __m128i a2 = _mm_srl_epi32(a.native(), count2.native());
    __m128i a3 = _mm_srl_epi32(a.native(), count3.native());
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
    int32<4> qcount = neg((int32<4>)count);
    return vshlq_u32(a.native(), qcount.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_sr(a.native(), count.native());
#elif SIMDPP_USE_MSA
    return (v4u32) __msa_srl_w((v4i32)a.native(), (v4i32)count.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint32<8> i_shift_r_v(const uint32<8>& a, const uint32<8>& count)
{
    return _mm256_srlv_epi32(a.native(), count.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint32<16> i_shift_r_v(const uint32<16>& a, const uint32<16>& count)
{
    return _mm512_srlv_epi32(a.native(), count.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
int32<4> i_shift_r_v(const int32<4>& a, const uint32<4>& count)
{
#if SIMDPP_USE_NULL
    return detail::null::shift_r_v(a, count);
#elif SIMDPP_USE_AVX2
    return _mm_srav_epi32(a.native(), count.native());
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

    __m128i a0 = _mm_sra_epi32(a.native(), count0.native());
    __m128i a1 = _mm_sra_epi32(a.native(), count1.native());
    __m128i a2 = _mm_sra_epi32(a.native(), count2.native());
    __m128i a3 = _mm_sra_epi32(a.native(), count3.native());
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
    int32<4> qcount = neg((int32<4>)count);
    return vshlq_s32(a.native(), qcount.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_sra(a.native(), count.native());
#elif SIMDPP_USE_MSA
    return __msa_sra_w(a.native(), (v4i32)count.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int32<8> i_shift_r_v(const int32<8>& a, const uint32<8>& count)
{
    return _mm256_srav_epi32(a.native(), count.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
int32<16> i_shift_r_v(const int32<16>& a, const uint32<16>& count)
{
    return _mm512_srav_epi32(a.native(), count.native());
}
#endif

// -----------------------------------------------------------------------------

template<class V, class U> SIMDPP_INL
V i_shift_r_v(const V& a, const U& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(V, i_shift_r_v, a, b);
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
