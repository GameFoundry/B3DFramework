/*  Copyright (C) 2016  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMD_CORE_TEST_BITS_H
#define LIBSIMDPP_SIMD_CORE_TEST_BITS_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/expr/test_bits.h>
#include <simdpp/detail/get_expr.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Returns true if any bits are set in the given vector.
*/
template<unsigned N, class V> SIMDPP_INL
bool test_bits_any(const any_vec<N,V>& a)
{
    static_assert(!is_mask<V>::value, "Mask types are not supported");
    using expr = typename detail::get_expr_nosign<V, typename V::expr_type>::type;
    return detail::e_test_bits_any(expr(a));
}


} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
