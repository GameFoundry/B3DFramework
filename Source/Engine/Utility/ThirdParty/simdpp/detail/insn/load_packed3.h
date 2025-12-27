/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_LOAD_PACKED3_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_LOAD_PACKED3_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/align.h>
#include <simdpp/detail/insn/mem_unpack.h>
#include <simdpp/core/load.h>
#include <simdpp/core/transpose.h>
#include <simdpp/detail/null/memory.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {


// collect some boilerplate
template<class V> SIMDPP_INL
void v128_load_packed3(V& a, V& b, V& c, const char* p);
template<class V> SIMDPP_INL
void v256_load_packed3(V& a, V& b, V& c, const char* p);
template<class V> SIMDPP_INL
void v512_load_packed3(V& a, V& b, V& c, const char* p);

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_load_packed3(uint8x16& a, uint8x16& b, uint8x16& c, const char* p)
{
    p = detail::assume_aligned(p, 16);
#if SIMDPP_USE_NULL
    detail::null::load_packed3(a, b, c, p);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    v128_load_packed3(a, b, c, p);
#elif SIMDPP_USE_NEON
    auto r = vld3q_u8(reinterpret_cast<const uint8_t*>(p));
    a = r.val[0];
    b = r.val[1];
    c = r.val[2];
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_load_packed3(uint8x32& a, uint8x32& b, uint8x32& c, const char* p)
{
    v256_load_packed3(a, b, c, p);
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL void i_load_packed3(uint8<64>& a, uint8<64>& b, uint8<64>& c, const char* p)
{
    v512_load_packed3(a, b, c, p);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_load_packed3(uint16x8& a, uint16x8& b, uint16x8& c,
                               const char* p)
{
    p = detail::assume_aligned(p, 16);
#if SIMDPP_USE_NULL
    detail::null::load_packed3(a, b, c, p);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    v128_load_packed3(a, b, c, p);
#elif SIMDPP_USE_NEON
    auto r = vld3q_u16(reinterpret_cast<const uint16_t*>(p));
    a = r.val[0];
    b = r.val[1];
    c = r.val[2];
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_load_packed3(uint16x16& a, uint16x16& b, uint16x16& c,
                               const char* p)
{
    v256_load_packed3(a, b, c, p);
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL void i_load_packed3(uint16<32>& a, uint16<32>& b, uint16<32>& c,
                               const char* p)
{
    v512_load_packed3(a, b, c, p);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_load_packed3(uint32x4& a, uint32x4& b, uint32x4&c, const char* p)
{
    p = detail::assume_aligned(p, 16);
#if SIMDPP_USE_NULL
    detail::null::load_packed3(a, b, c, p);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    v128_load_packed3(a, b, c, p);
#elif SIMDPP_USE_NEON
    auto r = vld3q_u32(reinterpret_cast<const uint32_t*>(p));
    a = r.val[0];
    b = r.val[1];
    c = r.val[2];
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_load_packed3(uint32x8& a, uint32x8& b, uint32x8& c, const char* p)
{
    v256_load_packed3(a, b, c, p);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_load_packed3(uint32<16>& a, uint32<16>& b, uint32<16>& c, const char* p)
{
    v512_load_packed3(a, b, c, p);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_load_packed3(uint64x2& a, uint64x2& b, uint64x2& c, const char* p)
{
    p = detail::assume_aligned(p, 16);
#if SIMDPP_USE_SSE2 || SIMDPP_USE_VSX_207 || SIMDPP_USE_MSA
    v128_load_packed3(a, b, c, p);
#elif SIMDPP_USE_NEON64
    auto r = vld3q_u64(reinterpret_cast<const uint64_t*>(p));
    a = r.val[0];
    b = r.val[1];
    c = r.val[2];
#elif SIMDPP_USE_NEON32
    uint64x2 a0, b0, c0;
    a0 = load(p);
    b0 = load(p+16);
    c0 = load(p+32);

    int64x1_t al, bl, cl, ah, bh, ch;
    al = vget_low_u64(a0.native());
    ah = vget_high_u64(a0.native());
    bl = vget_low_u64(b0.native());
    bh = vget_high_u64(b0.native());
    cl = vget_low_u64(c0.native());
    ch = vget_high_u64(c0.native());
    a = vcombine_u64(al, bh);
    b = vcombine_u64(ah, cl);
    c = vcombine_u64(bl, ch);
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC
    detail::null::load_packed3(a, b, c, p);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_load_packed3(uint64x4& a, uint64x4& b, uint64x4& c, const char* p)
{
    v256_load_packed3(a, b, c, p);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_load_packed3(uint64<8>& a, uint64<8>& b, uint64<8>& c,
                               const char* p)
{
    v512_load_packed3(a, b, c, p);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_load_packed3(float32x4& a, float32x4& b, float32x4& c, const char* p)
{
    p = detail::assume_aligned(p, 16);
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    detail::null::load_packed3(a, b, c, p);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    v128_load_packed3(a, b, c, p);
#elif SIMDPP_USE_NEON
    auto r = vld3q_f32(reinterpret_cast<const float*>(p));
    a = r.val[0];
    b = r.val[1];
    c = r.val[2];
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
void i_load_packed3(float32x8& a, float32x8& b, float32x8& c, const char* p)
{
    v256_load_packed3(a, b, c, p);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_load_packed3(float32<16>& a, float32<16>& b, float32<16>& c,
                               const char* p)
{
    v512_load_packed3(a, b, c, p);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_load_packed3(float64x2& a, float64x2& b, float64x2& c, const char* p)
{
    p = detail::assume_aligned(p, 16);
#if SIMDPP_USE_NEON64
    auto r = vld3q_f64(reinterpret_cast<const double*>(p));
    a = r.val[0];
    b = r.val[1];
    c = r.val[2];
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_VSX_206 || SIMDPP_USE_MSA
    v128_load_packed3(a, b, c, p);
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC
    detail::null::load_packed3(a, b, c, p);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
void i_load_packed3(float64x4& a, float64x4& b, float64x4& c,
                               const char* p)
{
    v256_load_packed3(a, b, c, p);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_load_packed3(float64<8>& a, float64<8>& b, float64<8>& c,
                               const char* p)
{
    v512_load_packed3(a, b, c, p);
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
void v128_load_packed3(V& a, V& b, V& c, const char* p)
{
    p = detail::assume_aligned(p, 16);
    a = load(p);
    b = load(p + 16);
    c = load(p + 32);
    mem_unpack3(a, b, c);
}

template<class V> SIMDPP_INL
void v256_load_packed3(V& a, V& b, V& c, const char* p)
{
    p = detail::assume_aligned(p, 32);
    a = load(p);
    b = load(p + 32);
    c = load(p + 64);
    mem_unpack3(a, b, c);
}

template<class V> SIMDPP_INL
void v512_load_packed3(V& a, V& b, V& c, const char* p)
{
    p = detail::assume_aligned(p, 64);
    a = load(p);
    b = load(p + 64);
    c = load(p + 128);
    mem_unpack3(a, b, c);
}


template<class V> SIMDPP_INL
void i_load_packed3(V& a, V& b, V& c, const char* p)
{
    const unsigned veclen = V::base_vector_type::length_bytes;

    p = detail::assume_aligned(p, veclen);
    for (unsigned i = 0; i < V::vec_length; ++i) {
        i_load_packed3(a.vec(i), b.vec(i), c.vec(i), p);
        p += veclen*3;
    }
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

