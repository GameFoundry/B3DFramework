/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_STORE_PACKED2_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_STORE_PACKED2_H

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
void v128_store_pack2(char* p, const V& ca, const V& cb);
template<class V> SIMDPP_INL
void v256_store_pack2(char* p, const V& ca, const V& cb);
template<class V> SIMDPP_INL
void v512_store_pack2(char* p, const V& ca, const V& cb);

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_store_packed2(char* p, const uint8x16& a, const uint8x16& b)
{
    p = detail::assume_aligned(p, 16);
#if SIMDPP_USE_NULL
    detail::null::store_packed2(p, a, b);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    v128_store_pack2(p, a, b);
#elif SIMDPP_USE_NEON
    uint8x16x2_t t;
    t.val[0] = a.native();
    t.val[1] = b.native();
    vst2q_u8(reinterpret_cast<uint8_t*>(p), t);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_store_packed2(char* p, const uint8x32& a, const uint8x32& b)
{
    v256_store_pack2(p, a, b);
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL void i_store_packed2(char* p, const uint8<64>& a, const uint8<64>& b)
{
    v512_store_pack2(p, a, b);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_store_packed2(char* p, const uint16x8& a, const uint16x8& b)
{
    p = detail::assume_aligned(p, 16);
#if SIMDPP_USE_NULL
    detail::null::store_packed2(p, a, b);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    v128_store_pack2(p, a, b);
#elif SIMDPP_USE_NEON
    uint16x8x2_t t;
    t.val[0] = a.native();
    t.val[1] = b.native();
    vst2q_u16(reinterpret_cast<uint16_t*>(p), t);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_store_packed2(char* p, const uint16x16& a, const uint16x16& b)
{
    v256_store_pack2(p, a, b);
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL void i_store_packed2(char* p, const uint16<32>& a, const uint16<32>& b)
{
    v512_store_pack2(p, a, b);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_store_packed2(char* p, const uint32x4& a, const uint32x4& b)
{
    p = detail::assume_aligned(p, 16);
#if SIMDPP_USE_NULL
    detail::null::store_packed2(p, a, b);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    v128_store_pack2(p, a, b);
#elif SIMDPP_USE_NEON
    uint32x4x2_t t;
    t.val[0] = a.native();
    t.val[1] = b.native();
    vst2q_u32(reinterpret_cast<uint32_t*>(p), t);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_store_packed2(char* p, const uint32x8& a, const uint32x8& b)
{
    v256_store_pack2(p, a, b);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_store_packed2(char* p, const uint32<16>& a, const uint32<16>& b)
{
    v512_store_pack2(p, a, b);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_store_packed2(char* p, const uint64x2& a, const uint64x2& b)
{
#if SIMDPP_USE_NEON64
    uint64x2x2_t t;
    t.val[0] = a.native();
    t.val[1] = b.native();
    vst2q_u64(reinterpret_cast<uint64_t*>(p), t);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_VSX_207 || SIMDPP_USE_MSA
    v128_store_pack2(p, a, b);
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC
    detail::null::store_packed2(p, a, b);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_store_packed2(char* p, const uint64x4& a, const uint64x4& b)
{
    v256_store_pack2(p, a, b);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_store_packed2(char* p, const uint64<8>& a, const uint64<8>& b)
{
    v512_store_pack2(p, a, b);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_store_packed2(char* p, const float32x4& a, const float32x4& b)
{
    p = detail::assume_aligned(p, 16);
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    detail::null::store_packed2(p, a, b);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    v128_store_pack2(p, a, b);
#elif SIMDPP_USE_NEON
    float32x4x2_t t;
    t.val[0] = a.native();
    t.val[1] = b.native();
    vst2q_f32(reinterpret_cast<float*>(p), t);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
void i_store_packed2(char* p, const float32x8& a, const float32x8& b)
{
    v256_store_pack2(p, a, b);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_store_packed2(char* p, const float32<16>& a, const float32<16>& b)
{
    v512_store_pack2(p, a, b);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_store_packed2(char* p, const float64x2& a, const float64x2& b)
{
#if SIMDPP_USE_NEON64
    float64x2x2_t t;
    t.val[0] = a.native();
    t.val[1] = b.native();
    vst2q_f64(reinterpret_cast<double*>(p), t);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_VSX_206 || SIMDPP_USE_MSA
    v128_store_pack2(p, a, b);
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC
    detail::null::store_packed2(p, a, b);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
void i_store_packed2(char* p, const float64x4& a, const float64x4& b)
{
    v256_store_pack2(p, a, b);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_store_packed2(char* p, const float64<8>& a, const float64<8>& b)
{
    v512_store_pack2(p, a, b);
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
void v128_store_pack2(char* p, const V& ca, const V& cb)
{
    p = detail::assume_aligned(p, 32);
    V a = ca, b = cb;
    mem_pack2(a, b);
    i_store(p, a);
    i_store(p + 16, b);
}

template<class V> SIMDPP_INL
void v256_store_pack2(char* p, const V& ca, const V& cb)
{
    p = detail::assume_aligned(p, 32);
    V a = ca, b = cb;
    mem_pack2(a, b);
    i_store(p, a);
    i_store(p + 32, b);
}

template<class V> SIMDPP_INL
void v512_store_pack2(char* p, const V& ca, const V& cb)
{
    p = detail::assume_aligned(p, 32);
    V a = ca, b = cb;
    mem_pack2(a, b);
    i_store(p, a);
    i_store(p + 64, b);
}


template<class V> SIMDPP_INL
void i_store_packed2(char* p, const V& ca, const V& cb)
{
    const unsigned veclen = V::base_vector_type::length_bytes;
    typename detail::remove_sign<V>::type a = ca, b = cb;

    p = detail::assume_aligned(p, veclen);
    for (unsigned i = 0; i < V::vec_length; ++i) {
        i_store_packed2(p, a.vec(i), b.vec(i));
        p += veclen*2;
    }
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
