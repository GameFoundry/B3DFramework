/*  Copyright (C) 2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_FLOAT_TO_INT64_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_FLOAT_TO_INT64_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/conv_any_to_float64.h>
#include <simdpp/detail/vector_array_conv_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {


static SIMDPP_INL
uint64<2> i_to_uint64(const float64<2>& a)
{
#if SIMDPP_USE_AVX512DQ && SIMDPP_USE_AVX512VL
    return _mm_cvttpd_epu64(a.native());
#elif SIMDPP_USE_AVX512DQ
    __m512d a512 = _mm512_castpd128_pd512(a.native());
    return _mm512_castsi512_si128(_mm512_cvttpd_epu64(a512));
#elif SIMDPP_USE_NEON64
    return vcvtq_u64_f64(a.native());
#elif SIMDPP_USE_VSX_206
    uint32<4> r;
    r = (__vector uint32_t) vec_ctu(a.native(), 0);
    return (uint64<2>) r;
#elif SIMDPP_USE_VSX_207
    return vec_ctu(a.native(), 0);
#elif SIMDPP_USE_MSA
    return __msa_ftrunc_u_d(a.native());
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC
    detail::mem_block<uint64<2>> r;
    r[0] = uint64_t(a.el(0));
    r[1] = uint64_t(a.el(1));
    return r;
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
uint64<4> i_to_uint64(const float64<4>& a)
{
#if SIMDPP_USE_AVX512DQ && SIMDPP_USE_AVX512VL
    return _mm256_cvttpd_epu64(a.native());
#elif SIMDPP_USE_AVX512DQ
    __m512d a512 = _mm512_castpd256_pd512(a.native());
    return _mm512_castsi512_si256(_mm512_cvttpd_epu64(a512));
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint64<8> i_to_uint64(const float64<8>& a)
{
#if SIMDPP_USE_AVX512DQ
    return _mm512_cvttpd_epu64(a.native());
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}
#endif

template<unsigned N> SIMDPP_INL
uint64<N> i_to_uint64(const float64<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_EXTRACT(uint64<N>, i_to_uint64, a)
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
int64<2> i_to_int64(const float64<2>& a)
{
#if SIMDPP_USE_AVX512DQ && SIMDPP_USE_AVX512VL
    return _mm_cvttpd_epi64(a.native());
#elif SIMDPP_USE_AVX512DQ
    __m512d a512 = _mm512_castpd128_pd512(a.native());
    return _mm512_castsi512_si128(_mm512_cvttpd_epi64(a512));
#elif SIMDPP_USE_NEON64
    return vcvtq_s64_f64(a.native());
#elif SIMDPP_USE_VSX_207
    return vec_cts(a.native(), 0);
#elif SIMDPP_USE_VSX_206
    int32<4> r;
    r = (__vector int32_t) vec_cts(a.native(), 0);
    return (int64<2>) r;
#elif SIMDPP_USE_MSA
    return __msa_ftrunc_s_d(a.native());
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC
    detail::mem_block<int64<2>> r;
    r[0] = int64_t(a.el(0));
    r[1] = int64_t(a.el(1));
    return r;
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
int64<4> i_to_int64(const float64<4>& a)
{
#if SIMDPP_USE_AVX512DQ && SIMDPP_USE_AVX512VL
    return _mm256_cvttpd_epi64(a.native());
#elif SIMDPP_USE_AVX512DQ
    __m512d a512 = _mm512_castpd256_pd512(a.native());
    return _mm512_castsi512_si256(_mm512_cvttpd_epi64(a512));
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
int64<8> i_to_int64(const float64<8>& a)
{
#if SIMDPP_USE_AVX512DQ
    return _mm512_cvttpd_epi64(a.native());
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}
#endif

template<unsigned N> SIMDPP_INL
int64<N> i_to_int64(const float64<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_EXTRACT(int64<N>, i_to_int64, a)
}

// ----------------------------------------------------------------------------

static SIMDPP_INL
uint64<4> i_to_uint64(const float32<4>& a)
{
#if SIMDPP_USE_AVX512DQ && SIMDPP_USE_AVX512VL
    return _mm256_cvttps_epu64(a.native());
#elif SIMDPP_USE_AVX512DQ
    __m256 a256 = _mm256_castps128_ps256(a.native());
    return _mm512_castsi512_si256(_mm512_cvttps_epu64(a256));
#elif SIMDPP_USE_NEON64 || SIMDPP_USE_VSX_206 || SIMDPP_USE_MSA
    return i_to_uint64(i_to_float64(a));
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC
    detail::mem_block<uint64<4>> r;
    detail::mem_block<float32<4>> mi(a);
    r[0] = int64_t(mi[0]);
    r[1] = int64_t(mi[1]);
    r[2] = int64_t(mi[2]);
    r[3] = int64_t(mi[3]);
    return r;
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
uint64<8> i_to_uint64(const float32<8>& a)
{
#if SIMDPP_USE_AVX512DQ && SIMDPP_USE_AVX512VL
    return _mm512_cvttps_epu64(a.native());
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint64<16> i_to_uint64(const float32<16>& a)
{
#if SIMDPP_USE_AVX512DQ
    float32<8> a0, a1;
    uint64<16> r;
    split(a, a0, a1);

    r.vec(0) = _mm512_cvttps_epu64(a0.native());
    r.vec(1) = _mm512_cvttps_epu64(a1.native());

    return r;
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}
#endif

template<unsigned N> SIMDPP_INL
uint64<N> i_to_uint64(const float32<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_INSERT(uint64<N>, i_to_uint64, a)
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
int64<4> i_to_int64(const float32<4>& a)
{
#if SIMDPP_USE_AVX512DQ && SIMDPP_USE_AVX512VL
    return _mm256_cvttps_epi64(a.native());
#elif SIMDPP_USE_AVX512DQ
    __m256 a256 = _mm256_castps128_ps256(a.native());
    return _mm512_castsi512_si256(_mm512_cvttps_epi64(a256));
#elif SIMDPP_USE_NEON64 || SIMDPP_USE_VSX_206 || SIMDPP_USE_MSA
    return i_to_int64(i_to_float64(a));
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC
    detail::mem_block<int64<4>> r;
    detail::mem_block<float32<4>> mi(a);
    r[0] = int64_t(mi[0]);
    r[1] = int64_t(mi[1]);
    r[2] = int64_t(mi[2]);
    r[3] = int64_t(mi[3]);
    return r;
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
int64<8> i_to_int64(const float32<8>& a)
{
#if SIMDPP_USE_AVX512DQ && SIMDPP_USE_AVX512VL
    return _mm512_cvttps_epi64(a.native());
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
int64<16> i_to_int64(const float32<16>& a)
{
#if SIMDPP_USE_AVX512DQ
    float32<8> a0, a1;
    int64<16> r;
    split(a, a0, a1);

    r.vec(0) = _mm512_cvttps_epi64(a0.native());
    r.vec(1) = _mm512_cvttps_epi64(a1.native());

    return r;
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}
#endif

template<unsigned N> SIMDPP_INL
int64<N> i_to_int64(const float32<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_INSERT(int64<N>, i_to_int64, a)
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif


