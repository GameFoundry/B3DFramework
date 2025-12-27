/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_TO_INT8_H
#define LIBSIMDPP_SIMDPP_CORE_TO_INT8_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/conv_shrink_to_int8.h>
#include <simdpp/detail/insn/conv_float_to_int8.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Converts elements within a vector to 8-bit signed values.

    The conversion rules are as follows:
    16-bit and wider integers are truncated.
    floating-point numbers are converted to integer values and truncated.
    If floating-point value can not be represented in 8-bit signed integer,
    the behavior is different for different instruction sets.

    @code
    r0 = (int8_t) a0
    ...
    rN = (int8_t) aN
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
int8<N,expr_empty> to_int8(const int8<N,E>& a)
{
    return a.eval();
}
template<unsigned N, class E> SIMDPP_INL
int8<N,expr_empty> to_int8(const uint8<N,E>& a)
{
    return int8<N>(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int8<N,expr_empty> to_int8(const int16<N,E>& a)
{
    return detail::insn::i_to_uint8(uint16<N>(a.eval()));
}
template<unsigned N, class E> SIMDPP_INL
int8<N,expr_empty> to_int8(const uint16<N,E>& a)
{
    return detail::insn::i_to_uint8(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int8<N,expr_empty> to_int8(const int32<N,E>& a)
{
    return detail::insn::i_to_uint8(uint32<N>(a.eval()));
}
template<unsigned N, class E> SIMDPP_INL
int8<N,expr_empty> to_int8(const uint32<N,E>& a)
{
    return detail::insn::i_to_uint8(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int8<N,expr_empty> to_int8(const int64<N,E>& a)
{
    return detail::insn::i_to_uint8(uint64<N>(a.eval()));
}
template<unsigned N, class E> SIMDPP_INL
int8<N,expr_empty> to_int8(const uint64<N,E>& a)
{
    return detail::insn::i_to_uint8(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int8<N,expr_empty> to_int8(const float32<N,E>& a)
{
    return detail::insn::i_to_int8(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int8<N,expr_empty> to_int8(const float64<N,E>& a)
{
    return detail::insn::i_to_int8(a.eval());
}

/** Converts elements within a vector to 8-bit unsigned values.

    The conversion rules are as follows:
    16-bit and wider integers are truncated.
    If floating-point value can not be represented in 8-bit unsigned integer,
    the behavior is different for different instruction sets.

    @code
    r0 = (uint8_t) a0
    ...
    rN = (uint8_t) aN
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
uint8<N,expr_empty> to_uint8(const int8<N,E>& a)
{
    return uint8<N>(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint8<N,expr_empty> to_uint8(const uint8<N,E>& a)
{
    return a;
}
template<unsigned N, class E> SIMDPP_INL
uint8<N,expr_empty> to_uint8(const int16<N,E>& a)
{
    return detail::insn::i_to_uint8(uint16<N>(a.eval()));
}
template<unsigned N, class E> SIMDPP_INL
uint8<N,expr_empty> to_uint8(const uint16<N,E>& a)
{
    return detail::insn::i_to_uint8(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint8<N,expr_empty> to_uint8(const int32<N,E>& a)
{
    return detail::insn::i_to_uint8(uint32<N>(a.eval()));
}
template<unsigned N, class E> SIMDPP_INL
uint8<N,expr_empty> to_uint8(const uint32<N,E>& a)
{
    return detail::insn::i_to_uint8(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint8<N,expr_empty> to_uint8(const int64<N,E>& a)
{
    return detail::insn::i_to_uint8(uint64<N>(a.eval()));
}
template<unsigned N, class E> SIMDPP_INL
uint8<N,expr_empty> to_uint8(const uint64<N,E>& a)
{
    return detail::insn::i_to_uint8(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint8<N,expr_empty> to_uint8(const float32<N,E>& a)
{
    return detail::insn::i_to_uint8(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint8<N,expr_empty> to_uint8(const float64<N,E>& a)
{
    return detail::insn::i_to_uint8(a.eval());
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

