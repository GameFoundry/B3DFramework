/*  Copyright (C) 2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_PERMUTE_SSE_INT32_4x2_H
#define LIBSIMDPP_SIMDPP_DETAIL_PERMUTE_SSE_INT32_4x2_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/setup_arch.h>
#include <simdpp/types.h>
#include <simdpp/core/blend.h>
#include <simdpp/detail/shuffle/shuffle_mask.h>
#include <simdpp/detail/shuffle/sse_float32_4x2.h>

#if SIMDPP_USE_SSE2

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace sse_shuffle4x2_int32 {

/*  The code below implements generalized permutations for 2 elements sets
    within uint32 vectors.
*/

template<unsigned s0, unsigned s1, unsigned s2, unsigned s3>
struct impl_selector {

    // 0 1 2 3
    // 4 5 6 7
    static const bool is1_zip_lo1 = (s0==0 && s1==4 && s2==1 && s3==5);
    static const bool is2_zip_lo2 = (s0==4 && s1==0 && s2==5 && s3==1);
    static const bool is3_zip_hi1 = (s0==2 && s1==6 && s2==3 && s3==7);
    static const bool is4_zip_hi2 = (s0==6 && s1==2 && s2==7 && s3==3);
#if SIMDPP_USE_SSE4_1
    static const bool is5_blend = (s0==0 || s0==4) && (s1==1 || s1==5) &&
                                  (s2==2 || s2==6) && (s3==3 || s3==7);
#else
    static const bool is5_blend = false;
#endif
#if SIMDPP_USE_SSSE3
    static const bool is6_align = (s0==s1-1) && (s1==s2-1) && (s2==s3-1);
#else
    static const bool is6_align = false;
#endif

    static const int impl = is1_zip_lo1 ? 1 :
                            is2_zip_lo2 ? 2 :
                            is3_zip_hi1 ? 3 :
                            is4_zip_hi2 ? 4 :
                            is5_blend ? 5 :
                            is6_align ? 6 : 7;
};

template<unsigned N> struct shuffle_impl {};

// zip_lo1
template<> struct shuffle_impl<1> {
    template<unsigned, unsigned, unsigned, unsigned> SIMDPP_INL
    static uint32<4> run(const uint32<4>& a, const uint32<4>& b)
    {
        return _mm_unpacklo_epi32(a.native(), b.native());
    }
#if SIMDPP_USE_AVX2
    template<unsigned, unsigned, unsigned, unsigned> SIMDPP_INL
    static uint32<8> run(const uint32<8>& a, const uint32<8>& b)
    {
        return _mm256_unpacklo_epi32(a.native(), b.native());
    }
#endif
#if SIMDPP_USE_AVX512F
    template<unsigned, unsigned, unsigned, unsigned> SIMDPP_INL
    static uint32<16> run(const uint32<16>& a, const uint32<16>& b)
    {
        return _mm512_unpacklo_epi32(a.native(), b.native());
    }
#endif
};

// zip_lo2
template<> struct shuffle_impl<2> {
    template<unsigned, unsigned, unsigned, unsigned, unsigned N> SIMDPP_INL
    static uint32<N> run(const uint32<N>& a, const uint32<N>& b)
    {
        return shuffle_impl<1>::run<0,0,0,0>(b, a);
    }
};

// zip_hi1
template<> struct shuffle_impl<3> {
    template<unsigned, unsigned, unsigned, unsigned> SIMDPP_INL
    static uint32<4> run(const uint32<4>& a, const uint32<4>& b)
    {
        return _mm_unpackhi_epi32(a.native(), b.native());
    }
#if SIMDPP_USE_AVX2
    template<unsigned, unsigned, unsigned, unsigned> SIMDPP_INL
    static uint32<8> run(const uint32<8>& a, const uint32<8>& b)
    {
        return _mm256_unpackhi_epi32(a.native(), b.native());
    }
#endif
#if SIMDPP_USE_AVX512F
    template<unsigned, unsigned, unsigned, unsigned> SIMDPP_INL
    static uint32<16> run(const uint32<16>& a, const uint32<16>& b)
    {
        return _mm512_unpackhi_epi32(a.native(), b.native());
    }
#endif
};

// zip_hi2
template<> struct shuffle_impl<4> {
    template<unsigned, unsigned, unsigned, unsigned, unsigned N> SIMDPP_INL
    static uint32<N> run(const uint32<N>& a, const uint32<N>& b)
    {
        return shuffle_impl<3>::run<0,0,0,0>(b, a);
    }
};

// is5_blend
#if SIMDPP_USE_SSE4_1
template<> struct shuffle_impl<5> {
    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static uint32<4> run(const uint32<4>& a, const uint32<4>& b)
    {
        const unsigned mask = (s0<4 ? 0 : 0x03) | (s1<4 ? 0 : 0x0c) |
                              (s2<4 ? 0 : 0x30) | (s3<4 ? 0 : 0xc0);
        return _mm_blend_epi16(a.native(), b.native(), mask);
    }
#if SIMDPP_USE_AVX2
    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static uint32<8> run(const uint32<8>& a, const uint32<8>& b)
    {
        const unsigned mask = (s0<4 ? 0 : 1) | (s1<4 ? 0 : 2) |
                              (s2<4 ? 0 : 4) | (s3<4 ? 0 : 8);
        return _mm256_blend_epi32(a.native(), b.native(), mask | mask << 4);
    }
#endif
#if SIMDPP_USE_AVX512F
    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static uint32<16> run(const uint32<16>& a, const uint32<16>& b)
    {
        const unsigned mask = (s0<4 ? 0 : 1) | (s1<4 ? 0 : 2) |
                              (s2<4 ? 0 : 4) | (s3<4 ? 0 : 8);
        const unsigned mask2 = mask | mask << 4 | mask << 8 | mask << 12;
        return _mm512_mask_blend_epi32(mask2, a.native(), b.native());
    }
#endif
};
#endif

