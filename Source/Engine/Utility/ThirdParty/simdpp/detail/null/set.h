/*  Copyright (C) 2011-2012  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_DETAIL_NULL_SET_H
#define LIBSIMDPP_DETAIL_NULL_SET_H
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace null {

template<class V, class E = typename V::element_type> SIMDPP_INL
V make_vec(E v0)
{
    V r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = v0;
    }
    return r;
}

template<class V, class E = typename V::element_type> SIMDPP_INL
V make_vec(E v0, E v1)
{
    V r;
    for (unsigned i = 0; i < V::length; i+=2) {
        r.el(i) = v0;
        r.el(i+1) = v1;
    }
    return r;
}

template<class V, class E = typename V::element_type> SIMDPP_INL
V make_vec(E v0, E v1, E v2, E v3)
{
    V r;
    for (unsigned i = 0; i < V::length; i+=4) {
        r.el(i) = v0;
        r.el(i+1) = v1;
        r.el(i+2) = v2;
        r.el(i+3) = v3;
    }
    return r;
}

template<class V, class E = typename V::element_type> SIMDPP_INL
V make_vec(E v0, E v1, E v2, E v3, E v4, E v5, E v6, E v7)
{
    V r;
    for (unsigned i = 0; i < V::length; i+=8) {
        r.el(i) = v0;
        r.el(i+1) = v1;
        r.el(i+2) = v2;
        r.el(i+3) = v3;
        r.el(i+4) = v4;
        r.el(i+5) = v5;
        r.el(i+6) = v6;
        r.el(i+7) = v7;
    }
    return r;
}

template<class V, class E = typename V::element_type> SIMDPP_INL
V make_vec(E v0, E v1, E v2, E v3, E v4, E v5, E v6, E v7,
           E v8, E v9, E v10, E v11, E v12, E v13, E v14, E v15)
{
    V r;
    for (unsigned i = 0; i < V::length; i+=16) {
        r.el(i) = v0;
        r.el(i+1) = v1;
        r.el(i+2) = v2;
        r.el(i+3) = v3;
        r.el(i+4) = v4;
        r.el(i+5) = v5;
        r.el(i+6) = v6;
        r.el(i+7) = v7;
        r.el(i+8) = v8;
        r.el(i+9) = v9;
        r.el(i+10) = v10;
        r.el(i+11) = v11;
        r.el(i+12) = v12;
        r.el(i+13) = v13;
        r.el(i+14) = v14;
        r.el(i+15) = v15;
    }
    return r;
}

} // namespace null
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
#endif
