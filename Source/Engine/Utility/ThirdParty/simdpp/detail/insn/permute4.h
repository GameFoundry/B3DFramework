/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_PERMUTE4_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_PERMUTE4_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/make_shuffle_bytes_mask.h>
#include <simdpp/core/permute_bytes16.h>
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

// forward declarations due to circular dependencies
template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
uint64x4 permute_emul(const uint64x4& a);
template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
float64x4 permute_emul(const float64x4& a);

// ----

template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
uint16x8 i_permute4(const uint16x8& a)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
#if SIMDPP_USE_NULL
    return detail::null::permute<s0,s1,s2,s3>(a);
#elif SIMDPP_USE_SSE2
    uint16<8> b = a;
    b = _mm_shufflelo_epi16(b.native(), SIMDPP_SHUFFLE_MASK_4x4(s0, s1, s2, s3));
    b = _mm_shufflehi_epi16(b.native(), SIMDPP_SHUFFLE_MASK_4x4(s0, s1, s2, s3));
    return b;
#elif SIMDPP_USE_NEON
    return detail::neon_shuffle_int16x8::permute4<s0,s1,s2,s3>(a);
#elif SIMDPP_USE_ALTIVEC
    // TODO optimize
    uint16x8 mask = make_shuffle_bytes16_mask<s0,s1,s2,s3>(mask);
    return permute_bytes16(a, mask);
#elif SIMDPP_USE_MSA
    return (v8u16) __msa_shf_h((v8i16) a.native(), SIMDPP_SHUFFLE_MASK_4x4(s0,s1,s2,s3));
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
uint16x16 i_permute4(const uint16x16& a)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    uint16<16> b = a;
    b = _mm256_shufflelo_epi16(b.native(), SIMDPP_SHUFFLE_MASK_4x4(s0, s1, s2, s3));
    b = _mm256_shufflehi_epi16(b.native(), SIMDPP_SHUFFLE_MASK_4x4(s0, s1, s2, s3));
    return b;
}
#endif

#if SIMDPP_USE_AVX512BW
template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
uint16<32> i_permute4(const uint16<32>& a)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    uint16<32> r = a;
    r = _mm512_shufflelo_epi16(r.native(), SIMDPP_SHUFFLE_MASK_4x4(s0, s1, s2, s3));
    r = _mm512_shufflehi_epi16(r.native(), SIMDPP_SHUFFLE_MASK_4x4(s0, s1, s2, s3));
    return r;
}
#endif

template<unsigned s0, unsigned s1, unsigned s2, unsigned s3, unsigned N> SIMDPP_INL
uint16<N> i_permute4(const uint16<N>& a)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    SIMDPP_VEC_ARRAY_IMPL1(uint16<N>, (i_permute4<s0,s1,s2,s3>), a);
}

// -----------------------------------------------------------------------------

template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
uint32x4 i_permute4(const uint32x4& a)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
#if SIMDPP_USE_NULL
    return detail::null::permute<s0,s1,s2,s3>(a);
#elif SIMDPP_USE_SSE2
    return _mm_shuffle_epi32(a.native(), _MM_SHUFFLE(s3, s2, s1, s0));
#elif SIMDPP_USE_NEON
    return detail::neon_shuffle_int32x4::permute4<s0,s1,s2,s3>(a);
#elif SIMDPP_USE_ALTIVEC
    // TODO optimize
    uint32x4 mask = make_shuffle_bytes16_mask<s0,s1,s2,s3>(mask);
    return permute_bytes16(a, mask);
#elif SIMDPP_USE_MSA
    return (v4u32) __msa_shf_w((v4i32) a.native(), SIMDPP_SHUFFLE_MASK_4x4(s0,s1,s2,s3));
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
uint32x8 i_permute4(const uint32x8& a)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    return _mm256_shuffle_epi32(a.native(), _MM_SHUFFLE(s3, s2, s1, s0));
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
uint32<16> i_permute4(const uint32<16>& a)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    return _mm512_shuffle_epi32(a.native(), _MM_PERM_ENUM(_MM_SHUFFLE(s3, s2, s1, s0)));
}
#endif

template<unsigned s0, unsigned s1, unsigned s2, unsigned s3, unsigned N> SIMDPP_INL
uint32<N> i_permute4(const uint32<N>& a)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    SIMDPP_VEC_ARRAY_IMPL1(uint32<N>, (i_permute4<s0,s1,s2,s3>), a);
}

