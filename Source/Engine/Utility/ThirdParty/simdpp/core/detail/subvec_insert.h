/*  Copyright (C) 2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_DETAIL_VEC_INSERT_H
#define LIBSIMDPP_SIMDPP_CORE_DETAIL_VEC_INSERT_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/setup_arch.h>
#include <simdpp/types.h>

#include <cstring>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

template<class R, class V> SIMDPP_INL
void subvec_insert_impl(R& r, const V& v, unsigned n)
{
    static_assert(V::length >= R::base_length, "Too small vector to insert");

    for (unsigned i = 0; i < V::vec_length; ++i) {
        r.vec(n*v.vec_length + i) = v.vec(i); //TODO combine or split as needed
    }
}

// Sets the elements [M*n .. M*(n+1)) of @a a to the contents of @a x
template<unsigned N, unsigned M> SIMDPP_INL
void subvec_insert(uint8<N>& a, const uint8<M>& x, unsigned n) { subvec_insert_impl(a, x, n); }
template<unsigned N, unsigned M> SIMDPP_INL
void subvec_insert(uint16<N>& a, const uint16<M>& x, unsigned n) { subvec_insert_impl(a, x, n); }
template<unsigned N, unsigned M> SIMDPP_INL
void subvec_insert(uint32<N>& a, const uint32<M>& x, unsigned n) { subvec_insert_impl(a, x, n); }
template<unsigned N, unsigned M> SIMDPP_INL
void subvec_insert(uint64<N>& a, const uint64<M>& x, unsigned n) { subvec_insert_impl(a, x, n); }

template<unsigned N, unsigned M> SIMDPP_INL
void subvec_insert(int8<N>& a, const int8<M>& x, unsigned n) { subvec_insert_impl(a, x, n); }
template<unsigned N, unsigned M> SIMDPP_INL
void subvec_insert(int16<N>& a, const int16<M>& x, unsigned n) { subvec_insert_impl(a, x, n); }
template<unsigned N, unsigned M> SIMDPP_INL
void subvec_insert(int32<N>& a, const int32<M>& x, unsigned n) { subvec_insert_impl(a, x, n); }
template<unsigned N, unsigned M> SIMDPP_INL
void subvec_insert(int64<N>& a, const int64<M>& x, unsigned n) { subvec_insert_impl(a, x, n); }

template<unsigned N, unsigned M> SIMDPP_INL
void subvec_insert(float32<N>& a, const float32<M>& x, unsigned n) { subvec_insert_impl(a, x, n); }
template<unsigned N, unsigned M> SIMDPP_INL
void subvec_insert(float64<N>& a, const float64<M>& x, unsigned n) { subvec_insert_impl(a, x, n); }

} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
