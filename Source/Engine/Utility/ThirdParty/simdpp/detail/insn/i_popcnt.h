/*  Copyright (C) 2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_I_POPCNT_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_I_POPCNT_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/bit_and.h>
#include <simdpp/core/extract.h>
#include <simdpp/core/i_add.h>
#include <simdpp/core/i_shift_r.h>
#include <simdpp/core/i_sub.h>
#include <simdpp/core/i_mul.h>
#include <simdpp/core/insert.h>
#include <simdpp/detail/null/bitwise.h>
#include <simdpp/detail/width.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

template<class V> SIMDPP_INL
V v_emul_popcnt_u8(const V& a)
{
    // We're using 16-bit ops because on SSE/AVX no 8-bit shift is available
    // There's no difference on other architectures
    using w_b16 = typename same_width<V>::u16;

    w_b16 p = (w_b16)a;
    w_b16 m55 = splat(0x5555);
    w_b16 m33 = splat(0x3333);
    w_b16 m0f = splat(0x0f0f);

    p = sub(p, bit_and(shift_r<1>(p), m55));
    p = add(bit_and(p, m33), bit_and(shift_r<2>(p), m33));
    p = bit_and(add(p, shift_r<4>(p)), m0f);
    return (V) p;
}

static SIMDPP_INL
uint8<16> i_popcnt(const uint8<16>& a)
{
#if SIMDPP_USE_NULL
    uint8<16> r;
    for (unsigned i = 0; i < a.length; i++) {
        r.el(i) = detail::null::el_popcnt8(a.el(i));
    }
    return r;
#elif SIMDPP_USE_NEON
    return vcntq_u8(a.native());
#elif SIMDPP_USE_VSX_207
    return vec_vpopcnt(a.native());
#elif SIMDPP_USE_MSA
    return (v16u8) __msa_pcnt_b((v16i8) a.native());
#else
    return v_emul_popcnt_u8(a);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint8<32> i_popcnt(const uint8<32>& a)
{
    return v_emul_popcnt_u8(a);
}
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
uint8<64> i_popcnt(const uint8<64>& a)
{
    return v_emul_popcnt_u8(a);
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
V v_emul_popcnt_u16(const V& a)
{
    V p = a;
    V m55 = splat(0x5555);
    V m33 = splat(0x3333);
    V m0f = splat(0x0f0f);
    V res_mask = splat(0x00ff);


    p = sub(p, bit_and(shift_r<1>(p), m55));
    p = add(bit_and(p, m33), bit_and(shift_r<2>(p), m33));
    p = bit_and(add(p, shift_r<4>(p)), m0f);
    p = add(p, shift_r<8>(p));
    p = bit_and(p, res_mask);
    return p;
}

static SIMDPP_INL
uint16<8> i_popcnt(const uint16<8>& a)
{
#if SIMDPP_USE_NULL
    uint16<8> r;
    for (unsigned i = 0; i < a.length; i++) {
        r.el(i) = detail::null::el_popcnt16(a.el(i));
    }
    return r;
#elif SIMDPP_USE_NEON
    uint8x16_t p8 = vcntq_u8(vreinterpretq_u8_u16(a.native()));
    return vpaddlq_u8(p8);
#elif SIMDPP_USE_VSX_207
    return vec_vpopcnt(a.native());
#elif SIMDPP_USE_MSA
    return (v8u16) __msa_pcnt_h((v8i16) a.native());
#else
    return v_emul_popcnt_u16(a);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint16<16> i_popcnt(const uint16<16>& a)
{
    return v_emul_popcnt_u16(a);
}
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
uint16<32> i_popcnt(const uint16<32>& a)
{
    return v_emul_popcnt_u16(a);
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
V v_emul_popcnt_u32(const V& a)
{
    V p = a;
    V m55 = splat(0x55555555);
    V m33 = splat(0x33333333);
    V m0f = splat(0x0f0f0f0f);

    p = sub(p, bit_and(shift_r<1>(p), m55));
    p = add(bit_and(p, m33), bit_and(shift_r<2>(p), m33));
    p = bit_and(add(p, shift_r<4>(p)), m0f);
#if SIMDPP_USE_SSE4_1 || SIMDPP_USE_NEON || SIMDPP_USE_MSA
    V m01 = splat(0x01010101);
    // rather than doing 2 adds + 2 shifts we can do 1 mul + 1 shift
    p = shift_r<24>(mul_lo(p, m01));
#else
    V res_mask = splat(0x000000ff);
    p = add(p, shift_r<8>(p));
    p = add(p, shift_r<16>(p));
    p = bit_and(p, res_mask);
#endif
    return p;
}

static SIMDPP_INL
uint32<4> i_popcnt(const uint32<4>& a)
{
#if SIMDPP_USE_NULL
    uint32<4> r;
    for (unsigned i = 0; i < a.length; i++) {
        r.el(i) = detail::null::el_popcnt32(a.el(i));
    }
    return r;
#elif SIMDPP_USE_X86_POPCNT_INSN
    // slightly faster than the vectorized version
    unsigned a0 = _mm_popcnt_u32(extract<0>(a));
    unsigned a1 = _mm_popcnt_u32(extract<1>(a));
    unsigned a2 = _mm_popcnt_u32(extract<2>(a));
    unsigned a3 = _mm_popcnt_u32(extract<3>(a));
    uint16<8> r = _mm_cvtsi32_si128(a0);
    r = insert<2>(r, a1);
    r = insert<4>(r, a2);
    r = insert<6>(r, a3);
    return (uint32<4>) r;
#elif SIMDPP_USE_NEON
    uint8x16_t p8 = vcntq_u8(vreinterpretq_u8_u32(a.native()));
    uint16x8_t p16 = vpaddlq_u8(p8);
    return vpaddlq_u16(p16);
#elif SIMDPP_USE_VSX_207
    return vec_vpopcnt(a.native());
#elif SIMDPP_USE_MSA
    return (v4u32) __msa_pcnt_w((v4i32) a.native());
#else
    return v_emul_popcnt_u32(a);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint32<8> i_popcnt(const uint32<8>& a)
{
    return v_emul_popcnt_u32(a);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint32<16> i_popcnt(const uint32<16>& a)
{
    // TODO: support AVX512VPOPCNTDQ
    return v_emul_popcnt_u32(a);
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
V v_emul_popcnt_u64(const V& a)
{
    V p = a;
    V m55 = splat(0x5555555555555555);
    V m33 = splat(0x3333333333333333);
    V m0f = splat(0x0f0f0f0f0f0f0f0f);
    V res_mask = splat(0x00000000000000ff);


    p = sub(p, bit_and(shift_r<1>(p), m55));
    p = add(bit_and(p, m33), bit_and(shift_r<2>(p), m33));
    p = bit_and(add(p, shift_r<4>(p)), m0f);
    p = add(p, shift_r<8>(p));
    p = add(p, shift_r<16>(p));
    p = add(p, shift_r<32>(p));
    p = bit_and(p, res_mask);
    return p;
}

static SIMDPP_INL
uint64<2> i_popcnt(const uint64<2>& a)
{
#if SIMDPP_USE_NULL
    uint64<2> r;
    for (unsigned i = 0; i < a.length; i++) {
        r.el(i) = detail::null::el_popcnt64(a.el(i));
    }
    return r;
#elif SIMDPP_USE_X86_POPCNT_INSN
    unsigned a0, a1;
#if SIMDPP_64_BITS
    a0 = _mm_popcnt_u64(extract<0>(a));
    a1 = _mm_popcnt_u64(extract<1>(a));
#else
    uint32<4> a32; a32 = a;
    a0 =  _mm_popcnt_u32(extract<0>(a32));
    a0 += _mm_popcnt_u32(extract<1>(a32));
    a1 =  _mm_popcnt_u32(extract<2>(a32));
    a1 += _mm_popcnt_u32(extract<3>(a32));
#endif
    uint16<8> r = _mm_cvtsi32_si128(a0);
    r = insert<4>(r, a1);
    return (uint64<2>) r;
#elif SIMDPP_USE_SSE2
    uint8<16> p8 = v_emul_popcnt_u8((uint8<16>) a);
    return _mm_sad_epu8(p8.native(), _mm_setzero_si128());
#elif SIMDPP_USE_NEON
    uint8x16_t p8 = vcntq_u8(vreinterpretq_u8_u64(a.native()));
    uint16x8_t p16 = vpaddlq_u8(p8);
    uint32x4_t p32 = vpaddlq_u16(p16);
    return vpaddlq_u32(p32);
#elif SIMDPP_USE_VSX_207
    return vec_vpopcnt(a.native());
#elif SIMDPP_USE_MSA
    return (v2u64) __msa_pcnt_d((v2i64) a.native());
#else
    return v_emul_popcnt_u64(a);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint64<4> i_popcnt(const uint64<4>& a)
{
#if SIMDPP_USE_X86_POPCNT_INSN && SIMDPP_64_BITS
    uint64<2> a0, a1;
    split(a, a0, a1);
    a0 = i_popcnt(a0);
    a1 = i_popcnt(a1);
    return combine(a0, a1);
#else
    uint8<32> p8 = v_emul_popcnt_u8((uint8<32>) a);
    return _mm256_sad_epu8(p8.native(), _mm256_setzero_si256());
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint64<8> i_popcnt(const uint64<8>& a)
{
    // TODO: support AVX512VPOPCNTDQ
#if SIMDPP_USE_AVX512BW
    uint8<64> p8 = v_emul_popcnt_u8((uint8<64>) a);
    return _mm512_sad_epu8(p8.native(), _mm512_setzero_si512());
#else
    return v_emul_popcnt_u64(a);
#endif
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
V i_popcnt(const V& a)
{
    SIMDPP_VEC_ARRAY_IMPL1(V, i_popcnt, a)
}


} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

