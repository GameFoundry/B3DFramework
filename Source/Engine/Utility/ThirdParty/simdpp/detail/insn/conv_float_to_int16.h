/*  Copyright (C) 2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_FLOAT_TO_INT16_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_FLOAT_TO_INT16_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/conv_float_to_int32.h>
#include <simdpp/detail/insn/conv_shrink_to_int16.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {


template<unsigned N> SIMDPP_INL
uint16<N> i_to_uint16(const float32<N>& a)
{
    return i_to_uint16( (uint32<N>) i_to_int32(a));
}

template<unsigned N> SIMDPP_INL
uint16<N> i_to_uint16(const float64<N>& a)
{
    return i_to_uint16( (uint32<N>) i_to_int32(a));
}

template<unsigned N> SIMDPP_INL
int16<N> i_to_int16(const float32<N>& a)
{
    return (int16<N>) i_to_uint16((uint32<N>) i_to_int32(a));
}

template<unsigned N> SIMDPP_INL
int16<N> i_to_int16(const float64<N>& a)
{
    return (int16<N>) i_to_uint16((uint32<N>) i_to_int32(a));
}


} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif


