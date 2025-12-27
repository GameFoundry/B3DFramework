/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_I_SUB_SAT_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_I_SUB_SAT_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/null/math.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

static SIMDPP_INL
int8<16> i_sub_sat(const int8<16>& a, const int8<16>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::sub_sat(a, b);
#elif SIMDPP_USE_SSE2
    return _mm_subs_epi8(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vqsubq_s8(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_subs(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_subs_s_b(a.native(), b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int8<32> i_sub_sat(const int8<32>& a, const int8<32>& b)
{
    return _mm256_subs_epi8(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
int8<64> i_sub_sat(const int8<64>& a, const int8<64>& b)
{
    return _mm512_subs_epi8(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
int16<8> i_sub_sat(const int16<8>& a, const int16<8>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::sub_sat(a, b);
#elif SIMDPP_USE_SSE2
    return _mm_subs_epi16(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vqsubq_s16(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_subs(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_subs_s_h(a.native(), b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int16<16> i_sub_sat(const int16<16>& a, const int16<16>& b)
{
    return _mm256_subs_epi16(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
int16<32> i_sub_sat(const int16<32>& a, const int16<32>& b)
{
    return _mm512_subs_epi16(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint8<16> i_sub_sat(const uint8<16>& a, const uint8<16>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::sub_sat(a, b);
#elif SIMDPP_USE_SSE2
    return _mm_subs_epu8(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vqsubq_u8(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_subs(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_subs_u_b(a.native(), b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint8<32> i_sub_sat(const uint8<32>& a, const uint8<32>& b)
{
    return _mm256_subs_epu8(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
uint8<64> i_sub_sat(const uint8<64>& a, const uint8<64>& b)
{
    return _mm512_subs_epu8(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint16<8> i_sub_sat(const uint16<8>& a, const uint16<8>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::sub_sat(a, b);
#elif SIMDPP_USE_SSE2
    return _mm_subs_epu16(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vqsubq_u16(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_subs(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_subs_u_h(a.native(), b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint16<16> i_sub_sat(const uint16<16>& a, const uint16<16>& b)
{
    return _mm256_subs_epu16(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
uint16<32> i_sub_sat(const uint16<32>& a, const uint16<32>& b)
{
    return _mm512_subs_epu16(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
V i_sub_sat(const V& a, const V& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(V, i_sub_sat, a, b)
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

