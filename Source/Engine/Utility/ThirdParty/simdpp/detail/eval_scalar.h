/*  Copyright (C) 2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_EVAL_SCALAR_H
#define LIBSIMDPP_SIMDPP_DETAIL_EVAL_SCALAR_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/setup_arch.h>
#include <simdpp/expr.h>
#include <simdpp/detail/expr/scalar.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

template<class R, class E>
struct eval_maybe_scalar {
    static SIMDPP_INL R eval(const E& e) { return (R) e.eval(); }
};

template<class R>
struct eval_maybe_scalar<R, int> {
    static SIMDPP_INL R eval(const int& e) { return expr_eval_scalar<R>(e); }
};

template<class R>
struct eval_maybe_scalar<R, long> {
    static SIMDPP_INL R eval(const long& e) { return expr_eval_scalar<R>(e); }
};

template<class R>
struct eval_maybe_scalar<R, long long> {
    static SIMDPP_INL R eval(const long long& e) { return expr_eval_scalar<R>(e); }
};

template<class R>
struct eval_maybe_scalar<R, unsigned> {
    static SIMDPP_INL R eval(const unsigned& e) { return expr_eval_scalar<R>(e); }
};

template<class R>
struct eval_maybe_scalar<R, unsigned long> {
    static SIMDPP_INL R eval(const unsigned long& e) { return expr_eval_scalar<R>(e); }
};

template<class R>
struct eval_maybe_scalar<R, unsigned long long> {
    static SIMDPP_INL R eval(const unsigned long long& e) { return expr_eval_scalar<R>(e); }
};

template<class R>
struct eval_maybe_scalar<R, float> {
    static SIMDPP_INL R eval(const float& e) { return expr_eval_scalar<R>(e); }
};

template<class R>
struct eval_maybe_scalar<R, double> {
    static SIMDPP_INL R eval(const double& e) { return expr_eval_scalar<R>(e); }
};

template<class R, class E>
struct eval_maybe_scalar_bitwise {
    static SIMDPP_INL R eval(const E& e) { return (R) e.eval(); }
};

template<class R>
struct eval_maybe_scalar_bitwise<R, int> {
    static SIMDPP_INL R eval(const int& e) { return expr_eval_scalar_bitwise<R>(e); }
};

template<class R>
struct eval_maybe_scalar_bitwise<R, long> {
    static SIMDPP_INL R eval(const long& e) { return expr_eval_scalar_bitwise<R>(e); }
};

template<class R>
struct eval_maybe_scalar_bitwise<R, long long> {
    static SIMDPP_INL R eval(const long long& e) { return expr_eval_scalar_bitwise<R>(e); }
};

template<class R>
struct eval_maybe_scalar_bitwise<R, unsigned> {
    static SIMDPP_INL R eval(const unsigned& e) { return expr_eval_scalar_bitwise<R>(e); }
};

template<class R>
struct eval_maybe_scalar_bitwise<R, unsigned long> {
    static SIMDPP_INL R eval(const unsigned long& e) { return expr_eval_scalar_bitwise<R>(e); }
};

template<class R>
struct eval_maybe_scalar_bitwise<R, unsigned long long> {
    static SIMDPP_INL R eval(const unsigned long long& e) { return expr_eval_scalar_bitwise<R>(e); }
};

template<class R>
struct eval_maybe_scalar_bitwise<R, float> {
    static SIMDPP_INL R eval(const float& e) { return expr_eval_scalar_bitwise<R>(e); }
};

template<class R>
struct eval_maybe_scalar_bitwise<R, double> {
    static SIMDPP_INL R eval(const double& e) { return expr_eval_scalar_bitwise<R>(e); }
};

} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
