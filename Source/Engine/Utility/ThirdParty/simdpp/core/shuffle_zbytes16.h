/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_SHUFFLE_ZBYTES16_H
#define LIBSIMDPP_SIMDPP_CORE_SHUFFLE_ZBYTES16_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/get_expr.h>
#include <simdpp/detail/insn/shuffle_zbytes16.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Selects bytes from two vectors according to a mask, optionally selecting
    zero. Each byte within the mask defines which element to select:
     * Bit 7 results in the result byte being zeroed, if set.
     * Bits 6-5 must be zero or the behavior is undefined
     * Bit 4 defines which vector to select. 0 corresponds to @a a, 1 to @a b.
     * Bits 3-0 define the element within the selected vector.
*/
template<unsigned N, class V1, class V2, class E3> SIMDPP_INL
typename detail::get_expr_nomask<V1>::empty
    shuffle_zbytes16(const any_vec8<N,V1>& a, const any_vec8<N,V2>& b,
                    const uint8<N,E3>& mask)
{
    typename detail::get_expr_nomask<V1>::type ra = a.wrapped().eval(),
                                               rb = b.wrapped().eval();
    return detail::insn::i_shuffle_zbytes16(ra, rb, mask.eval());
}

template<unsigned N, class V1, class V2, class E3> SIMDPP_INL
typename detail::get_expr_nomask<V1>::empty
    shuffle_zbytes16(const any_vec16<N,V1>& a, const any_vec16<N,V2>& b,
                    const uint16<N,E3>& mask)
{
    typename detail::get_expr_nomask<V1>::type ra = a.wrapped().eval(),
                                               rb = b.wrapped().eval();
    return detail::insn::i_shuffle_zbytes16(ra, rb, mask.eval());
}

template<unsigned N, class V1, class V2, class E3> SIMDPP_INL
typename detail::get_expr_nomask<V1>::empty
    shuffle_zbytes16(const any_vec32<N,V1>& a, const any_vec32<N,V2>& b,
                    const uint32<N,E3>& mask)
{
    typename detail::get_expr_nomask<V1>::type ra = a.wrapped().eval(),
                                               rb = b.wrapped().eval();
    return detail::insn::i_shuffle_zbytes16(ra, rb, mask.eval());
}

template<unsigned N, class V1, class V2, class E3> SIMDPP_INL
typename detail::get_expr_nomask<V1>::empty
    shuffle_zbytes16(const any_vec64<N,V1>& a, const any_vec64<N,V2>& b,
                    const uint64<N,E3>& mask)
{
    typename detail::get_expr_nomask<V1>::type ra = a.wrapped().eval(),
                                               rb = b.wrapped().eval();
    return detail::insn::i_shuffle_zbytes16(ra, rb, mask.eval());
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

