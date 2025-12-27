/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMD_INSERT_H
#define LIBSIMDPP_SIMD_INSERT_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/insert.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {


/** Inserts an element into a vector at the position identified by @a id.

    @code
    r0 = (id == 0) ? x : a0
    ...
    rN = (id == N) ? x : aN
    @endcode

    This function may have very high latency.
    It is expected that the argument comes from a general-purpose register.
*/
template<unsigned id, unsigned N> SIMDPP_INL
uint8<N> insert(const uint8<N>& a, uint8_t x)
{
    static_assert(id < N, "index out of bounds");
    return detail::insn::i_insert<id>(a, x);
}

template<unsigned id, unsigned N> SIMDPP_INL
int8<N> insert(const int8<N>& a, int8_t x)
{
    static_assert(id < N, "index out of bounds");
    return (int8<N>) detail::insn::i_insert<id>((uint8<N>) a, (uint8_t)x);
}

template<unsigned id, unsigned N> SIMDPP_INL
uint16<N> insert(const uint16<N>& a, uint16_t x)
{
    static_assert(id < N, "index out of bounds");
    return detail::insn::i_insert<id>(a, x);
}

template<unsigned id, unsigned N> SIMDPP_INL
int16<N> insert(const int16<N>& a, int16_t x)
{
    static_assert(id < N, "index out of bounds");
    return (int16<N>) detail::insn::i_insert<id>((uint16<N>) a, (uint16_t)x);
}

template<unsigned id, unsigned N> SIMDPP_INL
uint32<N> insert(const uint32<N>& a, uint32_t x)
{
    static_assert(id < N, "index out of bounds");
    return detail::insn::i_insert<id>(a, x);
}

template<unsigned id, unsigned N> SIMDPP_INL
int32<N> insert(const int32<N>& a, int32_t x)
{
    static_assert(id < N, "index out of bounds");
    return (int32<N>) detail::insn::i_insert<id>((uint32<N>)a, (uint32_t)x);
}

template<unsigned id, unsigned N> SIMDPP_INL
uint64<N> insert(const uint64<N>& a, uint64_t x)
{
    static_assert(id < N, "index out of bounds");
    return detail::insn::i_insert<id>(a, x);
}

template<unsigned id, unsigned N> SIMDPP_INL
int64<N> insert(const int64<N>& a, int64_t x)
{
    static_assert(id < N, "index out of bounds");
    return (int64<N>) detail::insn::i_insert<id>((uint64<N>)a, (uint64_t)x);
}

template<unsigned id, unsigned N> SIMDPP_INL
float32<N> insert(const float32<N>& a, float x)
{
    static_assert(id < N, "index out of bounds");
    return detail::insn::i_insert<id>(a, x);
}

template<unsigned id, unsigned N> SIMDPP_INL
float64<N> insert(const float64<N>& a, double x)
{
    static_assert(id < N, "index out of bounds");
    return detail::insn::i_insert<id>(a, x);
}


} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
