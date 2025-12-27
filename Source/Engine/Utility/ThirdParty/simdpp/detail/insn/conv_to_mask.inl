/*  Copyright (C) 2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_TO_MASK_INL
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_TO_MASK_INL

#include <simdpp/types.h>
#include <simdpp/detail/insn/conv_to_mask.h>
#include <simdpp/core/cmp_neq.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

static SIMDPP_INL
mask_int8<16> i_to_mask(const uint8<16>& a)
{
#if SIMDPP_USE_NULL
    return cmp_neq(a, (uint8<16>) make_zero());
#elif SIMDPP_USE_AVX512VL
    return _mm_movepi8_mask(a.native());
#else
    return a.native();
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
mask_int8<32> i_to_mask(const uint8<32>& a)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_movepi8_mask(a.native());
#else
    return a.native();
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
mask_int8<64> i_to_mask(const uint8<64>& a)
{
    return _mm512_movepi8_mask(a.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
mask_int16<8> i_to_mask(const uint16<8>& a)
{
#if SIMDPP_USE_NULL
    return cmp_neq(a, (uint16<8>) make_zero());
#elif SIMDPP_USE_AVX512VL
    return _mm_movepi16_mask(a.native());
#else
    return a.native();
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
mask_int16<16> i_to_mask(const uint16<16>& a)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_movepi16_mask(a.native());
#else
    return a.native();
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
mask_int16<32> i_to_mask(const uint16<32>& a)
{
    return _mm512_movepi16_mask(a.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
mask_int32<4> i_to_mask(const uint32<4>& a)
{
#if SIMDPP_USE_NULL
    return cmp_neq(a, (uint32<4>) make_zero());
#elif SIMDPP_USE_AVX512VL
    return _mm_movepi32_mask(a.native());
#else
    return a.native();
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
mask_int32<8> i_to_mask(const uint32<8>& a)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_movepi32_mask(a.native());
#else
    return a.native();
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
mask_int32<16> i_to_mask(const uint32<16>& a)
{
    return _mm512_test_epi32_mask(a.native(), a.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
mask_int64<2> i_to_mask(const uint64<2>& a)
{
#if SIMDPP_USE_AVX512VL
    return _mm_movepi64_mask(a.native());
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_VSX_207 || SIMDPP_USE_MSA
    return a.native();
#else
    return cmp_neq(a, (uint64<2>) make_zero());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
mask_int64<4> i_to_mask(const uint64<4>& a)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_movepi64_mask(a.native());
#else
    return a.native();
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
mask_int64<8> i_to_mask(const uint64<8>& a)
{
    return _mm512_test_epi64_mask(a.native(), a.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
mask_float32<4> i_to_mask(const float32<4>& a)
{
#if SIMDPP_USE_AVX512VL
    __m128i ia = _mm_castps_si128(a.native());
    return _mm_test_epi32_mask(ia, ia);
#elif SIMDPP_USE_NULL || (SIMDPP_USE_NEON && !SIMDPP_USE_NEON_FLT_SP)
    return cmp_neq(a, (float32<4>) make_zero());
#else
    return a.native();
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
mask_float32<8> i_to_mask(const float32<8>& a)
{
#if SIMDPP_USE_AVX512VL
    __m256i ia = _mm256_castps_si256(a.native());
    return _mm256_test_epi32_mask(ia, ia);
#else
    return a.native();
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
mask_float32<16> i_to_mask(const float32<16>& a)
{
    __m512i ia = _mm512_castps_si512(a.native());
    return _mm512_test_epi32_mask(ia, ia);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
mask_float64<2> i_to_mask(const float64<2>& a)
{
#if SIMDPP_USE_AVX512VL
    __m128i ia = _mm_castpd_si128(a.native());
    return _mm_test_epi64_mask(ia, ia);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON64 || SIMDPP_USE_VSX_206 || SIMDPP_USE_MSA
    return a.native();
#else
    return cmp_neq(a, (float64<2>) make_zero());
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
mask_float64<4> i_to_mask(const float64<4>& a)
{
#if SIMDPP_USE_AVX512VL
    __m256i ia = _mm256_castpd_si256(a.native());
    return _mm256_test_epi64_mask(ia, ia);
#else
    return a.native();
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
mask_float64<8> i_to_mask(const float64<8>& a)
{
    __m512i ia = _mm512_castpd_si512(a.native());
    return _mm512_test_epi64_mask(ia, ia);
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
typename V::mask_vector_type i_to_mask(const V& a)
{
    SIMDPP_VEC_ARRAY_IMPL1(typename V::mask_vector_type, i_to_mask, a)
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif // LIBSIMDPP_SIMDPP_DETAIL_TO_MASK_INL
