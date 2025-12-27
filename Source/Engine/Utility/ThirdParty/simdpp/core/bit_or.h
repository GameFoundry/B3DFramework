/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_BIT_OR_H
#define LIBSIMDPP_SIMDPP_CORE_BIT_OR_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/bit_or.h>
#include <simdpp/detail/expr/bit_or.h>
#include <simdpp/core/detail/get_expr_bitwise.h>
#include <simdpp/core/detail/scalar_arg_impl.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Computes bitwise OR of integer vectors.

    @code
    r0 = a0 | b0
    ...
    rN = aN | bN
    @endcode

    @todo icost
*/
template<unsigned N, class V1, class V2> SIMDPP_INL
typename detail::get_expr_bit_or<V1, V2>::type
        bit_or(const any_vec<N,V1>& a, const any_vec<N,V2>& b)
{
    return { { a.wrapped(), b.wrapped() } };
}

// support scalar arguments
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_or, unsigned, V>::type
        bit_or(const unsigned& a, const any_vec<N,V>& b)
{
    return { { a, b.wrapped() } };
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_or, unsigned long, V>::type
        bit_or(const unsigned long& a, const any_vec<N,V>& b)
{
    return { { a, b.wrapped() } };
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_or, unsigned long long, V>::type
        bit_or(const unsigned long long& a, const any_vec<N,V>& b)
{
    return { { a, b.wrapped() } };
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_or, int, V>::type
        bit_or(const int& a, const any_vec<N,V>& b)
{
    return { { a, b.wrapped() } };
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_or, long, V>::type
        bit_or(const long& a, const any_vec<N,V>& b)
{
    return { { a, b.wrapped() } };
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_or, long long, V>::type
        bit_or(const long long& a, const any_vec<N,V>& b)
{
    return { { a, b.wrapped() } };
}

template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_or, V, unsigned>::type
        bit_or(const any_vec<N,V>& a, const unsigned& b)
{
    return { { a.wrapped(), b } };
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_or, V, unsigned long>::type
        bit_or(const any_vec<N,V>& a, const unsigned long& b)
{
    return { { a.wrapped(), b } };
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_or, V, unsigned long long>::type
        bit_or(const any_vec<N,V>& a, const unsigned long long& b)
{
    return { { a.wrapped(), b } };
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_or, V, int>::type
        bit_or(const any_vec<N,V>& a, const int& b)
{
    return { { a.wrapped(), b } };
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_or, V, long>::type
        bit_or(const any_vec<N,V>& a, const long& b)
{
    return { { a.wrapped(), b } };
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_or, V, long long>::type
        bit_or(const any_vec<N,V>& a, const long long& b)
{
    return { { a.wrapped(), b } };
}


} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif


