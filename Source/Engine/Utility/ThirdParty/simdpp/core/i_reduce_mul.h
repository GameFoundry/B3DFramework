/*  Copyright (C) 2016  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_I_REDUCE_MUL_H
#define LIBSIMDPP_SIMDPP_CORE_I_REDUCE_MUL_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/i_reduce_mul.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {


/** Computes the product of the elements in the vector. Intermediate and the
    final result is computed in 32-bit precision in 16 bit case. The
    behavior is undefined in the case of an overflow.

    @code
    r0 = a0 * a1 * a2 * ...
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
int32_t reduce_mul(const int16<N,E>& a)
{
    return detail::insn::i_reduce_mul(a.eval());
}

template<unsigned N, class E> SIMDPP_INL
uint32_t reduce_mul(const uint16<N,E>& a)
{
    return detail::insn::i_reduce_mul(a.eval());
}

template<unsigned N, class E> SIMDPP_INL
int32_t reduce_mul(const int32<N,E>& a)
{
    return detail::insn::i_reduce_mul(uint32<N>(a.eval()));
}

template<unsigned N, class E> SIMDPP_INL
uint32_t reduce_mul(const uint32<N,E>& a)
{
    return detail::insn::i_reduce_mul(a.eval());
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

