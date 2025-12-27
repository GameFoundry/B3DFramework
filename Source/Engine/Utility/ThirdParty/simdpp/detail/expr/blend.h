/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_EXPR_BLEND_H
#define LIBSIMDPP_SIMDPP_DETAIL_EXPR_BLEND_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/blend.h>
#include <simdpp/detail/insn/blend.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

template<class R, class E1, class E2, class E3>
struct expr_eval<R, expr_blend<E1, E2, E3>> {
    static SIMDPP_INL R eval(const expr_blend<E1, E2, E3>& e)
    {
        using E = get_expr_blend<E1, E2, E3>;
        return (R) insn::i_blend(
                eval_maybe_scalar<typename E::v1_final_type, E1>::eval(e.on),
                eval_maybe_scalar<typename E::v2_final_type, E2>::eval(e.off),
                eval_maybe_scalar<typename E::v3_final_type, E3>::eval(e.mask));
    }
};


} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

