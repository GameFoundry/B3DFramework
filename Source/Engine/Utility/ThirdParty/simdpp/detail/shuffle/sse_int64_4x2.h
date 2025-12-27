/*  Copyright (C) 2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_PERMUTE_SSE_INT64_4x2_H
#define LIBSIMDPP_SIMDPP_DETAIL_PERMUTE_SSE_INT64_4x2_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/setup_arch.h>
#include <simdpp/types.h>
#include <simdpp/core/combine.h>
#include <simdpp/core/split.h>
#include <simdpp/detail/insn/shuffle2x2.h>
#include <simdpp/detail/shuffle/shuffle_mask.h>

#if SIMDPP_USE_AVX2

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace sse_shuffle4x2_int64 {

/*  The code below implements generalized permutations for 4 elements sets
    within uint64 vectors.
*/

template<unsigned s0, unsigned s1, unsigned s2, unsigned s3>
struct impl_selector {

    // 0 1 2 3
    // 4 5 6 7
    static const bool is1_zip_lo1 = (s0==0 && s1==4 && s2==2 && s3==6);
    static const bool is2_zip_lo2 = (s0==4 && s1==0 && s2==6 && s3==2);
    static const bool is3_zip_hi1 = (s0==1 && s1==5 && s2==3 && s3==7);
    static const bool is4_zip_hi2 = (s0==5 && s1==1 && s2==7 && s3==3);
    static const bool is5_blend = (s0==0 || s0==4) && (s1==1 || s1==5) &&
                                  (s2==2 || s2==6) && (s3==3 || s3==7);
#if SIMDPP_USE_AVX512F
    static const bool is6_swap1 = false;
    static const bool is7_swap2 = false;
    static const bool is8_permx2var = true;
#else
    static const bool is6_swap1 = (s0==1 && s1==4 && s2==3 && s3==6);
    static const bool is7_swap2 = (s0==4 && s1==1 && s2==6 && s3==3);
    static const bool is8_permx2var = false;
#endif
    static const int impl = is1_zip_lo1 ? 1 :
                            is2_zip_lo2 ? 2 :
                            is3_zip_hi1 ? 3 :
                            is4_zip_hi2 ? 4 :
                            is5_blend ? 5 :
                            is6_swap1 ? 6 :
                            is7_swap2 ? 7 :
                            is8_permx2var ? 8 : 9;
};

template<unsigned N> struct shuffle_impl {};

// zip_lo1
template<> struct shuffle_impl<1> {
    template<unsigned, unsigned, unsigned, unsigned> SIMDPP_INL
    static uint64<4> run(const uint64<4>& a, const uint64<4>& b)
    {
        return _mm256_unpacklo_epi64(a.native(), b.native());
    }
#if SIMDPP_USE_AVX512F
    template<unsigned, unsigned, unsigned, unsigned> SIMDPP_INL
    static uint64<8> run(const uint64<8>& a, const uint64<8>& b)
    {
        return _mm512_unpacklo_epi64(a.native(), b.native());
    }
#endif
};

// zip_lo2
template<> struct shuffle_impl<2> {
    template<unsigned, unsigned, unsigned, unsigned, unsigned N> SIMDPP_INL
    static uint64<N> run(const uint64<N>& a, const uint64<N>& b)
    {
        return shuffle_impl<1>::run<0,0,0,0>(b, a);
    }
};

// zip_hi1
template<> struct shuffle_impl<3> {
    template<unsigned, unsigned, unsigned, unsigned> SIMDPP_INL
    static uint64<4> run(const uint64<4>& a, const uint64<4>& b)
    {
        return _mm256_unpackhi_epi64(a.native(), b.native());
    }
#if SIMDPP_USE_AVX512F
    template<unsigned, unsigned, unsigned, unsigned> SIMDPP_INL
    static uint64<8> run(const uint64<8>& a, const uint64<8>& b)
    {
        return _mm512_unpackhi_epi64(a.native(), b.native());
    }
#endif
};

// zip_hi2
template<> struct shuffle_impl<4> {
    template<unsigned, unsigned, unsigned, unsigned, unsigned N> SIMDPP_INL
    static uint64<N> run(const uint64<N>& a, const uint64<N>& b)
    {
        return shuffle_impl<3>::run<0,0,0,0>(b, a);
    }
};

