/*  Copyright (C) 2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_TO_MASK_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_TO_MASK_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

static SIMDPP_INL
mask_int8<16> i_to_mask(const uint8<16>& a);

#if SIMDPP_USE_AVX2
static SIMDPP_INL
mask_int8<32> i_to_mask(const uint8<32>& a);
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
mask_int8<64> i_to_mask(const uint8<64>& a);
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
mask_int16<8> i_to_mask(const uint16<8>& a);

#if SIMDPP_USE_AVX2
static SIMDPP_INL
mask_int16<16> i_to_mask(const uint16<16>& a);
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
mask_int16<32> i_to_mask(const uint16<32>& a);
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
mask_int32<4> i_to_mask(const uint32<4>& a);

#if SIMDPP_USE_AVX2
static SIMDPP_INL
mask_int32<8> i_to_mask(const uint32<8>& a);
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
mask_int32<16> i_to_mask(const uint32<16>& a);
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
mask_int64<2> i_to_mask(const uint64<2>& a);

#if SIMDPP_USE_AVX2
static SIMDPP_INL
mask_int64<4> i_to_mask(const uint64<4>& a);
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
mask_int64<8> i_to_mask(const uint64<8>& a);
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
mask_float32<4> i_to_mask(const float32<4>& a);

#if SIMDPP_USE_AVX2
static SIMDPP_INL
mask_float32<8> i_to_mask(const float32<8>& a);
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
mask_float32<16> i_to_mask(const float32<16>& a);
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
mask_float64<2> i_to_mask(const float64<2>& a);

#if SIMDPP_USE_AVX2
static SIMDPP_INL
mask_float64<4> i_to_mask(const float64<4>& a);
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
mask_float64<8> i_to_mask(const float64<8>& a);
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
typename V::mask_vector_type i_to_mask(const V& a);

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif


