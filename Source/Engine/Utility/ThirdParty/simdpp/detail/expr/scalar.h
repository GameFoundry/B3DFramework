/*  Copyright (C) 2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_EXPR_SCALAR_H
#define LIBSIMDPP_SIMDPP_DETAIL_EXPR_SCALAR_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/expr.h>
#include <simdpp/detail/insn/make_const.h>
#include <simdpp/core/detail/scalar_arg_impl.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

template<class Int> SIMDPP_INL void scalar_convert(Int& d, uint64_t x) { d = static_cast<Int>(x); }
static SIMDPP_INL
void scalar_convert(float& d, uint64_t x) { d = bit_cast<float>(uint32_t(x)); }
static SIMDPP_INL
void scalar_convert(double& d, uint64_t x) { d = bit_cast<double>(x); }

static SIMDPP_INL
uint64_t cast_int(const int& x) { return (unsigned int) x; }
static SIMDPP_INL
uint64_t cast_int(const long& x) { return (unsigned long) x; }
static SIMDPP_INL
uint64_t cast_int(const long long& x) { return (unsigned long long) x; }
static SIMDPP_INL
uint64_t cast_int(const unsigned& x) { return x; }
static SIMDPP_INL
uint64_t cast_int(const unsigned long& x) { return x; }
static SIMDPP_INL
uint64_t cast_int(const unsigned long long& x) { return x; }

template<class V> SIMDPP_INL
V make_const_bitwise(uint64_t t)
{
    typename detail::remove_sign<V>::type r;
    expr_vec_make_const<typename V::element_type, 1> e;
    scalar_convert(e.a[0], t);
    insn::i_make_const(r, e, 0);
    return V(r);
}

template<class R, class EL> SIMDPP_INL
R expr_eval_scalar(const EL& q)
{
    typename detail::remove_sign<R>::type r;
    expr_vec_make_const<typename R::element_type, 1> e;
    e.a[0] = static_cast<typename R::element_type>(q);
    insn::i_make_const(r, e, 0);
    return R(r);
}

template<class R, class EL> SIMDPP_INL
R expr_eval_scalar_bitwise(const EL& q)
{
    return make_const_bitwise<R>(cast_int(q));
}

} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
