/*  Copyright (C) 2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_DETAIL_VEC_EXTRACT_H
#define LIBSIMDPP_SIMDPP_CORE_DETAIL_VEC_EXTRACT_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/setup_arch.h>
#include <simdpp/types.h>
#include <simdpp/core/insert.h>
#include <simdpp/core/extract.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

template<class R, class V> SIMDPP_INL
R subvec_extract_impl(const V& a, unsigned n)
{
    static_assert(R::length >= V::base_length, "Too small vector to extract");

    R r;
    for (unsigned i = 0; i < r.vec_length; ++i) {
        r.vec(i) = a.vec(n*r.vec_length + i);
    }
    return r;

}

// extract a sub-vector consisting of [M*n .. M*(n+1)) elements
template<unsigned M, unsigned N> SIMDPP_INL
uint8<M> subvec_extract(const uint8<N>& a, unsigned n)
{
    return subvec_extract_impl<uint8<M>>(a, n);
}

template<unsigned M, unsigned N> SIMDPP_INL
uint16<M> subvec_extract(const uint16<N>& a, unsigned n)
{
    return subvec_extract_impl<uint16<M>>(a, n);
}

template<unsigned M, unsigned N> SIMDPP_INL
uint32<M> subvec_extract(const uint32<N>& a, unsigned n)
{
    return subvec_extract_impl<uint32<M>>(a, n);
}

template<unsigned M, unsigned N> SIMDPP_INL
uint64<M> subvec_extract(const uint64<N>& a, unsigned n)
{
    return subvec_extract_impl<uint64<M>>(a, n);
}

template<unsigned M, unsigned N> SIMDPP_INL
int8<M> subvec_extract(const int8<N>& a, unsigned n)
{
    return subvec_extract_impl<int8<M>>(a, n);
}

template<unsigned M, unsigned N> SIMDPP_INL
int16<M> subvec_extract(const int16<N>& a, unsigned n)
{
    return subvec_extract_impl<int16<M>>(a, n);
}

template<unsigned M, unsigned N> SIMDPP_INL
int32<M> subvec_extract(const int32<N>& a, unsigned n)
{
    return subvec_extract_impl<int32<M>>(a, n);
}

template<unsigned M, unsigned N> SIMDPP_INL
int64<M> subvec_extract(const int64<N>& a, unsigned n)
{
    return subvec_extract_impl<int64<M>>(a, n);
}

template<unsigned M, unsigned N> SIMDPP_INL
float32<M> subvec_extract(const float32<N>& a, unsigned n)
{
    return subvec_extract_impl<float32<M>>(a, n);
}

template<unsigned M, unsigned N> SIMDPP_INL
float64<M> subvec_extract(const float64<N>& a, unsigned n)
{
    return subvec_extract_impl<float64<M>>(a, n);
}


} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
