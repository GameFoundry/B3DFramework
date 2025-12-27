/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_I_MUL_LO_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_I_MUL_LO_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/i_mull.h>
#include <simdpp/core/move_l.h>
#include <simdpp/core/permute4.h>
#include <simdpp/core/shuffle2.h>
#include <simdpp/detail/null/math.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

static SIMDPP_INL
uint16<8> i_mul_lo(const uint16<8>& a, const uint16<8>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::mul(a, b);
#elif SIMDPP_USE_SSE2
    return _mm_mullo_epi16(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vmulq_u16(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_mladd(a.native(), b.native(),
                     ((uint16x8) make_zero()).native());
#elif SIMDPP_USE_MSA
    return (v8u16) __msa_mulv_h((v8i16) a.native(), (v8i16) b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint16<16> i_mul_lo(const uint16<16>& a, const uint16<16>& b)
{
    return _mm256_mullo_epi16(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
uint16<32> i_mul_lo(const uint16<32>& a, const uint16<32>& b)
{
    return _mm512_mullo_epi16(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint32<4> i_mul_lo(const uint32<4>& a, const uint32<4>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::mul(a, b);
#elif SIMDPP_USE_SSE4_1
    return _mm_mullo_epi32(a.native(), b.native());
#elif SIMDPP_USE_SSE2
    uint32x4 a1, b1, r;
    a1 = move4_l<1>(a);
    b1 = move4_l<1>(b);
    r = _mm_mul_epu32(a.native(), b.native());
    a1 = _mm_mul_epu32(a1.native(), b1.native());
    r = shuffle2<0,2,0,2>(r, a1); // moves to FP domain, additional latency unavoidable
    r = permute4<0,2,1,3>(r);
    return r;
#elif SIMDPP_USE_NEON
    return vmulq_u32(a.native(), b.native());
#elif SIMDPP_USE_VSX_207
#if __GNUC__
    // BUG: GCC does not have support for vmuluwm yet
    __vector uint32_t va = a.native(), vb = b.native();
    __vector uint32_t vr;
    asm("vmuluwm	%0, %1, %2" : "=v"(vr) : "v"(va), "v"(vb));
    return vr;
#else
    return vec_vmuluwm(a.native(), b.native());
#endif
#elif SIMDPP_USE_ALTIVEC
    // implement in terms of 16-bit multiplies
    //   *  ah  al
    //      bh  bl
    //      ======
    //   + (al*bl) <-  l_ab
    //+ (ah*bl)    <-  h_ab
    //+ (al*bh)    <-  h_ba

    uint16<8> ra, rb; ra = a, rb = b;
#if SIMDPP_BIG_ENDIAN
    uint16<8> sa = move8_r<1>(ra);
    uint16<8> sb = move8_r<1>(rb);

    uint32<4> l_ab = vec_mulo(ra.native(), rb.native());
    uint32<4> h_ab = vec_mulo(ra.native(), sb.native());
    uint32<4> h_ba = vec_mulo(sa.native(), rb.native());
#else
    uint16<8> sa = move8_l<1>(ra);
    uint16<8> sb = move8_l<1>(rb);

    uint32<4> l_ab = vec_mule(ra.native(), rb.native());
    uint32<4> h_ab = vec_mule(ra.native(), sb.native());
    uint32<4> h_ba = vec_mule(sa.native(), rb.native());
#endif

    h_ab = shift_l<16>(add(h_ab, h_ba));
    h_ab = add(h_ab, l_ab);
    return h_ab;
#elif SIMDPP_USE_MSA
    return (v4u32) __msa_mulv_w((v4i32) a.native(), (v4i32) b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint32<8> i_mul_lo(const uint32<8>& a, const uint32<8>& b)
{
    return _mm256_mullo_epi32(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint32<16> i_mul_lo(const uint32<16>& a, const uint32<16>& b)
{
    return _mm512_mullo_epi32(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
V i_mul_lo(const V& a, const V& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(V, i_mul_lo, a, b)
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

