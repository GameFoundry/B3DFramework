/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_SHRINK_TO_INT16_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_SHRINK_TO_INT16_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/permute4.h>
#include <simdpp/core/unzip_lo.h>
#include <simdpp/detail/insn/conv_shrink_to_int32.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {


// -----------------------------------------------------------------------------

SIMDPP_INL uint16<8> i_to_uint16(const uint32<8>& a)
{
#if SIMDPP_USE_NULL
    uint16<8> r;
    for (unsigned i = 0; i < 8; i++) {
        r.el(i) = uint16_t(a.vec(i/4).el(i%4));
    }
    return r;
#elif SIMDPP_USE_AVX512VL
    return _mm256_cvtepi32_epi16(a.native());
#elif SIMDPP_USE_SSSE3
    uint16<16> perm_mask = make_shuffle_bytes16_mask<0,2,4,6,0,0,0,0>(perm_mask);
    uint16<16> a16;
    uint64<4> a64;
    a16 = a;
    a64 = permute_bytes16(a16, perm_mask);
#if SIMDPP_USE_AVX2
    a64 = permute4<0,2,0,2>(a64);
    return _mm256_castsi256_si128(a64.native());
#else
    return (uint16<8>) zip2_lo(a64.vec(0), a64.vec(1));
#endif
#elif SIMDPP_USE_NEON64
    uint16x4_t low = vmovn_u32(a.vec(0).native());
    return vmovn_high_u32(low, a.vec(1).native());
#elif SIMDPP_USE_NEON
    uint16x4_t low = vmovn_u32(a.vec(0).native());
    uint16x4_t high = vmovn_u32(a.vec(1).native());
    return vcombine_u16(low, high);
#elif SIMDPP_USE_ALTIVEC
    return vec_pack(a.vec(0).native(), a.vec(1).native());
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_MSA
    uint16<8> r1, r2;
    r1 = a.vec(0);
    r2 = a.vec(1);
    return unzip8_lo(r1, r2);
#endif
}

