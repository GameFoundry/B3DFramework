/*  Copyright (C) 2016  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_STORE_MASKED_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_STORE_MASKED_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/load.h>
#include <simdpp/core/store.h>
#include <simdpp/detail/null/memory.h>
#include <simdpp/detail/align.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

static SIMDPP_INL
void i_store_masked(char* p, const uint32<4>& a, const mask_int32<4>& mask)
{
#if SIMDPP_USE_NULL
    null::store_masked(p, a, mask);
#elif SIMDPP_USE_AVX512VL
    _mm_mask_store_epi32(p, mask.native(), a.native());
#elif SIMDPP_USE_AVX2
    _mm_maskstore_epi32(reinterpret_cast<int*>(p), mask.native(), a.native());
#elif SIMDPP_USE_AVX
    _mm_maskstore_ps(reinterpret_cast<float*>(p), mask.native(),
                     _mm_castsi128_ps(a.native()));
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    uint32<4> b = load(p);
    b = blend(a, b, mask);
    store(p, b);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_store_masked(char* p, const uint32<8>& a, const mask_int32<8>& mask)
{
#if SIMDPP_USE_AVX512VL
    _mm256_mask_store_epi32(p, mask.native(), a.native());
#else
    _mm256_maskstore_epi32(reinterpret_cast<int*>(p), mask.native(), a.native());
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_store_masked(char* p, const uint32<16>& a, const mask_int32<16>& mask)
{
    _mm512_mask_store_epi32(reinterpret_cast<int*>(p), mask.native(), a.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_store_masked(char* p, const uint64<2>& a, const mask_int64<2>& mask)
{
#if SIMDPP_USE_AVX512VL
#if __INTEL_COMPILER
    _mm_mask_store_epi64(reinterpret_cast<__int64*>(p), mask.native(),
                         a.native());
#else
    _mm_mask_store_epi64(reinterpret_cast<long long*>(p), mask.native(),
                         a.native());
#endif
#elif SIMDPP_USE_AVX2
#if __INTEL_COMPILER
    _mm_maskstore_epi64(reinterpret_cast<__int64*>(p), mask.native(), a.native());
#else
    _mm_maskstore_epi64(reinterpret_cast<long long*>(p), mask.native(), a.native());
#endif
#elif SIMDPP_USE_AVX
    _mm_maskstore_pd(reinterpret_cast<double*>(p), mask.native(), _mm_castsi128_pd(a.native()));
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_VSX_207 || SIMDPP_USE_MSA
    uint64<2> b = load(p);
    b = blend(a, b, mask);
    store(p, b);
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC
    null::store_masked(p, a, mask);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_store_masked(char* p, const uint64<4>& a, const mask_int64<4>& mask)
{
#if SIMDPP_USE_AVX512VL
#if __INTEL_COMPILER
    _mm256_mask_store_epi64(reinterpret_cast<__int64*>(p), mask.native(),
                            a.native());
#else
    _mm256_mask_store_epi64(reinterpret_cast<long long*>(p), mask.native(),
                            a.native());
#endif
#else
#if __INTEL_COMPILER
    _mm256_maskstore_epi64(reinterpret_cast<__int64*>(p), mask.native(), a.native());
#else
    _mm256_maskstore_epi64(reinterpret_cast<long long*>(p), mask.native(), a.native());
#endif
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_store_masked(char* p, const uint64<8>& a, const mask_int64<8>& mask)
{
#if __INTEL_COMPILER
    _mm512_mask_store_epi64(reinterpret_cast<__int64*>(p), mask.native(), a.native());
#else
    _mm512_mask_store_epi64(reinterpret_cast<long long*>(p), mask.native(), a.native());
#endif
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_store_masked(char* p, const float32<4>& a, const mask_float32<4>& mask)
{
#if SIMDPP_USE_NULL
    null::store_masked(p, a, mask);
#elif SIMDPP_USE_AVX512VL
    _mm_mask_store_ps(reinterpret_cast<float*>(p), mask.native(), a.native());
#elif SIMDPP_USE_AVX
    _mm_maskstore_ps(reinterpret_cast<float*>(p),
                     _mm_castps_si128(mask.native()), a.native());
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    float32<4> b = load(p);
    b = blend(a, b, mask);
    store(p, b);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
void i_store_masked(char* p, const float32<8>& a, const mask_float32<8>& mask)
{
#if SIMDPP_USE_AVX512VL
    _mm256_mask_store_ps(reinterpret_cast<float*>(p), mask.native(),
                         a.native());
#else
    _mm256_maskstore_ps(reinterpret_cast<float*>(p),
                        _mm256_castps_si256(mask.native()), a.native());
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_store_masked(char* p, const float32<16>& a, const mask_float32<16>& mask)
{
    _mm512_mask_store_ps(reinterpret_cast<float*>(p), mask.native(), a.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_store_masked(char* p, const float64<2>& a, const mask_float64<2>& mask)
{
#if SIMDPP_USE_AVX512VL
    _mm_mask_store_pd(reinterpret_cast<double*>(p), mask.native(), a.native());
#elif SIMDPP_USE_AVX
    _mm_maskstore_pd(reinterpret_cast<double*>(p),
                     _mm_castpd_si128(mask.native()), a.native());
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON64 || SIMDPP_USE_VSX_206 || SIMDPP_USE_MSA
    float64<2> b = load(p);
    b = blend(a, b, mask);
    store(p, b);
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC
    null::store_masked(p, a, mask);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
void i_store_masked(char* p, const float64<4>& a, const mask_float64<4>& mask)
{
#if SIMDPP_USE_AVX512VL
    _mm256_mask_store_pd(reinterpret_cast<double*>(p), mask.native(),
                         a.native());
#else
    _mm256_maskstore_pd(reinterpret_cast<double*>(p),
                        _mm256_castpd_si256(mask.native()), a.native());
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_store_masked(char* p, const float64<8>& a, const mask_float64<8>& mask)
{
    _mm512_mask_store_pd(reinterpret_cast<double*>(p), mask.native(), a.native());
}
#endif

// -----------------------------------------------------------------------------

template<class V, class M>
void i_store_masked(char* p, const V& a, const M& mask)
{
    const unsigned veclen = V::base_vector_type::length_bytes;

    for (unsigned i = 0; i < a.vec_length; ++i) {
        i_store_masked(p, a.vec(i), mask.vec(i));
        p += veclen;
    }
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

