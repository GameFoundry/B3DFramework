/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_SPLIT_H
#define LIBSIMDPP_SIMDPP_CORE_SPLIT_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/split.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

namespace detail {

template<class A, class R> SIMDPP_INL
void v256_split(A a, R& r1, R& r2)
{
#if SIMDPP_USE_AVX2
    r1 = _mm256_extracti128_si256(a.native(), 0);
    r2 = _mm256_extracti128_si256(a.native(), 1);
#else
    r1 = a[0];
    r2 = a[1];
#endif
}

} // namespcae detail

/** Splits a vector into two vectors half as wide. This function is useful when
    the ISA supports multiple vector sizes and the user does some operations
    with vectors that are narrower than the widest native vector.

    For example, on AVX, a __m256 vector can be split into two __m128 vectors.

    @code
    [ r1, r2 ] = a
    @endcode

    @icost{AVX2, 1}
    @icost{SSE2-AVX, NEON, ALTIVEC, 0}
*/
template<unsigned N> SIMDPP_INL
void split(const uint8<N>& a, uint8<N/2>& r1, uint8<N/2>& r2)
{
    detail::insn::i_split(a, r1, r2);
}

template<unsigned N> SIMDPP_INL
void split(const uint16<N>& a, uint16<N/2>& r1, uint16<N/2>& r2)
{
    detail::insn::i_split(a, r1, r2);
}

template<unsigned N> SIMDPP_INL
void split(const uint32<N>& a, uint32<N/2>& r1, uint32<N/2>& r2)
{
    detail::insn::i_split(a, r1, r2);
}

template<unsigned N> SIMDPP_INL
void split(const uint64<N>& a, uint64<N/2>& r1, uint64<N/2>& r2)
{
    detail::insn::i_split(a, r1, r2);
}

template<unsigned N> SIMDPP_INL
void split(const int8<N>& a, int8<N/2>& r1, int8<N/2>& r2)
{
    uint8<N/2> q1, q2;  q1 = r1;  q2 = r2;
    detail::insn::i_split(uint8<N>(a), q1, q2);
    r1 = q1;  r2 = q2;
}

template<unsigned N> SIMDPP_INL
void split(const int16<N>& a, int16<N/2>& r1, int16<N/2>& r2)
{
    uint16<N/2> q1, q2;  q1 = r1;  q2 = r2;
    detail::insn::i_split(uint16<N>(a), q1, q2);
    r1 = q1;  r2 = q2;
}

template<unsigned N> SIMDPP_INL
void split(const int32<N>& a, int32<N/2>& r1, int32<N/2>& r2)
{
    uint32<N/2> q1, q2;  q1 = r1;  q2 = r2;
    detail::insn::i_split(uint32<N>(a), q1, q2);
    r1 = q1;  r2 = q2;
}

template<unsigned N> SIMDPP_INL
void split(const int64<N>& a, int64<N/2>& r1, int64<N/2>& r2)
{
    uint64<N/2> q1, q2;  q1 = r1;  q2 = r2;
    detail::insn::i_split(uint64<N>(a), q1, q2);
    r1 = q1;  r2 = q2;
}

template<unsigned N> SIMDPP_INL
void split(const float32<N>& a, float32<N/2>& r1, float32<N/2>& r2)
{
    detail::insn::i_split(a, r1, r2);
}

template<unsigned N> SIMDPP_INL
void split(const float64<N>& a, float64<N/2>& r1, float64<N/2>& r2)
{
    detail::insn::i_split(a, r1, r2);
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
