/*  Copyright (C) 2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_EXPR_BROADCAST_H
#define LIBSIMDPP_SIMDPP_DETAIL_EXPR_BROADCAST_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/splat_n.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

template<class R, unsigned S, class E>
struct expr_eval<R, expr_splat2<S, E>> {
    static SIMDPP_INL R eval(const expr_splat2<S, E>& e)
    {
        return insn::i_splat2<S>(e.a.eval());
    }
};

template<class R, unsigned S, class E>
struct expr_eval<R, expr_splat4<S, E>> {
    static SIMDPP_INL R eval(const expr_splat4<S, E>& e)
    {
        return insn::i_splat4<S>(e.a.eval());
    }
};

template<class R, unsigned S, class E>
struct expr_eval<R, expr_splat8<S, E>> {
    static SIMDPP_INL R eval(const expr_splat8<S, E>& e)
    {
        return insn::i_splat8<S>(e.a.eval());
    }
};

template<class R, unsigned S, class E>
struct expr_eval<R, expr_splat16<S, E>> {
    static SIMDPP_INL R eval(const expr_splat16<S, E>& e)
    {
        return insn::i_splat16<S>(e.a.eval());
    }
};

} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

