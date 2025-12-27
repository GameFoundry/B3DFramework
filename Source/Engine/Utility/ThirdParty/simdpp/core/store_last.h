/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_STORE_LAST_H
#define LIBSIMDPP_SIMDPP_CORE_STORE_LAST_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/store_last.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Stores the last @a n elements of an 128-bit or 256-bit integer, 32-bit or
    64-bit floating point vector to memory. @a n must be in range [0..N-1]
    where @a N is the number of elements in the vector. If @a n is zero, no
    store is made.

    The function may write entire block of 128 or 256 bits.

    @code
    *(p+N-n) = a{N-n}
    ...
    *(p+N-2) = a{N-2}
    *(p+N-1) = a{N-1}
    @endcode

    This function results in several instructions. It is best not to use it in
    inner loops.

    @par 128-bit version:
    @a p must be aligned to 16 bytes.
    @par 256-bit version:
    @a p must be aligned to 32 bytes.
*/
template<class T, unsigned N, class V> SIMDPP_INL
void store_last(T* p, const any_vec<N,V>& a, unsigned n)
{
    static_assert(!is_mask<V>::value, "Masks can not be stored"); // FIXME: automatically convert
    detail::insn::i_store_last(reinterpret_cast<char*>(p),
                                a.wrapped().eval(), n);
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

