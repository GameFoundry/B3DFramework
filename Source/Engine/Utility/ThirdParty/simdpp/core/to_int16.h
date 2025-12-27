/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_TO_INT16_H
#define LIBSIMDPP_SIMDPP_CORE_TO_INT16_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/conv_extend_to_int16.h>
#include <simdpp/detail/insn/conv_shrink_to_int16.h>
#include <simdpp/detail/insn/conv_float_to_int16.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {


/** Converts elements within a vector to 16-bit signed values.

    The conversion rules are as follows:
    8-bit signed integers are sign-extended to 16 bits.
    8-bit unsigned integers are zero-extended to 16 bits.
    32-bit and wider integers are truncated.
    floating-point numbers are converted to integer values and truncated.
    If floating-point value can not be represented in 16-bit signed integer,
    the behavior is different for different instruction sets.

    @code
    r0 = (int16_t) a0
    ...
    rN = (int16_t) aN
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
int16<N,expr_empty> to_int16(const int8<N,E>& a)
{
    return detail::insn::i_to_int16(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int16<N,expr_empty> to_int16(const uint8<N,E>& a)
{
    return (int16<N>) detail::insn::i_to_uint16(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int16<N,expr_empty> to_int16(const int16<N,E>& a)
{
    return a.eval();
}
template<unsigned N, class E> SIMDPP_INL
int16<N,expr_empty> to_int16(const uint16<N,E>& a)
{
    return int16<N>(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int16<N,expr_empty> to_int16(const int32<N,E>& a)
{
    return detail::insn::i_to_uint16(uint32<N>(a.eval()));
}
template<unsigned N, class E> SIMDPP_INL
int16<N,expr_empty> to_int16(const uint32<N,E>& a)
{
    return detail::insn::i_to_uint16(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int16<N,expr_empty> to_int16(const int64<N,E>& a)
{
    return detail::insn::i_to_uint16(uint64<N>(a.eval()));
}
template<unsigned N, class E> SIMDPP_INL
int16<N,expr_empty> to_int16(const uint64<N,E>& a)
{
    return detail::insn::i_to_uint16(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int16<N,expr_empty> to_int16(const float32<N,E>& a)
{
    return detail::insn::i_to_int16(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int16<N,expr_empty> to_int16(const float64<N,E>& a)
{
    return detail::insn::i_to_int16(a.eval());
}

/** Converts elements within a vector to 16-bit unsigned values.

    The conversion rules are as follows:
    8-bit signed integers are sign-extended to 16 bits.
    8-bit unsigned integers are zero-extended to 16 bits.
    32-bit and wider integers are truncated.
    If floating-point value can not be represented in 16-bit unsigned integer,
    the behavior is different for different instruction sets.

    @code
    r0 = (uint16_t) a0
    ...
    rN = (uint16_t) aN
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
uint16<N,expr_empty> to_uint16(const int8<N,E>& a)
{
    return (uint16<N>) detail::insn::i_to_int16(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint16<N,expr_empty> to_uint16(const uint8<N,E>& a)
{
    return detail::insn::i_to_uint16(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint16<N,expr_empty> to_uint16(const int16<N,E>& a)
{
    return uint16<N>(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint16<N,expr_empty> to_uint16(const uint16<N,E>& a)
{
    return a.eval();
}
template<unsigned N, class E> SIMDPP_INL
uint16<N,expr_empty> to_uint16(const int32<N,E>& a)
{
    return detail::insn::i_to_uint16(uint32<N>(a.eval()));
}
template<unsigned N, class E> SIMDPP_INL
uint16<N,expr_empty> to_uint16(const uint32<N,E>& a)
{
    return detail::insn::i_to_uint16(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint16<N,expr_empty> to_uint16(const int64<N,E>& a)
{
    return detail::insn::i_to_uint16(uint64<N>(a.eval()));
}
template<unsigned N, class E> SIMDPP_INL
uint16<N,expr_empty> to_uint16(const uint64<N,E>& a)
{
    return detail::insn::i_to_uint16(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint16<N,expr_empty> to_uint16(const float32<N,E>& a)
{
    return detail::insn::i_to_uint16(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint16<N,expr_empty> to_uint16(const float64<N,E>& a)
{
    return detail::insn::i_to_uint16(a.eval());
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

