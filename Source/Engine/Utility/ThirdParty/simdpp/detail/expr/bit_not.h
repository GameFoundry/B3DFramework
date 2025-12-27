/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_EXPR_BIT_NOT_H
#define LIBSIMDPP_SIMDPP_DETAIL_EXPR_BIT_NOT_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/bit_not.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

template<class R, class E>
struct expr_eval<R, expr_bit_not<E>> {
    static SIMDPP_INL R eval(const expr_bit_not<E>& e)
    {
        // FIXME: this expression is not emitted at the moment
        typename detail::get_expr_nosign<E>::type a;
        a = e.a.eval();
        return (R) insn::i_bit_not(a);
    }
};

} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
