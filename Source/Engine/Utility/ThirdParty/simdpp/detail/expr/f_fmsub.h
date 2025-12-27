/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_EXPR_F_FMSUB_H
#define LIBSIMDPP_SIMDPP_DETAIL_EXPR_F_FMSUB_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/f_fmsub.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

template<class R, class E1, class E2, class E3>
struct expr_eval<R, expr_fmsub<E1, E2, E3>> {
    static SIMDPP_INL R eval(const expr_fmsub<E1, E2, E3>& e)
    {
        return (R) insn::i_fmsub(e.a.eval(), e.b.eval(), e.c.eval());
    }
};

} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

