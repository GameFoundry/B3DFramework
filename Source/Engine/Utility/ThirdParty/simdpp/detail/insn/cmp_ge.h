/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_CMP_GE_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_CMP_GE_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/make_shuffle_bytes_mask.h>
#include <simdpp/detail/null/compare.h>
#include <simdpp/detail/insn/bit_not.h>
#include <simdpp/detail/insn/cmp_lt.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

SIMDPP_INL mask_int8<16> i_cmp_ge(const int8<16>& a, const int8<16>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::cmp_ge(a, b);
#elif SIMDPP_USE_AVX512VL
    return _mm_cmpge_epi8_mask(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vcgeq_s8(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return (v16u8) __msa_cle_s_b(b.native(), a.native());
#else
    return i_bit_not(i_cmp_lt(a, b));
#endif
}

#if SIMDPP_USE_AVX2
SIMDPP_INL mask_int8<32> i_cmp_ge(const int8<32>& a, const int8<32>& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_cmpge_epi8_mask(a.native(), b.native());
#else
    return i_bit_not(i_cmp_lt(a, b));
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL mask_int8<64> i_cmp_ge(const int8<64>& a, const int8<64>& b)
{
    return _mm512_cmpge_epi8_mask(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

SIMDPP_INL mask_int8<16> i_cmp_ge(const uint8<16>& ca, const uint8<16>& cb)
{
    uint8<16> a = ca, b = cb;
#if SIMDPP_USE_NULL
    return detail::null::cmp_ge(a, b);
#elif SIMDPP_USE_AVX512VL
    return _mm_cmpge_epu8_mask(a.native(), b.native());
#elif SIMDPP_USE_XOP && !SIMDPP_WORKAROUND_XOP_COM
    return _mm_comge_epu8(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vcgeq_u8(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return (v16u8) __msa_cle_u_b(b.native(), a.native());
#else
    return i_bit_not(i_cmp_lt(a, b));
#endif
}

#if SIMDPP_USE_AVX2
SIMDPP_INL mask_int8<32> i_cmp_ge(const uint8<32>& a, const uint8<32>& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_cmpge_epu8_mask(a.native(), b.native());
#else
    return i_bit_not(i_cmp_lt(a, b));
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL mask_int8<64> i_cmp_ge(const uint8<64>& a, const uint8<64>& b)
{
    return _mm512_cmpge_epu8_mask(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

SIMDPP_INL mask_int16<8> i_cmp_ge(const int16<8>& a, const int16<8>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::cmp_ge(a, b);
#elif SIMDPP_USE_AVX512VL
    return _mm_cmpge_epi16_mask(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vcgeq_s16(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return (v8u16) __msa_cle_s_h(b.native(), a.native());
#else
    return i_bit_not(i_cmp_lt(a, b));
#endif
}

#if SIMDPP_USE_AVX2
SIMDPP_INL mask_int16<16> i_cmp_ge(const int16<16>& a, const int16<16>& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_cmpge_epi16_mask(a.native(), b.native());
#else
    return i_bit_not(i_cmp_lt(a, b));
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL mask_int16<32> i_cmp_ge(const int16<32>& a, const int16<32>& b)
{
    return _mm512_cmpge_epi16_mask(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

SIMDPP_INL mask_int16<8> i_cmp_ge(const uint16<8>& ca, const uint16<8>& cb)
{
    uint16<8> a = ca, b = cb;
#if SIMDPP_USE_NULL
    return detail::null::cmp_ge(a, b);
#elif SIMDPP_USE_AVX512VL
    return _mm_cmpge_epu16_mask(a.native(), b.native());
#elif SIMDPP_USE_XOP && !SIMDPP_WORKAROUND_XOP_COM
    return _mm_comge_epu16(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vcgeq_u16(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return (v8u16) __msa_cle_u_h(b.native(), a.native());
#else
    return i_bit_not(i_cmp_lt(a, b));
#endif
}

#if SIMDPP_USE_AVX2
SIMDPP_INL mask_int16<16> i_cmp_ge(const uint16<16>& a, const uint16<16>& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_cmpge_epu16_mask(a.native(), b.native());
#else
    return i_bit_not(i_cmp_lt(a, b));
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL mask_int16<32> i_cmp_ge(const uint16<32>& a, const uint16<32>& b)
{
    return _mm512_cmpge_epu16_mask(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

SIMDPP_INL mask_int32<4> i_cmp_ge(const int32<4>& a, const int32<4>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::cmp_ge(a, b);
#elif SIMDPP_USE_AVX512VL
    return _mm_cmpge_epi32_mask(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vcgeq_s32(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return (v4u32) __msa_cle_s_w(b.native(), a.native());
#else
    return i_bit_not(i_cmp_lt(a, b));
#endif
}

#if SIMDPP_USE_AVX2
SIMDPP_INL mask_int32<8> i_cmp_ge(const int32<8>& a, const int32<8>& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_cmpge_epi32_mask(a.native(), b.native());
#else
    return i_bit_not(i_cmp_lt(a, b));
#endif
}
#endif

#if SIMDPP_USE_AVX512F
SIMDPP_INL mask_int32<16> i_cmp_ge(const int32<16>& a, const int32<16>& b)
{
    return _mm512_cmpge_epi32_mask(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

SIMDPP_INL mask_int32<4> i_cmp_ge(const uint32<4>& ca, const uint32<4>& cb)
{
    uint32<4> a = ca, b = cb;
#if SIMDPP_USE_NULL
    return detail::null::cmp_ge(a, b);
#elif SIMDPP_USE_AVX512VL
    return _mm_cmpge_epu32_mask(a.native(), b.native());
#elif SIMDPP_USE_XOP && !SIMDPP_WORKAROUND_XOP_COM
    return _mm_comge_epu32(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vcgeq_u32(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return (v4u32) __msa_cle_u_w(b.native(), a.native());
#else
    return i_bit_not(i_cmp_lt(a, b));
#endif
}

#if SIMDPP_USE_AVX2
SIMDPP_INL mask_int32<8> i_cmp_ge(const uint32<8>& a, const uint32<8>& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_cmpge_epu32_mask(a.native(), b.native());
#else
    return i_bit_not(i_cmp_lt(a, b));
#endif
}
#endif

#if SIMDPP_USE_AVX512F
SIMDPP_INL mask_int32<16> i_cmp_ge(const uint32<16>& a, const uint32<16>& b)
{
    // FIXME: BUG: GCC does not have _mm512_cmpge_epu32_mask
    return _mm512_cmp_epu32_mask(a.native(), b.native(), _MM_CMPINT_NLT);
}
#endif

// -----------------------------------------------------------------------------

SIMDPP_INL mask_int64<2> i_cmp_ge(const int64<2>& a, const int64<2>& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm_cmpge_epi64_mask(a.native(), b.native());
#elif SIMDPP_USE_XOP && !SIMDPP_WORKAROUND_XOP_COM
    return _mm_comge_epi64(a.native(), b.native());
#elif SIMDPP_USE_NEON64
    return vcgeq_s64(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return (v2u64) __msa_cle_s_d(b.native(), a.native());
#elif SIMDPP_USE_NULL
    return detail::null::cmp_ge(a, b);
#else
    return i_bit_not(i_cmp_lt(a, b));
#endif
}

#if SIMDPP_USE_AVX2
SIMDPP_INL mask_int64<4> i_cmp_ge(const int64<4>& a, const int64<4>& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_cmpge_epi64_mask(a.native(), b.native());
#else
    return i_bit_not(i_cmp_lt(a, b));
#endif
}
#endif

#if SIMDPP_USE_AVX512F
SIMDPP_INL mask_int64<8> i_cmp_ge(const int64<8>& a, const int64<8>& b)
{
    // GCC does not have _mm512_cmpge_epi64_mask
    return _mm512_cmp_epi64_mask(a.native(), b.native(), _MM_CMPINT_NLT);
}
#endif

// -----------------------------------------------------------------------------

SIMDPP_INL mask_int64<2> i_cmp_ge(const uint64<2>& a, const uint64<2>& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm_cmpge_epu64_mask(a.native(), b.native());
#elif SIMDPP_USE_XOP && !SIMDPP_WORKAROUND_XOP_COM
    return _mm_comge_epu64(a.native(), b.native());
#elif SIMDPP_USE_NEON64
    return vcgeq_u64(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return (v2u64) __msa_cle_u_d(b.native(), a.native());
#elif SIMDPP_USE_NULL
    return detail::null::cmp_ge(a, b);
#else
    return i_bit_not(i_cmp_lt(a, b));
#endif
}

#if SIMDPP_USE_AVX2
SIMDPP_INL mask_int64<4> i_cmp_ge(const uint64<4>& a, const uint64<4>& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_cmpge_epu64_mask(a.native(), b.native());
#else
    return i_bit_not(i_cmp_lt(a, b));
#endif
}
#endif

#if SIMDPP_USE_AVX512F
SIMDPP_INL mask_int64<8> i_cmp_ge(const uint64<8>& a, const uint64<8>& b)
{
    return _mm512_cmp_epu64_mask(a.native(), b.native(), _MM_CMPINT_NLT);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
mask_float32<4> i_cmp_ge(const float32<4>& a, const float32<4>& b)
{
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    return detail::null::cmp_ge(a, b);
#elif SIMDPP_USE_AVX512VL
    return _mm_cmp_ps_mask(a.native(), b.native(), _CMP_GE_OQ);
#elif SIMDPP_USE_AVX
    return _mm_cmp_ps(a.native(), b.native(), _CMP_GE_OQ);
#elif SIMDPP_USE_SSE2
    return _mm_cmpge_ps(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vreinterpretq_f32_u32(vcgeq_f32(a.native(), b.native()));
#elif SIMDPP_USE_ALTIVEC
    return vec_cmpge(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return (v4f32) __msa_fcle_w(b.native(), a.native());
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
mask_float32<8> i_cmp_ge(const float32<8>& a, const float32<8>& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_cmp_ps_mask(a.native(), b.native(), _CMP_GE_OQ);
#else
    return _mm256_cmp_ps(a.native(), b.native(), _CMP_GE_OQ);
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
mask_float32<16> i_cmp_ge(const float32<16>& a, const float32<16>& b)
{
    return _mm512_cmp_ps_mask(a.native(), b.native(), _CMP_GE_OQ);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
mask_float64<2> i_cmp_ge(const float64<2>& a, const float64<2>& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm_cmp_pd_mask(a.native(), b.native(), _CMP_GE_OQ);
#elif SIMDPP_USE_AVX
    return _mm_cmp_pd(a.native(), b.native(), _CMP_GE_OQ);
#elif SIMDPP_USE_SSE2
    return _mm_cmpge_pd(a.native(), b.native());
#elif SIMDPP_USE_NEON64
    return vreinterpretq_f64_u64(vcgeq_f64(a.native(), b.native()));
#elif SIMDPP_USE_VSX_206
    return (__vector double) vec_cmpge(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return (v2f64) __msa_fcle_d(b.native(), a.native());
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC
    return detail::null::cmp_ge(a, b);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
mask_float64<4> i_cmp_ge(const float64<4>& a, const float64<4>& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_cmp_pd_mask(a.native(), b.native(), _CMP_GE_OQ);
#else
    return _mm256_cmp_pd(a.native(), b.native(), _CMP_GE_OQ);
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
mask_float64<8> i_cmp_ge(const float64<8>& a, const float64<8>& b)
{
    return _mm512_cmp_pd_mask(a.native(), b.native(), _CMP_GE_OQ);
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
typename V::mask_vector_type i_cmp_ge(const V& a, const V& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(typename V::mask_vector_type, i_cmp_ge, a, b);
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

