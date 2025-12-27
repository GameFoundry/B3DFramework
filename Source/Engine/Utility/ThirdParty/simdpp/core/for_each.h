/*  Copyright (C) 2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_FOR_EACH_H
#define LIBSIMDPP_SIMDPP_CORE_FOR_EACH_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/for_each.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Executes the given function on all elements of the vector.

    Equivalent to:
    @code
    function(extract<0>(v));
    function(extract<1>(v));
    ...
    function(extract<N>(v));
    @endcode
*/
template<unsigned N, class V, class F> SIMDPP_INL
void for_each(const any_vec<N, V>& v, F function)
{
    detail::for_each(v.wrapped().eval(), function);
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

