/*  Copyright (C) 2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_LOAD_SPLAT_H
#define LIBSIMDPP_SIMDPP_CORE_LOAD_SPLAT_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/load_splat.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Loads a value from a memory location and broadcasts it to all elements of a
    vector.

    @code
    r0 = *p
    ...
    rN = *p
    @endcode

    @a p must have the alignment of the element of the target vector.
*/
// FIXME: return empty expression
template<class T>
SIMDPP_INL expr_vec_load_splat load_splat(const T* p)
{
    return expr_vec_load_splat(reinterpret_cast<const char*>(p));
}

template<class V, class T> SIMDPP_INL
V load_splat(const T* p)
{
    static_assert(is_vector<V>::value && !is_mask<V>::value,
                  "V must be a non-mask vector");
    return detail::insn::i_load_splat_any<V>(reinterpret_cast<const char*>(p));
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

