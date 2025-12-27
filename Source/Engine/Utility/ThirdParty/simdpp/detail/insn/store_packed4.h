/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_STORE_PACKED4_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_STORE_PACKED4_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/align.h>
#include <simdpp/detail/insn/mem_pack.h>
#include <simdpp/core/store.h>
#include <simdpp/detail/null/memory.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {


// collect some boilerplate
template<class V> SIMDPP_INL
void v128_store_pack4(char* p, const V& ca, const V& cb, const V& cc, const V& dd);
template<class V> SIMDPP_INL
void v256_store_pack4(char* p, const V& ca, const V& cb, const V& cc, const V& dd);
template<class V> SIMDPP_INL
void v512_store_pack4(char* p, const V& ca, const V& cb, const V& cc, const V& dd);

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_store_packed4(char* p,
                     const uint8x16& a, const uint8x16& b,
                     const uint8x16& c, const uint8x16& d)
{
    p = detail::assume_aligned(p, 16);
#if SIMDPP_USE_NULL
    detail::null::store_packed4(p, a, b, c, d);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    v128_store_pack4(p, a, b, c, d);
#elif SIMDPP_USE_NEON
    uint8x16x4_t t;
    t.val[0] = a.native();
    t.val[1] = b.native();
    t.val[2] = c.native();
    t.val[3] = d.native();
    vst4q_u8(reinterpret_cast<uint8_t*>(p), t);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_store_packed4(char* p,
                     const uint8x32& a, const uint8x32& b,
                     const uint8x32& c, const uint8x32& d)
{
    v256_store_pack4(p, a, b, c, d);
}
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
void i_store_packed4(char* p,
                     const uint8<64>& a, const uint8<64>& b,
                     const uint8<64>& c, const uint8<64>& d)
{
    v512_store_pack4(p, a, b, c, d);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_store_packed4(char* p,
                     const uint16x8& a, const uint16x8& b,
                     const uint16x8& c, const uint16x8& d)
{
    p = detail::assume_aligned(p, 16);
#if SIMDPP_USE_NULL
    detail::null::store_packed4(p, a, b, c, d);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    v128_store_pack4(p, a, b, c, d);
#elif SIMDPP_USE_NEON
    uint16x8x4_t t;
    t.val[0] = a.native();
    t.val[1] = b.native();
    t.val[2] = c.native();
    t.val[3] = d.native();
    vst4q_u16(reinterpret_cast<uint16_t*>(p), t);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_store_packed4(char* p,
                     const uint16x16& a, const uint16x16& b,
                     const uint16x16& c, const uint16x16& d)
{
    v256_store_pack4(p, a, b, c, d);
}
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
void i_store_packed4(char* p,
                     const uint16<32>& a, const uint16<32>& b,
                     const uint16<32>& c, const uint16<32>& d)
{
    v512_store_pack4(p, a, b, c, d);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_store_packed4(char* p,
                     const uint32x4& a, const uint32x4& b,
                     const uint32x4& c, const uint32x4& d)
{
    p = detail::assume_aligned(p, 16);
#if SIMDPP_USE_NULL
    detail::null::store_packed4(p, a, b, c, d);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    v128_store_pack4(p, a, b, c, d);
#elif SIMDPP_USE_NEON
    uint32x4x4_t t;
    t.val[0] = a.native();
    t.val[1] = b.native();
    t.val[2] = c.native();
    t.val[3] = d.native();
    vst4q_u32(reinterpret_cast<uint32_t*>(p), t);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_store_packed4(char* p,
                     const uint32x8& a, const uint32x8& b,
                     const uint32x8& c, const uint32x8& d)
{
    v256_store_pack4(p, a, b, c, d);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_store_packed4(char* p,
                     const uint32<16>& a, const uint32<16>& b,
                     const uint32<16>& c, const uint32<16>& d)
{
    v512_store_pack4(p, a, b, c, d);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_store_packed4(char* p,
                     const uint64x2& a, const uint64x2& b,
                     const uint64x2& c, const uint64x2& d)
{
#if SIMDPP_USE_NEON64
    uint64x2x4_t t;
    t.val[0] = a.native();
    t.val[1] = b.native();
    t.val[2] = c.native();
    t.val[3] = d.native();
    vst4q_u64(reinterpret_cast<uint64_t*>(p), t);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_VSX_207 || SIMDPP_USE_MSA
    v128_store_pack4(p, a, b, c, d);
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC
    detail::null::store_packed4(p, a, b, c, d);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_store_packed4(char* p,
                     const uint64x4& a, const uint64x4& b,
                     const uint64x4& c, const uint64x4& d)
{
    v256_store_pack4(p, a, b, c, d);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_store_packed4(char* p,
                     const uint64<8>& a, const uint64<8>& b,
                     const uint64<8>& c, const uint64<8>& d)
{
    v512_store_pack4(p, a, b, c, d);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_store_packed4(char* p,
                     const float32x4& a, const float32x4& b,
                     const float32x4& c, const float32x4& d)
{
    p = detail::assume_aligned(p, 16);
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    detail::null::store_packed4(p, a, b, c, d);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    v128_store_pack4(p, a, b, c, d);
#elif SIMDPP_USE_NEON
    float32x4x4_t t;
    t.val[0] = a.native();
    t.val[1] = b.native();
    t.val[2] = c.native();
    t.val[3] = d.native();
    vst4q_f32(reinterpret_cast<float*>(p), t);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
void i_store_packed4(char* p,
                     const float32x8& a, const float32x8& b,
                     const float32x8& c, const float32x8& d)
{
    v256_store_pack4(p, a, b, c, d);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_store_packed4(char* p,
                     const float32<16>& a, const float32<16>& b,
                     const float32<16>& c, const float32<16>& d)
{
    v512_store_pack4(p, a, b, c, d);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_store_packed4(char* p,
                     const float64x2& a, const float64x2& b,
                     const float64x2& c, const float64x2& d)
{
    p = detail::assume_aligned(p, 16);
#if SIMDPP_USE_SSE2 || SIMDPP_USE_VSX_206 || SIMDPP_USE_MSA
    v128_store_pack4(p, a, b, c, d);
#elif SIMDPP_USE_NEON64
    float64x2x4_t t;
    t.val[0] = a.native();
    t.val[1] = b.native();
    t.val[2] = c.native();
    t.val[3] = d.native();
    vst4q_f64(reinterpret_cast<double*>(p), t);
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC
    detail::null::store_packed4(p, a, b, c, d);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
void i_store_packed4(char* p,
                     const float64x4& a, const float64x4& b,
                     const float64x4& c, const float64x4& d)
{
    v256_store_pack4(p, a, b, c, d);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_store_packed4(char* p,
                     const float64<8>& a, const float64<8>& b,
                     const float64<8>& c, const float64<8>& d)
{
    v512_store_pack4(p, a, b, c, d);
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
void v128_store_pack4(char* p, const V& ca, const V& cb, const V& cc, const V& dd)
{
    p = detail::assume_aligned(p, 16);
    V a = ca, b = cb, c = cc, d = dd;
    mem_pack4(a, b, c, d);
    i_store(p, a);
    i_store(p + 16, b);
    i_store(p + 32, c);
    i_store(p + 48, d);
}

template<class V> SIMDPP_INL
void v256_store_pack4(char* p, const V& ca, const V& cb, const V& cc, const V& dd)
{
    p = detail::assume_aligned(p, 32);
    V a = ca, b = cb, c = cc, d = dd;
    mem_pack4(a, b, c, d);
    i_store(p, a);
    i_store(p + 32, b);
    i_store(p + 64, c);
    i_store(p + 96, d);
}

template<class V> SIMDPP_INL
void v512_store_pack4(char* p, const V& ca, const V& cb, const V& cc, const V& dd)
{
    p = detail::assume_aligned(p, 64);
    V a = ca, b = cb, c = cc, d = dd;
    mem_pack4(a, b, c, d);
    i_store(p, a);
    i_store(p + 64, b);
    i_store(p + 128, c);
    i_store(p + 192, d);
}

template<class V> SIMDPP_INL
void i_store_packed4(char* p, const V& ca, const V& cb, const V& cc, const V& dd)
{
    const unsigned veclen = V::base_vector_type::length_bytes;
    typename detail::remove_sign<V>::type a = ca, b = cb, c = cc, d = dd;

    p = detail::assume_aligned(p, veclen);
    for (unsigned i = 0; i < V::vec_length; ++i) {
        i_store_packed4(p, a.vec(i), b.vec(i), c.vec(i), d.vec(i));
        p += veclen*4;
    }
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
