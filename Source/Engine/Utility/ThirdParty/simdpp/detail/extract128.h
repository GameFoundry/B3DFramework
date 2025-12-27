/*  Copyright (C) 2013  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_DETAIL_EXTRACT128_H
#define LIBSIMDPP_DETAIL_EXTRACT128_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

#if SIMDPP_USE_AVX2
template<unsigned s>
SIMDPP_INL uint8x16 extract128(const uint8x32& a)
{
    return s == 0 ? _mm256_castsi256_si128(a.native()) : _mm256_extracti128_si256(a.native(), 1);
}

template<unsigned s>
SIMDPP_INL uint16x8 extract128(const uint16x16& a) { return (uint16x8) extract128<s>(uint8x32(a)); }
template<unsigned s>
SIMDPP_INL uint32x4 extract128(const uint32x8& a) { return (uint32x4) extract128<s>(uint8x32(a)); }
template<unsigned s>
SIMDPP_INL uint64x2 extract128(const uint64x4& a) { return (uint64x2) extract128<s>(uint8x32(a)); }

template<unsigned s>
SIMDPP_INL int8x16 extract128(const int8x32& a) { return (int8x16) extract128<s>(uint8x32(a)); }
template<unsigned s>
SIMDPP_INL int16x8 extract128(const int16x16& a) { return (int16x8) extract128<s>(uint8x32(a)); }
template<unsigned s>
SIMDPP_INL int32x4 extract128(const int32x8& a) { return (int32x4) extract128<s>(uint8x32(a)); }
template<unsigned s>
SIMDPP_INL int64x2 extract128(const int64x4& a) { return (int64x2) extract128<s>(uint8x32(a)); }
#endif

#if SIMDPP_USE_AVX
template<unsigned s>
SIMDPP_INL float32x4 extract128(const float32x8& a)
{
    return s == 0 ? _mm256_castps256_ps128(a.native()) : _mm256_extractf128_ps(a.native(), 1);
}

template<unsigned s>
SIMDPP_INL float64x2 extract128(const float64x4& a)
{
    return s == 0 ? _mm256_castpd256_pd128(a.native()) : _mm256_extractf128_pd(a.native(), 1);
}
#endif

#if SIMDPP_USE_AVX512BW
template<unsigned s>
SIMDPP_INL uint8<16> extract128(const uint8<64>& a)
{
    return _mm512_extracti32x4_epi32(a.native(), s);
}
template<unsigned s>
SIMDPP_INL int8<16> extract128(const int8<64>& a) { return (int8<16>) extract128<s>(uint8<64>(a)); }

template<unsigned s>
SIMDPP_INL uint16<8> extract128(const uint16<32>& a) { return (uint16<8>) extract128<s>(uint8<64>(a)); }
template<unsigned s>
SIMDPP_INL int16<8> extract128(const int16<32>& a) { return (int16<8>) extract128<s>(uint8<64>(a)); }
#endif

#if SIMDPP_USE_AVX512F
template<unsigned s>
SIMDPP_INL uint32x4 extract128(const uint32<16>& a)
{
    return _mm512_extracti32x4_epi32(a.native(), s);
}

template<unsigned s>
SIMDPP_INL uint64x2 extract128(const uint64<8>& a) { return (uint64x2) extract128<s>(uint32<16>(a)); }

template<unsigned s>
SIMDPP_INL int32x4 extract128(const int32<16>& a) { return (int32x4) extract128<s>(uint32<16>(a)); }
template<unsigned s>
SIMDPP_INL int64x2 extract128(const int64<8>& a) { return (int64x2) extract128<s>(uint32<16>(a)); }

template<unsigned s>
SIMDPP_INL float32x4 extract128(const float32<16>& a)
{
    return _mm512_extractf32x4_ps(a.native(), s);
}

template<unsigned s>
SIMDPP_INL float64x2 extract128(const float64<8>& a)
{
    return _mm_castps_pd(_mm512_extractf32x4_ps(_mm512_castpd_ps(a.native()), s));
}

template<unsigned s>
SIMDPP_INL uint32x8 extract256(const uint32<16>& a)
{
    return _mm512_extracti64x4_epi64(a.native(), s);
}

template<unsigned s>
SIMDPP_INL uint64x4 extract256(const uint64<8>& a) { return (uint64x4) extract256<s>(uint32<16>(a)); }

template<unsigned s>
SIMDPP_INL int32x8 extract256(const int32<16>& a) { return (int32x8) extract256<s>(uint32<16>(a)); }
template<unsigned s>
SIMDPP_INL int64x4 extract256(const int64<8>& a) { return (int64x4) extract256<s>(uint32<16>(a)); }

template<unsigned s>
SIMDPP_INL float32<8> extract256(const float32<16>& a)
{
    return _mm256_castpd_ps(_mm512_extractf64x4_pd(_mm512_castps_pd(a.native()), s));
}

template<unsigned s>
SIMDPP_INL float64<4> extract256(const float64<8>& a)
{
    return _mm512_extractf64x4_pd(a.native(), s);
}
#endif

#if SIMDPP_USE_AVX512BW
template<unsigned s>
SIMDPP_INL uint8<32> extract256(const uint8<64>& a)
{
    return _mm512_extracti64x4_epi64(a.native(), s);
}

template<unsigned s>
SIMDPP_INL uint16<16> extract256(const uint16<32>& a)
{
    return _mm512_extracti64x4_epi64(a.native(), s);
}

template<unsigned s>
SIMDPP_INL int8<32> extract256(const int8<64>& a)
{
    return _mm512_extracti64x4_epi64(a.native(), s);
}

template<unsigned s>
SIMDPP_INL int16<16> extract256(const int16<32>& a)
{
    return _mm512_extracti64x4_epi64(a.native(), s);
}
#endif

} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
