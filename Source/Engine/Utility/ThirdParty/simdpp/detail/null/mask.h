/*  Copyright (C) 2013  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_DETAIL_NULL_MASK_H
#define LIBSIMDPP_DETAIL_NULL_MASK_H
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/cast.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace null {

template<class V, class M> SIMDPP_INL
V unmask_mask(const M& m)
{
    static_assert(V::length == M::length && V::size_tag == M::size_tag,
                  "Can't convert mask to different type");
    V r;
    typename V::uint_element_type all_bits_uint = V::all_bits;
    typename V::element_type all_bits = bit_cast<typename V::element_type>(all_bits_uint);
    for (unsigned i = 0; i < V::vec_length; ++i) {
        for (unsigned j = 0; j < V::base_vector_type::length; j++) {
            r.vec(i).el(j) = m.vec(i).el(j) ? all_bits : 0;
        }
    }
    return r;
}


} // namespace null
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
#endif
