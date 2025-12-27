/*  Copyright (C) 2011-2012  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_WIDTH_H
#define LIBSIMDPP_SIMDPP_DETAIL_WIDTH_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

struct vec128_impl {
    using i8  =  int8x16;
    using u8  = uint8x16;
    using i16 =  int16x8;
    using u16 = uint16x8;
    using i32 =  int32x4;
    using u32 = uint32x4;
    using i64 =  int64x2;
    using u64 = uint64x2;
    using f32 = float32x4;
    using f64 = float64x2;
};

struct vec256_impl {
    using i8  =  int8x32;
    using u8  = uint8x32;
    using i16 =  int16x16;
    using u16 = uint16x16;
    using i32 = int32x8;
    using u32 = uint32x8;
    using i64 = int64x4;
    using u64 = uint64x4;
    using f32 = float32x8;
    using f64 = float64x4;
};

struct vec512_impl {
    using i8  =  int8<64>;
    using u8  = uint8<64>;
    using i16 =  int16<32>;
    using u16 = uint16<32>;
    using i32 =  int32<16>;
    using u32 = uint32<16>;
    using i64 =  int64<8>;
    using u64 = uint64<8>;
    using f32 = float32<16>;
    using f64 = float64<8>;
};

template<class T> struct same_width;
template<> struct same_width<int8x16  > : vec128_impl {};
template<> struct same_width<uint8x16 > : vec128_impl {};
template<> struct same_width<int16x8  > : vec128_impl {};
template<> struct same_width<uint16x8 > : vec128_impl {};
template<> struct same_width<int32x4  > : vec128_impl {};
template<> struct same_width<uint32x4 > : vec128_impl {};
template<> struct same_width<int64x2  > : vec128_impl {};
template<> struct same_width<uint64x2 > : vec128_impl {};
template<> struct same_width<float32x4> : vec128_impl {};
template<> struct same_width<float64x2> : vec128_impl {};

template<> struct same_width< int8x32 > : vec256_impl {};
template<> struct same_width<uint8x32 > : vec256_impl {};
template<> struct same_width< int16x16> : vec256_impl {};
template<> struct same_width<uint16x16> : vec256_impl {};
template<> struct same_width< int32x8 > : vec256_impl {};
template<> struct same_width<uint32x8 > : vec256_impl {};
template<> struct same_width< int64x4 > : vec256_impl {};
template<> struct same_width<uint64x4 > : vec256_impl {};
template<> struct same_width<float32x8> : vec256_impl {};
template<> struct same_width<float64x4> : vec256_impl {};

template<> struct same_width< int8<64>  > : vec512_impl {};
template<> struct same_width<uint8<64>  > : vec512_impl {};
template<> struct same_width< int16<32> > : vec512_impl {};
template<> struct same_width<uint16<32> > : vec512_impl {};
template<> struct same_width< int32<16> > : vec512_impl {};
template<> struct same_width<uint32<16> > : vec512_impl {};
template<> struct same_width< int64<8>  > : vec512_impl {};
template<> struct same_width<uint64<8>  > : vec512_impl {};
template<> struct same_width<float32<16>> : vec512_impl {};
template<> struct same_width<float64<8> > : vec512_impl {};

} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
