/*  Copyright (C) 2013  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_ALIGN_V128_H
#define LIBSIMDPP_SIMDPP_DETAIL_ALIGN_V128_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/align.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

template<unsigned shift, unsigned N> SIMDPP_INL
uint8<N> align_v128(const uint8<N>& a, const uint8<N>& b) { return align16<shift>(a, b); }
template<unsigned shift, unsigned N> SIMDPP_INL
uint16<N> align_v128(const uint16<N>& a, const uint16<N>& b) { return align8<shift>(a, b); }
template<unsigned shift, unsigned N> SIMDPP_INL
uint32<N> align_v128(const uint32<N>& a, const uint32<N>& b) { return align4<shift>(a, b); }
template<unsigned shift, unsigned N> SIMDPP_INL
uint64<N> align_v128(const uint64<N>& a, const uint64<N>& b) { return align2<shift>(a, b); }

template<unsigned shift, unsigned N> SIMDPP_INL
int8<N> align_v128(const int8<N>& a, const int8<N>& b) { return align16<shift>(a, b); }
template<unsigned shift, unsigned N> SIMDPP_INL
int16<N> align_v128(const int16<N>& a, const int16<N>& b) { return align8<shift>(a, b); }
template<unsigned shift, unsigned N> SIMDPP_INL
int32<N> align_v128(const int32<N>& a, const int32<N>& b) { return align4<shift>(a, b); }
template<unsigned shift, unsigned N> SIMDPP_INL
int64<N> align_v128(const int64<N>& a, const int64<N>& b) { return align2<shift>(a, b); }

template<unsigned shift, unsigned N> SIMDPP_INL
float32<N> align_v128(const float32<N>& a, const float32<N>& b) { return align4<shift>(a, b); }
template<unsigned shift, unsigned N> SIMDPP_INL
float64<N> align_v128(const float64<N>& a, const float64<N>& b) { return align2<shift>(a, b); }

} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

