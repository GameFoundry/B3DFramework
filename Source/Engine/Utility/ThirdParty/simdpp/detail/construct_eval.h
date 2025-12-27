/*  Copyright (C) 2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_CONSTRUCT_EVAL_H
#define LIBSIMDPP_SIMDPP_DETAIL_CONSTRUCT_EVAL_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/setup_arch.h>
#include <simdpp/expr.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

// put forward declarations of all construct_eval functions here. Definitions
// are needed only if the corresponding expression is used

template<class V> SIMDPP_INL
void construct_eval(V& v, const expr_vec_load_splat& e);

template<class V, class VE> SIMDPP_INL
void construct_eval(V& v, const expr_vec_set_splat<VE>& e);

template<class V> SIMDPP_INL
void construct_eval(V& v, const expr_vec_load& e);

template<class V> SIMDPP_INL
void construct_eval(V& v, const expr_vec_load_u& e);

template<class V, class VE, unsigned N> SIMDPP_INL
void construct_eval(V& v, const expr_vec_make_const<VE, N>& e);

template<class V> SIMDPP_INL
void construct_eval(V& v, const expr_vec_make_ones& e);

template<class V, class E> SIMDPP_INL
void construct_eval_wrapper(V& v, const E& e)
{
    construct_eval(v, e);
}

} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