#if SIMDPP_USE_SSSE3
template<> struct shuffle_impl<6> {
    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static uint32<4> run(const uint32<4>& a, const uint32<4>& b)
    {
        return _mm_alignr_epi8(b.native(), a.native(), s0*4);
    }
#if SIMDPP_USE_AVX2
    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static uint32<8> run(const uint32<8>& a, const uint32<8>& b)
    {
        return _mm256_alignr_epi8(b.native(), a.native(), s0*4);
    }
#endif
#if SIMDPP_USE_AVX512F
    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static uint32<16> run(const uint32<16>& a, const uint32<16>& b)
    {
        // note that _mm512_alignr_epi32 operates on entire vector
        __m512i ap = _mm512_alignr_epi32(a.native(), a.native(), s0);
        const int mask = SIMDPP_SHUFFLE_MASK_4x2_4(s0>3, s0>2, s0>1, s0>0);
        return _mm512_mask_alignr_epi32(ap, mask, b.native(), b.native(), (s0+12)%16);
    }
#endif
};
#endif

template<> struct shuffle_impl<7> {
    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static uint32<4> run(const uint32<4>& a, const uint32<4>& b)
    {
#if SIMDPP_USE_AVX2
        const unsigned mask = SIMDPP_SHUFFLE_MASK_4x4(s0%4, s1%4, s2%4, s3%4);
        __m128i pa = _mm_shuffle_epi32(a.native(), mask);
        __m128i pb = _mm_shuffle_epi32(b.native(), mask);
        return _mm_blend_epi32(pa, pb, SIMDPP_SHUFFLE_MASK_4x2(s0/4,s1/4,s2/4,s3/4));
#elif SIMDPP_USE_SSE4_1
        const unsigned mask = SIMDPP_SHUFFLE_MASK_4x4(s0%4, s1%4, s2%4, s3%4);
        __m128i pa = _mm_shuffle_epi32(a.native(), mask);
        __m128i pb = _mm_shuffle_epi32(b.native(), mask);
        return _mm_blend_epi16(pa, pb, SIMDPP_SHUFFLE_MASK_4x4(s0/4*0x3,s1/4*0x3,s2/4*0x3,s3/4*0x3));
#else
        __m128 na = _mm_castsi128_ps(a.native());
        __m128 nb = _mm_castsi128_ps(b.native());
        __m128 ab1 = _mm_shuffle_ps(na, nb, _MM_SHUFFLE(s1%4, s0%4, s1%4, s0%4));
        __m128 ab2 = _mm_shuffle_ps(na, nb, _MM_SHUFFLE(s3%4, s2%4, s3%4, s2%4));
        float32<4> r = _mm_shuffle_ps(ab1, ab2, _MM_SHUFFLE(s3/4?3:1, s2/4?2:0, s1/4?3:1, s0/4?2:0));
        return uint32<4>(r);
#endif
    }
#if SIMDPP_USE_AVX2
    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static uint32<8> run(const uint32<8>& a, const uint32<8>& b)
    {
        const unsigned mask = SIMDPP_SHUFFLE_MASK_4x4(s0%4, s1%4, s2%4, s3%4);
        __m256i pa = _mm256_shuffle_epi32(a.native(), mask);
        __m256i pb = _mm256_shuffle_epi32(b.native(), mask);
        return _mm256_blend_epi32(pa, pb, SIMDPP_SHUFFLE_MASK_4x2_2(s0/4,s1/4,s2/4,s3/4));
    }
#endif
#if SIMDPP_USE_AVX512F
    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static uint32<16> run(const uint32<16>& a, const uint32<16>& b)
    {
        const unsigned shuf_mask = SIMDPP_SHUFFLE_MASK_4x4(s0%4,s1%4,s2%4,s3%4);
        __m512i ap = _mm512_shuffle_epi32(a.native(), _MM_PERM_ENUM(shuf_mask));
        const int mask = SIMDPP_SHUFFLE_MASK_4x2_4(s0/4,s1/4,s2/4,s3/4);
        return _mm512_mask_shuffle_epi32(ap, mask, b.native(),
                                         _MM_PERM_ENUM(shuf_mask));
    }
#endif
};

template<unsigned s0, unsigned s1, unsigned s2, unsigned s3, unsigned N>
uint32<N> do_shuffle(const uint32<N>& a, const uint32<N>& b)
{
    return shuffle_impl<impl_selector<s0, s1, s2, s3>::impl>::template run<s0, s1, s2, s3>(a, b);
}

} // namespace sse_shuffle4x2_uint32
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
#endif
