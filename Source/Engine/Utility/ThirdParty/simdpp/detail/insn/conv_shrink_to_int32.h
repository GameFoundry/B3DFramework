/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_SHRINK_TO_INT32_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_SHRINK_TO_INT32_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/permute4.h>
#include <simdpp/core/unzip_lo.h>
#include <simdpp/core/zip_lo.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {


SIMDPP_INL uint32<4> i_to_uint32(const uint64<4>& a)
{
#if SIMDPP_USE_NULL
    uint32<4> r;
    for (unsigned i = 0; i < 4; i++) {
        r.el(i) = uint32_t(a.vec(i/2).el(i%2));
    }
    return r;
#elif SIMDPP_USE_AVX512VL
    return _mm256_cvtepi64_epi32(a.native());
#elif SIMDPP_USE_AVX2
    uint64<4> a64;
    a64 = permute4<0,2,0,2>((uint32<8>) a);
    a64 = permute4<0,2,0,2>(a64);
    return _mm256_castsi256_si128(a64.native());
#elif SIMDPP_USE_NEON64
    uint32x2_t low = vmovn_u64(a.vec(0).native());
    return vmovn_high_u64(low, a.vec(1).native());
#elif SIMDPP_USE_NEON
    uint32x2_t low = vmovn_u64(a.vec(0).native());
    uint32x2_t high = vmovn_u64(a.vec(1).native());
    return vcombine_u32(low, high);
#elif SIMDPP_USE_VSX_207
    return vec_pack(a.vec(0).native(), a.vec(1).native());
#elif SIMDPP_USE_ALTIVEC && SIMDPP_BIG_ENDIAN
    uint32<4> r1, r2;
    r1 = a.vec(0);
    r2 = a.vec(1);
    return unzip4_hi(r1, r2);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_MSA || (SIMDPP_USE_ALTIVEC && SIMDPP_LITTLE_ENDIAN)
    uint32<4> r1, r2;
    r1 = a.vec(0);
    r2 = a.vec(1);
    return unzip4_lo(r1, r2);
#endif
}

#if SIMDPP_USE_AVX2
SIMDPP_INL uint32<8> i_to_uint32(const uint64<8>& a)
{
#if SIMDPP_USE_AVX512F
    return _mm512_cvtepi64_epi32(a.native());
#else
    uint64<4> b0, b1;
    b0 = permute4<0,2,0,2>((uint32<8>) a.vec(0));
    b1 = permute4<0,2,0,2>((uint32<8>) a.vec(1));
    b0 = zip2_lo(b0, b1);
    b0 = permute4<0,2,1,3>(b0);
    return (uint32<8>) b0;
#endif
}
#endif

#if SIMDPP_USE_AVX512F
SIMDPP_INL uint32<16> i_to_uint32(const uint64<16>& a)
{
    uint32<8> r1 = _mm512_cvtepi64_epi32(a.vec(0).native());
    uint32<8> r2 = _mm512_cvtepi64_epi32(a.vec(1).native());
    return combine(r1, r2);
}
#endif

template<unsigned N> SIMDPP_INL
uint32<N> i_to_uint32(const uint64<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_EXTRACT(uint32<N>, i_to_uint32, a)
}


} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif


