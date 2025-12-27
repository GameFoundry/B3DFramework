/*  Copyright (C) 2013  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_MEM_BLOCK_H
#define LIBSIMDPP_SIMDPP_DETAIL_MEM_BLOCK_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/types/traits.h>
#include <simdpp/core/store.h>
#include <simdpp/core/load.h>
#include <cstring>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

/** A block of memory that stores a vector and allows access to its elements.
    Data transfer is not explicit -- the compiler is allowed to optimize it
    however it wants, failing back to storing and loading from memory, if
    necessary.
*/
template<class V>
class mem_block {
public:
    static_assert(is_vector<V>::value, "Non-vector types are not supported");

    using element_type = typename V::element_type;
    static const unsigned length = V::length;

    SIMDPP_INL mem_block() = default;
    SIMDPP_INL mem_block(const mem_block&) = default;
    SIMDPP_INL mem_block(const V& v) { store(d_, v); }

    SIMDPP_INL mem_block& operator=(const V& v) { store(d_, v); return *this; }

    SIMDPP_INL operator V() const { V r = load(d_); return r; }

    SIMDPP_INL const element_type& operator[](unsigned id) const { return d_[id]; }
    SIMDPP_INL element_type& operator[](unsigned id) { return d_[id]; }

    SIMDPP_INL const element_type* data() const { return d_; }
private:
#if SIMDPP_USE_NEON32
    // On NEON the stack and vector types are not themselves 16-byte aligned
    SIMDPP_ALIGN(16) element_type d_[length];
#else
    union {
        element_type d_[length];
        V align_;
    };
#endif
};

} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

