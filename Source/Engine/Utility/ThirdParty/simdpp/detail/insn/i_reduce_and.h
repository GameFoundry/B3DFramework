/*  Copyright (C) 2016  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_I_REDUCE_AND_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_I_REDUCE_AND_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/bit_and.h>
#include <simdpp/core/extract.h>
#include <simdpp/core/move_l.h>
#include <simdpp/core/make_uint.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

static SIMDPP_INL
uint8_t i_reduce_and(const uint8x16& a)
{
#if SIMDPP_USE_NULL
    uint8_t r = a.el(0);
    for (unsigned i = 0; i < a.length; i++) {
        r &= a.el(i);
    }
    return r;
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    uint8x16 r = bit_and(a, move16_l<8>(a));
    r = bit_and(r, move16_l<4>(r));
    r = bit_and(r, move16_l<2>(r));
    r = bit_and(r, move16_l<1>(r));
    return extract<0>(r);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint8_t i_reduce_and(const uint8x32& a)
{
    uint8x16 r = detail::extract128<0>(a);
    r = bit_and(r, detail::extract128<1>(a));
    return i_reduce_and(r);
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint8_t i_reduce_and(const uint8<64>& a)
{
    uint8<32> r = detail::extract256<0>(a);
    r = bit_and(r, detail::extract256<1>(a));
    return i_reduce_and(r);
}
#endif

template<unsigned N>
SIMDPP_INL uint8_t i_reduce_and(const uint8<N>& a)
{
#if SIMDPP_USE_NULL
    uint8_t r = 0xff;
    for (unsigned j = 0; j < a.vec_length; ++j) {
        for (unsigned i = 0; i < a.base_length; i++) {
            r &= a.vec(j).el(i);
        }
    }
    return r;
#else
    uint8v r = a.vec(0);
    for (unsigned j = 1; j < a.vec_length; ++j) {
        r = bit_and(r, a.vec(j));
    }
    return i_reduce_and(r);
#endif
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint16_t i_reduce_and(const uint16x8& a)
{
#if SIMDPP_USE_NULL
    uint16_t r = a.el(0);
    for (unsigned i = 0; i < a.length; i++) {
        r &= a.el(i);
    }
    return r;
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    uint16x8 r = bit_and(a, move8_l<4>(a));
    r = bit_and(r, move8_l<2>(r));
    r = bit_and(r, move8_l<1>(r));
    return extract<0>(r);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint16_t i_reduce_and(const uint16x16& a)
{
    uint16x8 r = detail::extract128<0>(a);
    r = bit_and(r, detail::extract128<1>(a));
    return i_reduce_and(r);
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint16_t i_reduce_and(const uint16<32>& a)
{
    uint16<16> r = detail::extract256<0>(a);
    r = bit_and(r, detail::extract256<1>(a));
    return i_reduce_and(r);
}
#endif

template<unsigned N>
SIMDPP_INL uint16_t i_reduce_and(const uint16<N>& a)
{
#if SIMDPP_USE_NULL
    uint16_t r = 0xffff;
    for (unsigned j = 0; j < a.vec_length; ++j) {
        for (unsigned i = 0; i < a.base_length; i++) {
            r &= a.vec(j).el(i);
        }
    }
    return r;
#else
    uint16v r = a.vec(0);
    for (unsigned j = 1; j < a.vec_length; ++j) {
        r = bit_and(r, a.vec(j));
    }
    return i_reduce_and(r);
#endif
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint32_t i_reduce_and(const uint32x4& a)
{
#if SIMDPP_USE_NULL
    uint32_t r = a.el(0);
    for (unsigned i = 0; i < a.length; i++) {
        r &= a.el(i);
    }
    return r;
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    uint32x4 r = bit_and(a, move4_l<2>(a));
    r = bit_and(r, move4_l<1>(r));
    return extract<0>(r);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint32_t i_reduce_and(const uint32x8& a)
{
    uint32x4 r = detail::extract128<0>(a);
    r = bit_and(r, detail::extract128<1>(a));
    r = bit_and(r, move4_l<2>(r));
    r = bit_and(r, move4_l<1>(r));
    return extract<0>(r);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint32_t i_reduce_and(const uint32<16>& a)
{
    return i_reduce_and(bit_and(extract256<0>(a), extract256<1>(a)));
}
#endif

template<unsigned N>
SIMDPP_INL uint32_t i_reduce_and(const uint32<N>& a)
{
#if SIMDPP_USE_NULL
    uint32_t r = 0xffffffff;
    for (unsigned j = 0; j < a.vec_length; ++j) {
        for (unsigned i = 0; i < a.base_length; i++) {
            r &= a.vec(j).el(i);
        }
    }
    return r;
#else
    uint32v r = a.vec(0);
    for (unsigned j = 1; j < a.vec_length; ++j) {
        r = bit_and(r, a.vec(j));
    }
    return i_reduce_and(r);
#endif
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint64_t i_reduce_and(const uint64x2& a)
{
#if SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_VSX_207 || SIMDPP_USE_MSA
    uint64x2 r = bit_and(a, move2_l<1>(a));
    return extract<0>(r);
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC
    uint64_t r = a.el(0);
    for (unsigned i = 0; i < a.length; i++) {
        r &= a.el(i);
    }
    return r;
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint64_t i_reduce_and(const uint64x4& a)
{
    uint64x2 r = detail::extract128<0>(a);
    r = bit_and(r, detail::extract128<1>(a));
    r = bit_and(r, move2_l<1>(r));
    return extract<0>(r);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint64_t i_reduce_and(const uint64<8>& a)
{
    return i_reduce_and(bit_and(extract256<0>(a), extract256<1>(a)));
}
#endif

template<unsigned N>
SIMDPP_INL uint64_t i_reduce_and(const uint64<N>& a)
{
#if SIMDPP_USE_NULL || (SIMDPP_USE_ALTIVEC && !SIMDPP_USE_VSX_207)
    uint64_t r = 0xffffffffffffffff;
    for (unsigned j = 0; j < a.vec_length; ++j) {
        for (unsigned i = 0; i < a.base_length; i++) {
            r &= a.vec(j).el(i);
        }
    }
    return r;
#else
    uint64v r = a.vec(0);
    for (unsigned j = 1; j < a.vec_length; ++j) {
        r = bit_and(r, a.vec(j));
    }
    return i_reduce_and(r);
#endif
}

// -----------------------------------------------------------------------------

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

