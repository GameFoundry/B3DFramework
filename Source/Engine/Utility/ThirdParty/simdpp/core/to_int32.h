/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_TO_INT32_H
#define LIBSIMDPP_SIMDPP_CORE_TO_INT32_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/capabilities.h>
#include <simdpp/detail/insn/conv_extend_to_int32.h>
#include <simdpp/detail/insn/conv_shrink_to_int32.h>
#include <simdpp/detail/insn/conv_float_to_int32.h>
#include <simdpp/detail/not_implemented.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Converts elements within a vector to 32-bit signed values.

    The conversion rules are as follows:
    16-bit and narrower signed integers are sign-extended to 32 bits.
    16-bit and narrower unsigned integers are zero-extended to 32 bits.
    64-bit integers are truncated.
    floating-point numbers are converted to integer values and truncated.
    If floating-point value can not be represented in 32-bit signed integer,
    the behavior is different for different instruction sets.

    SSE specific:
    If the value can not be represented by int32_t, @c 0x80000000 is returned
    TODO: NaN handling

    NEON, ALTIVEC specific:
    If the value can not be represented by int32_t, either @c 0x80000000 or @c
    0x7fffffff is returned depending on the sign of the operand (saturation
    occurs). Conversion of NaNs results in @a 0.

    @code
    r0 = (int32_t) a0
    ...
    rN = (int32_t) aN
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
int32<N,expr_empty> to_int32(const int8<N,E>& a)
{
    return detail::insn::i_to_int32(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int32<N,expr_empty> to_int32(const uint8<N,E>& a)
{
    return (int32<N>) detail::insn::i_to_uint32(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int32<N,expr_empty> to_int32(const int16<N,E>& a)
{
    return detail::insn::i_to_int32(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int32<N,expr_empty> to_int32(const uint16<N,E>& a)
{
    return (int32<N>) detail::insn::i_to_uint32(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int32<N,expr_empty> to_int32(const int32<N,E>& a)
{
    return a;
}
template<unsigned N, class E> SIMDPP_INL
int32<N,expr_empty> to_int32(const uint32<N,E>& a)
{
    return int32<N>(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int32<N,expr_empty> to_int32(const int64<N,E>& a)
{
    return detail::insn::i_to_uint32(uint64<N>(a.eval()));
}
template<unsigned N, class E> SIMDPP_INL
int32<N,expr_empty> to_int32(const uint64<N,E>& a)
{
    return detail::insn::i_to_uint32(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int32<N,expr_empty> to_int32(const float32<N,E>& a)
{
    return detail::insn::i_to_int32(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
int32<N,expr_empty> to_int32(const float64<N,E>& a)
{
    return detail::insn::i_to_int32(a.eval());
}

/** Converts elements within a vector to 32-bit unsigned values.

    The conversion rules are as follows:
    16-bit and narrower signed integers are sign-extended to 32 bits.
    16-bit and narrower unsigned integers are zero-extended to 32 bits.
    64-bit integers are truncated.
    If floating-point value can not be represented in 32-bit unsigned integer,
    the behavior is different for different instruction sets.

    SSE specific:
    If the value can not be represented by uint32_t, @c 0x80000000 is returned
    @todo NaN handling

    NEON VFP specific:
    If the value can not be represented by uint32_t, either @c 0x80000000 or @c
    0x7fffffff is returned depending on the sign of the operand. Conversion of
    NaNs results in @a 0.

    @code
    r0 = (uint32_t) a0
    ...
    rN = (uint32_t) aN
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
uint32<N,expr_empty> to_uint32(const int8<N,E>& a)
{
    return (uint32<N>) detail::insn::i_to_int32(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint32<N,expr_empty> to_uint32(const uint8<N,E>& a)
{
    return detail::insn::i_to_uint32(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint32<N,expr_empty> to_uint32(const int16<N,E>& a)
{
    return (uint32<N>) detail::insn::i_to_int32(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint32<N,expr_empty> to_uint32(const uint16<N,E>& a)
{
    return detail::insn::i_to_uint32(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint32<N,expr_empty> to_uint32(const int32<N,E>& a)
{
    return a.eval();
}
template<unsigned N, class E> SIMDPP_INL
uint32<N,expr_empty> to_uint32(const uint32<N,E>& a)
{
    return a;
}
template<unsigned N, class E> SIMDPP_INL
uint32<N,expr_empty> to_uint32(const int64<N,E>& a)
{
    return detail::insn::i_to_uint32(uint64<N>(a.eval()));
}
template<unsigned N, class E> SIMDPP_INL
uint32<N,expr_empty> to_uint32(const uint64<N,E>& a)
{
    return detail::insn::i_to_uint32(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint32<N,expr_empty> to_uint32(const float32<N,E>& a)
{
    return detail::insn::i_to_uint32(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
uint32<N,expr_empty> to_uint32(const float64<N,E>& a)
{
#if SIMDPP_HAS_FLOAT64_TO_UINT32_CONVERSION
    return detail::insn::i_to_uint32(a.eval());
#else
    return SIMDPP_NOT_IMPLEMENTED_TEMPLATE1(E, a);
#endif
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