#if SIMDPP_USE_AVX2
SIMDPP_INL uint16<16> i_to_uint16(const uint32<16>& a)
{
#if SIMDPP_USE_AVX512BW
    return _mm512_cvtepi32_epi16(a.native());
#elif SIMDPP_USE_AVX512F
    uint16<32> perm_mask = make_shuffle_bytes16_mask<0,2,4,6,0,0,0,0>(perm_mask);
    uint64<8> a64;
    uint64<4> a64_0, a64_1;
    a64 = permute_bytes16((uint16<32>) a, perm_mask);
    split(a64, a64_0, a64_1);
    a64_0 = zip2_lo(a64_0, a64_1);
    a64_0 = permute4<0,2,1,3>(a64_0);
    return (uint16<16>) a64_0;
#else
    uint16<16> perm_mask = make_shuffle_bytes16_mask<0,2,4,6,0,0,0,0>(perm_mask);
    uint64<4> a64_0, a64_1;
    a64_0 = permute_bytes16((uint16<16>) a.vec(0), perm_mask);
    a64_1 = permute_bytes16((uint16<16>) a.vec(1), perm_mask);
    a64_0 = zip2_lo(a64_0, a64_1);
    a64_0 = permute4<0,2,1,3>(a64_0);
    return (uint16<16>) a64_0;
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint16<32> i_to_uint16(const uint32<32>& a)
{
    uint16<16> r1 = _mm512_cvtepi32_epi16(a.vec(0).native());
    uint16<16> r2 = _mm512_cvtepi32_epi16(a.vec(1).native());
    return combine(r1, r2);
}
#endif

template<unsigned N> SIMDPP_INL
uint16<N> i_to_uint16(const uint32<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_EXTRACT(uint16<N>, i_to_uint16, a)
}

// -----------------------------------------------------------------------------

SIMDPP_INL uint16<8> i_to_uint16(const uint64<8>& a)
{
#if SIMDPP_USE_NULL
    uint16<8> r;
    for (unsigned i = 0; i < 8; i++) {
        r.el(i) = uint16_t(a.vec(i/2).el(i%2));
    }
    return r;
#elif SIMDPP_USE_AVX512F
    return _mm512_cvtepi64_epi16(a.native());
#elif SIMDPP_USE_AVX2
    uint16<16> perm_mask = make_shuffle_bytes16_mask<0,4,0,0,0,0,0,0>(perm_mask);
    uint32<8> a32_0, a32_1;
    uint64<4> a64_0;
    uint32<4> b32;
    a32_0 = permute_bytes16((uint16<16>) a.vec(0), perm_mask);
    a32_1 = permute_bytes16((uint16<16>) a.vec(1), perm_mask);
    a64_0 = zip4_lo(a32_0, a32_1);
    a32_0 = permute4<0,2,1,3>(a64_0);
    b32 = _mm256_castsi256_si128(a32_0.native());
    b32 = permute4<0,2,1,3>(b32);
    return (uint16<8>) b32;
#elif SIMDPP_USE_SSSE3
    uint16<8> perm_mask = make_shuffle_bytes16_mask<0,4,0,0,0,0,0,0>(perm_mask);
    uint32<4> a32_0, a32_1, a32_2, a32_3;
    uint64<2> a64_0, a64_1;
    a32_0 = permute_bytes16((uint16<8>) a.vec(0), perm_mask);
    a32_1 = permute_bytes16((uint16<8>) a.vec(1), perm_mask);
    a32_2 = permute_bytes16((uint16<8>) a.vec(2), perm_mask);
    a32_3 = permute_bytes16((uint16<8>) a.vec(3), perm_mask);
    a64_0 = zip4_lo(a32_0, a32_1);
    a64_1 = zip4_lo(a32_2, a32_3);
    a64_0 = zip2_lo(a64_0, a64_1);
    return (uint16<8>) a64_0;
#else
    uint32<8> a32 = i_to_uint32(a);
    return i_to_uint16(a32);
#endif
}

#if SIMDPP_USE_AVX2
SIMDPP_INL uint16<16> i_to_uint16(const uint64<16>& a)
{
#if SIMDPP_USE_AVX512F
    uint16<8> r0 = _mm512_cvtepi64_epi16(a.vec(0).native());
    uint16<8> r1 = _mm512_cvtepi64_epi16(a.vec(1).native());
    return combine(r0, r1);
#else
    uint16<16> perm_mask = make_shuffle_bytes16_mask<0,4,0,0,0,0,0,0>(perm_mask);
    uint32<8> a32_0, a32_1, a32_2, a32_3;
    uint64<4> a64_0, a64_1;
    a32_0 = permute_bytes16((uint16<16>) a.vec(0), perm_mask);
    a32_1 = permute_bytes16((uint16<16>) a.vec(1), perm_mask);
    a32_2 = permute_bytes16((uint16<16>) a.vec(2), perm_mask);
    a32_3 = permute_bytes16((uint16<16>) a.vec(3), perm_mask);
    a64_0 = zip4_lo(a32_0, a32_1);
    a64_1 = zip4_lo(a32_2, a32_3);
    a64_0 = zip2_lo(a64_0, a64_1);
    a32_0 = permute4<0,2,1,3>(a64_0);
    a32_0 = permute4<0,2,1,3>(a32_0);
    return (uint16<16>) a32_0;
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint16<32> i_to_uint16(const uint64<32>& a)
{
    uint16<8> r0 = _mm512_cvtepi64_epi16(a.vec(0).native());
    uint16<8> r1 = _mm512_cvtepi64_epi16(a.vec(1).native());
    uint16<8> r2 = _mm512_cvtepi64_epi16(a.vec(2).native());
    uint16<8> r3 = _mm512_cvtepi64_epi16(a.vec(3).native());
    return combine(combine(r0, r1), combine(r2, r3));
}
#endif

template<unsigned N> SIMDPP_INL
uint16<N> i_to_uint16(const uint64<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_EXTRACT(uint16<N>, i_to_uint16, a)
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif


