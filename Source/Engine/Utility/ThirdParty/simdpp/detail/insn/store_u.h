/*  Copyright (C) 2015

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_STORE_U_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_STORE_U_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/null/memory.h>
#include <simdpp/detail/align.h>
#include <simdpp/core/cast.h>

namespace simdpp {
#ifndef SIMDPP_DOXYGEN
namespace SIMDPP_ARCH_NAMESPACE {
#endif
namespace detail {
namespace insn {

static SIMDPP_INL
void i_store_u(char* p, const uint8<16>& a)
{
#if SIMDPP_USE_NULL
    detail::null::store(p, a);
#elif SIMDPP_USE_SSE2
    _mm_storeu_si128(reinterpret_cast<__m128i*>(p), a.native());
#elif SIMDPP_USE_NEON
    vst1q_u8(reinterpret_cast<uint8_t*>(p), a.native());
#elif SIMDPP_USE_VSX_206
    uint8_t* q = reinterpret_cast<uint8_t*>(p);
    vec_vsx_st(a.native(), 0, q);
#elif SIMDPP_USE_ALTIVEC
    // From https://web.archive.org/web/20110305043420/http://developer.apple.com/hardwaredrivers/ve/alignment.html
    uint8_t* q = reinterpret_cast<uint8_t*>(p);
    __vector uint8_t msq, lsq, edges;
    __vector uint8_t edge_align, align;
    msq = vec_ld(0, q);                         // most significant quadword
    lsq = vec_ld(15, q);                        // least significant quadword
    // The address offset is 15 to take into account storing into aligned
    // addresses. If 16 were used, then when q is 16-byte aligned we would
    // access the next second 16-byte block, which could be on different page
    // and inaccessible.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
    edge_align = vec_lvsl(0, q);                // permute map to extract edges
    edges = vec_perm(lsq, msq, edge_align);     // extract the edges
    align = vec_lvsr(0, q);                     // permute map to misalign data
#pragma GCC diagnostic pop
    msq = vec_perm(edges, a.native(), align);   // misalign the data (msq)
    lsq = vec_perm(a.native(), edges, align);   // misalign the data (lsq)
    vec_st(lsq, 15, q);                         // Store the lsq part first
    vec_st(msq, 0, q);                          // Store the msq part
#elif SIMDPP_USE_MSA
    __msa_st_b((v16i8) a.native(), p, 0);
#endif
}

static SIMDPP_INL
void i_store_u(char* p, const uint16<8>& a)
{
#if SIMDPP_USE_NEON
    vst1q_u16(reinterpret_cast<uint16_t*>(p), a.native());
#elif SIMDPP_USE_MSA
    __msa_st_h((v8i16) a.native(), p, 0);
#else
    i_store_u(p, uint8<16>(a));
#endif
}

static SIMDPP_INL
void i_store_u(char* p, const uint32<4>& a)
{
#if SIMDPP_USE_NEON
    vst1q_u32(reinterpret_cast<uint32_t*>(p), a.native());
#elif SIMDPP_USE_VSX_206
    vec_vsx_st(a.native(), 0, reinterpret_cast<__vector uint32_t*>(p));
#elif SIMDPP_USE_MSA
    __msa_st_w((v4i32) a.native(), p, 0);
#else
    i_store_u(p, uint8<16>(a));
#endif
}

static SIMDPP_INL
void i_store_u(char* p, const uint64<2>& a)
{
#if SIMDPP_USE_NEON
    vst1q_u64(reinterpret_cast<uint64_t*>(p), a.native());
#elif SIMDPP_USE_VSX_207
    vec_vsx_st((__vector uint32_t) a.native(), 0,
               reinterpret_cast<__vector uint32_t*>(p));
#elif SIMDPP_USE_MSA
    __msa_st_d((v2i64) a.native(), p, 0);
#else
    i_store_u(p, uint8<16>(a));
#endif
}

static SIMDPP_INL
void i_store_u(char* p, const float32x4& a)
{
    float* q = reinterpret_cast<float*>(p);
    (void) q;
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    detail::null::store(p, a);
#elif SIMDPP_USE_SSE2
    _mm_storeu_ps(q, a.native());
#elif SIMDPP_USE_NEON
    vst1q_f32(q, a.native());
#elif SIMDPP_USE_VSX_206
    vec_vsx_st(a.native(), 0, q);
#elif SIMDPP_USE_ALTIVEC
    uint32x4 b = bit_cast<uint32x4>(a.eval());
    i_store_u(reinterpret_cast<char*>(q), b);
#elif SIMDPP_USE_MSA
    __msa_st_w((v4i32) a.native(), q, 0);
#endif
}

static SIMDPP_INL
void i_store_u(char* p, const float64x2& a)
{
    double* q = reinterpret_cast<double*>(p);
    (void) q;
#if SIMDPP_USE_SSE2
    _mm_storeu_pd(q, a.native());
#elif SIMDPP_USE_NEON64
    vst1q_f64(q, a.native());
#elif SIMDPP_USE_VSX_206
    vec_vsx_st(a.native(), 0, q);
#elif SIMDPP_USE_MSA
    __msa_st_d((v2i64) a.native(), q, 0);
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC
    detail::null::store(p, a);
#endif
}


#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_store_u(char* p, const uint8<32>& a)
{
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(p), a.native());
}

static SIMDPP_INL
void i_store_u(char* p, const uint16<16>& a)
{
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(p), a.native());
}

static SIMDPP_INL
void i_store_u(char* p, const uint32<8>& a)
{
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(p), a.native());
}

static SIMDPP_INL
void i_store_u(char* p, const uint64<4>& a)
{
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(p), a.native());
}
#endif

#if SIMDPP_USE_AVX
static SIMDPP_INL
void i_store_u(char* p, const float32x8& a)
{
    _mm256_storeu_ps(reinterpret_cast<float*>(p), a.native());
}

static SIMDPP_INL
void i_store_u(char* p, const float64x4& a)
{
    _mm256_storeu_pd(reinterpret_cast<double*>(p), a.native());
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL void i_store_u(char* p, const uint8<64>& a)
{
    _mm512_storeu_si512(reinterpret_cast<__m512i*>(p), a.native());
}

SIMDPP_INL void i_store_u(char* p, const uint16<32>& a)
{
    _mm512_storeu_si512(reinterpret_cast<__m512i*>(p), a.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_store_u(char* p, const uint32<16>& a)
{
    _mm512_storeu_si512(reinterpret_cast<__m512i*>(p), a.native());
}

static SIMDPP_INL
void i_store_u(char* p, const uint64<8>& a)
{
    _mm512_storeu_si512(reinterpret_cast<__m512i*>(p), a.native());
}

static SIMDPP_INL
void i_store_u(char* p, const float32<16>& a)
{
    _mm512_storeu_ps(p, a.native());
}

static SIMDPP_INL
void i_store_u(char* p, const float64<8>& a)
{
    _mm512_storeu_pd(p, a.native());
}
#endif

template<class V> SIMDPP_INL
void v_store_u(char* p, const V& a)
{
    const unsigned veclen = V::base_vector_type::length_bytes;

    for (unsigned i = 0; i < V::vec_length; ++i) {
        i_store_u(p, a.vec(i));
        p += veclen;
    }
}

} // namespace insn
} // namespace detail
#ifndef SIMDPP_DOXYGEN
} // namespace SIMDPP_ARCH_NAMESPACE
#endif
} // namespace simdpp

#endif