// is5_blend
template<> struct shuffle_impl<5> {
    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static uint64<4> run(const uint64<4>& a, const uint64<4>& b)
    {
        const unsigned mask = (s0<4 ? 0 : 0x03) | (s1<4 ? 0 : 0x0c) |
                              (s2<4 ? 0 : 0x30) | (s3<4 ? 0 : 0xc0);
        return _mm256_blend_epi32(a.native(), b.native(), mask);
    }
#if SIMDPP_USE_AVX512F
    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static uint64<8> run(const uint64<8>& a, const uint64<8>& b)
    {
        const unsigned mask = (s0<4 ? 0 : 1) | (s1<4 ? 0 : 2) |
                              (s2<4 ? 0 : 4) | (s3<4 ? 0 : 8);
        const unsigned mask2 = mask | mask << 4;
        return _mm512_mask_blend_epi64(mask2, a.native(), b.native());
    }
#endif
};

// is6_swap1 (only AVX2)
template<> struct shuffle_impl<6> {
    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static uint64<4> run(const uint64<4>& a, const uint64<4>& b)
    {
        __m256i n = _mm256_blend_epi32(a.native(), b.native(), 0x33);
        return _mm256_permute4x64_epi64(n, _MM_SHUFFLE(2,3,0,1));
    }
};

// is7_swap2 (only AVX2)
template<> struct shuffle_impl<7> {
    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static uint64<4> run(const uint64<4>& a, const uint64<4>& b)
    {
        __m256i n = _mm256_blend_epi32(a.native(), b.native(), 0xcc);
        return _mm256_permute4x64_epi64(n, _MM_SHUFFLE(2,3,0,1));
    }
};

// is8_perm2xvar
#if SIMDPP_USE_AVX512F
template<> struct shuffle_impl<8> {

    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static uint64<4> run(const uint64<4>& a, const uint64<4>& b)
    {
#if SIMDPP_USE_AVX512VL
        uint8<16> mask = make_uint(s0, s1, s2, s3);
        return _mm256_permutex2var_epi64(a.native(),
                                         _mm256_cvtepi8_epi64(mask.native()),
                                         b.native());
#else
        const unsigned p0 = s0<4 ? s0 : s0+4;
        const unsigned p1 = s1<4 ? s1 : s1+4;
        const unsigned p2 = s2<4 ? s2 : s2+4;
        const unsigned p3 = s3<4 ? s3 : s3+4;
        uint8<16> mask = make_uint(p0, p1, p2, p3);
        __m512i res = _mm512_permutex2var_epi64(_mm512_castsi256_si512(a.native()),
                                                _mm512_cvtepi8_epi64(mask.native()),
                                                _mm512_castsi256_si512(b.native()));
        return _mm512_castsi512_si256(res);
#endif
    }

    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static uint64<8> run(const uint64<8>& a, const uint64<8>& b)
    {
        const unsigned p0 = s0<4 ? s0 : s0+4;
        const unsigned p1 = s1<4 ? s1 : s1+4;
        const unsigned p2 = s2<4 ? s2 : s2+4;
        const unsigned p3 = s3<4 ? s3 : s3+4;
        uint8<16> mask = make_uint(p0, p1, p2, p3, p0+4, p1+4, p2+4, p3+4);
        return _mm512_permutex2var_epi64(a.native(),
                                         _mm512_cvtepi8_epi64(mask.native()),
                                         b.native());
    }
};
#endif

// any (only on AVX2)
template<> struct shuffle_impl<9> {
    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static uint64<4> run(const uint64<4>& a, const uint64<4>& b)
    {
        const unsigned shuf_mask = SIMDPP_SHUFFLE_MASK_4x4(s0%4, s1%4, s2%4, s3%4);
        const unsigned bl_mask = (s0<4 ? 0 : 0x03) | (s1<4 ? 0 : 0x0c) |
                                 (s2<4 ? 0 : 0x30) | (s3<4 ? 0 : 0xc0);
        __m256i ta = _mm256_permute4x64_epi64(a.native(), shuf_mask);
        __m256i tb = _mm256_permute4x64_epi64(b.native(), shuf_mask);
        return _mm256_blend_epi32(ta, tb, bl_mask);
    }
};


template<unsigned s0, unsigned s1, unsigned s2, unsigned s3, unsigned N>
uint64<N> do_shuffle(const uint64<N>& a, const uint64<N>& b)
{
    const unsigned selector = impl_selector<s0, s1, s2, s3>::impl;
    return shuffle_impl<selector>::template run<s0, s1, s2, s3>(a, b);
}

} // namespace sse_shuffle4x2_int64
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
#endif
