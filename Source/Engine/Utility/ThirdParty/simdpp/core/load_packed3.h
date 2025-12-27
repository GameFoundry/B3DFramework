/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_LOAD_PACKED3_H
#define LIBSIMDPP_SIMDPP_CORE_LOAD_PACKED3_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/load_packed3.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {


/** Loads values packed in triplets, de-interleaves them and stores the result
    into three vectors.

    @code
    a = [ *(p),   *(p+3), *(p+6), ... , *(p+M*3-3) ]
    b = [ *(p+1), *(p+4), *(p+7), ... , *(p+M*3-2) ]
    c = [ *(p+2), *(p+5), *(p+8), ... , *(p+M*3-1) ]
    @endcode

    Here M is the number of elements in the vector

    @a p must be aligned to the vector size in bytes
*/
template<unsigned N, class V, class T> SIMDPP_INL
void load_packed3(any_vec<N,V>& a, any_vec<N,V>& b, any_vec<N,V>& c,
                  const T* p)
{
    static_assert(!is_mask<V>::value, "Mask types can not be loaded");
    typename detail::get_expr_nosign<V>::type ra, rb, rc;
    detail::insn::i_load_packed3(ra, rb, rc, reinterpret_cast<const char*>(p));
    a.wrapped() = ra;
    b.wrapped() = rb;
    c.wrapped() = rc;
}


} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

