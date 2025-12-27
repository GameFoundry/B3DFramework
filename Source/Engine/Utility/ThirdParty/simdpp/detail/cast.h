/*  Copyright (C) 2013  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_DETAIL_CAST_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_DETAIL_CAST_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/setup_arch.h>
#include <simdpp/core/cast.h>

#include <cstring>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

enum {
    CAST_MASK_MEMCPY,
    CAST_MASK_UNMASK,
    CAST_MASK_REMASK
};

enum {
    CAST_TYPE_OTHER,
    CAST_TYPE_VECTOR_TO_VECTOR,
    CAST_TYPE_MASK_TO_VECTOR,
    CAST_TYPE_VECTOR_TO_MASK,
    CAST_TYPE_MASK_TO_MASK_BITWISE,
    CAST_TYPE_MASK_TO_MASK_UNMASK,
    CAST_TYPE_MASK_TO_MASK_REMASK
};

// defined in cast.inl
template<unsigned CastType> struct cast_wrapper;


} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
