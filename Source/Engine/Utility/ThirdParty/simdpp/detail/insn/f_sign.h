/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_F_SIGN_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_F_SIGN_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/bit_and.h>
#include <simdpp/detail/null/math.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {


template<unsigned N> SIMDPP_INL
float32<N> i_sign(const float32<N>& a)
{
    return bit_and(a, 0x80000000);
}

template<unsigned N> SIMDPP_INL
float64<N> i_sign(const float64<N>& a)
{
    return bit_and(a, 0x8000000000000000);
}


} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

