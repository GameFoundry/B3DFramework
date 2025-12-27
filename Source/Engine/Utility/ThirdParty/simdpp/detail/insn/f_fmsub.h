/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_F_FMSUB_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_F_FMSUB_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/null/math.h>
#include <simdpp/detail/not_implemented.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

static SIMDPP_INL
float32<4> i_fmsub(const float32<4>& a, const float32<4>& b,
                   const float32<4>& c)
{
#if SIMDPP_USE_NULL
    return detail::null::fmsub(a, b, c);
#elif SIMDPP_USE_FMA3
    return _mm_fmsub_ps(a.native(), b.native(), c.native());
#elif SIMDPP_USE_FMA4
    return _mm_msub_ps(a.native(), b.native(), c.native());
#elif SIMDPP_USE_NEON64
    // FIXME: also in vfpv4
    return vfmsq_f32(a.native(), b.native(), c.native());
#elif SIMDPP_USE_MSA
    return __msa_fmsub_w(c.native(), a.native(), b.native());
#else
    return SIMDPP_NOT_IMPLEMENTED3(a, b, c);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float32<8> i_fmsub(const float32<8>& a, const float32<8>& b,
                   const float32<8>& c)
{
#if SIMDPP_USE_FMA3
    return _mm256_fmsub_ps(a.native(), b.native(), c.native());
#elif SIMDPP_USE_FMA4
    return _mm256_msub_ps(a.native(), b.native(), c.native());
#else
    return SIMDPP_NOT_IMPLEMENTED3(a, b, c);
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float32<16> i_fmsub(const float32<16>& a, const float32<16>& b,
                    const float32<16>& c)
{
    return _mm512_fmsub_ps(a.native(), b.native(), c.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
float64<2> i_fmsub(const float64<2>& a, const float64<2>& b,
                   const float64<2>& c)
{
#if SIMDPP_USE_NULL
    return detail::null::fmsub(a, b, c);
#elif SIMDPP_USE_FMA3
    return _mm_fmsub_pd(a.native(), b.native(), c.native());
#elif SIMDPP_USE_FMA4
    return _mm_msub_pd(a.native(), b.native(), c.native());
#elif SIMDPP_USE_NEON64
    // FIXME: also in vfpv4
    return vfmsq_f64(a.native(), b.native(), c.native());
#elif SIMDPP_USE_MSA
    return __msa_fmsub_d(c.native(), a.native(), b.native());
#else
    return SIMDPP_NOT_IMPLEMENTED3(a, b, c);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float64<4> i_fmsub(const float64<4>& a, const float64<4>& b,
                   const float64<4>& c)
{
#if SIMDPP_USE_FMA3
    return _mm256_fmsub_pd(a.native(), b.native(), c.native());
#elif SIMDPP_USE_FMA4
    return _mm256_msub_pd(a.native(), b.native(), c.native());
#else
    return SIMDPP_NOT_IMPLEMENTED3(a, b, c);
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float64<8> i_fmsub(const float64<8>& a, const float64<8>& b,
                   const float64<8>& c)
{
    return _mm512_fmsub_pd(a.native(), b.native(), c.native());
}
#endif

// -----------------------------------------------------------------------------

template<class V> SIMDPP_INL
V i_fmsub(const V& a, const V& b, const V& c)
{
    SIMDPP_VEC_ARRAY_IMPL3(V, i_fmsub, a, b, c);
}

} // namsepace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

