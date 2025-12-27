/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_I_MUL_HI_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_I_MUL_HI_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/i_mull.h>
#include <simdpp/core/unzip_hi.h>
#include <simdpp/detail/null/math.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

static SIMDPP_INL
int16<8> i_mul_hi(const int16<8>& a, const int16<8>& b)
{
#if SIMDPP_USE_NULL
    uint16<8> r;
    for (unsigned i = 0; i < a.length; i++) {
        r.el(i) = (int32_t(a.el(i)) * b.el(i)) >> 16;
    }
    return r;
#elif SIMDPP_USE_SSE2
    return _mm_mulhi_epi16(a.native(), b.native());
#elif SIMDPP_USE_NEON
    int32x4 lo = vmull_s16(vget_low_s16(a.native()), vget_low_s16(b.native()));
    int32x4 hi = vmull_s16(vget_high_s16(a.native()), vget_high_s16(b.native()));
    return unzip8_hi(int16x8(lo), int16x8(hi));
#elif SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
#if SIMDPP_BIG_ENDIAN
    int16<16> ab;
    ab = mull(a, b);
    return unzip8_lo(ab.vec(0), ab.vec(1));
#else
    int16<16> ab;
    ab = mull(a, b);
    return unzip8_hi(ab.vec(0), ab.vec(1));
#endif
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int16<16> i_mul_hi(const int16<16>& a, const int16<16>& b)
{
    return _mm256_mulhi_epi16(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
int16<32> i_mul_hi(const int16<32>& a, const int16<32>& b)
{
    return _mm512_mulhi_epi16(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint16<8> i_mul_hi(const uint16<8>& a, const uint16<8>& b)
{
#if SIMDPP_USE_NULL
    uint16<8> r;
    for (unsigned i = 0; i < a.length; i++) {
        r.el(i) = (uint32_t(a.el(i)) * b.el(i)) >> 16;
    }
    return r;
#elif SIMDPP_USE_SSE2
    return _mm_mulhi_epu16(a.native(), b.native());
#elif SIMDPP_USE_NEON
    uint32x4 lo = vmull_u16(vget_low_u16(a.native()), vget_low_u16(b.native()));
    uint32x4 hi = vmull_u16(vget_high_u16(a.native()), vget_high_u16(b.native()));
    return unzip8_hi(uint16x8(lo), uint16x8(hi));
#elif SIMDPP_USE_ALTIVEC && SIMDPP_BIG_ENDIAN
    uint16<16> ab;
    ab = mull(a, b);
    return unzip8_lo(ab.vec(0), ab.vec(1));
#elif (SIMDPP_USE_ALTIVEC && SIMDPP_LITTLE_ENDIAN) || SIMDPP_USE_MSA
    uint16<16> ab;
    ab = mull(a, b);
    return unzip8_hi(ab.vec(0), ab.vec(1));
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint16<16> i_mul_hi(const uint16<16>& a, const uint16<16>& b)
{
    return _mm256_mulhi_epu16(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
uint16<32> i_mul_hi(const uint16<32>& a, const uint16<32>& b)
{
    return _mm512_mulhi_epu16(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
V i_mul_hi(const V& a, const V& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(V, i_mul_hi, a, b)
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

