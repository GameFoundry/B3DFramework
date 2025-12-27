/*  Copyright (C) 2013  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_LOAD_U_H
#define LIBSIMDPP_SIMDPP_CORE_LOAD_U_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/load_u.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Loads a 128-bit or 256-bit integer, 32-bit or 64-bit float vector from an
    unaligned memory location.

    @par 128-bit version:

    @code
    a[0..127] = *(p)
    @endcode

    @a p must be aligned to the element size. If @a p is aligned to 16 bytes
    only the referenced 16 byte block is accessed. Otherwise, memory within the
    smallest 16-byte aligned 32-byte block may be accessed.

    @icost{ALTIVEC, 4}

    @par 256-bit version:

    @code
    a[0..255] = *(p)
    @endcode
    @a p must be aligned to 32 bytes.
    @icost{SSE2-SSE4.1, NEON, 2}
    @icost{ALTIVEC, 6}

    @a p must be aligned to the element size. If @a p is aligned to 32 bytes
    only the referenced 16 byte block is accessed. Otherwise, memory within the
    smallest 32-byte aligned 64-byte block may be accessed.
*/
// Fixme return empty expression
template<class T>
SIMDPP_INL expr_vec_load_u load_u(const T* p)
{
    return { reinterpret_cast<const char*>(p) };
}

template<class V, class T> SIMDPP_INL
V load_u(const T* p)
{
    static_assert(is_vector<V>::value && !is_mask<V>::value,
                  "V must be a non-mask vector");
    return detail::insn::i_load_u_any<V>(reinterpret_cast<const char*>(p));
}
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

