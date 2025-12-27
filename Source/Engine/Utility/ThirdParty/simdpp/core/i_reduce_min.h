/*  Copyright (C) 2016  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_I_REDUCE_MIN_H
#define LIBSIMDPP_SIMDPP_CORE_I_REDUCE_MIN_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/i_reduce_min.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {


/** Computes the minimum of the elements in the vector

    @code
    r0 = min(a0, a1, a2, ...)
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
int8_t reduce_min(const int8<N,E>& a)
{
    return detail::insn::i_reduce_min(a.eval());
}

template<unsigned N, class E> SIMDPP_INL
uint8_t reduce_min(const uint8<N,E>& a)
{
    return detail::insn::i_reduce_min(a.eval());
}

template<unsigned N, class E> SIMDPP_INL
int16_t reduce_min(const int16<N,E>& a)
{
    return detail::insn::i_reduce_min(a.eval());
}

template<unsigned N, class E> SIMDPP_INL
uint16_t reduce_min(const uint16<N,E>& a)
{
    return detail::insn::i_reduce_min(a.eval());
}

template<unsigned N, class E> SIMDPP_INL
int32_t reduce_min(const int32<N,E>& a)
{
    return detail::insn::i_reduce_min(a.eval());
}

template<unsigned N, class E> SIMDPP_INL
uint32_t reduce_min(const uint32<N,E>& a)
{
    return detail::insn::i_reduce_min(a.eval());
}

template<unsigned N, class E> SIMDPP_INL
int64_t reduce_min(const int64<N,E>& a)
{
    return detail::insn::i_reduce_min(a.eval());
}

template<unsigned N, class E> SIMDPP_INL
uint64_t reduce_min(const uint64<N,E>& a)
{
    return detail::insn::i_reduce_min(a.eval());
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

