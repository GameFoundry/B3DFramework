/*  Copyright (C) 2012-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_TYPES_FWD_H
#define LIBSIMDPP_SIMDPP_TYPES_FWD_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif
#include <simdpp/setup_arch.h>
#include <simdpp/types/tag.h>

/*  This is an internal helper file that contains forward declarations of the
    vector templates and typedefs of specific vector types urcoah
*/
namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

namespace detail {
//template<int> void construct_eval();
} // namespace detail

// types
template<unsigned N, class E = void> class float32;
template<unsigned N, class E = void> class mask_float32;

using float32x4 = float32<4>;
using float32x8 = float32<8>;
using mask_float32x4 = mask_float32<4>;
using mask_float32x8 = mask_float32<8>;

template<unsigned N, class E = void> class float64;
template<unsigned N, class E = void> class mask_float64;

using float64x2 = float64<2>;
using float64x4 = float64<4>;
using mask_float64x2 = mask_float64<2>;
using mask_float64x4 = mask_float64<4>;

template<unsigned N, class E = void> class int8;
template<unsigned N, class E = void> class uint8;
template<unsigned N, class E = void> class mask_int8;

using int8x16 = int8<16>;
using int8x32 = int8<32>;
using uint8x16 = uint8<16>;
using uint8x32 = uint8<32>;
using mask_int8x16 = mask_int8<16>;
using mask_int8x32 = mask_int8<32>;

template<unsigned N, class E = void> class int16;
template<unsigned N, class E = void> class uint16;
template<unsigned N, class E = void> class mask_int16;

using int16x8 = int16<8>;
using int16x16 = int16<16>;
using uint16x8 = uint16<8>;
using uint16x16 = uint16<16>;
using mask_int16x8 = mask_int16<8>;
using mask_int16x16 = mask_int16<16>;

template<unsigned N, class E = void> class int32;
template<unsigned N, class E = void> class uint32;
template<unsigned N, class E = void> class mask_int32;

using int32x4 = int32<4>;
using int32x8 = int32<8>;
using uint32x4 = uint32<4>;
using uint32x8 = uint32<8>;
using mask_int32x4 = mask_int32<4>;
using mask_int32x8 = mask_int32<8>;

template<unsigned N, class E = void> class int64;
template<unsigned N, class E = void> class uint64;
template<unsigned N, class E = void> class mask_int64;

using int64x2 = int64<2>;
using int64x4 = int64<4>;
using uint64x2 = uint64<2>;
using uint64x4 = uint64<4>;
using mask_int64x2 = mask_int64<2>;
using mask_int64x4 = mask_int64<4>;

#if SIMDPP_USE_AVX512F
#define SIMDPP_FAST_FLOAT32_SIZE 16
#define SIMDPP_FAST_FLOAT64_SIZE 8
#elif SIMDPP_USE_AVX
#define SIMDPP_FAST_FLOAT32_SIZE 8
#define SIMDPP_FAST_FLOAT64_SIZE 4
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA || SIMDPP_USE_NULL
#define SIMDPP_FAST_FLOAT32_SIZE 4
#define SIMDPP_FAST_FLOAT64_SIZE 2
#endif

#if SIMDPP_USE_AVX512BW
#define SIMDPP_FAST_INT8_SIZE 64
#define SIMDPP_FAST_INT16_SIZE 32
#define SIMDPP_FAST_INT32_SIZE 16
#define SIMDPP_FAST_INT64_SIZE 8
#elif SIMDPP_USE_AVX512F
#define SIMDPP_FAST_INT8_SIZE 32
#define SIMDPP_FAST_INT16_SIZE 16
#define SIMDPP_FAST_INT32_SIZE 16
#define SIMDPP_FAST_INT64_SIZE 8
#elif SIMDPP_USE_AVX2
#define SIMDPP_FAST_INT8_SIZE 32
#define SIMDPP_FAST_INT16_SIZE 16
#define SIMDPP_FAST_INT32_SIZE 8
#define SIMDPP_FAST_INT64_SIZE 4
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA || SIMDPP_USE_NULL
#define SIMDPP_FAST_INT8_SIZE 16
#define SIMDPP_FAST_INT16_SIZE 8
#define SIMDPP_FAST_INT32_SIZE 4
#define SIMDPP_FAST_INT64_SIZE 2
#endif

