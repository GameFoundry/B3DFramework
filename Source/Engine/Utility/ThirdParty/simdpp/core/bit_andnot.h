/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_BIT_ANDNOT_H
#define LIBSIMDPP_SIMDPP_CORE_BIT_ANDNOT_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/bit_andnot.h>
#include <simdpp/detail/expr/bit_andnot.h>
#include <simdpp/core/detail/get_expr_bitwise.h>
#include <simdpp/core/detail/scalar_arg_impl.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Computes bitwise AND NOT of two integer or floating-point vectors.

    @code
    r0 = a0 & ~b0
    ...
    rN = aN & ~bN
    @endcode

    @todo: icost
*/
template<unsigned N, class V1, class V2> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_andnot, V1, V2>::type
        bit_andnot(const any_vec<N,V1>& a, const any_vec<N,V2>& b)
{
    return { { a.wrapped(), b.wrapped() } };
}

// support scalar arguments
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_andnot, unsigned, V>::type
        bit_andnot(const unsigned& a, const any_vec<N,V>& b)
{
    return { { a, b.wrapped() } };
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_andnot, unsigned long, V>::type
        bit_andnot(const unsigned long& a, const any_vec<N,V>& b)
{
    return { { a, b.wrapped() } };
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_andnot, unsigned long long, V>::type
        bit_andnot(const unsigned long long& a, const any_vec<N,V>& b)
{
    return { { a, b.wrapped() } };
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_andnot, int, V>::type
        bit_andnot(const int& a, const any_vec<N,V>& b)
{
    return { { a, b.wrapped() } };
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_andnot, long, V>::type
        bit_andnot(const long& a, const any_vec<N,V>& b)
{
    return { { a, b.wrapped() } };
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_andnot, long long, V>::type
        bit_andnot(const long long& a, const any_vec<N,V>& b)
{
    return { { a, b.wrapped() } };
}

template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_andnot, V, unsigned>::type
        bit_andnot(const any_vec<N,V>& a, const unsigned& b)
{
    return { { a.wrapped(), b } };
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_andnot, V, unsigned long>::type
        bit_andnot(const any_vec<N,V>& a, const unsigned long& b)
{
    return { { a.wrapped(), b } };
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_andnot, V, unsigned long long>::type
        bit_andnot(const any_vec<N,V>& a, const unsigned long long& b)
{
    return { { a.wrapped(), b } };
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_andnot, V, int>::type
        bit_andnot(const any_vec<N,V>& a, const int& b)
{
    return { { a.wrapped(), b } };
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_andnot, V, long>::type
        bit_andnot(const any_vec<N,V>& a, const long& b)
{
    return { { a.wrapped(), b } };
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr_bitwise2_and<expr_bit_andnot, V, long long>::type
        bit_andnot(const any_vec<N,V>& a, const long long& b)
{
    return { { a.wrapped(), b } };
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

