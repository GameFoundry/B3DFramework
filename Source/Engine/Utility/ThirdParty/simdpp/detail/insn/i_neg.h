/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_I_NEG_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_I_NEG_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/i_sub.h>
#include <simdpp/detail/null/math.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

static SIMDPP_INL
int8<16> i_ineg(const int8<16>& a)
{
#if SIMDPP_USE_NULL
    return detail::null::neg(a);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    return sub(0, a);
#elif SIMDPP_USE_NEON
    return vnegq_s8(a.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int8<32> i_ineg(const int8<32>& a)
{
    return sub(0, a);
}
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
int8<64> i_ineg(const int8<64>& a)
{
    return sub(0, a);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
int16<8> i_ineg(const int16<8>& a)
{
#if SIMDPP_USE_NULL
    return detail::null::neg(a);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    return sub(0, a);
#elif SIMDPP_USE_NEON
    return vnegq_s16(a.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int16<16> i_ineg(const int16<16>& a)
{
    return sub(0, a);
}
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
int16<32> i_ineg(const int16<32>& a)
{
    return sub(0, a);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
int32<4> i_ineg(const int32<4>& a)
{
#if SIMDPP_USE_NULL
    return detail::null::neg(a);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    return sub(0, a);
#elif SIMDPP_USE_NEON
    return vnegq_s32(a.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int32<8> i_ineg(const int32<8>& a)
{
    return sub(0, a);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
int32<16> i_ineg(const int32<16>& a)
{
    return sub(0, a);
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
int64<2> i_ineg(const int64<2>& a)
{
#if SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_VSX_207 || SIMDPP_USE_MSA
    return sub(0, a);
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC
    return detail::null::neg(a);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint64<4> i_ineg(const int64<4>& a)
{
    return sub(0, a);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint64<8> i_ineg(const int64<8>& a)
{
    return sub(0, a);
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
V i_ineg(const V& a)
{
    SIMDPP_VEC_ARRAY_IMPL1(V, i_ineg, a)
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