using float32v =           float32<SIMDPP_FAST_FLOAT32_SIZE>;
using mask_float32v = mask_float32<SIMDPP_FAST_FLOAT32_SIZE>;
using float64v =           float64<SIMDPP_FAST_FLOAT64_SIZE>;
using mask_float64v = mask_float64<SIMDPP_FAST_FLOAT64_SIZE>;

using int8v =           int8<SIMDPP_FAST_INT8_SIZE>;
using uint8v =         uint8<SIMDPP_FAST_INT8_SIZE>;
using mask_int8v = mask_int8<SIMDPP_FAST_INT8_SIZE>;

using int16v =           int16<SIMDPP_FAST_INT16_SIZE>;
using uint16v =         uint16<SIMDPP_FAST_INT16_SIZE>;
using mask_int16v = mask_int16<SIMDPP_FAST_INT16_SIZE>;

using int32v =           int32<SIMDPP_FAST_INT32_SIZE>;
using uint32v =         uint32<SIMDPP_FAST_INT32_SIZE>;
using mask_int32v = mask_int32<SIMDPP_FAST_INT32_SIZE>;

using int64v =           int64<SIMDPP_FAST_INT64_SIZE>;
using uint64v =         uint64<SIMDPP_FAST_INT64_SIZE>;
using mask_int64v = mask_int64<SIMDPP_FAST_INT64_SIZE>;

using mask_float32v2 = mask_float32<SIMDPP_FAST_FLOAT32_SIZE*2>;
using float64v2 =           float64<SIMDPP_FAST_FLOAT64_SIZE*2>;
using mask_float64v2 = mask_float64<SIMDPP_FAST_FLOAT64_SIZE*2>;

using int8v2 =           int8<SIMDPP_FAST_INT8_SIZE*2>;
using uint8v2 =         uint8<SIMDPP_FAST_INT8_SIZE*2>;
using mask_int8v2 = mask_int8<SIMDPP_FAST_INT8_SIZE*2>;

using int16v2 =           int16<SIMDPP_FAST_INT16_SIZE*2>;
using uint16v2 =         uint16<SIMDPP_FAST_INT16_SIZE*2>;
using mask_int16v2 = mask_int16<SIMDPP_FAST_INT16_SIZE*2>;

using int32v2 =           int32<SIMDPP_FAST_INT32_SIZE*2>;
using uint32v2 =         uint32<SIMDPP_FAST_INT32_SIZE*2>;
using mask_int32v2 = mask_int32<SIMDPP_FAST_INT32_SIZE*2>;

using int64v2 =           int64<SIMDPP_FAST_INT64_SIZE*2>;
using uint64v2 =         uint64<SIMDPP_FAST_INT64_SIZE*2>;
using mask_int64v2 = mask_int64<SIMDPP_FAST_INT64_SIZE*2>;


using float32v4 =           float32<SIMDPP_FAST_FLOAT32_SIZE*4>;
using mask_float32v4 = mask_float32<SIMDPP_FAST_FLOAT32_SIZE*4>;
using float64v4 =           float64<SIMDPP_FAST_FLOAT64_SIZE*4>;
using mask_float64v4 = mask_float64<SIMDPP_FAST_FLOAT64_SIZE*4>;

using int8v4 =           int8<SIMDPP_FAST_INT8_SIZE*4>;
using uint8v4 =         uint8<SIMDPP_FAST_INT8_SIZE*4>;
using mask_int8v4 = mask_int8<SIMDPP_FAST_INT8_SIZE*4>;

using int16v4 =           int16<SIMDPP_FAST_INT16_SIZE*4>;
using uint16v4 =         uint16<SIMDPP_FAST_INT16_SIZE*4>;
using mask_int16v4 = mask_int16<SIMDPP_FAST_INT16_SIZE*4>;

using int32v4 =           int32<SIMDPP_FAST_INT32_SIZE*4>;
using uint32v4 =         uint32<SIMDPP_FAST_INT32_SIZE*4>;
using mask_int32v4 = mask_int32<SIMDPP_FAST_INT32_SIZE*4>;

using int64v4 =           int64<SIMDPP_FAST_INT64_SIZE*4>;
using uint64v4 =         uint64<SIMDPP_FAST_INT64_SIZE*4>;
using mask_int64v4 = mask_int64<SIMDPP_FAST_INT64_SIZE*4>;

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
