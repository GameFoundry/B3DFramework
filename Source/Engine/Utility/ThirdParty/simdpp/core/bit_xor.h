/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_BIT_XOR_H
#define LIBSIMDPP_SIMDPP_CORE_BIT_XOR_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/get_expr.h>
#include <simdpp/detail/insn/bit_xor.h>
#include <simdpp/core/detail/scalar_arg_impl.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Computes bitwise XOR of integer or floating-point vectors.

    @code
    r0 = a0 ^ b0
    ...
    rN = aN ^ bN
    @endcode

    @par 256-bit version:
    @icost{SSE2-AVX, NEON, ALTIVEC, 2}
*/
template<unsigned N, class V1, class V2> SIMDPP_INL
typename detail::get_expr2<V1, V2>::empty
    bit_xor(const any_vec<N,V1>& a, const any_vec<N,V2>& b)
{
    typename detail::get_expr2_nosign<V1, V2>::type ra, rb;
    ra = a.wrapped().eval();
    rb = b.wrapped().eval();
    return detail::insn::i_bit_xor(ra, rb);
}

// support scalar arguments
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr2<typename detail::get_expr_nomask<V>::type, V>::empty
        bit_xor(const unsigned& a, const any_vec<N,V>& b)
{
    return bit_xor(detail::make_const_bitwise<typename detail::get_expr_nomask<V>::type>(a), b);
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr2<typename detail::get_expr_nomask<V>::type, V>::empty
        bit_xor(const unsigned long& a, const any_vec<N,V>& b)
{
    return bit_xor(detail::make_const_bitwise<typename detail::get_expr_nomask<V>::type>(a), b);
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr2<typename detail::get_expr_nomask<V>::type, V>::empty
        bit_xor(const unsigned long long& a, const any_vec<N,V>& b)
{
    return bit_xor(detail::make_const_bitwise<typename detail::get_expr_nomask<V>::type>(a), b);
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr2<typename detail::get_expr_nomask<V>::type, V>::empty
        bit_xor(const int& a, const any_vec<N,V>& b)
{
    return bit_xor(detail::make_const_bitwise<typename detail::get_expr_nomask<V>::type>(a), b);
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr2<typename detail::get_expr_nomask<V>::type, V>::empty
        bit_xor(const long& a, const any_vec<N,V>& b)
{
    return bit_xor(detail::make_const_bitwise<typename detail::get_expr_nomask<V>::type>(a), b);
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr2<typename detail::get_expr_nomask<V>::type, V>::empty
        bit_xor(const long long& a, const any_vec<N,V>& b)
{
    return bit_xor(detail::make_const_bitwise<typename detail::get_expr_nomask<V>::type>(a), b);
}


template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr2<V, typename detail::get_expr_nomask<V>::type>::empty
        bit_xor(const any_vec<N,V>& a, const unsigned& b)
{
    return bit_xor(a, detail::make_const_bitwise<typename detail::get_expr_nomask<V>::type>(b));
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr2<V, typename detail::get_expr_nomask<V>::type>::empty
        bit_xor(const any_vec<N,V>& a, const unsigned long& b)
{
    return bit_xor(a, detail::make_const_bitwise<typename detail::get_expr_nomask<V>::type>(b));
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr2<V, typename detail::get_expr_nomask<V>::type>::empty
        bit_xor(const any_vec<N,V>& a, const unsigned long long& b)
{
    return bit_xor(a, detail::make_const_bitwise<typename detail::get_expr_nomask<V>::type>(b));
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr2<V, typename detail::get_expr_nomask<V>::type>::empty
        bit_xor(const any_vec<N,V>& a, const int& b)
{
    return bit_xor(a, detail::make_const_bitwise<typename detail::get_expr_nomask<V>::type>(b));
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr2<V, typename detail::get_expr_nomask<V>::type>::empty
        bit_xor(const any_vec<N,V>& a, const long& b)
{
    return bit_xor(a, detail::make_const_bitwise<typename detail::get_expr_nomask<V>::type>(b));
}
template<unsigned N, class V> SIMDPP_INL
typename detail::get_expr2<V, typename detail::get_expr_nomask<V>::type>::empty
        bit_xor(const any_vec<N,V>& a, const long long& b)
{
    return bit_xor(a, detail::make_const_bitwise<typename detail::get_expr_nomask<V>::type>(b));
}


} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif


