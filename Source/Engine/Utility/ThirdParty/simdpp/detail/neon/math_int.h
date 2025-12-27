/*  Copyright (C) 2012  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_DETAIL_NEON_MATH_INT_H
#define LIBSIMDPP_DETAIL_NEON_MATH_INT_H
#if SIMDPP_USE_NEON

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace neon {

/** Multiplies the values of two int8x16 vectors and returns the low
    part of the multiplication

    @code
    r0 = low(a0 * b0)
    ...
    r15 = low(a15 * b15)
    @endcode
*/
static SIMDPP_INL
uint8x16 mul_lo(const uint8x16& a, const uint8x16& b)
{
    return vmulq_u8(a.native(), b.native());
}

/** Multiplies the first 8 values of two signed int8x16 vectors and expands the
    results to 16 bits.

    @code
    r0 = a0 * b0
    ...
    r7 = a7 * b7
    @endcode
*/
static SIMDPP_INL
int16x8 mull_lo(const int8x16& a, const int8x16& b)
{
    return vmull_s8(vget_low_s8(a.native()), vget_low_s8(b.native()));
}

/** Multiplies the first 8 values of two unsigned int8x16 vectors and expands the
    results to 16 bits.

    @code
    r0 = a0 * b0
    ...
    r7 = a7 * b7
    @endcode
*/
static SIMDPP_INL
uint16x8 mull_lo(const uint8x16& a, const uint8x16& b)
{
    return vmull_u8(vget_low_u8(a.native()), vget_low_u8(b.native()));
}

/** Multiplies the last 8 values of two signed int8x16 vectors and expands the
    results to 16 bits.

    @code
    r0 = a8 * b8
    ...
    r7 = a15 * b15
    @endcode
*/
static SIMDPP_INL
int16x8 mull_hi(const int8x16& a, const int8x16& b)
{
    return vmull_s8(vget_high_s8(a.native()), vget_high_s8(b.native()));
}


/** Multiplies the last 8 values of two unsigned int8x16 vectors and expands
    the results to 16 bits.

    @code
    r0 = a8 * b8
    ...
    r7 = a15 * b15
    @endcode
*/
static SIMDPP_INL
uint16x8 mull_hi(const uint8x16& a, const uint8x16& b)
{
    return vmull_u8(vget_high_u8(a.native()), vget_high_u8(b.native()));
}

} // namespace neon
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
#endif
