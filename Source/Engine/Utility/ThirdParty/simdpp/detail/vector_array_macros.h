/*  Copyright (C) 2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_VECTOR_ARRAY_MACROS_H
#define LIBSIMDPP_SIMDPP_DETAIL_VECTOR_ARRAY_MACROS_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>

#define SIMDPP_VEC_ARRAY_IMPL1(RTYPE, OP, V1)                   \
    RTYPE r; for (unsigned i = 0; i < r.vec_length; ++i) { \
        r.vec(i) = OP((V1).vec(i)); }                           \
    return r;

#define SIMDPP_VEC_ARRAY_IMPL2(RTYPE, OP, V1, V2)               \
    RTYPE r; for (unsigned i = 0; i < r.vec_length; ++i) { \
        r.vec(i) = OP((V1).vec(i), (V2).vec(i)); }              \
    return r;

#define SIMDPP_VEC_ARRAY_IMPL2S(RTYPE, OP, V1, A2)              \
    RTYPE r; for (unsigned i = 0; i < r.vec_length; ++i) { \
        r.vec(i) = OP((V1).vec(i), (A2)); }                     \
    return r;

#define SIMDPP_VEC_ARRAY_IMPL3(RTYPE, OP, V1, V2, V3)           \
    RTYPE r; for (unsigned i = 0; i < r.vec_length; ++i) { \
        r.vec(i) = OP((V1).vec(i), (V2).vec(i), (V3).vec(i));   \
    }                                                           \
    return r;

#define SIMDPP_VEC_ARRAY_IMPL_REF1(RTYPE, OP, V1)               \
    for (unsigned i = 0; i < RTYPE::vec_length; ++i) {          \
        OP((V1).vec(i)); }

#define SIMDPP_VEC_ARRAY_IMPL_REF2(RTYPE, OP, V1, V2)           \
    for (unsigned i = 0; i < RTYPE::vec_length; ++i) {          \
        OP((V1).vec(i), (V2).vec(i)); }

#define SIMDPP_VEC_ARRAY_IMPL_REF3(RTYPE, OP, V1, V2, V3)       \
    for (unsigned i = 0; i < RTYPE::vec_length; ++i) {          \
        OP((V1).vec(i), (V2).vec(i), (V3).vec(i)); }

#define SIMDPP_VEC_ARRAY_IMPL_REF4(RTYPE, OP, V1, V2, V3, V4)   \
    for (unsigned i = 0; i < RTYPE::vec_length; ++i) {          \
        OP((V1).vec(i), (V2).vec(i), (V3).vec(i), (V4).vec(i)); }

#endif
