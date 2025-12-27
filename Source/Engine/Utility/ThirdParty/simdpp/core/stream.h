/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_STREAM_H
#define LIBSIMDPP_SIMDPP_CORE_STREAM_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/stream.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {


/** Stores a 128-bit or 256-bit integer, 32-bit or 64-bit floating point vector
    to memory without polluting the caches, if possible.

    @par 128-bit version:

    @code
    *(p) = a[0..127]
    @endcode
    @a p must be aligned to 16 bytes.

    @par 256-bit version:

    @code
    *(p) = a[0..255]
    @endcode
    @a p must be aligned to 32 bytes.
    @icost{SSE2-SSE4.1, NEON, ALTIVEC, 2}
    @icost{AVX (integer vectors), 2}
*/
template<class T, unsigned N, class V> SIMDPP_INL
void stream(T* p, const any_vec<N,V>& a)
{
    static_assert(!is_mask<V>::value, "Masks can not be stored"); // FIXME: convert automatically
    detail::insn::i_stream(reinterpret_cast<char*>(p), a.wrapped().eval());
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

