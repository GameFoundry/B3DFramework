/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_F_NEG_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_F_NEG_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/bit_xor.h>
#include <simdpp/detail/null/math.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

static SIMDPP_INL
float32<4> i_fneg(const float32<4>& a)
{
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    return detail::null::neg(a);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    // reversion of the sign bit required even for NaNs and zeros
    return bit_xor(a, 0x80000000);
#elif SIMDPP_USE_NEON_FLT_SP
    return vnegq_f32(a.native());
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float32<8> i_fneg(const float32<8>& a)
{
    return bit_xor(a, 0x80000000);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float32<16> i_fneg(const float32<16>& a)
{
    // FIXME: check whether we can simply use sub
    return bit_xor(a, 0x80000000);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
float64x2 i_fneg(const float64<2>& a)
{
#if SIMDPP_USE_SSE2 || SIMDPP_USE_VSX_206 || SIMDPP_USE_MSA
    return bit_xor(a, 0x8000000000000000);
#elif SIMDPP_USE_NEON64
    return vnegq_f64(a.native());
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC
    return detail::null::neg(a);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float64x4 i_fneg(const float64<4>& a)
{
    return bit_xor(a, 0x8000000000000000);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float64<8> i_fneg(const float64<8>& a)
{
    // FIXME: check whether we can simply use sub
    return bit_xor(a, 0x8000000000000000);
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
V i_fneg(const V& a)
{
    SIMDPP_VEC_ARRAY_IMPL1(V, i_fneg, a)
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

