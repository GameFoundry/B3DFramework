/*  Copyright (C) 2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_VECTOR_ARRAY_CONV_MACROS_H
#define LIBSIMDPP_SIMDPP_DETAIL_VECTOR_ARRAY_CONV_MACROS_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/detail/subvec_insert.h>
#include <simdpp/core/detail/subvec_extract.h>

// Used when the native vector of source vector corresponds to multiple native
// vectors in the destination vector. This happens when a vector type of
// smaller element size is converted to a vector type with larger element size
#define SIMDPP_VEC_ARRAY_IMPL_CONV_INSERT(RTYPE, OP, A)                         \
    RTYPE r; for (unsigned i = 0; i < (A).vec_length; ++i) {                    \
        detail::subvec_insert(r, OP(a.vec(i)), i); }                            \
    return r;

// Used when the native vector of destination vector corresponds to multiple
// native vectors in the source vector. This happens when a vector type of
// larger element size is converted to a vector type with smaller element size.
#define SIMDPP_VEC_ARRAY_IMPL_CONV_EXTRACT(RTYPE, OP, A)                        \
    RTYPE r; for (unsigned i = 0; i < r.vec_length; ++i) {                      \
        r.vec(i) = OP(detail::subvec_extract<RTYPE::base_length>((A), i)); }    \
    return r;

#endif
