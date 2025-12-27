/*  Copyright (C) 2011-2012  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_DETAIL_NULL_TRANSPOSE_H
#define LIBSIMDPP_DETAIL_NULL_TRANSPOSE_H
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <utility>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace null {

template<class V> SIMDPP_INL
void transpose2(V& a0, V& a1)
{
    for (unsigned j = 0; j < V::length; j+=2) {
        std::swap(a0.el(j+1), a1.el(j));
    }
}

template<class V> SIMDPP_INL
void transpose4(V& a0, V& a1, V& a2, V& a3)
{
    for (unsigned j = 0; j < V::length; j+=4) {
        std::swap(a0.el(j+1), a1.el(j));
        std::swap(a0.el(j+2), a2.el(j));
        std::swap(a0.el(j+3), a3.el(j));
        std::swap(a1.el(j+2), a2.el(j+1));
        std::swap(a1.el(j+3), a3.el(j+1));
        std::swap(a2.el(j+3), a3.el(j+2));
    }
}

} // namespace null
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
#endif
