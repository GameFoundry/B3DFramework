/*  Copyright (C) 2015  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_PREPROCESSOR_H
#define LIBSIMDPP_SIMDPP_DETAIL_PREPROCESSOR_H

// This file contains several common preprocessor utilities

// Concatenates x1 and x2. The concatenation is performed before the arguments
// are evaluated
#define SIMDPP_PP_CAT22(x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18, x19, x20, x21, x22) \
    x1 ## x2 ## x3 ## x4 ## x5 ## x6 ## x7 ## x8 ## x9 ## x10 ## x11 ## x12 ## x13 ## x14 ## x15 ## x16 ## x17 ## x18 ## x19 ## x20 ## x21 ## x22

// Evaluates the arguments and concatenates the result
#define SIMDPP_PP_PASTE22(x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18, x19, x20, x21, x22) \
    SIMDPP_PP_CAT22(x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18, x19, x20, x21, x22)

#endif

