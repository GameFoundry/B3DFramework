/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_LOAD_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_LOAD_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/types/traits.h>
#include <simdpp/detail/align.h>
#include <simdpp/detail/insn/mem_unpack.h>
#include <simdpp/core/transpose.h>
#include <simdpp/detail/null/memory.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

static SIMDPP_INL
void i_load(uint8x16& a, const char* p)
{
    p = detail::assume_aligned(p, 16);
#if SIMDPP_USE_NULL
    detail::null::load(a, p);
#elif SIMDPP_USE_SSE2
    a = _mm_load_si128(reinterpret_cast<const __m128i*>(p));
#elif SIMDPP_USE_NEON
    a = vreinterpretq_u8_u64(vld1q_u64(reinterpret_cast<const uint64_t*>(p)));
#elif SIMDPP_USE_ALTIVEC
    a = vec_ld(0, reinterpret_cast<const uint8_t*>(p));
#elif SIMDPP_USE_MSA
    a = (v16u8) __msa_ld_b(p, 0);
#endif
}

static SIMDPP_INL
void i_load(uint16x8& a, const char* p) { uint8x16 r; i_load(r, p); a = r;  }
static SIMDPP_INL
void i_load(uint32x4& a, const char* p) { uint8x16 r; i_load(r, p); a = r;  }

static SIMDPP_INL
void i_load(uint64x2& a, const char* p)
{
#if SIMDPP_USE_NULL || (SIMDPP_USE_ALTIVEC && !SIMDPP_USE_VSX_207)
    p = detail::assume_aligned(p, 16);
    detail::null::load(a, p);
#else
    uint8x16 r; i_load(r, p); a = r;
#endif
}

static SIMDPP_INL
void i_load(float32x4& a, const char* p)
{
    p = detail::assume_aligned(p, 16);
    const float* q = reinterpret_cast<const float*>(p);
    (void) q;
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    detail::null::load(a, p);
#elif SIMDPP_USE_SSE2
    a = _mm_load_ps(q);
#elif SIMDPP_USE_NEON
    a = vld1q_f32(q);
#elif SIMDPP_USE_ALTIVEC
    a = vec_ld(0, q);
#elif SIMDPP_USE_MSA
    a = (v4f32) __msa_ld_w(q, 0);
#endif
}

static SIMDPP_INL
void i_load(float64x2& a, const char* p)
{
    p = detail::assume_aligned(p, 16);
    const double* q = reinterpret_cast<const double*>(p);
    (void) q;
#if SIMDPP_USE_SSE2
    a = _mm_load_pd(q);
#elif SIMDPP_USE_NEON64
    a = vld1q_f64(q);
#elif SIMDPP_USE_VSX_206
    a = vec_ld(0, reinterpret_cast<const __vector double*>(q));
#elif SIMDPP_USE_MSA
    a = (v2f64) __msa_ld_d(q, 0);
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC || SIMDPP_USE_NEON32
    detail::null::load(a, p);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_load(uint8x32& a,  const char* p)
{
    a = _mm256_load_si256(reinterpret_cast<const __m256i*>(p));
}
static SIMDPP_INL
void i_load(uint16x16& a, const char* p)
{
    a = _mm256_load_si256(reinterpret_cast<const __m256i*>(p));
}
static SIMDPP_INL
void i_load(uint32x8& a,  const char* p)
{
    a = _mm256_load_si256(reinterpret_cast<const __m256i*>(p));
}
static SIMDPP_INL
void i_load(uint64x4& a,  const char* p)
{
    a = _mm256_load_si256(reinterpret_cast<const __m256i*>(p));
}
#endif
#if SIMDPP_USE_AVX
static SIMDPP_INL
void i_load(float32x8& a, const char* p)
{
    a = _mm256_load_ps(reinterpret_cast<const float*>(p));
}
static SIMDPP_INL
void i_load(float64x4& a, const char* p)
{
    a = _mm256_load_pd(reinterpret_cast<const double*>(p));
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL void i_load(uint8<64>& a, const char* p)
{
    a = _mm512_load_epi32(p);
}
SIMDPP_INL void i_load(uint16<32>& a, const char* p)
{
    a = _mm512_load_epi32(p);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_load(uint32<16>& a,  const char* p)
{
    a = _mm512_load_epi32(p);
}
static SIMDPP_INL
void i_load(uint64<8>& a,  const char* p)
{
    a = _mm512_load_epi64(p);
}
static SIMDPP_INL
void i_load(float32<16>& a, const char* p)
{
    a = _mm512_load_ps(reinterpret_cast<const float*>(p));
}
static SIMDPP_INL
void i_load(float64<8>& a, const char* p)
{
    a = _mm512_load_pd(reinterpret_cast<const double*>(p));
}
#endif

template<class V> SIMDPP_INL
void i_load(V& a, const char* p)
{
    const unsigned veclen = V::base_vector_type::length_bytes;

    for (unsigned i = 0; i < V::vec_length; ++i) {
        i_load(a.vec(i), p);
        p += veclen;
    }
}

template<class V> SIMDPP_INL
V i_load_any(const char* p)
{
    typename detail::remove_sign<V>::type r;
    i_load(r, p);
    return V(r);
}

} // namespace insn

template<class V> SIMDPP_INL
void construct_eval(V& v, const expr_vec_load& e)
{
    v = insn::i_load_any<V>(e.a);
}

} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

