/*  Copyright (C) 2012-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_SHUFFLE128_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_SHUFFLE128_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

/** Shuffles 128 bit parts within the vectors.

@code
    switch(s0):
        case 0: r[0..127] = a[0..127]
        case 1: r[0..127] = a[128..255]
        case 2: r[0..127] = a[256..383]
        case 3: r[0..127] = a[384..511]

    switch(s1):
        case 0: r[128..255] = a[0..127]
        case 1: r[128..255] = a[128..255]
        case 2: r[128..255] = a[256..383]
        case 3: r[128..255] = a[384..511]

    switch(s2):
        case 0: r[256..383] = b[0..127]
        case 1: r[256..383] = b[128..255]
        case 2: r[256..383] = b[256..383]
        case 3: r[256..383] = b[384..511]

    switch(s3):
        case 0: r[384..511] = b[0..127]
        case 1: r[384..511] = b[128..255]
        case 2: r[384..511] = b[256..383]
        case 3: r[384..511] = b[384..511]
@endcode
*/
#if SIMDPP_USE_AVX512BW
template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
uint8<64> shuffle2_128(const uint8<64>& a, const uint8<64>& b)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    return _mm512_shuffle_i32x4(a.native(), b.native(), (s3<<6) + (s2<<4) + (s1<<2) + s0);
}

template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
uint16<32> shuffle2_128(const uint16<32>& a, const uint16<32>& b)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    return _mm512_shuffle_i32x4(a.native(), b.native(), (s3<<6) + (s2<<4) + (s1<<2) + s0);
}
#endif
#if SIMDPP_USE_AVX512F
template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
uint32<16> shuffle2_128(const uint32<16>& a, const uint32<16>& b)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    return _mm512_shuffle_i32x4(a.native(), b.native(), (s3<<6) + (s2<<4) + (s1<<2) + s0);
}
template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
uint64<8> shuffle2_128(const uint64<8>& a, const uint64<8>& b)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    return _mm512_shuffle_i64x2(a.native(), b.native(), (s3<<6) + (s2<<4) + (s1<<2) + s0);
}
template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
float32<16> shuffle2_128(const float32<16>& a, const float32<16>& b)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    return _mm512_shuffle_f32x4(a.native(), b.native(), (s3<<6) + (s2<<4) + (s1<<2) + s0);
}
template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
float64<8> shuffle2_128(const float64<8>& a, const float64<8>& b)
{
    static_assert(s0 < 4 && s1 < 4 && s2 < 4 && s3 < 4, "Selector out of range");
    return _mm512_shuffle_f64x2(a.native(), b.native(), (s3<<6) + (s2<<4) + (s1<<2) + s0);
}
#endif

/** Shuffles 128 bit parts within the vectors.

    For larger than 256-bits vectors the same operation is applied to each
    256-bit element.

@code
    switch(s0):
        case 0: r[0..127] = a[0..127]
        case 1: r[0..127] = a[128..255]

    switch(s1):
        case 0: r[128..255] = b[0..127]
        case 1: r[128..255] = b[128..255]
@endcode
*/
template<unsigned s0, unsigned s1> SIMDPP_INL
uint8x32 shuffle1_128(const uint8x32& a, const uint8x32& b)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
#if SIMDPP_USE_AVX2
    return _mm256_permute2x128_si256(a.native(), b.native(), ((s1+2)<<4) + s0);
#else
    uint8x32 r;
    r.vec(0) = a.vec(s0);
    r.vec(1) = b.vec(s1);
    return r;
#endif
}
template<unsigned s0, unsigned s1> SIMDPP_INL
uint16x16 shuffle1_128(const uint16x16& a, const uint16x16& b) { return (uint16x16)shuffle1_128<s0,s1>(uint8x32(a), uint8x32(b)); }
template<unsigned s0, unsigned s1> SIMDPP_INL
uint32x8 shuffle1_128(const uint32x8& a, const uint32x8& b) { return (uint32x8)shuffle1_128<s0,s1>(uint8x32(a), uint8x32(b)); }
template<unsigned s0, unsigned s1> SIMDPP_INL
uint64x4 shuffle1_128(const uint64x4& a, const uint64x4& b) { return (uint64x4)shuffle1_128<s0,s1>(uint8x32(a), uint8x32(b)); }

template<unsigned s0, unsigned s1> SIMDPP_INL
float32x8 shuffle1_128(const float32x8& a, const float32x8& b)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
#if SIMDPP_USE_AVX
    return _mm256_permute2f128_ps(a.native(), b.native(), ((s1+2)<<4) + s0);
#else
    float32x8 r;
    r.vec(0) = a.vec(s0);
    r.vec(1) = b.vec(s1);
    return r;
#endif
}
template<unsigned s0, unsigned s1> SIMDPP_INL
float64x4 shuffle1_128(const float64x4& a, const float64x4& b)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
#if SIMDPP_USE_AVX
    return _mm256_permute2f128_pd(a.native(), b.native(), ((s1+2)<<4) + s0);
#else
    float64x4 r;
    r.vec(0) = a.vec(s0);
    r.vec(1) = b.vec(s1);
    return r;
#endif
}

#if SIMDPP_USE_AVX512F
template<unsigned s0, unsigned s1> SIMDPP_INL
uint32<16> shuffle1_128(const uint32<16>& a, const uint32<16>& b)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
    return shuffle2_128<s0,s1,s0+2,s1+2>(a, b);
}

template<unsigned s0, unsigned s1> SIMDPP_INL
uint64<8> shuffle1_128(const uint64<8>& a, const uint64<8>& b)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
    return shuffle2_128<s0,s1,s0+2,s1+2>(a, b);
}

template<unsigned s0, unsigned s1> SIMDPP_INL
float32<16> shuffle1_128(const float32<16>& a, const float32<16>& b)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
    return shuffle2_128<s0,s1,s0+2,s1+2>(a, b);
}

template<unsigned s0, unsigned s1> SIMDPP_INL
float64<8> shuffle1_128(const float64<8>& a, const float64<8>& b)
{
    static_assert(s0 < 2 && s1 < 2, "Selector out of range");
    return shuffle2_128<s0,s1,s0+2,s1+2>(a, b);
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned s0, unsigned s1, unsigned s2, unsigned s3, class V> SIMDPP_INL
V permute4_128(const V& a)
{
    return shuffle2_128<s0,s1,s2,s3>(a, a);
}
#endif

} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

