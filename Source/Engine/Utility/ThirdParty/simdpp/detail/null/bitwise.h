/*  Copyright (C) 2011-2012  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_DETAIL_NULL_BITWISE_H
#define LIBSIMDPP_DETAIL_NULL_BITWISE_H
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/cast.h>
#include <simdpp/detail/null/mask.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace null {

/* *_vm variants accept a vector as the first argument and a mask as the second
   *_mm variants accept masks as both arguments
*/

template<class V> SIMDPP_INL
V bit_and(const V& a, const V& b)
{
    V r;
    using E = typename V::element_type;
    using U = typename V::uint_element_type;
    for (unsigned i = 0; i < V::length; i++) {
        U a1 = bit_cast<U, E>(a.el(i));
        U b1 = bit_cast<U, E>(b.el(i));
        r.el(i) = bit_cast<E, U>(a1 & b1);
    }
    return r;
}

template<class V, class M> SIMDPP_INL
V bit_and_vm(const V& a, const M& m)
{
    V r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = m.el(i) ? a.el(i) : 0;
    }
    return r;
}

template<class M> SIMDPP_INL
M bit_and_mm(const M& a, const M& b)
{
    M r;
    for (unsigned i = 0; i < M::length; i++) {
        r.el(i) = a.el(i) & b.el(i);
    }
    return r;
}


template<class V> SIMDPP_INL
V bit_andnot(const V& a, const V& b)
{
    V r;
    using E = typename V::element_type;
    using U = typename V::uint_element_type;
    for (unsigned i = 0; i < V::length; i++) {
        U a1 = bit_cast<U, E>(a.el(i));
        U b1 = bit_cast<U, E>(b.el(i));
        r.el(i) = bit_cast<E, U>(a1 & ~b1);
    }
    return r;
}

template<class V, class M> SIMDPP_INL
V bit_andnot_vm(const V& a, const M& m)
{
    V r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = m.el(i) ? 0 : a.el(i);
    }
    return r;
}

template<class M> SIMDPP_INL
M bit_andnot_mm(const M& a, const M& b)
{
    M r;
    for (unsigned i = 0; i < M::length; i++) {
        r.el(i) = a.el(i) & (b.el(i) ^ 1);
    }
    return r;
}


template<class V> SIMDPP_INL
V bit_or(const V& a, const V& b)
{
    V r;
    using E = typename V::element_type;
    using U = typename V::uint_element_type;
    for (unsigned i = 0; i < V::length; i++) {
        U a1 = bit_cast<U, E>(a.el(i));
        U b1 = bit_cast<U, E>(b.el(i));
        r.el(i) = bit_cast<E, U>(a1 | b1);
    }
    return r;
}

template<class M> SIMDPP_INL
M bit_or_mm(const M& a, const M& b)
{
    M r;
    for (unsigned i = 0; i < M::length; i++) {
        r.el(i) = a.el(i) | b.el(i);
    }
    return r;
}

template<class V> SIMDPP_INL
V bit_xor(const V& a, const V& b)
{
    V r;
    using E = typename V::element_type;
    using U = typename V::uint_element_type;
    for (unsigned i = 0; i < V::length; i++) {
        U a1 = bit_cast<U, E>(a.el(i));
        U b1 = bit_cast<U, E>(b.el(i));
        r.el(i) = bit_cast<E, U>(a1 ^ b1);
    }
    return r;
}

template<class M> SIMDPP_INL
M bit_xor_mm(const M& a, const M& b)
{
    M r;
    for (unsigned i = 0; i < M::length; i++) {
        r.el(i) = a.el(i) ^ b.el(i);
    }
    return r;
}

template<class M> SIMDPP_INL
M bit_not_mm(const M& a)
{
    M r;
    for (unsigned i = 0; i < M::length; i++) {
        r.el(i) = a.el(i) ^ 1;
    }
    return r;
}

template<class V> SIMDPP_INL
bool test_bits_any(const V& a)
{
    using U = typename V::uint_element_type;
    U r = 0;
    for (unsigned i = 0; i < a.length; ++i) {
        r |= bit_cast<U>(a.el(i));
    }
    return r != 0;
}

static SIMDPP_INL
uint8_t el_popcnt8(uint8_t v)
{
    uint8_t m55 = 0x55;
    uint8_t m33 = 0x33;
    uint8_t m0f = 0x0f;
    v = v - ((v >> 1) & m55);
    v = (v & m33) + ((v >> 2) & m33);
    v = (v + (v >> 4)) & m0f;
    return v;
}

static SIMDPP_INL
uint16_t el_popcnt16(uint16_t v)
{
    uint16_t m55 = 0x5555;
    uint16_t m33 = 0x3333;
    uint16_t m0f = 0x0f0f;
    v = v - ((v >> 1) & m55);
    v = (v & m33) + ((v >> 2) & m33);
    v = (v + (v >> 4)) & m0f;
    v = (v + (v >> 8));
    v = v & 0x00ff;
    return v;
}

static SIMDPP_INL
uint32_t el_popcnt32(uint32_t v)
{
    uint32_t m55 = 0x55555555;
    uint32_t m33 = 0x33333333;
    uint32_t m0f = 0x0f0f0f0f;
    v = v - ((v >> 1) & m55);
    v = (v & m33) + ((v >> 2) & m33);
    v = (v + (v >> 4)) & m0f;
    // rather than doing 2 adds + 2 shifts we can do 1 mul + 1 shift
    v = (v * 0x01010101) >> 24;
    return v;
}

static SIMDPP_INL
uint64_t el_popcnt64(uint64_t v)
{
    uint64_t m55 = 0x5555555555555555;
    uint64_t m33 = 0x3333333333333333;
    uint64_t m0f = 0x0f0f0f0f0f0f0f0f;
    v = v - ((v >> 1) & m55);
    v = (v & m33) + ((v >> 2) & m33);
    v = (v + (v >> 4)) & m0f;
    // rather than doing 3 adds + 3 shifts we can do 1 mul + 1 shift
    v = (v * 0x0101010101010101) >> 56;
    return v;
}

} // namespace null
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
#endif
