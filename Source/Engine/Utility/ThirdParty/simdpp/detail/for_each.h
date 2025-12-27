/*  Copyright (C) 2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_DETAIL_FOR_EACH_H
#define LIBSIMDPP_DETAIL_FOR_EACH_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/extract.h>
#include <simdpp/detail/mem_block.h>
#include <type_traits>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

#if SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_MSA
template<class V, class F> SIMDPP_INL
void foreach_impl(std::integral_constant<unsigned, 2>,
                  const V& v, F function)
{
    function(extract<0>(v));
    function(extract<1>(v));
}

template<class V, class F> SIMDPP_INL
void foreach_impl(std::integral_constant<unsigned, 4>,
                  const V& v, F function)
{
    function(extract<0>(v));
    function(extract<1>(v));
    function(extract<2>(v));
    function(extract<3>(v));
}

template<unsigned N, class V, class F> SIMDPP_INL
void foreach_impl(std::integral_constant<unsigned, N>,
                  const V& v, F function)
{
    // When we're operating on more than 4-5 elements it makes sense to move
    // the vector to memory and load data from there. This has higher latency,
    // but this is masked by extracting the first several elements directly
    // from the SIMD register set. For the rest of elements it's very likely
    // that loading through memory has higher throughput.
    //
    // Recent x86 (since Sandy Bridge) and NEON (since Cortex A73) processors
    // are able to sustain more than one load memory access per cycle.
    // All x86 processors (at least up to Skylake, newer not checked) are only
    // able to sustain single cross domain data access instruction per cycle.

    // TODO: needs tuning on ARM and MIPS
    function(extract<0>(v));
    function(extract<1>(v));
    mem_block<V> mem(v);
    for (unsigned i = 2; i < N; ++i)
        function(mem[i]);
}
#else
template<unsigned N, class V, class F> SIMDPP_INL
void foreach_impl(std::integral_constant<unsigned, N>,
                  const V& v, F function)
{
    mem_block<V> mem(v);
    for (unsigned i = 0; i < N; ++i)
        function(mem[i]);
}
#endif

template<unsigned N, class V, class F> SIMDPP_INL
void for_each(const any_vec<N, V>& v, F function)
{
    using size_tag = std::integral_constant<unsigned, V::base_vector_type::length>;
    for (unsigned i = 0; i < V::vec_length; ++i)
        foreach_impl(size_tag(), v.wrapped().vec(i), function);
}


} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
