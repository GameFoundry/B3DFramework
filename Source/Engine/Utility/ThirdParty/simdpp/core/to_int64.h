/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_TO_INT64_H
#define LIBSIMDPP_SIMDPP_CORE_TO_INT64_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/capabilities.h>
#include <simdpp/detail/insn/conv_extend_to_int64.h>
#include <simdpp/detail/insn/conv_float_to_int64.h>
#include <simdpp/detail/not_implemented.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Converts elements within a vector to 64-bit signed values.

    The conversion rules are as follows:
    32-bit and narrower signed integers are sign-extended to 32 bits.
    32-bit and narrower unsigned integers are zero-extended to 32 bits.
    floating-point numbers are converted to integer values and truncated.
    If floating-point value can not be represented in 64-bit signed integer,
    the behavior is different for different instruction sets.

    SSE specific:
    If the value can not be represented by int64_t, @c 0x8000000000000000
    is returned

    @code
    r0 = (int64_t) a0
    ...
    rN = (int64_t) aN
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
int64<N,expr_empty> to_int64(const int8<N,E>& a)
{
    return detail::insn::i_to_int64(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int64<N,expr_empty> to_int64(const uint8<N,E>& a)
{
    return (int64<N>) detail::insn::i_to_uint64(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int64<N,expr_empty> to_int64(const int16<N,E>& a)
{
    return detail::insn::i_to_int64(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int64<N,expr_empty> to_int64(const uint16<N,E>& a)
{
    return (int64<N>) detail::insn::i_to_uint64(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int64<N,expr_empty> to_int64(const int32<N,E>& a)
{
    return detail::insn::i_to_int64(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int64<N,expr_empty> to_int64(const uint32<N,E>& a)
{
    return (int64<N>) detail::insn::i_to_uint64(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int64<N,expr_empty> to_int64(const int64<N,E>& a)
{
    return a;
}
template<unsigned N, class E> SIMDPP_INL
int64<N,expr_empty> to_int64(const uint64<N,E>& a)
{
    return int64<N>(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int64<N,expr_empty> to_int64(const float32<N,E>& a)
{
#if SIMDPP_HAS_FLOAT32_TO_INT64_CONVERSION
    return detail::insn::i_to_int64(a.eval());
#else
    return SIMDPP_NOT_IMPLEMENTED_TEMPLATE1(E, a);
#endif
}
template<unsigned N, class E> SIMDPP_INL
int64<N,expr_empty> to_int64(const float64<N,E>& a)
{
#if SIMDPP_HAS_FLOAT64_TO_INT64_CONVERSION
    return detail::insn::i_to_int64(a.eval());
#else
    return SIMDPP_NOT_IMPLEMENTED_TEMPLATE1(E, a);
#endif
}

/** Converts elements within a vector to 64-bit unsigned values.

    The conversion rules are as follows:
    32-bit and narrower signed integers are sign-extended to 32 bits.
    32-bit and narrower unsigned integers are zero-extended to 32 bits.
    If floating-point value can not be represented in 64-bit unsigned integer,
    the behavior is different for different instruction sets.

    SSE specific:
    If the value can not be represented by uint64_t, @c 0x8000000000000000
    is returned
    @todo NaN handling

    @code
    r0 = (uint64_t) a0
    ...
    rN = (uint64_t) aN
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
uint64<N,expr_empty> to_uint64(const int8<N,E>& a)
{
    return (uint64<N>) detail::insn::i_to_int64(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint64<N,expr_empty> to_uint64(const uint8<N,E>& a)
{
    return detail::insn::i_to_uint64(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint64<N,expr_empty> to_uint64(const int16<N,E>& a)
{
    return (uint64<N>) detail::insn::i_to_int64(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint64<N,expr_empty> to_uint64(const uint16<N,E>& a)
{
    return detail::insn::i_to_uint64(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint64<N,expr_empty> to_uint64(const int32<N,E>& a)
{
    return (uint64<N>) detail::insn::i_to_int64(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint64<N,expr_empty> to_uint64(const uint32<N,E>& a)
{
    return detail::insn::i_to_uint64(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint64<N,expr_empty> to_uint64(const int64<N,E>& a)
{
    return uint64<N>(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint64<N,expr_empty> to_uint64(const uint64<N,E>& a)
{
    return a.eval();
}
template<unsigned N, class E> SIMDPP_INL
uint64<N,expr_empty> to_uint64(const float32<N,E>& a)
{
#if SIMDPP_HAS_FLOAT32_TO_UINT64_CONVERSION
    return detail::insn::i_to_uint64(a.eval());
#else
    return SIMDPP_NOT_IMPLEMENTED_TEMPLATE1(E, a);
#endif
}
template<unsigned N, class E> SIMDPP_INL
uint64<N,expr_empty> to_uint64(const float64<N,E>& a)
{
#if SIMDPP_HAS_FLOAT64_TO_UINT64_CONVERSION
    return detail::insn::i_to_uint64(a.eval());
#else
    return SIMDPP_NOT_IMPLEMENTED_TEMPLATE1(E, a);
#endif
}


} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

