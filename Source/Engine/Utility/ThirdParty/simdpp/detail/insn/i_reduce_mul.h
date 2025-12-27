/*  Copyright (C) 2016  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_I_REDUCE_MUL_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_I_REDUCE_MUL_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/extract.h>
#include <simdpp/core/f_mul.h>
#include <simdpp/core/i_mul.h>
#include <simdpp/core/move_l.h>
#include <simdpp/core/make_uint.h>
#include <simdpp/core/to_int32.h>
#include <simdpp/detail/mem_block.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

// forward declarations
template<unsigned N, class E> SIMDPP_INL
int32_t reduce_mul(const int16<N,E>& a);
template<unsigned N, class E> SIMDPP_INL
uint32_t reduce_mul(const uint16<N,E>& a);
template<unsigned N, class E> SIMDPP_INL
int32_t reduce_mul(const int32<N,E>& a);
template<unsigned N, class E> SIMDPP_INL
uint32_t reduce_mul(const uint32<N,E>& a);

namespace detail {
namespace insn {

static SIMDPP_INL
uint32_t i_reduce_mul(const uint16x8& a)
{
#if SIMDPP_USE_NULL
    uint32_t r = a.el(0);
    for (unsigned i = 1; i < a.length; i++) {
        r *= a.el(i);
    }
    return r;
#elif SIMDPP_USE_SSE2
    uint32x4 ca = (uint32x4)a;
    // shift data zeroing out bits
    uint32x4 al = shift_r<16>(ca);
    uint32x4 ah = shift_l<16>(ca);
    // due to zeroing bits in previous steps, the result can be OR'ed
    uint32x4 lo = _mm_mullo_epi16(ca.native(), al.native());
    uint32x4 hi = _mm_mulhi_epu16(ca.native(), ah.native());
    uint32x4 r = bit_or(lo, hi);
    r = _mm_mul_epu32(r.native(), move4_l<1>(r).eval().native());
    r = _mm_mul_epu32(r.native(), move4_l<2>(r).eval().native());
    return extract<0>(r);
#elif SIMDPP_USE_NEON
    uint32x4 r = vmull_u16(vget_low_u16(a.native()),
                           vget_high_u16(a.native()));
    r = mul_lo(r, move4_l<2>(r));
    r = mul_lo(r, move4_l<1>(r));
    return extract<0>(r);
#elif SIMDPP_USE_ALTIVEC
    uint16x8 a2 = move8_l<1>(a);
    uint32x4 r = vec_mule(a2.native(), a.native());
    mem_block<uint32x4> b = r;
    return b[0] * b[1] * b[2] * b[3];
#elif SIMDPP_USE_MSA
    uint32<8> a32 = to_uint32(a);
    uint32<4> r = mul_lo(a32.vec(0), a32.vec(1));
    r = mul_lo(r, move4_l<2>(r));
    r = mul_lo(r, move4_l<1>(r));
    return extract<0>(r);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint32_t i_reduce_mul(const uint16x16& a)
{
    uint32x8 ca = (uint32x8) a;
    // shift data zeroing out bits
    uint32x8 al = shift_r<16>(ca);
    uint32x8 ah = shift_l<16>(ca);
    // due to zoreing bits in previous steps, the result can be OR'ed
    uint32x8 lo = _mm256_mullo_epi16(ca.native(), al.native());
    uint32x8 hi = _mm256_mulhi_epu16(ca.native(), ah.native());
    uint32x8 r = bit_or(lo, hi);
    r = _mm256_mul_epu32(r.native(), move4_l<1>(r).eval().native());
    uint32x4 r2 = _mm_mul_epu32(detail::extract128<0>(r).native(),
                                detail::extract128<1>(r).native());
    r2 = _mm_mul_epu32(r2.native(), move4_l<2>(r2).eval().native());
    return extract<0>(r2);
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint32_t i_reduce_mul(const uint16<32>& a)
{
    uint32<16> ca = (uint32<16>) a;
    // shift data zeroing out bits
    uint32<16> al = shift_r<16>(ca);
    uint32<16> ah = shift_l<16>(ca);
    // due to zeroing bits in previous steps, the result can be OR'ed
    uint32<16> lo = _mm512_mullo_epi16(ca.native(), al.native());
    uint32<16> hi = _mm512_mulhi_epu16(ca.native(), ah.native());
    uint32<16> r = bit_or(lo, hi);
    return reduce_mul(r);
}
#endif

template<unsigned N>
SIMDPP_INL uint32_t i_reduce_mul(const uint16<N>& a)
{
#if SIMDPP_USE_NULL
    uint32_t r = 1;
    for (unsigned j = 0; j < a.vec_length; ++j) {
        for (unsigned i = 0; i < a.base_length; i++) {
            r *= a.vec(j).el(i);
        }
    }
    return r;
#elif SIMDPP_USE_AVX512BW
    uint32<16> prod = make_uint(1);
    for (unsigned j = 0; j < a.vec_length; ++j) {
        uint32<16> ca = (uint32<16>) a.vec(j);
        // shift data zeroing out bits
        uint32<16> al = shift_r<16>(ca);
        uint32<16> ah = shift_l<16>(ca);
        // due to zoreing bits in previous steps, the result can be OR'ed
        uint32<16> lo = _mm512_mullo_epi16(ca.native(), al.native());
        uint32<16> hi = _mm512_mulhi_epu16(ca.native(), ah.native());
        uint32<16> iprod = bit_or(lo, hi);
        iprod = _mm512_mul_epu32(iprod.native(), move4_l<1>(iprod).eval().native());
        prod = _mm512_mul_epu32(prod.native(), iprod.native());
    }
    return reduce_mul(prod);
#elif SIMDPP_USE_AVX2
    uint32x8 prod = make_uint(1);
    for (unsigned j = 0; j < a.vec_length; ++j) {
        uint32x8 ca = (uint32x8) a.vec(j);
        // shift data zeroing out bits
        uint32x8 al = shift_r<16>(ca);
        uint32x8 ah = shift_l<16>(ca);
        // due to zoreing bits in previous steps, the result can be OR'ed
        uint32x8 lo = _mm256_mullo_epi16(ca.native(), al.native());
        uint32x8 hi = _mm256_mulhi_epu16(ca.native(), ah.native());
        uint32x8 iprod = bit_or(lo, hi);
        iprod = _mm256_mul_epu32(iprod.native(), move4_l<1>(iprod).eval().native());
        prod = _mm256_mul_epu32(prod.native(), iprod.native());
    }
    uint32x4 r2 = _mm_mul_epu32(detail::extract128<0>(prod).native(),
                                detail::extract128<1>(prod).native());
    r2 = _mm_mul_epu32(r2.native(), move4_l<2>(r2).eval().native());
    return extract<0>(r2);
#elif SIMDPP_USE_SSE2
    uint32x4 prod = make_uint(1);
    for (unsigned j = 0; j < a.vec_length; ++j) {
        uint32x4 ca = (uint32x4) a.vec(j);
        // shift data zeroing out bits
        uint32x4 al = shift_r<16>(ca);
        uint32x4 ah = shift_l<16>(ca);
        // due to zoreing bits in previous steps, the result can be OR'ed
        uint32x4 lo = _mm_mullo_epi16(ca.native(), al.native());
        uint32x4 hi = _mm_mulhi_epu16(ca.native(), ah.native());
        uint32x4 r = bit_or(lo, hi);
        r = _mm_mul_epu32(r.native(), move4_l<1>(r).eval().native());
        prod = _mm_mul_epu32(prod.native(), r.native());
    }
    prod = _mm_mul_epu32(prod.native(), move4_l<2>(prod).eval().native());
    return extract<0>(prod);
#elif SIMDPP_USE_NEON
    uint32x4 prod = make_uint(1);
    for (unsigned j = 0; j < a.vec_length; ++j) {
        uint32x4 r = vmull_u16(vget_low_u16(a.vec(j).native()),
                               vget_high_u16(a.vec(j).native()));
        prod = mul_lo(prod, r);
    }
    prod = mul_lo(prod, move4_l<2>(prod));
    prod = mul_lo(prod, move4_l<1>(prod));
    return extract<0>(prod);
#elif SIMDPP_USE_ALTIVEC
    uint32_t r = i_reduce_mul(a.vec(0));
    for (unsigned j = 1; j < a.vec_length; ++j) {
        r *= i_reduce_mul(a.vec(j));
    }
    return r;
#elif SIMDPP_USE_MSA
    uint32x4 prod = make_uint(1);
    for (unsigned j = 0; j < a.vec_length; ++j) {
        uint32<8> a32 = to_uint32(a.vec(j));
        prod = mul_lo(prod, mul_lo(a32.vec(0), a32.vec(1)));
    }
    prod = mul_lo(prod, move4_l<2>(prod));
    prod = mul_lo(prod, move4_l<1>(prod));
    return extract<0>(prod);
#endif
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
int32_t i_reduce_mul(const int16x8& a)
{
#if SIMDPP_USE_NULL
    int32_t r = a.el(0);
    for (unsigned i = 1; i < a.length; i++) {
        r *= a.el(i);
    }
    return r;
#elif SIMDPP_USE_SSE2
    uint32x4 ca = (uint32x4) a;
    // shift data zeroing out bits
    uint32x4 al = shift_r<16>(ca);
    uint32x4 ah = shift_l<16>(ca);
    // due to zoreing bits in previous steps, the result can be OR'ed
    uint32x4 lo = _mm_mullo_epi16(ca.native(), al.native());
    uint32x4 hi = _mm_mulhi_epi16(ca.native(), ah.native());
    uint32x4 r = bit_or(lo, hi);
    r = _mm_mul_epu32(r.native(), move4_l<1>(r).eval().native());
    r = _mm_mul_epu32(r.native(), move4_l<2>(r).eval().native());
    return extract<0>(r);
#elif SIMDPP_USE_NEON
    int32x4 r = vmull_s16(vget_low_s16(a.native()), vget_high_s16(a.native()));
    r = mul_lo(r, move4_l<2>(r));
    r = mul_lo(r, move4_l<1>(r));
    return extract<0>(r);
#elif SIMDPP_USE_ALTIVEC
    int16x8 a2 = move8_l<1>(a);
    int32x4 r = vec_mule(a2.native(), a.native());
    mem_block<int32x4> b = r;
    return b[0] * b[1] * b[2] * b[3];
#elif SIMDPP_USE_MSA
    int32<8> a32 = to_int32(a);
    int32<4> r = mul_lo(a32.vec(0), a32.vec(1));
    r = mul_lo(r, move4_l<2>(r));
    r = mul_lo(r, move4_l<1>(r));
    return extract<0>(r);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int32_t i_reduce_mul(const int16x16& a)
{
    uint32x8 ca = (uint32x8) a;
    // shift data zeroing out bits
    uint32x8 al = shift_r<16>(ca);
    uint32x8 ah = shift_l<16>(ca);
    // due to zoreing bits in previous steps, the result can be OR'ed
    uint32x8 lo = _mm256_mullo_epi16(ca.native(), al.native());
    uint32x8 hi = _mm256_mulhi_epi16(ca.native(), ah.native());
    uint32x8 r = bit_or(lo, hi);
    r = _mm256_mul_epu32(r.native(), move4_l<1>(r).eval().native());
    uint32x4 r2 = _mm_mul_epu32(detail::extract128<0>(r).native(),
                                detail::extract128<1>(r).native());
    r2 = _mm_mul_epu32(r2.native(), move4_l<2>(r2).eval().native());
    return extract<0>(r2);
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL int32_t i_reduce_mul(const int16<32>& a)
{
    uint32<16> ca = (uint32<16>) a;
    // shift data zeroing out bits
    uint32<16> al = shift_r<16>(ca);
    uint32<16> ah = shift_l<16>(ca);
    // due to zoreing bits in previous steps, the result can be OR'ed
    uint32<16> lo = _mm512_mullo_epi16(ca.native(), al.native());
    uint32<16> hi = _mm512_mulhi_epi16(ca.native(), ah.native());
    uint32<16> r = bit_or(lo, hi);
    return reduce_mul(r);
}
#endif

template<unsigned N>
SIMDPP_INL int32_t i_reduce_mul(const int16<N>& a)
{
#if SIMDPP_USE_NULL
    uint32_t r = 1;
    for (unsigned j = 0; j < a.vec_length; ++j) {
        for (unsigned i = 0; i < a.base_length; i++) {
            r *= a.vec(j).el(i);
        }
    }
    return r;
#elif SIMDPP_USE_AVX512BW
    uint32<16> prod = make_uint(1);
    for (unsigned j = 0; j < a.vec_length; ++j) {
        uint32<16> ca = (uint32<16>) a.vec(j);
        // shift data zeroing out bits
        uint32<16> al = shift_r<16>(ca);
        uint32<16> ah = shift_l<16>(ca);
        // due to zoreing bits in previous steps, the result can be OR'ed
        uint32<16> lo = _mm512_mullo_epi16(ca.native(), al.native());
        uint32<16> hi = _mm512_mulhi_epi16(ca.native(), ah.native());
        uint32<16> iprod = bit_or(lo, hi);
        iprod = _mm512_mul_epu32(iprod.native(), move4_l<1>(iprod).eval().native());
        prod = _mm512_mul_epu32(prod.native(), iprod.native());
    }
    return reduce_mul(prod);
#elif SIMDPP_USE_AVX2
    uint32x8 prod = make_uint(1);
    for (unsigned j = 0; j < a.vec_length; ++j) {
        uint32x8 ca = (uint32x8) a.vec(j);
        // shift data zeroing out bits
        uint32x8 al = shift_r<16>(ca);
        uint32x8 ah = shift_l<16>(ca);
        // due to zoreing bits in previous steps, the result can be OR'ed
        uint32x8 lo = _mm256_mullo_epi16(ca.native(), al.native());
        uint32x8 hi = _mm256_mulhi_epi16(ca.native(), ah.native());
        uint32x8 iprod = bit_or(lo, hi);
        iprod = _mm256_mul_epu32(iprod.native(), move4_l<1>(iprod).eval().native());
        prod = _mm256_mul_epu32(prod.native(), iprod.native());
    }
    uint32x4 r2 = _mm_mul_epu32(detail::extract128<0>(prod).native(),
                                detail::extract128<1>(prod).native());
    r2 = _mm_mul_epu32(r2.native(), move4_l<2>(r2).eval().native());
    return extract<0>(r2);
#elif SIMDPP_USE_SSE2
    uint32x4 prod = make_uint(1);
    for (unsigned j = 0; j < a.vec_length; ++j) {
        uint32x4 ca = (uint32x4) a.vec(j);
        // shift data zeroing out bits
        uint32x4 al = shift_r<16>(ca);
        uint32x4 ah = shift_l<16>(ca);
        // due to zoreing bits in previous steps, the result can be OR'ed
        uint32x4 lo = _mm_mullo_epi16(ca.native(), al.native());
        uint32x4 hi = _mm_mulhi_epi16(ca.native(), ah.native());
        uint32x4 r = bit_or(lo, hi);
        r = _mm_mul_epu32(r.native(), move4_l<1>(r).eval().native());
        prod = _mm_mul_epu32(prod.native(), r.native());
    }
    prod = _mm_mul_epu32(prod.native(), move4_l<2>(prod).eval().native());
    return extract<0>(prod);
#elif SIMDPP_USE_NEON
    int32x4 prod = make_uint(1);
    for (unsigned j = 0; j < a.vec_length; ++j) {
        int32x4 r = vmull_s16(vget_low_s16(a.vec(j).native()),
                              vget_high_s16(a.vec(j).native()));
        prod = mul_lo(prod, r);
    }
    prod = mul_lo(prod, move4_l<2>(prod));
    prod = mul_lo(prod, move4_l<1>(prod));
    return extract<0>(prod);
#elif SIMDPP_USE_ALTIVEC
    int32_t r = i_reduce_mul(a.vec(0));
    for (unsigned j = 1; j < a.vec_length; ++j) {
        r *= i_reduce_mul(a.vec(j));
    }
    return r;
#elif SIMDPP_USE_MSA
    int32x4 prod = make_uint(1);
    for (unsigned j = 0; j < a.vec_length; ++j) {
        int32<8> a32 = to_int32(a.vec(j));
        prod = mul_lo(prod, mul_lo(a32.vec(0), a32.vec(1)));
    }
    prod = mul_lo(prod, move4_l<2>(prod));
    prod = mul_lo(prod, move4_l<1>(prod));
    return extract<0>(prod);
#endif
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint32_t i_reduce_mul(const uint32x4& a)
{
#if SIMDPP_USE_NULL
    uint32_t r = a.el(0);
    for (unsigned i = 1; i < a.length; i++) {
        r *= a.el(i);
    }
    return r;
#elif SIMDPP_USE_SSE2
    uint32x4 r = _mm_mul_epu32(a.native(), move4_l<1>(a).eval().native());
    r = _mm_mul_epu32(r.native(), move4_l<2>(r).eval().native());
    return extract<0>(r);
#elif SIMDPP_USE_NEON || SIMDPP_USE_MSA
    uint32x4 r = a;
    r = mul_lo(r, move4_l<2>(r));
    r = mul_lo(r, move4_l<1>(r));
    return extract<0>(r);
#elif SIMDPP_USE_ALTIVEC
    mem_block<uint32x4> b = a;
    return b[0] * b[1] * b[2] * b[3];
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint32_t i_reduce_mul(const uint32x8& a)
{
    uint32x8 ra = _mm256_mul_epu32(a.native(), move4_l<1>(a).eval().native());
    uint32x4 r = _mm_mul_epu32(detail::extract128<0>(ra).native(),
                               detail::extract128<1>(ra).native());
    r = _mm_mul_epu32(r.native(), move4_l<2>(r).eval().native());
    return extract<0>(r);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint32_t i_reduce_mul(const uint32<16>& a)
{
    return i_reduce_mul(mul_lo(extract256<0>(a), extract256<1>(a)));
}
#endif

template<unsigned N>
SIMDPP_INL uint32_t i_reduce_mul(const uint32<N>& a)
{
#if SIMDPP_USE_NULL
    uint32_t r = 1;
    for (unsigned j = 0; j < a.vec_length; ++j) {
        for (unsigned i = 0; i < a.base_length; i++) {
            r *= a.vec(j).el(i);
        }
    }
    return r;
#elif SIMDPP_USE_AVX2
    uint32x8 prod = make_uint(1);
    for (unsigned j = 0; j < a.vec_length; ++j) {
        uint32x8 ai = a.vec(j);
        uint32x8 ra = _mm256_mul_epu32(ai.native(), move4_l<1>(ai).eval().native());
        prod = _mm256_mul_epu32(prod.native(), ra.native());
    }
    uint32x4 r = _mm_mul_epu32(detail::extract128<0>(prod).native(),
                               detail::extract128<1>(prod).native());
    r = _mm_mul_epu32(r.native(), move4_l<2>(r).eval().native());
    return extract<0>(r);
#elif SIMDPP_USE_SSE2
    uint32x4 r = make_uint(1);
    for (unsigned j = 0; j < a.vec_length; ++j) {
        uint32x4 ai = a.vec(j);
        uint32x4 ra = _mm_mul_epu32(ai.native(), move4_l<1>(ai).eval().native());
        r = _mm_mul_epu32(r.native(), ra.native());
    }
    r = _mm_mul_epu32(r.native(), move4_l<2>(r).eval().native());
    return extract<0>(r);
#elif SIMDPP_USE_NEON || SIMDPP_USE_MSA
    uint32x4 prod = make_uint(1);
    for (unsigned j = 0; j < a.vec_length; ++j) {
        prod = mul_lo(prod, a.vec(j));
    }
    prod = mul_lo(prod, move4_l<2>(prod));
    prod = mul_lo(prod, move4_l<1>(prod));
    return extract<0>(prod);
#elif SIMDPP_USE_ALTIVEC
    uint32_t r = i_reduce_mul(a.vec(0));
    for (unsigned j = 1; j < a.vec_length; ++j) {
        r *= i_reduce_mul(a.vec(j));
    }
    return r;
#endif
}

// -----------------------------------------------------------------------------


} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

