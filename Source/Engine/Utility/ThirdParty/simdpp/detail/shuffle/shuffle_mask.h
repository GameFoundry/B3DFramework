/*  Copyright (C) 2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_SHUFFLE_SHUFFLE_MASK_H
#define LIBSIMDPP_SIMDPP_DETAIL_SHUFFLE_SHUFFLE_MASK_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#if SIMDPP_LITTLE_ENDIAN
#define SIMDPP_VSX_SHUFFLE_MASK_2x2(x0, x1) ((((x0) << 1) | (x1)) & 0x3)
#else
#define SIMDPP_VSX_SHUFFLE_MASK_2x2(x0, x1) ((((x1) << 1) | (x0)) & 0x3)
#endif

#define SIMDPP_SHUFFLE_MASK_2x2(x0, x1) ((((x1) << 1) | (x0)) & 0x3)

#define SIMDPP_SHUFFLE_MASK_2x2_2(x0, x1)                                       \
    ((SIMDPP_SHUFFLE_MASK_2x2(x0, x1) << 2) |                                   \
      SIMDPP_SHUFFLE_MASK_2x2(x0, x1))

#define SIMDPP_SHUFFLE_MASK_2x2_4(x0, x1)                                       \
    ((SIMDPP_SHUFFLE_MASK_2x2_2(x0, x1) << 4) |                                 \
      SIMDPP_SHUFFLE_MASK_2x2_2(x0, x1))

#define SIMDPP_SHUFFLE_MASK_4x2(x0, x1, x2, x3)                                 \
    ((((x3) << 3) | ((x2) << 2) | ((x1) << 1) | (x0)) & 0xf)

#define SIMDPP_SHUFFLE_MASK_4x2_2(x0, x1, x2, x3)                               \
    ((SIMDPP_SHUFFLE_MASK_4x2(x0, x1, x2, x3) << 4) |                           \
      SIMDPP_SHUFFLE_MASK_4x2(x0, x1, x2, x3))

#define SIMDPP_SHUFFLE_MASK_4x2_4(x0, x1, x2, x3)                               \
    ((SIMDPP_SHUFFLE_MASK_4x2_2(x0, x1, x2, x3) << 8) |                         \
      SIMDPP_SHUFFLE_MASK_4x2_2(x0, x1, x2, x3))

#define SIMDPP_SHUFFLE_MASK_4x4(x0, x1, x2, x3)                                 \
    ((((x3) << 6) | ((x2) << 4) | ((x1) << 2) | (x0)) & 0xff)

#endif
