/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_STORE_PACKED3_H
#define LIBSIMDPP_SIMDPP_CORE_STORE_PACKED3_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/store_packed3.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {


/** Interleaves values from three vectors and stores the result into successive
    locations starting from @a p.

    @par 128-bit version:
    @code
    [ *(p),   *(p+3), *(p+6), ... , *(p+M*3-3) ] = a
    [ *(p+1), *(p+4), *(p+7), ... , *(p+M*3-2) ] = b
    [ *(p+2), *(p+5), *(p+8), ... , *(p+M*3-1) ] = c
    @endcode

    Here M is the number of elements in the vector

    @a p must be aligned to the vector size in bytes
*/
template<class T, unsigned N, class V1, class V2, class V3> SIMDPP_INL
void store_packed3(T* p, const any_vec<N,V1>& a, const any_vec<N,V2>& b,
                   const any_vec<N,V3>& c)
{
    static_assert(!is_mask<V1>::value && !is_mask<V2>::value && !is_mask<V3>::value,
                  "Mask types can not be stored"); // FIXME
    static_assert(V1::size_tag == V2::size_tag && V1::size_tag == V3::size_tag,
                  "Vector elements must have the same size");
    detail::insn::i_store_packed3(reinterpret_cast<char*>(p),
                                  a.wrapped().eval(), b.wrapped().eval(), c.wrapped().eval());
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

