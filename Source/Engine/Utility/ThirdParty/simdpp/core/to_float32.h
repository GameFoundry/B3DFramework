/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_TO_FLOAT32_H
#define LIBSIMDPP_SIMDPP_CORE_TO_FLOAT32_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/capabilities.h>
#include <simdpp/detail/insn/conv_any_to_float32.h>
#include <simdpp/detail/not_implemented.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Converts elements within a vector to 32-bit floating-point values.

    SSE specific:

    If only inexact conversion can be performed, the current rounding mode is
    used.

    NEON, ALTIVEC specific:

    If only inexact conversion can be performed, round to nearest mode is used.

    @code
    r0 = (float) a0
    ...
    rN = (float) aN
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
float32<N,expr_empty> to_float32(const int8<N,E>& a)
{
    return detail::insn::i_to_float32(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
float32<N,expr_empty> to_float32(const uint8<N,E>& a)
{
    return detail::insn::i_to_float32(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
float32<N,expr_empty> to_float32(const int16<N,E>& a)
{
    return detail::insn::i_to_float32(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
float32<N,expr_empty> to_float32(const uint16<N,E>& a)
{
    return detail::insn::i_to_float32(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
float32<N,expr_empty> to_float32(const int32<N,E>& a)
{
    return detail::insn::i_to_float32(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
float32<N,expr_empty> to_float32(const uint32<N,E>& a)
{
    return detail::insn::i_to_float32(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
float32<N,expr_empty> to_float32(const int64<N,E>& a)
{
#if SIMDPP_HAS_INT64_TO_FLOAT32_CONVERSION
    return detail::insn::i_to_float32(a.eval());
#else
    return SIMDPP_NOT_IMPLEMENTED_TEMPLATE1(E, a);
#endif
}
template<unsigned N, class E> SIMDPP_INL
float32<N,expr_empty> to_float32(const uint64<N,E>& a)
{
#if SIMDPP_HAS_UINT64_TO_FLOAT32_CONVERSION
    return detail::insn::i_to_float32(a.eval());
#else
    return SIMDPP_NOT_IMPLEMENTED_TEMPLATE1(E, a);
#endif
}
template<unsigned N, class E> SIMDPP_INL
float32<N,expr_empty> to_float32(const float32<N,E>& a)
{
    return a;
}
template<unsigned N, class E> SIMDPP_INL
float32<N,expr_empty> to_float32(const float64<N,E>& a)
{
    return detail::insn::i_to_float32(a.eval());
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

