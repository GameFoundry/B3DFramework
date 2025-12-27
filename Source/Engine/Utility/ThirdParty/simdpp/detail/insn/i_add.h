/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_I_ADD_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_I_ADD_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/bit_and.h>
#include <simdpp/detail/null/math.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

static SIMDPP_INL
uint8<16> i_iadd(const uint8<16>& a, const uint8<16>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::add(a, b);
#elif SIMDPP_USE_SSE2
    return _mm_add_epi8(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vaddq_u8(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_add(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return (v16u8) __msa_addv_b((v16i8) a.native(), (v16i8) b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint8<32> i_iadd(const uint8<32>& a, const uint8<32>& b)
{
    return _mm256_add_epi8(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
uint8<64> i_iadd(const uint8<64>& a, const uint8<64>& b)
{
    return _mm512_add_epi8(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint16<8> i_iadd(const uint16<8>& a, const uint16<8>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::add(a, b);
#elif SIMDPP_USE_SSE2
    return _mm_add_epi16(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vaddq_u16(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_add(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return (v8u16) __msa_addv_h((v8i16) a.native(), (v8i16) b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint16<16> i_iadd(const uint16<16>& a, const uint16<16>& b)
{
    return _mm256_add_epi16(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
uint16<32> i_iadd(const uint16<32>& a, const uint16<32>& b)
{
    return _mm512_add_epi16(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint32<4> i_iadd(const uint32<4>& a, const uint32<4>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::add(a, b);
#elif SIMDPP_USE_SSE2
    return _mm_add_epi32(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vaddq_u32(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_add(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return (v4u32) __msa_addv_w((v4i32) a.native(), (v4i32) b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint32<8> i_iadd(const uint32<8>& a, const uint32<8>& b)
{
    return _mm256_add_epi32(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint32<16> i_iadd(const uint32<16>& a, const uint32<16>& b)
{
    return _mm512_add_epi32(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint64<2> i_iadd(const uint64<2>& a, const uint64<2>& b)
{
#if SIMDPP_USE_SSE2
    return _mm_add_epi64(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vaddq_u64(a.native(), b.native());
#elif SIMDPP_USE_VSX_207
    return vec_add(a.native(), b.native());
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC
    return detail::null::add(a, b);
#elif SIMDPP_USE_MSA
    return (v2u64) __msa_addv_d((v2i64) a.native(), (v2i64) b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint64<4> i_iadd(const uint64<4>& a, const uint64<4>& b)
{
    return _mm256_add_epi64(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint64<8> i_iadd(const uint64<8>& a, const uint64<8>& b)
{
    return _mm512_add_epi64(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
V i_iadd(const V& a, const V& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(V, i_iadd, a, b)
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

