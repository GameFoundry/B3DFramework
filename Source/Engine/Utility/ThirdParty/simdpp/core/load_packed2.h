/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_LOAD_PACKED2_H
#define LIBSIMDPP_SIMDPP_CORE_LOAD_PACKED2_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/load_packed2.h>
#include <simdpp/detail/traits.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Loads values packed in pairs, de-interleaves them and stores the result
    into two vectors.

    @code
    a = [ *(p),   *(p+2), *(p+4), ... , *(p+M*2-2) ]
    b = [ *(p+1), *(p+3), *(p+5), ... , *(p+M*2-1) ]
    @endcode

    Here M is the number of elements in the vector

    @a p must be aligned to the vector size in bytes
*/
template<unsigned N, class V, class T> SIMDPP_INL
void load_packed2(any_vec<N,V>& a, any_vec<N,V>& b, const T* p)
{
    static_assert(!is_mask<V>::value, "Mask types can not be loaded");
    typename detail::get_expr_nosign<V>::type ra, rb;
    detail::insn::i_load_packed2(ra, rb, reinterpret_cast<const char*>(p));
    a.wrapped() = ra;
    b.wrapped() = rb;
}


} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

