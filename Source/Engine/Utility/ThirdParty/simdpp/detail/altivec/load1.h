/*  Copyright (C) 2013  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_ALTIVEC_LOAD1_H
#define LIBSIMDPP_ALTIVEC_LOAD1_H
#if SIMDPP_USE_ALTIVEC

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace altivec {

/** Loads a single element from unaligned memory location and places it to the
    first position in the vector. The contents of the rest of the vector are
    undefined.

    @code
    a.vec(0) = *p
    @endcode

    @icost{ALTIVEC, 2}
*/
static SIMDPP_INL
uint8x16 load1_u(uint8x16& a, const uint8_t* p)
{
    a = vec_lde(0, p);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
    __vector uint8_t perm = vec_lvsl(0, p);
#pragma GCC diagnostic pop
    a = (__vector uint8_t) vec_perm(a.native(), a.native(), perm);
    return a;
}

static SIMDPP_INL
uint16x8 load1_u(uint16x8& a, const uint16_t* p)
{
    __vector uint16_t r = vec_lde(0, p);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
    __vector uint8_t perm = vec_lvsl(0, p);
#pragma GCC diagnostic pop
    a = (__vector uint16_t) vec_perm((__vector uint8_t)r,
                                     (__vector uint8_t)r, perm);
    return a;
}

static SIMDPP_INL
uint32x4 load1_u(uint32x4& a, const uint32_t* p)
{
    __vector uint32_t r = vec_lde(0, p);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
    __vector uint8_t perm = vec_lvsl(0, p);
#pragma GCC diagnostic pop
    a = (__vector uint32_t) vec_perm((__vector uint8_t)r,
                                     (__vector uint8_t)r, perm);
    return a;
}

static SIMDPP_INL
float32x4 load1_u(float32x4& a, const float* p)
{
    __vector float r = vec_lde(0, p);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
    __vector uint8_t perm = vec_lvsl(0, p);
#pragma GCC diagnostic pop
    a = (__vector float) vec_perm((__vector uint8_t)r,
                                  (__vector uint8_t)r, perm);
    return a;
}

/** Loads a single element from a memory location and places it to the vector.
    The position of the element is determined by the last 4 address @a p bits.
    The contents of the rest of the vector are undefined.

    @code
    i = (p % 15) / sizeof(element)
    a[i] = *p
    @endcode
*/
static SIMDPP_INL
uint8x16 load1(uint8x16& a, const uint8_t* p)
{
    a = vec_lde(0, p);
    return a;
}

static SIMDPP_INL
uint16x8 load1(uint16x8& a, const uint16_t* p)
{
    a = vec_lde(0, p);
    return a;
}

static SIMDPP_INL
uint32x4 load1(uint32x4& a, const uint32_t* p)
{
    a = vec_lde(0, p);
    return a;
}

static SIMDPP_INL
float32x4 load1(float32x4& a, const float* p)
{
    a = vec_lde(0, p);
    return a;
}

} // namespace altivec
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
#endif
