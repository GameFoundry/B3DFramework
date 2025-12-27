/*  Copyright (C) 2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_DETAIL_SCALAR_ARG_IMPL_H
#define LIBSIMDPP_SIMDPP_CORE_DETAIL_SCALAR_ARG_IMPL_H

#include <simdpp/types.h>
#include <simdpp/expr.h>
#include <simdpp/core/make_float.h>
#include <simdpp/core/make_int.h>
#include <simdpp/core/make_uint.h>
#include <simdpp/detail/expr/scalar.h>

/*  The following implements the boilerplate for binary function wrappers that
    accept values as scalar arguments.
*/

// simple implementation returning empty expression
#define SIMDPP_SCALAR_ARG_IMPL_VEC_IMPL(FUNC, RET_VEC, EXPR, NEW_VEC)                                                                   \
template<unsigned N, class V> SIMDPP_INL RET_VEC<N,expr_empty> FUNC(const unsigned& a,              const EXPR<N,V>& b) { return FUNC(make_uint<NEW_VEC>(a), b); }   \
template<unsigned N, class V> SIMDPP_INL RET_VEC<N,expr_empty> FUNC(const unsigned long& a,         const EXPR<N,V>& b) { return FUNC(make_uint<NEW_VEC>(a), b); }   \
template<unsigned N, class V> SIMDPP_INL RET_VEC<N,expr_empty> FUNC(const unsigned long long& a,    const EXPR<N,V>& b) { return FUNC(make_uint<NEW_VEC>(a), b); }   \
template<unsigned N, class V> SIMDPP_INL RET_VEC<N,expr_empty> FUNC(const int& a,                   const EXPR<N,V>& b) { return FUNC(make_int<NEW_VEC>(a), b); }    \
template<unsigned N, class V> SIMDPP_INL RET_VEC<N,expr_empty> FUNC(const long& a,                  const EXPR<N,V>& b) { return FUNC(make_int<NEW_VEC>(a), b); }    \
template<unsigned N, class V> SIMDPP_INL RET_VEC<N,expr_empty> FUNC(const long long& a,             const EXPR<N,V>& b) { return FUNC(make_int<NEW_VEC>(a), b); }    \
template<unsigned N, class V> SIMDPP_INL RET_VEC<N,expr_empty> FUNC(const float& a,                 const EXPR<N,V>& b) { return FUNC(make_float<NEW_VEC>(a), b); }  \
template<unsigned N, class V> SIMDPP_INL RET_VEC<N,expr_empty> FUNC(const double& a,                const EXPR<N,V>& b) { return FUNC(make_float<NEW_VEC>(a), b); }  \
                                                                                                                                                        \
template<unsigned N, class V> SIMDPP_INL RET_VEC<N,expr_empty> FUNC(const EXPR<N,V>& a, const unsigned& b          ) { return FUNC(a, make_uint<NEW_VEC>(b)); }   \
template<unsigned N, class V> SIMDPP_INL RET_VEC<N,expr_empty> FUNC(const EXPR<N,V>& a, const unsigned long& b     ) { return FUNC(a, make_uint<NEW_VEC>(b)); }   \
template<unsigned N, class V> SIMDPP_INL RET_VEC<N,expr_empty> FUNC(const EXPR<N,V>& a, const unsigned long long& b) { return FUNC(a, make_uint<NEW_VEC>(b)); }   \
template<unsigned N, class V> SIMDPP_INL RET_VEC<N,expr_empty> FUNC(const EXPR<N,V>& a, const int& b               ) { return FUNC(a, make_int<NEW_VEC>(b)); }    \
template<unsigned N, class V> SIMDPP_INL RET_VEC<N,expr_empty> FUNC(const EXPR<N,V>& a, const long& b              ) { return FUNC(a, make_int<NEW_VEC>(b)); }    \
template<unsigned N, class V> SIMDPP_INL RET_VEC<N,expr_empty> FUNC(const EXPR<N,V>& a, const long long& b         ) { return FUNC(a, make_int<NEW_VEC>(b)); }    \
template<unsigned N, class V> SIMDPP_INL RET_VEC<N,expr_empty> FUNC(const EXPR<N,V>& a, const float& b             ) { return FUNC(a, make_float<NEW_VEC>(b)); }  \
template<unsigned N, class V> SIMDPP_INL RET_VEC<N,expr_empty> FUNC(const EXPR<N,V>& a, const double& b            ) { return FUNC(a, make_float<NEW_VEC>(b)); }
// end #define


#define SIMDPP_SCALAR_ARG_IMPL_VEC(FUNC, RET_VEC, VEC) \
    SIMDPP_SCALAR_ARG_IMPL_VEC_IMPL(FUNC, RET_VEC, VEC, VEC<N>)
#define SIMDPP_SCALAR_ARG_IMPL_VEC_EXPR(FUNC, RET_VEC, EXPR) \
    SIMDPP_SCALAR_ARG_IMPL_VEC_IMPL(FUNC, RET_VEC, EXPR, typename detail::get_expr<V>::type)
// end #define

// implementation returning an expression for vector arguments
#define SIMDPP_SCALAR_ARG_IMPL_EXPR(FUNC, EXPR, RET_VEC, VEC)                   \
template<unsigned N, class V> SIMDPP_INL                                        \
RET_VEC<N, EXPR<unsigned, VEC<N,V>>>                                            \
    FUNC(const unsigned& a, const VEC<N,V>& b)                                  \
{ return { { a, b } }; }                                                        \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
RET_VEC<N, EXPR<unsigned long, VEC<N,V>>>                                       \
    FUNC(const unsigned long& a, const VEC<N,V>& b)                             \
{ return { { a, b } }; }                                                        \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
RET_VEC<N, EXPR<unsigned long long, VEC<N,V>>>                                  \
    FUNC(const unsigned long long& a, const VEC<N,V>& b)                        \
{ return { { a, b } }; }                                                        \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
RET_VEC<N, EXPR<int, VEC<N,V>>>                                                 \
    FUNC(const int& a, const VEC<N,V>& b)                                       \
{ return { { a, b } }; }                                                        \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
RET_VEC<N, EXPR<long, VEC<N,V>>>                                                \
    FUNC(const long& a, const VEC<N,V>& b)                                      \
{ return { { a, b } }; }                                                        \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
RET_VEC<N, EXPR<long long, VEC<N,V>>>                                           \
    FUNC(const long long& a, const VEC<N,V>& b)                                 \
{ return { { a, b } }; }                                                        \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
RET_VEC<N, EXPR<float, VEC<N,V>>>                                               \
    FUNC(const float& a, const VEC<N,V>& b)                                     \
{ return { { a, b } }; }                                                        \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
RET_VEC<N, EXPR<double, VEC<N,V>>>                                              \
    FUNC(const double& a, const VEC<N,V>& b)                                    \
{ return { { a, b } }; }                                                        \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
RET_VEC<N, EXPR<VEC<N,V>, unsigned>>                                            \
    FUNC(const VEC<N,V>& a, const unsigned& b)                                  \
{ return { { a, b } }; }                                                        \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
RET_VEC<N, EXPR<VEC<N,V>, unsigned long>>                                       \
    FUNC(const VEC<N,V>& a, const unsigned long& b)                             \
{ return { { a, b } }; }                                                        \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
RET_VEC<N, EXPR<VEC<N,V>, unsigned long long>>                                  \
    FUNC(const VEC<N,V>& a, const unsigned long long& b)                        \
{ return { { a, b } }; }                                                        \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
RET_VEC<N, EXPR<VEC<N,V>, int>>                                                 \
    FUNC(const VEC<N,V>& a, const int& b)                                       \
{ return { { a, b } }; }                                                        \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
RET_VEC<N, EXPR<VEC<N,V>, long>>                                                \
    FUNC(const VEC<N,V>& a, const long& b)                                      \
{ return { { a, b } }; }                                                        \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
RET_VEC<N, EXPR<VEC<N,V>, long long>>                                           \
    FUNC(const VEC<N,V>& a, const long long& b)                                 \
{ return { { a, b } }; }                                                        \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
RET_VEC<N, EXPR<VEC<N,V>, float>>                                               \
    FUNC(const VEC<N,V>& a, const float& b)                                     \
{ return { { a, b } }; }                                                        \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
RET_VEC<N, EXPR<VEC<N,V>, double>>                                              \
    FUNC(const VEC<N,V>& a, const double& b)                                    \
{ return { { a, b } }; }
// end #define

// a implementation for integer operations that use get_expr_uint
#define SIMDPP_SCALAR_ARG_IMPL_INT_UNSIGNED(FUNC, EXPR, VEC, INT_VEC)           \
template<unsigned N, class V> SIMDPP_INL                                        \
typename detail::get_expr_uint<EXPR, V, unsigned>::type                         \
        FUNC(const VEC<N,V>& a, const unsigned& b)                              \
{ return { { a.wrapped(), b } }; }                                              \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
typename detail::get_expr_uint<EXPR, V, unsigned long>::type                    \
        FUNC(const VEC<N,V>& a, const unsigned long& b)                         \
{ return { { a.wrapped(), b } }; }                                              \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
typename detail::get_expr_uint<EXPR, V, unsigned long long>::type               \
        FUNC(const VEC<N,V>& a, const unsigned long long& b)                    \
{ return { { a.wrapped(), b } }; }                                              \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
typename detail::get_expr_uint<EXPR, V, int>::type                              \
        FUNC(const VEC<N,V>& a, const int& b)                                   \
{ return { { a.wrapped(), b } }; }                                              \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
typename detail::get_expr_uint<EXPR, V, long>::type                             \
        FUNC(const VEC<N,V>& a, const long& b)                                  \
{ return { { a.wrapped(), b } }; }                                              \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
typename detail::get_expr_uint<EXPR, V, long long>::type                        \
        FUNC(const VEC<N,V>& a, const long long& b)                             \
{ return { { a.wrapped(), b } }; }                                              \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
typename detail::get_expr_uint<EXPR, V, float>::type                            \
        FUNC(const VEC<N,V>& a, const float& b)                                 \
{ return { { a.wrapped(), b } }; }                                              \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
typename detail::get_expr_uint<EXPR, V, double>::type                           \
        FUNC(const VEC<N,V>& a, const double& b)                                \
{ return { { a.wrapped(), b } }; }                                              \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
typename detail::get_expr_uint<EXPR, unsigned, V>::type                         \
        FUNC(const unsigned& a, const VEC<N,V>& b)                              \
{ return { { a, b.wrapped() } }; }                                              \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
typename detail::get_expr_uint<EXPR, unsigned long, V>::type                    \
        FUNC(const unsigned long& a, const VEC<N,V>& b)                         \
{ return { { a, b.wrapped() } }; }                                              \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
typename detail::get_expr_uint<EXPR, unsigned long long, V>::type               \
        FUNC(const unsigned long long& a, const VEC<N,V>& b)                    \
{ return { { a, b.wrapped() } }; }                                              \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
typename detail::get_expr_uint<EXPR, int, V>::type                              \
        FUNC(const int& a, const VEC<N,V>& b)                                   \
{ return { { a, b.wrapped() } }; }                                              \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
typename detail::get_expr_uint<EXPR, long, V>::type                             \
        FUNC(const long& a, const VEC<N,V>& b)                                  \
{ return { { a, b.wrapped() } }; }                                              \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
typename detail::get_expr_uint<EXPR, long long, V>::type                        \
        FUNC(const long long& a, const VEC<N,V>& b)                             \
{ return { { a, b.wrapped() } }; }                                              \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
typename detail::get_expr_uint<EXPR, float, V>::type                            \
        FUNC(const float& a, const VEC<N,V>& b)                                 \
{ return { { a, b.wrapped() } }; }                                              \
                                                                                \
template<unsigned N, class V> SIMDPP_INL                                        \
typename detail::get_expr_uint<EXPR, double, V>::type                           \
        FUNC(const double& a, const VEC<N,V>& b)                                \
{ return { { a, b.wrapped() } }; }
// end #define

#endif
