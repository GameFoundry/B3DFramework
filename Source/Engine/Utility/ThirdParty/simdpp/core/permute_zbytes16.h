/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_PERMUTE_ZBYTES16_H
#define LIBSIMDPP_SIMDPP_CORE_PERMUTE_ZBYTES16_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/get_expr.h>
#include <simdpp/detail/insn/permute_zbytes16.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Selects bytes from a vector according to a mask, optionally selecting zero.
    Each byte within the mask defines which element to select:
     * Bit 7 results in the result byte being zeroed, if set.
     * Bits 6-4 must be zero or the behavior is undefined
     * Bits 3-0 define the element within the given vector.
*/
template<unsigned N, class V1, class E2> SIMDPP_INL
typename detail::get_expr_nomask<V1>::empty
    permute_zbytes16(const any_vec8<N,V1>& a, const uint8<N,E2>& mask)
{
    typename detail::get_expr_nomask<V1>::type ra = a.wrapped().eval();
    return detail::insn::i_permute_zbytes16(ra, mask.eval());
}

template<unsigned N, class V1, class E2> SIMDPP_INL
typename detail::get_expr_nomask<V1>::empty
    permute_zbytes16(const any_vec16<N,V1>& a, const uint16<N,E2>& mask)
{
    typename detail::get_expr_nomask<V1>::type ra = a.wrapped().eval();
    return detail::insn::i_permute_zbytes16(ra, mask.eval());
}

template<unsigned N, class V1, class E2> SIMDPP_INL
typename detail::get_expr_nomask<V1>::empty
    permute_zbytes16(const any_vec32<N,V1>& a, const uint32<N,E2>& mask)
{
    typename detail::get_expr_nomask<V1>::type ra = a.wrapped().eval();
    return detail::insn::i_permute_zbytes16(ra, mask.eval());
}

template<unsigned N, class V1, class E2> SIMDPP_INL
typename detail::get_expr_nomask<V1>::empty
    permute_zbytes16(const any_vec64<N,V1>& a, const uint64<N,E2>& mask)
{
    typename detail::get_expr_nomask<V1>::type ra = a.wrapped().eval();
    return detail::insn::i_permute_zbytes16(ra, mask.eval());
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

