/*  Copyright (C) 2011-2012  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_DETAIL_NULL_COMPARE_H
#define LIBSIMDPP_DETAIL_NULL_COMPARE_H
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/detail/null/mask.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace null {

template<class V> SIMDPP_INL
typename V::mask_vector_type cmp_eq(const V& a, const V& b)
{
    typename V::mask_vector_type r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = (a.el(i) == b.el(i)) ? 1 : 0;
    }
    return r;
}

template<class V> SIMDPP_INL
typename V::mask_vector_type cmp_neq(const V& a, const V& b)
{
    typename V::mask_vector_type r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = (a.el(i) != b.el(i)) ? 1 : 0;
    }
    return r;
}

template<class V> SIMDPP_INL
typename V::mask_vector_type cmp_lt(const V& a, const V& b)
{
    typename V::mask_vector_type r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = (a.el(i) < b.el(i)) ? 1 : 0;
    }
    return r;
}

template<class V> SIMDPP_INL
typename V::mask_vector_type cmp_le(const V& a, const V& b)
{
    typename V::mask_vector_type r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = (a.el(i) <= b.el(i)) ? 1 : 0;
    }
    return r;
}

template<class V> SIMDPP_INL
typename V::mask_vector_type cmp_gt(const V& a, const V& b)
{
    typename V::mask_vector_type r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = (a.el(i) > b.el(i)) ? 1 : 0;
    }
    return r;
}

template<class V> SIMDPP_INL
typename V::mask_vector_type cmp_ge(const V& a, const V& b)
{
    typename V::mask_vector_type r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = (a.el(i) >= b.el(i)) ? 1 : 0;
    }
    return r;
}

} // namespace null
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
#endif
