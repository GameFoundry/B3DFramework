/*  Copyright (C) 2011-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMD_CORE_EXTRACT_BITS_H
#define LIBSIMDPP_SIMD_CORE_EXTRACT_BITS_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/extract_bits.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Extracts a bit from each byte of each element of a vector containing 8-bit
    elements.

    This operation is only sensible if each byte within the vector is either
    0x00 or 0xff.

    @code
    r = ((a[0] & 0x??) ? 0x01 : 0) |
        ((a[1] & 0x??) ? 0x02 : 0) |
        ...
        ((a[15] & 0x??) ? 0x80 : 0)
    @endcode
*/
SIMDPP_INL uint16_t extract_bits_any(const uint8<16>& a)
{
    return detail::insn::i_extract_bits_any(a);
}
SIMDPP_INL uint32_t extract_bits_any(const uint8<32>& a)
{
    return detail::insn::i_extract_bits_any(a);
}

/** Extracts specific bit from each byte of each element of a int8x16 vector.

    @code
    r = (a[0] & 0x80 >> 7) | (a[1] & 0x80 >> 6) | ...  | (a[15] & 0x80 << 8)
    @endcode
*/
template<unsigned id> SIMDPP_INL
uint16_t extract_bits(const uint8<16>& a)
{
    static_assert(id < 8, "index out of bounds");
    return detail::insn::i_extract_bits<id>(a);
}
template<unsigned id> SIMDPP_INL
uint32_t extract_bits(const uint8<32>& a)
{
    static_assert(id < 8, "index out of bounds");
    return detail::insn::i_extract_bits<id>(a);
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif


