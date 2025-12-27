/*  Copyright (C) 2011-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_EXTRACT_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_EXTRACT_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/cast.h>
#include <simdpp/core/move_l.h>
#include <simdpp/core/i_shift_l.h>
#include <simdpp/core/i_sub.h>
#include <simdpp/core/make_int.h>
#include <simdpp/detail/insn/split.h>
#include <simdpp/detail/mem_block.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

template<unsigned id> SIMDPP_INL
uint8_t i_extract(const uint8<16>& a)
{
#if SIMDPP_USE_NULL
    return a.el(id);
#elif SIMDPP_USE_SSE4_1
    // Explicit cast is needed due to bug in Clang headers (intrinsic
    // implemented as a macro with no appropriate casts) and a bug in Clang
    // (thinks explicit conversion operators have the same rank as the regular
    // ones)
    return _mm_extract_epi8(a.native(), id);
#elif SIMDPP_USE_SSE2
    unsigned shift = (id % 2 == 1) ? 8 : 0;
    return _mm_extract_epi16(a.native(), id/2) >> shift;
#elif SIMDPP_USE_NEON
    return vgetq_lane_u8(a.native(), id);
#elif SIMDPP_USE_ALTIVEC
    detail::mem_block<uint8x16> ax(a);
    vec_ste(a.native(), 0, &ax[id]);
    return ax[id];
#elif SIMDPP_USE_MSA
    return __msa_copy_u_b((v16i8) a.native(), id);
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned id> SIMDPP_INL
uint8_t i_extract(const uint8<32>& a)
{
    __m128i val = _mm256_extracti128_si256(a.native(), id / 16);
    return _mm_extract_epi8(val, id % 16);
}
#endif

#if SIMDPP_USE_AVX512BW
template<unsigned id> SIMDPP_INL
uint8_t i_extract(const uint8<64>& a)
{
    __m128i val = _mm512_extracti32x4_epi32(a.native(), id / 16);
    return _mm_extract_epi8(val, id % 16);
}
#endif

// -----------------------------------------------------------------------------

template<unsigned id> SIMDPP_INL
int8_t i_extract(const int8<16>& a)
{
#if SIMDPP_USE_MSA
    return __msa_copy_s_b(a.native(), id);
#else
    return i_extract<id>(uint8x16(a));
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned id> SIMDPP_INL
int8_t i_extract(const int8<32>& a)
{
    __m128i val = _mm256_extracti128_si256(a.native(), id / 16);
    return _mm_extract_epi8(val, id % 16);
}
#endif

#if SIMDPP_USE_AVX512BW
template<unsigned id> SIMDPP_INL
int8_t i_extract(const int8<64>& a)
{
    __m128i val = _mm512_extracti32x4_epi32(a.native(), id / 16);
    return _mm_extract_epi8(val, id % 16);
}
#endif

// -----------------------------------------------------------------------------

template<unsigned id> SIMDPP_INL
uint16_t i_extract(const uint16<8>& a)
{
#if SIMDPP_USE_NULL
    return a.el(id);
#elif SIMDPP_USE_SSE2
    return _mm_extract_epi16(a.native(), id);
#elif SIMDPP_USE_NEON
    return vgetq_lane_u16(a.native(), id);
#elif SIMDPP_USE_ALTIVEC
    detail::mem_block<uint16x8> ax(a);
    vec_ste(a.native(), 0, &ax[id]);
    return ax[id];
#elif SIMDPP_USE_MSA
    return __msa_copy_u_h((v8i16) a.native(), id);
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned id> SIMDPP_INL
uint16_t i_extract(const uint16<16>& a)
{
    __m128i val = _mm256_extracti128_si256(a.native(), id / 8);
    return _mm_extract_epi16(val, id % 8);
}
#endif

#if SIMDPP_USE_AVX512BW
template<unsigned id> SIMDPP_INL
uint16_t i_extract(const uint16<32>& a)
{
    __m128i val = _mm512_extracti32x4_epi32(a.native(), id / 8);
    return _mm_extract_epi16(val, id % 8);
}
#endif

// -----------------------------------------------------------------------------

template<unsigned id> SIMDPP_INL
int16_t i_extract(const int16<8>& a)
{
#if SIMDPP_USE_MSA
    return __msa_copy_s_h(a.native(), id);
#else
    return i_extract<id>(uint16x8(a));
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned id> SIMDPP_INL
int16_t i_extract(const int16<16>& a)
{
    __m128i val = _mm256_extracti128_si256(a.native(), id / 8);
    return _mm_extract_epi16(val, id % 8);
}
#endif

#if SIMDPP_USE_AVX512BW
template<unsigned id> SIMDPP_INL
int16_t i_extract(const int16<32>& a)
{
    __m128i val = _mm512_extracti32x4_epi32(a.native(), id / 8);
    return _mm_extract_epi16(val, id % 8);
}
#endif

// -----------------------------------------------------------------------------

template<unsigned id> SIMDPP_INL
uint32_t i_extract(const uint32<4>& a)
{
#if SIMDPP_USE_NULL
    return a.el(id);
#elif SIMDPP_USE_SSE4_1
    return _mm_extract_epi32(a.native(), id);
#elif SIMDPP_USE_SSE2
    // when id==0, move_l is template-specialized and does nothing
    return _mm_cvtsi128_si32(move4_l<id>(a).eval().native());
#elif SIMDPP_USE_NEON
    return vgetq_lane_u32(a.native(), id);
#elif SIMDPP_USE_ALTIVEC
    detail::mem_block<uint32x4> ax(a);
    vec_ste(a.native(), 0, &ax[id]);
    return ax[id];
#elif SIMDPP_USE_MSA
    return __msa_copy_u_w((v4i32) a.native(), id);
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned id> SIMDPP_INL
uint32_t i_extract(const uint32<8>& a)
{
    __m128i val = _mm256_extracti128_si256(a.native(), id / 4);
    return _mm_extract_epi32(val, id % 4);
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned id> SIMDPP_INL
uint32_t i_extract(const uint32<16>& a)
{
    __m128i val = _mm512_extracti32x4_epi32(a.native(), id / 4);
    return _mm_extract_epi32(val, id % 4);
}
#endif

// -----------------------------------------------------------------------------

template<unsigned id> SIMDPP_INL
int32_t i_extract(const int32<4>& a)
{
#if SIMDPP_USE_MSA
    return __msa_copy_s_w(a.native(), id);
#else
    return i_extract<id>(uint32x4(a));
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned id> SIMDPP_INL
int32_t i_extract(const int32<8>& a)
{
    __m128i val = _mm256_extracti128_si256(a.native(), id / 4);
    return _mm_extract_epi32(val, id % 4);
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned id> SIMDPP_INL
int32_t i_extract(const int32<16>& a)
{
    __m128i val = _mm512_extracti32x4_epi32(a.native(), id / 4);
    return _mm_extract_epi32(val, id % 4);
}
#endif

// -----------------------------------------------------------------------------

template<unsigned id> SIMDPP_INL
uint64_t i_extract(const uint64<2>& a)
{
#if SIMDPP_USE_NULL
    return a.el(id);
#elif SIMDPP_USE_SSE4_1
#if SIMDPP_32_BITS
    uint32x4 t = uint32x4(a);
    uint64_t r = i_extract<id*2>(t);
    r |= uint64_t(i_extract<id*2+1>(t)) << 32;
    return r;
#else
    return _mm_extract_epi64(a.native(), id);
#endif
#elif SIMDPP_USE_SSE2
#if SIMDPP_32_BITS
    uint32x4 t = uint32x4(a);
    uint64_t r = 0;
    t = move4_l<id*2>(t); // when id==0, move_l is template-specialized and does nothing
    r = i_extract<0>(t);
    t = move4_l<1>(t);
    r |= uint64_t(i_extract<0>(t)) << 32;
    return r;
#else
    uint64x2 t = a;
    if (id != 0) {
        t = move2_l<id>(t);
    }
    return _mm_cvtsi128_si64(t.native());
#endif
#elif SIMDPP_USE_NEON
    return vgetq_lane_u64(a.native(), id);
#elif SIMDPP_USE_ALTIVEC
    detail::mem_block<uint64x2> ax(a);
    return ax[id];
#elif SIMDPP_USE_MSA
#if SIMDPP_64_BITS
    return __msa_copy_u_d((v2i64) a.native(), id);
#else
    v4i32 a32 = (v4i32) a.native();
    uint64_t lo = __msa_copy_u_w(a32, id*2);
    uint64_t hi = __msa_copy_u_w(a32, id*2+1);
    return lo | (hi << 32);
#endif
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned id> SIMDPP_INL
uint64_t i_extract(const uint64<4>& a)
{
    uint64<2> val = _mm256_extracti128_si256(a.native(), id / 2);
    return i_extract<id % 2>(val);
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned id> SIMDPP_INL
uint64_t i_extract(const uint64<8>& a)
{
    uint64<2> val = _mm512_extracti32x4_epi32(a.native(), id / 2);
    return i_extract<id % 2>(val);
}
#endif

// -----------------------------------------------------------------------------

template<unsigned id> SIMDPP_INL
int64_t i_extract(const int64<2>& a)
{
#if SIMDPP_USE_MSA
#if SIMDPP_64_BITS
    return __msa_copy_s_d(a, id);
#else
    v4i32 a32 = (v4i32) a.native();
    int64_t lo = __msa_copy_s_w(a32, id*2);
    int64_t hi = __msa_copy_s_w(a32, id*2+1);
    return lo | (hi << 32);
#endif
#else
    return i_extract<id>(uint64x2(a));
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned id> SIMDPP_INL
int64_t i_extract(const int64<4>& a)
{
    uint64<2> val = _mm256_extracti128_si256(a.native(), id / 2);
    return i_extract<id % 2>(val);
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned id> SIMDPP_INL
int64_t i_extract(const int64<8>& a)
{
    uint64<2> val = _mm512_extracti32x4_epi32(a.native(), id / 2);
    return i_extract<id % 2>(val);
}
#endif

// -----------------------------------------------------------------------------

template<unsigned id> SIMDPP_INL
float i_extract(const float32<4>& a)
{
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    return a.el(id);
#elif SIMDPP_USE_SSE2
    return bit_cast<float>(i_extract<id>(int32x4(a)));
#elif SIMDPP_USE_NEON
    return vgetq_lane_f32(a.native(), id);
#elif SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    detail::mem_block<float32x4> ax(a);
    return ax[id];
#endif
}

#if SIMDPP_USE_AVX
template<unsigned id> SIMDPP_INL
float i_extract(const float32<8>& a)
{
    __m128 val = _mm256_extractf128_ps(a.native(), id / 4);
    return bit_cast<float>(_mm_extract_epi32(_mm_castps_si128(val), id % 4));
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned id> SIMDPP_INL
float i_extract(const float32<16>& a)
{
    __m128 val = _mm512_extractf32x4_ps(a.native(), id / 4);
    return bit_cast<float>(_mm_extract_epi32(_mm_castps_si128(val), id % 4));
}
#endif

// -----------------------------------------------------------------------------

template<unsigned id> SIMDPP_INL
double i_extract(const float64<2>& a)
{
#if SIMDPP_USE_NULL
    return a.el(id);
#elif SIMDPP_USE_SSE2
    return bit_cast<double>(i_extract<id>(int64x2(a)));
#elif SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    detail::mem_block<float64x2> ax(a);
    return ax[id];
#elif SIMDPP_USE_NEON64
    return vgetq_lane_f64(a.native(), id);
#endif
}

#if SIMDPP_USE_AVX
template<unsigned id> SIMDPP_INL
double i_extract(const float64<4>& a)
{
    __m128d val = _mm256_extractf128_pd(a.native(), id / 2);
    return bit_cast<double>(i_extract<id % 2>((uint64<2>)_mm_castpd_si128(val)));
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned id> SIMDPP_INL
double i_extract(const float64<8>& a)
{
    __m128 val = _mm512_extractf32x4_ps(_mm512_castpd_ps(a.native()), id / 2);
    return bit_cast<double>(i_extract<id % 2>((uint64<2>)_mm_castps_si128(val)));
}
#endif

// -----------------------------------------------------------------------------

template<unsigned id, class V> SIMDPP_INL
typename V::element_type i_extract(const V& a)
{
    typename V::base_vector_type base = a.vec(id / V::base_length);
    return i_extract<id % V::base_length>(base);
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
