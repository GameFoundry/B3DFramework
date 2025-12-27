/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_EXPR_I_SUB_SAT_H
#define LIBSIMDPP_SIMDPP_DETAIL_EXPR_I_SUB_SAT_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/get_expr.h>
#include <simdpp/detail/insn/i_sub_sat.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

template<class R, class E1, class E2>
struct expr_eval<R, expr_isub_sat<E1, E2>> {
    static SIMDPP_INL R eval(const expr_isub_sat<E1, E2>& e)
    {
        using E = get_expr2_same<E1, E2>;
        return (R) insn::i_sub_sat(
                eval_maybe_scalar<typename E::v1_final_type, E1>::eval(e.a),
                eval_maybe_scalar<typename E::v2_final_type, E2>::eval(e.b));
    }
};

} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

