/*  Copyright (C) 2016  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_I_REDUCE_AND_H
#define LIBSIMDPP_SIMDPP_CORE_I_REDUCE_AND_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/i_reduce_and.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {


/** Computes the bitwise AND of the elements in the vector

    @code
    r0 = a0 & a1 & a2 & ...
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
int8_t reduce_and(const int8<N,E>& a)
{
    return detail::insn::i_reduce_and(uint8<N>(a.eval()));
}

template<unsigned N, class E> SIMDPP_INL
uint8_t reduce_and(const uint8<N,E>& a)
{
    return detail::insn::i_reduce_and(a.eval());
}

template<unsigned N, class E> SIMDPP_INL
int16_t reduce_and(const int16<N,E>& a)
{
    return detail::insn::i_reduce_and(uint16<N>(a.eval()));
}

template<unsigned N, class E> SIMDPP_INL
uint16_t reduce_and(const uint16<N,E>& a)
{
    return detail::insn::i_reduce_and(a.eval());
}

template<unsigned N, class E> SIMDPP_INL
int32_t reduce_and(const int32<N,E>& a)
{
    return detail::insn::i_reduce_and(uint32<N>(a.eval()));
}

template<unsigned N, class E> SIMDPP_INL
uint32_t reduce_and(const uint32<N,E>& a)
{
    return detail::insn::i_reduce_and(a.eval());
}

template<unsigned N, class E> SIMDPP_INL
int64_t reduce_and(const int64<N,E>& a)
{
    return detail::insn::i_reduce_and(uint64<N>(a.eval()));
}

template<unsigned N, class E> SIMDPP_INL
uint64_t reduce_and(const uint64<N,E>& a)
{
    return detail::insn::i_reduce_and(a.eval());
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

