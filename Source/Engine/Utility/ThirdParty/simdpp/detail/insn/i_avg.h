/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_I_AVG_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_I_AVG_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/bit_xor.h>
#include <simdpp/core/bit_and.h>
#include <simdpp/core/i_add.h>
#include <simdpp/core/i_shift_r.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

template<class V> SIMDPP_INL
V v_emul_avg_u32(const V& a, const V& b);
template<class V> SIMDPP_INL
V v_emul_avg_i32(const V& a, const V& b);

static SIMDPP_INL
uint8x16 i_avg(const uint8x16& a, const uint8x16& b)
{
#if SIMDPP_USE_NULL
    uint8x16 r;
    for (unsigned i = 0; i < a.length; i++) {
        r.el(i) = (uint16_t(a.el(i)) + b.el(i) + 1) >> 1;
    }
    return r;
#elif SIMDPP_USE_SSE2
    return _mm_avg_epu8(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vrhaddq_u8(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_avg(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_aver_u_b(a.native(), b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint8x32 i_avg(const uint8x32& a, const uint8x32& b)
{
    return _mm256_avg_epu8(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint8<64> i_avg(const uint8<64>& a, const uint8<64>& b)
{
    return _mm512_avg_epu8(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
int8x16 i_avg(const int8x16& a, const int8x16& b)
{
#if SIMDPP_USE_NULL
    int8x16 r;
    for (unsigned i = 0; i < a.length; i++) {
        r.el(i) = (int16_t(a.el(i)) + b.el(i) + 1) >> 1;
    }
    return r;
#elif SIMDPP_USE_SSE2
    uint8x16 a2, b2, bias, r;
    bias = make_uint(0x80);
    a2 = bit_xor(a, bias); // add
    b2 = bit_xor(b, bias); // add
    r = i_avg(a2, b2); // unsigned
    r = bit_xor(r, bias); // sub
    return r;
#elif SIMDPP_USE_NEON
    return vrhaddq_s8(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_avg(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_aver_s_b(a.native(), b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int8x32 i_avg(const int8x32& a, const int8x32& b)
{
    uint8x32 a2, b2, bias, r;
    bias = make_uint(0x80);
    a2 = bit_xor(a, bias); // add
    b2 = bit_xor(b, bias); // add
    r = i_avg(a2, b2); // unsigned
    r = bit_xor(r, bias); // sub
    return r;
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL int8<64>i_avg(const int8<64>& a, const int8<64>& b)
{
    uint8<64> a2, b2, bias, r;
    bias = make_uint(0x80);
    a2 = bit_xor(a, bias); // add
    b2 = bit_xor(b, bias); // add
    r = i_avg(a2, b2); // unsigned
    r = bit_xor(r, bias); // sub
    return r;
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint16x8 i_avg(const uint16x8& a, const uint16x8& b)
{
#if SIMDPP_USE_NULL
    uint16x8 r;
    for (unsigned i = 0; i < a.length; i++) {
        r.el(i) = (uint32_t(a.el(i)) + b.el(i) + 1) >> 1;
    }
    return r;
#elif SIMDPP_USE_SSE2
    return _mm_avg_epu16(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vrhaddq_u16(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_avg(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_aver_u_h(a.native(), b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint16x16 i_avg(const uint16x16& a, const uint16x16& b)
{
    return _mm256_avg_epu16(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint16<32> i_avg(const uint16<32>& a, const uint16<32>& b)
{
    return _mm512_avg_epu16(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
int16x8 i_avg(const int16x8& a, const int16x8& b)
{
#if SIMDPP_USE_NULL
    int16x8 r;
    for (unsigned i = 0; i < a.length; i++) {
        r.el(i) = (int32_t(a.el(i)) + b.el(i) + 1) >> 1;
    }
    return r;
#elif SIMDPP_USE_SSE2
    uint16x8 a2, b2, r;
    a2 = bit_xor(a, 0x8000); // add
    b2 = bit_xor(b, 0x8000); // add
    r = i_avg(a2, b2); // unsigned
    r = bit_xor(r, 0x8000); // sub
    return r;
#elif SIMDPP_USE_NEON
    return vrhaddq_s16(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_avg(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_aver_s_h(a.native(), b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int16x16 i_avg(const int16x16& a, const int16x16& b)
{
    uint16x16 a2, b2, r;
    a2 = bit_xor(a, 0x8000); // add
    b2 = bit_xor(b, 0x8000); // add
    r = i_avg(a2, b2); // unsigned
    r = bit_xor(r, 0x8000); // sub
    return r;
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL int16<32> i_avg(const int16<32>& a, const int16<32>& b)
{
    uint16<32> a2, b2, r;
    a2 = bit_xor(a, 0x8000); // add
    b2 = bit_xor(b, 0x8000); // add
    r = i_avg(a2, b2); // unsigned
    r = bit_xor(r, 0x8000); // sub
    return r;
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint32x4 i_avg(const uint32x4& a, const uint32x4& b)
{
#if SIMDPP_USE_NULL
    uint32x4 r;
    for (unsigned i = 0; i < a.length; i++) {
        r.el(i) = (uint64_t(a.el(i)) + b.el(i) + 1) >> 1;
    }
    return r;
#elif SIMDPP_USE_SSE2
    return v_emul_avg_u32(a, b);
#elif SIMDPP_USE_NEON
    return vrhaddq_u32(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_avg(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_aver_u_w(a.native(), b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint32x8 i_avg(const uint32x8& a, const uint32x8& b)
{
    return v_emul_avg_u32(a, b);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint32<16> i_avg(const uint32<16>& a, const uint32<16>& b)
{
    return v_emul_avg_u32(a, b);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
int32x4 i_avg(const int32x4& a, const int32x4& b)
{
#if SIMDPP_USE_NULL
    int32x4 r;
    for (unsigned i = 0; i < a.length; i++) {
        r.el(i) = (int64_t(a.el(i)) + b.el(i) + 1) >> 1;
    }
    return r;
#elif SIMDPP_USE_SSE2
    return v_emul_avg_i32(a, b);

#elif SIMDPP_USE_NEON
    return vrhaddq_s32(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_avg(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_aver_s_w(a.native(), b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int32x8 i_avg(const int32x8& a, const int32x8& b)
{
    return v_emul_avg_i32(a, b);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
int32<16> i_avg(const int32<16>& a, const int32<16>& b)
{
    return v_emul_avg_i32(a, b);
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
V i_avg(const V& a, const V& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(V, i_avg, a, b);
}

// generic implementations

template<class V> SIMDPP_INL
V v_emul_avg_u32(const V& a, const V& b)
{
    // (x & y) + ((x ^ y) >> 1) + (x ^ y) & 1
    V x1, x2, round;
    x1 = bit_and(a, b);
    x2 = bit_xor(a, b);
    round = bit_and(x2, 1);
    x1 = add(x1, shift_r<1>(x2));
    x1 = add(x1, round);
    return x1;
}

template<class V> SIMDPP_INL
V v_emul_avg_i32(const V& a, const V& b)
{
    using VI = typename V::uint_vector_type;
    VI a2, b2, r;
    a2 = bit_xor(a, 0x80000000); // add
    b2 = bit_xor(b, 0x80000000); // add
    r = v_emul_avg_u32(a2, b2); // unsigned
    r = bit_xor(r, 0x80000000); // sub
    return r;
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