// -----------------------------------------------------------------------------

template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
float32x4 i_permute4(const float32x4& a)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
#if SIMDPP_USE_NULL
    return detail::null::permute<s0,s1,s2,s3>(a);
#elif SIMDPP_USE_SSE2
    return _mm_shuffle_ps(a.native(), a.native(), _MM_SHUFFLE(s3, s2, s1, s0));
#elif SIMDPP_USE_NEON
    return float32x4(detail::neon_shuffle_int32x4::permute4<s0,s1,s2,s3>(int32x4(a)));
#elif SIMDPP_USE_ALTIVEC
    // TODO optimize
    uint32x4 mask = make_shuffle_bytes16_mask<s0,s1,s2,s3>(mask);
    return permute_bytes16(a, mask);
#elif SIMDPP_USE_MSA
    return (v4f32) __msa_shf_w((v4i32) a.native(), SIMDPP_SHUFFLE_MASK_4x4(s0,s1,s2,s3));
#endif
}

#if SIMDPP_USE_AVX
template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
float32x8 i_permute4(const float32x8& a)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    return _mm256_shuffle_ps(a.native(), a.native(), _MM_SHUFFLE(s3, s2, s1, s0));
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
float32<16> i_permute4(const float32<16>& a)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    return _mm512_shuffle_ps(a.native(), a.native(), _MM_SHUFFLE(s3, s2, s1, s0));
}
#endif

template<unsigned s0, unsigned s1, unsigned s2, unsigned s3, unsigned N> SIMDPP_INL
float32<N> i_permute4(const float32<N>& a)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    SIMDPP_VEC_ARRAY_IMPL1(float32<N>, (i_permute4<s0,s1,s2,s3>), a);
}

// -----------------------------------------------------------------------------

template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
uint64x4 i_permute4(const uint64x4& a)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
#if SIMDPP_USE_AVX2
    return _mm256_permute4x64_epi64(a.native(), _MM_SHUFFLE(s3, s2, s1, s0));
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_VSX_207 || SIMDPP_USE_MSA
    return permute_emul<s0,s1,s2,s3>(a);
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC
    uint64x4 r;
    r.vec(0).el(0) = a.vec(s0/2).el(s0%2);
    r.vec(0).el(1) = a.vec(s1/2).el(s1%2);
    r.vec(1).el(0) = a.vec(s2/2).el(s2%2);
    r.vec(1).el(1) = a.vec(s3/2).el(s3%2);
    return r;
#endif
}

#if SIMDPP_USE_AVX512F
template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
uint64<8> i_permute4(const uint64<8>& a)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    return _mm512_permutex_epi64(a.native(), _MM_SHUFFLE(s3, s2, s1, s0));
}
#endif

template<unsigned s0, unsigned s1, unsigned s2, unsigned s3, unsigned N> SIMDPP_INL
uint64<N> i_permute4(const uint64<N>& a)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    SIMDPP_VEC_ARRAY_IMPL1(uint64<N>, (i_permute4<s0,s1,s2,s3>), a);
}

// -----------------------------------------------------------------------------

template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
float64x4 i_permute4(const float64x4& a)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
#if SIMDPP_USE_AVX2
    return _mm256_permute4x64_pd(a.native(), _MM_SHUFFLE(s3, s2, s1, s0));
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON64 || SIMDPP_USE_VSX_206 || SIMDPP_USE_MSA
    return permute_emul<s0,s1,s2,s3>(a);
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC
    float64x4 r;
    r.vec(0).el(0) = a.vec(s0/2).el(s0%2);
    r.vec(0).el(1) = a.vec(s1/2).el(s1%2);
    r.vec(1).el(0) = a.vec(s2/2).el(s2%2);
    r.vec(1).el(1) = a.vec(s3/2).el(s3%2);
    return r;
#endif
}

#if SIMDPP_USE_AVX512F
template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
float64<8> i_permute4(const float64<8>& a)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    return _mm512_permutex_pd(a.native(), _MM_SHUFFLE(s3, s2, s1, s0));
}
#endif

template<unsigned s0, unsigned s1, unsigned s2, unsigned s3, unsigned N> SIMDPP_INL
float64<N> i_permute4(const float64<N>& a)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    SIMDPP_VEC_ARRAY_IMPL1(float64<N>, (i_permute4<s0,s1,s2,s3>), a);
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

