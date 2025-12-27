/*  Copyright (C) 2016  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_STORE_MASKED_H
#define LIBSIMDPP_SIMDPP_CORE_STORE_MASKED_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/store_masked.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Stores the first @a n elements of an 128-bit or 256-bit integer, 32-bit or
    64-bit floating point vector to memory. @a n must be in range [0..N-1]
    where @a N is the number of elements in the vector. If @a n is zero, no
    store is made.

    The pointer must be aligned to the native vector size.
*/
template<class T, unsigned N, class V, class M> SIMDPP_INL
void store_masked(T* p, const any_int32<N,V>& a, const mask_int32<N,M>& mask)
{
    uint32<N> ra;
    ra = a.wrapped().eval();
    detail::insn::i_store_masked(reinterpret_cast<char*>(p), ra, mask.wrapped().eval());
}
template<class T, unsigned N, class V, class M> SIMDPP_INL
void store_masked(T* p, const any_int64<N,V>& a, const mask_int64<N,M>& mask)
{
    uint64<N> ra;
    ra = a.wrapped().eval();
    detail::insn::i_store_masked(reinterpret_cast<char*>(p), ra, mask.wrapped().eval());
}
template<class T, unsigned N, class V, class M> SIMDPP_INL
void store_masked(T* p, const any_float32<N,V>& a, const mask_float32<N,M>& mask)
{
    float32<N> ra;
    ra = a.wrapped().eval();
    detail::insn::i_store_masked(reinterpret_cast<char*>(p), ra, mask.wrapped().eval());
}
template<class T, unsigned N, class V, class M> SIMDPP_INL
void store_masked(T* p, const any_float64<N,V>& a, const mask_float64<N,M>& mask)
{
    float64<N> ra;
    ra = a.wrapped().eval();
    detail::insn::i_store_masked(reinterpret_cast<char*>(p), ra, mask.wrapped().eval());
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

