/*  Copyright (C) 2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_PERMUTE_SSE_FLOAT64_4x2_H
#define LIBSIMDPP_SIMDPP_DETAIL_PERMUTE_SSE_FLOAT64_4x2_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/setup_arch.h>
#include <simdpp/types.h>
#include <simdpp/core/combine.h>
#include <simdpp/core/split.h>
#include <simdpp/detail/insn/shuffle2x2.h>
#include <simdpp/detail/shuffle/shuffle_mask.h>

#if SIMDPP_USE_AVX

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace sse_shuffle4x2_float64 {

/*  The code below implements generalized permutations for 4 elements sets
    within float64 vectors.
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
    static const bool is6_shuffle1 = false;
    static const bool is7_shuffle2 = false;
    static const bool is8_permx2var = true;
#elif SIMDPP_USE_AVX2
    // single shuffle_pd. Other cases handled by zip* and blend
    static const bool is6_shuffle1 = (s0==1 && s1==4 && s2==3 && s3==6);
    static const bool is7_shuffle2 = (s0==4 && s1==1 && s2==6 && s3==3);
    static const bool is8_permx2var = false;
#else
    static const bool is6_shuffle1 = (s0/2==0) && (s1/2==2) && (s2/2==1) && (s3/2==3);
    static const bool is7_shuffle2 = (s0/2==2) && (s1/2==0) && (s2/2==3) && (s3/2==1);
    static const bool is8_permx2var = false;
#endif
#if SIMDPP_USE_AVX2
    static const bool is9_perm_blend = true;
#else
    static const bool is9_perm_blend = false;
#endif
    static const int impl = is1_zip_lo1 ? 1 :
                            is2_zip_lo2 ? 2 :
                            is3_zip_hi1 ? 3 :
                            is4_zip_hi2 ? 4 :
                            is5_blend ? 5 :
                            is6_shuffle1 ? 6 :
                            is7_shuffle2 ? 7 :
                            is8_permx2var ? 8 :
                            is9_perm_blend ? 9 : 10;
};

template<unsigned N> struct shuffle_impl {};

// zip_lo1
template<> struct shuffle_impl<1> {
    template<unsigned, unsigned, unsigned, unsigned> SIMDPP_INL
    static float64<4> run(const float64<4>& a, const float64<4>& b)
    {
        return _mm256_unpacklo_pd(a.native(), b.native());
    }
#if SIMDPP_USE_AVX512F
    template<unsigned, unsigned, unsigned, unsigned> SIMDPP_INL
    static float64<8> run(const float64<8>& a, const float64<8>& b)
    {
        return _mm512_unpacklo_pd(a.native(), b.native());
    }
#endif
};

// zip_lo2
template<> struct shuffle_impl<2> {
    template<unsigned, unsigned, unsigned, unsigned, unsigned N> SIMDPP_INL
    static float64<N> run(const float64<N>& a, const float64<N>& b)
    {
        return shuffle_impl<1>::run<0,0,0,0>(b, a);
    }
};

// zip_hi1
template<> struct shuffle_impl<3> {
    template<unsigned, unsigned, unsigned, unsigned> SIMDPP_INL
    static float64<4> run(const float64<4>& a, const float64<4>& b)
    {
        return _mm256_unpackhi_pd(a.native(), b.native());
    }
#if SIMDPP_USE_AVX512F
    template<unsigned, unsigned, unsigned, unsigned> SIMDPP_INL
    static float64<8> run(const float64<8>& a, const float64<8>& b)
    {
        return _mm512_unpackhi_pd(a.native(), b.native());
    }
#endif
};

// zip_hi2
template<> struct shuffle_impl<4> {
    template<unsigned, unsigned, unsigned, unsigned, unsigned N> SIMDPP_INL
    static float64<N> run(const float64<N>& a, const float64<N>& b)
    {
        return shuffle_impl<3>::run<0,0,0,0>(b, a);
    }
};

// is5_blend
template<> struct shuffle_impl<5> {
    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static float64<4> run(const float64<4>& a, const float64<4>& b)
    {
        const unsigned mask = (s0<4 ? 0 : 1) | (s1<4 ? 0 : 2) | (s2<4 ? 0 : 4) | (s3<4 ? 0 : 8);
        return _mm256_blend_pd(a.native(), b.native(), mask);
    }
#if SIMDPP_USE_AVX512F
    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static float64<8> run(const float64<8>& a, const float64<8>& b)
    {
        const unsigned mask = (s0<4 ? 0 : 1) | (s1<4 ? 0 : 2) | (s2<4 ? 0 : 4) | (s3<4 ? 0 : 8);
        const unsigned mask2 = mask | mask << 4;
        return _mm512_mask_blend_pd(mask2, a.native(), b.native());
    }
#endif
};

// is6_shuffle1 (only AVX-AVX2)
template<> struct shuffle_impl<6> {
    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static float64<4> run(const float64<4>& a, const float64<4>& b)
    {
        if (s0%2 != s2%2 || s1%2 != s3%2) {
            __m128d a1 = _mm256_castpd256_pd128(a.native());
            __m128d b1 = _mm256_castpd256_pd128(b.native());
            a1 = _mm_shuffle_pd(a1, b1, SIMDPP_SHUFFLE_MASK_2x2(s0, s1-4));
            __m256d t = _mm256_shuffle_pd(a.native(), b.native(),
                                          SIMDPP_SHUFFLE_MASK_2x2_2(s2-2,s3-6));
            t = _mm256_insertf128_pd(t, a1, 0);
            return t;
        } else {
            return _mm256_shuffle_pd(a.native(), b.native(),
                                     SIMDPP_SHUFFLE_MASK_2x2_2(s0, s1-4));
        }
    }
};

// is7_shuffle2 (only AVX-AVX2)
template<> struct shuffle_impl<7> {
    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static float64<4> run(const float64<4>& a, const float64<4>& b)
    {
        if (s0%2 != s2%2 || s1%2 != s3%2) {
            __m128d a1 = _mm256_castpd256_pd128(a.native());
            __m128d b1 = _mm256_castpd256_pd128(b.native());
            a1 = _mm_shuffle_pd(b1, a1, SIMDPP_SHUFFLE_MASK_2x2(s1,s0-4));
            __m256d t = _mm256_shuffle_pd(b.native(), a.native(),
                                          SIMDPP_SHUFFLE_MASK_2x2_2(s3-2,s2-6));
            t = _mm256_insertf128_pd(t, a1, 0);
            return t;
        } else {
            return _mm256_shuffle_pd(b.native(), a.native(),
                                     SIMDPP_SHUFFLE_MASK_2x2_2(s1,s0-4));
        }
    }
};

// is8_perm2xvar
#if SIMDPP_USE_AVX512F
template<> struct shuffle_impl<8> {

    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static float64<4> run(const float64<4>& a, const float64<4>& b)
    {
        uint8<16> mask = make_uint(s0, s1, s2, s3);
        // FIXME: GCC BUG
        // return  _mm256_permutex2var_pd(a.native(), _mm256_cvtepi8_epi64(mask), b);
        return _mm512_castpd512_pd256(_mm512_permutex2var_pd(_mm512_castpd256_pd512(a.native()),
                                                             _mm512_cvtepi8_epi64(mask.native()),
                                                             _mm512_castpd256_pd512(b.native())));
    }

    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static float64<8> run(const float64<8>& a, const float64<8>& b)
    {
        const unsigned p0 = s0<4 ? s0 : s0+4;
        const unsigned p1 = s1<4 ? s1 : s1+4;
        const unsigned p2 = s2<4 ? s2 : s2+4;
        const unsigned p3 = s3<4 ? s3 : s3+4;
        uint8<16> mask = make_uint(p0, p1, p2, p3, p0+4, p1+4, p2+4, p3+4);
        return _mm512_permutex2var_pd(a.native(), _mm512_cvtepi8_epi64(mask.native()), b.native());
    }
};
#endif

// is9_perm_blend
#if SIMDPP_USE_AVX2
template<> struct shuffle_impl<9> {
    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static float64<4> run(const float64<4>& a, const float64<4>& b)
    {
        const unsigned bl_mask = (s0<4 ? 0 : 1) | (s1<4 ? 0 : 2) | (s2<4 ? 0 : 4) | (s3<4 ? 0 : 8);
        __m256d ta = _mm256_permute4x64_pd(a.native(), SIMDPP_SHUFFLE_MASK_4x4(s0%4, s1%4, s2%4, s3%4));
        __m256d tb = _mm256_permute4x64_pd(b.native(), SIMDPP_SHUFFLE_MASK_4x4(s0%4, s1%4, s2%4, s3%4));
        return _mm256_blend_pd(ta, tb, bl_mask);
    }
};
#endif

template<unsigned s0, unsigned s1, class V>
V i_shuffle_emul_64x4_half(const V& a0, const V& a1, const V& b0, const V& b1)
{
    const V& h0 = s0 < 2 ? a0 :
                  s0 < 4 ? a1 :
                  s0 < 6 ? b0 : b1;
    const V& h1 = s1 < 2 ? a0 :
                  s1 < 4 ? a1 :
                  s1 < 6 ? b0 : b1;
    return insn::i_shuffle2x2<s0%2, s1%2+2>(h0, h1);
}

// any (only on AVX)
template<> struct shuffle_impl<10> {
    template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
    static float64<4> run(const float64<4>& a, const float64<4>& b)
    {
        float64<2> a0, a1, b0, b1, r0, r1;
        split(a, a0, a1);
        split(b, b0, b1);
        r0 = i_shuffle_emul_64x4_half<s0,s1>(a0, a1, b0, b1);
        r1 = i_shuffle_emul_64x4_half<s2,s3>(a0, a1, b0, b1);
        return combine(r0, r1);
    }
};

template<unsigned s0, unsigned s1, unsigned s2, unsigned s3, unsigned N>
float64<N> do_shuffle(const float64<N>& a, const float64<N>& b)
{
    return shuffle_impl<impl_selector<s0, s1, s2, s3>::impl>::run<s0, s1, s2, s3>(a, b);
}

} // namespace sse_shuffle4x2_float64
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
#endif
