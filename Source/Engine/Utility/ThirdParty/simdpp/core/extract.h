/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMD_EXTRACT_H
#define LIBSIMDPP_SIMD_EXTRACT_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/extract.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Extracts the @a id-th element from a vector.

    @code
    r = a[id]
    @endcode

    This function may have very high latency.
*/
template<unsigned id, unsigned N> SIMDPP_INL
uint8_t extract(const uint8<N>& a)
{
    static_assert(id < N, "index out of bounds");
    return detail::insn::i_extract<id>(a);
}

template<unsigned id, unsigned N> SIMDPP_INL
int8_t extract(const int8<N>& a)
{
    static_assert(id < N, "index out of bounds");
    return detail::insn::i_extract<id>(a);
}

template<unsigned id, unsigned N> SIMDPP_INL
uint16_t extract(const uint16<N>& a)
{
    static_assert(id < N, "index out of bounds");
    return detail::insn::i_extract<id>(a);
}

template<unsigned id, unsigned N> SIMDPP_INL
int16_t extract(const int16<N>& a)
{
    static_assert(id < N, "index out of bounds");
    return detail::insn::i_extract<id>(a);
}

template<unsigned id, unsigned N> SIMDPP_INL
uint32_t extract(const uint32<N>& a)
{
    static_assert(id < N, "index out of bounds");
    return detail::insn::i_extract<id>(a);
}

template<unsigned id, unsigned N> SIMDPP_INL
int32_t extract(const int32<N>& a)
{
    static_assert(id < N, "index out of bounds");
    return detail::insn::i_extract<id>(a);
}

template<unsigned id, unsigned N> SIMDPP_INL
uint64_t extract(const uint64<N>& a)
{
    static_assert(id < N, "index out of bounds");
    return detail::insn::i_extract<id>(a);
}

template<unsigned id, unsigned N> SIMDPP_INL
int64_t extract(const int64<N>& a)
{
    static_assert(id < N, "index out of bounds");
    return detail::insn::i_extract<id>(a);
}

template<unsigned id, unsigned N> SIMDPP_INL
float extract(const float32<N>& a)
{
    static_assert(id < N, "index out of bounds");
    return detail::insn::i_extract<id>(a);
}

template<unsigned id, unsigned N> SIMDPP_INL
double extract(const float64<N>& a)
{
    static_assert(id < N, "index out of bounds");
    return detail::insn::i_extract<id>(a);
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

