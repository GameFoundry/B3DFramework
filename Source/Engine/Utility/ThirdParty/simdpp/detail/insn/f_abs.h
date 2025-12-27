/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_F_ABS_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_F_ABS_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/bit_and.h>
#include <simdpp/detail/null/math.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

static SIMDPP_INL
float32<4> i_fabs(const float32<4>& a)
{
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    return detail::null::abs(a);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_MSA
    return bit_and(a, 0x7fffffff);
#elif SIMDPP_USE_NEON_FLT_SP
    return vabsq_f32(a.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_abs(a.native());
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float32<8> i_fabs(const float32<8>& a)
{
    return bit_and(a, 0x7fffffff);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float32<16> i_fabs(const float32<16>& a)
{
    // TODO: maybe cmp_lt is better, but what to do with negative zero?
    return bit_and(a, 0x7fffffff);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
float64x2 i_fabs(const float64<2>& a)
{
#if SIMDPP_USE_SSE2 || SIMDPP_USE_MSA
    return bit_and(a, 0x7fffffffffffffff);
#elif SIMDPP_USE_NEON64
    return vabsq_f64(a.native());
#elif SIMDPP_USE_VSX_206
    return vec_abs(a.native());
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC
    return detail::null::abs(a);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float64x4 i_fabs(const float64<4>& a)
{
    return bit_and(a, 0x7fffffffffffffff);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float64<8> i_fabs(const float64<8>& a)
{
    // TODO: maybe cmp_lt is better, but what to do with negative zero?
    return bit_and(a, 0x7fffffffffffffff);
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
V i_fabs(const V& a)
{
    SIMDPP_VEC_ARRAY_IMPL1(V, i_fabs, a)
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

