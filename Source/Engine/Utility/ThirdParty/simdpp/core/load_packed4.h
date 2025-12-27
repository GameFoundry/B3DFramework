/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_LOAD_PACKED4_H
#define LIBSIMDPP_SIMDPP_CORE_LOAD_PACKED4_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/load_packed4.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {


/** Loads values packed in quartets, de-interleaves them and stores the result
    into four vectors.

    @code
    a = [ *(p),   *(p+4), *(p+8),  ... , *(p+M*4-4) ]
    b = [ *(p+1), *(p+5), *(p+9),  ... , *(p+M*4-3) ]
    c = [ *(p+2), *(p+6), *(p+10), ... , *(p+M*4-2) ]
    d = [ *(p+3), *(p+7), *(p+11), ... , *(p+M*4-1) ]
    @endcode

    Here M is the number of elements in the vector

    @a p must be aligned to the vector size in bytes
*/
template<unsigned N, class V, class T> SIMDPP_INL
void load_packed4(any_vec<N,V>& a, any_vec<N,V>& b,
                  any_vec<N,V>& c, any_vec<N,V>& d,
                  const T* p)
{
    static_assert(!is_mask<V>::value, "Mask types can not be loaded");
    typename detail::get_expr_nosign<V>::type ra, rb, rc, rd;
    detail::insn::i_load_packed4(ra, rb, rc, rd, reinterpret_cast<const char*>(p));
    a.wrapped() = ra;
    b.wrapped() = rb;
    c.wrapped() = rc;
    d.wrapped() = rd;
}


} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

