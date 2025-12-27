/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_STORE_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_STORE_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/null/memory.h>
#include <simdpp/detail/align.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

static SIMDPP_INL
void i_store(char* p, const uint8x16& a)
{
    p = detail::assume_aligned(p, 16);
#if SIMDPP_USE_NULL
    detail::null::store(p, a);
#elif SIMDPP_USE_SSE2
    _mm_store_si128(reinterpret_cast<__m128i*>(p), a.native());
#elif SIMDPP_USE_NEON
    vst1q_u64(reinterpret_cast<uint64_t*>(p), vreinterpretq_u64_u8(a.native()));
#elif SIMDPP_USE_ALTIVEC
    vec_stl(a.native(), 0, reinterpret_cast<uint8_t*>(p));
#elif SIMDPP_USE_MSA
    __msa_st_b((v16i8)a.native(), p, 0);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_store(char* p, const uint8x32& a)
{
    p = detail::assume_aligned(p, 32);
    _mm256_store_si256(reinterpret_cast<__m256i*>(p), a.native());
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL void i_store(char* p, const uint8<64>& a)
{
    p = detail::assume_aligned(p, 64);
    _mm512_store_si512(reinterpret_cast<__m512i*>(p), a.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_store(char* p, const uint16<8>& a)
{
    i_store(p, uint8<16>(a));
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_store(char* p, const uint16<16>& a)
{
    i_store(p, uint8<32>(a));
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL void i_store(char* p, const uint16<32>& a)
{
    i_store(p, uint8<64>(a));
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_store(char* p, const uint32<4>& a)
{
    i_store(p, uint8<16>(a));
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_store(char* p, const uint32<8>& a)
{
    i_store(p, uint8<32>(a));
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_store(char* p, const uint32<16>& a)
{
    p = detail::assume_aligned(p, 64);
    _mm512_store_epi32(p, a.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_store(char* p, const uint64<2>& a)
{
#if SIMDPP_USE_NULL || (SIMDPP_USE_ALTIVEC && !SIMDPP_USE_VSX_207)
    p = detail::assume_aligned(p, 16);
    detail::null::store(p, a);
#else
    i_store(p, uint8<16>(a));
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_store(char* p, const uint64<4>& a)
{
    i_store(p, uint8<32>(a));
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_store(char* p, const uint64<8>& a)
{
    p = detail::assume_aligned(p, 64);
    _mm512_store_epi64(p, a.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_store(char* p, const float32x4& a)
{
    p = detail::assume_aligned(p, 16);
    float* q = reinterpret_cast<float*>(p);
    (void) q;
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    detail::null::store(p, a);
#elif SIMDPP_USE_SSE2
    _mm_store_ps(q, a.native());
#elif SIMDPP_USE_NEON
    vst1q_f32(q, a.native());
#elif SIMDPP_USE_ALTIVEC
    vec_stl(a.native(), 0, q);
#elif SIMDPP_USE_MSA
    __msa_st_w((v4i32) a.native(), q, 0);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
void i_store(char* p, const float32x8& a)
{
    float* q = reinterpret_cast<float*>(p);
    q = detail::assume_aligned(q, 32);
    _mm256_store_ps(q, a.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_store(char* p, const float32<16>& a)
{
    p = detail::assume_aligned(p, 64);
    _mm512_store_ps(p, a.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_store(char* p, const float64x2& a)
{
    p = detail::assume_aligned(p, 16);
    double* q = reinterpret_cast<double*>(p);
    (void) q;
#if SIMDPP_USE_SSE2
    _mm_store_pd(q, a.native());
#elif SIMDPP_USE_NEON64
    vst1q_f64(q, a.native());
#elif SIMDPP_USE_VSX_206
    vec_stl(a.native(), 0, q);
#elif SIMDPP_USE_MSA
    __msa_st_d((v2i64) a.native(), q, 0);
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC
    detail::null::store(p, a);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
void i_store(char* p, const float64x4& a)
{
    p = detail::assume_aligned(p, 32);
    _mm256_store_pd(reinterpret_cast<double*>(p), a.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_store(char* p, const float64<8>& a)
{
    p = detail::assume_aligned(p, 64);
    _mm512_store_pd(p, a.native());
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
void i_store(char* p, const V& ca)
{
    const unsigned veclen = V::base_vector_type::length_bytes;

    typename detail::remove_sign<V>::type a = ca;
    p = detail::assume_aligned(p, veclen);
    for (unsigned i = 0; i < V::vec_length; ++i) {
        i_store(p, a.vec(i));
        p += veclen;
    }
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
