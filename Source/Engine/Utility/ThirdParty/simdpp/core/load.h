/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_LOAD_H
#define LIBSIMDPP_SIMDPP_CORE_LOAD_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/load.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Loads a 128-bit or 256-bit integer, 32-bit or 64-bit float vector
    from an aligned memory location.

    @par 128-bit version:

    @code
    a[0..127] = *(p)
    @endcode
    @a p must be aligned to 16 bytes.

    @par 256-bit version:

    @code
    a[0..255] = *(p)
    @endcode
    @a p must be aligned to 32 bytes.

    @icost{SSE2-SSE4.1, NEON, ALTIVEC, 2}
    @icost{AVX (integer vectors), 2}
*/
// Fixme return empty expression
template<class T>
SIMDPP_INL expr_vec_load load(const T* p)
{
    expr_vec_load r;
    r.a = reinterpret_cast<const char*>(p);
    return r;
}

template<class V, class T> SIMDPP_INL
V load(const T* p)
{
    static_assert(is_vector<V>::value && !is_mask<V>::value,
                  "V must be a non-mask vector");
    return detail::insn::i_load_any<V>(reinterpret_cast<const char*>(p));
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

