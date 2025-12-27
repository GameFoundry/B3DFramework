/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_PERMUTE2_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_PERMUTE2_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/make_shuffle_bytes_mask.h>
#include <simdpp/core/permute4.h>
#include <simdpp/detail/null/shuffle.h>
#include <simdpp/detail/shuffle/neon_int16x8.h>
#include <simdpp/detail/shuffle/neon_int32x4.h>
#include <simdpp/detail/shuffle/neon_int64x2.h>
#include <simdpp/detail/shuffle/shuffle_mask.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {


template<unsigned s0, unsigned s1, unsigned N> SIMDPP_INL
uint16<N> i_permute2(const uint16<N>& a)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
    return i_permute4<s0,s1,s0+2,s1+2>(a);
}


template<unsigned s0, unsigned s1, unsigned N> SIMDPP_INL
uint32<N> i_permute2(const uint32<N>& a)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
    return i_permute4<s0,s1,s0+2,s1+2>(a);
}


template<unsigned s0, unsigned s1, unsigned N> SIMDPP_INL
float32<N> i_permute2(const float32<N>& a)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
    return i_permute4<s0,s1,s0+2,s1+2>(a);
}


template<unsigned s0, unsigned s1> SIMDPP_INL
uint64x2 i_permute2(const uint64x2& a)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
#if SIMDPP_USE_SSE2 || SIMDPP_USE_MSA
    return (uint64x2) i_permute4<s0*2, s0*2+1, s1*2, s1*2+1>(int32x4(a));
#elif SIMDPP_USE_NEON
    return detail::neon_shuffle_int64x2::permute2<s0,s1>(a);
#elif SIMDPP_USE_VSX_207
    return vec_xxpermdi(a.native(), a.native(),
                        SIMDPP_VSX_SHUFFLE_MASK_2x2(s0, s1));
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC
    return detail::null::permute<s0,s1>(a);
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned s0, unsigned s1> SIMDPP_INL
uint64x4 i_permute2(const uint64x4& a)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
    return i_permute4<s0,s1,s0+2,s1+2>(a);
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned s0, unsigned s1> SIMDPP_INL
uint64<8> i_permute2(const uint64<8>& a)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
    return i_permute4<s0,s1,s0+2,s1+2>(a);
}
#endif

template<unsigned s0, unsigned s1, unsigned N> SIMDPP_INL
uint64<N> i_permute2(const uint64<N>& a)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
    SIMDPP_VEC_ARRAY_IMPL1(uint64<N>, (i_permute2<s0,s1>), a);
}

// -----------------------------------------------------------------------------

template<unsigned s0, unsigned s1> SIMDPP_INL
float64x2 i_permute2(const float64x2& a)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
#if SIMDPP_USE_SSE2
    return _mm_shuffle_pd(a.native(), a.native(), SIMDPP_SHUFFLE_MASK_2x2(s0, s1));
#elif SIMDPP_USE_VSX_206
    return vec_xxpermdi(a.native(), a.native(),
                        SIMDPP_VSX_SHUFFLE_MASK_2x2(s0, s1));
#elif SIMDPP_USE_NEON64 || SIMDPP_USE_MSA
    return float64x2(i_permute2<s0,s1>(int64x2(a)));
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC
    return detail::null::permute<s0,s1>(a);
#endif
}

#if SIMDPP_USE_AVX
template<unsigned s0, unsigned s1> SIMDPP_INL
float64x4 i_permute2(const float64x4& a)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
#if SIMDPP_USE_AVX2
    return _mm256_permute4x64_pd(a.native(), s0 | s1<<2 | (s0+2)<<4 | (s1+2)<<6);
#else // SIMDPP_USE_AVX
    return _mm256_permute_pd(a.native(), s0 | s1<<1 | s0<<2 | s1<<3);
#endif
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned s0, unsigned s1> SIMDPP_INL
float64<8> i_permute2(const float64<8>& a)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
    return _mm512_permute_pd(a.native(), s0 | s1<<1 | s0<<2 | s1<<3 | s0<<4 | s1<<5 | s0<<6 | s1<<7);
}
#endif

template<unsigned s0, unsigned s1, unsigned N> SIMDPP_INL
float64<N> i_permute2(const float64<N>& a)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
    SIMDPP_VEC_ARRAY_IMPL1(float64<N>, (i_permute2<s0,s1>), a);
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

