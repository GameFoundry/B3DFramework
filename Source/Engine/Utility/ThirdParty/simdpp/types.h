/*  Copyright (C) 2012  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_TYPES_H
#define LIBSIMDPP_SIMDPP_TYPES_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif
#include <simdpp/setup_arch.h>

#include <simdpp/expr.h>
#include <simdpp/types/fwd.h>
#include <simdpp/types/any.h>
#include <simdpp/types/int8x16.h>
#include <simdpp/types/int8x32.h>
#include <simdpp/types/int8x64.h>
#include <simdpp/types/int16x8.h>
#include <simdpp/types/int16x16.h>
#include <simdpp/types/int16x32.h>
#include <simdpp/types/int32x4.h>
#include <simdpp/types/int32x8.h>
#include <simdpp/types/int32x16.h>
#include <simdpp/types/int64x2.h>
#include <simdpp/types/int64x4.h>
#include <simdpp/types/int64x8.h>
#include <simdpp/types/float32x4.h>
#include <simdpp/types/float32x8.h>
#include <simdpp/types/float32x16.h>
#include <simdpp/types/float64x2.h>
#include <simdpp/types/float64x4.h>
#include <simdpp/types/float64x8.h>
#include <simdpp/types/int8.h>
#include <simdpp/types/int16.h>
#include <simdpp/types/int32.h>
#include <simdpp/types/int64.h>
#include <simdpp/types/float32.h>
#include <simdpp/types/float64.h>
#include <simdpp/types/generic.h>
#include <simdpp/types/empty_expr.h>

#endif
