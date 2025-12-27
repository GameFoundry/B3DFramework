/*  Copyright (C) 2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_FLOAT_TO_INT32_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_FLOAT_TO_INT32_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/i_add.h>
#include <simdpp/core/f_sub.h>
#include <simdpp/core/cmp_ge.h>
#include <simdpp/detail/vector_array_conv_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

// -----------------------------------------------------------------------------

static SIMDPP_INL
int32<4> i_to_int32(const float32<4>& a)
{
#if SIMDPP_USE_NULL
    int32x4 r;
    for (unsigned i = 0; i < a.length; i++) {
        r.el(i) = int32_t(a.el(i));
    }
    return r;
#elif SIMDPP_USE_SSE2
    return _mm_cvttps_epi32(a.native());
#elif SIMDPP_USE_NEON && !SIMDPP_USE_NEON_FLT_SP
    detail::mem_block<float32x4> mf(a);
    detail::mem_block<int32x4> mi;
    mi[0] = int(mf[0]);
    mi[1] = int(mf[1]);
    mi[2] = int(mf[2]);
    mi[3] = int(mf[3]);
    return mi;
#elif SIMDPP_USE_NEON_FLT_SP
    return vcvtq_s32_f32(a.native());
#elif SIMDPP_USE_MSA
    return __msa_ftrunc_s_w(a.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_cts(a.native(), 0);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
int32<8> i_to_int32(const float32<8>& a)
{
#if SIMDPP_USE_AVX2
    return _mm256_cvttps_epi32(a.native());
#else
    __m256i r = _mm256_cvttps_epi32(a.native());
    uint32<4> r1, r2;
    r1 = _mm256_castsi256_si128(r);
    r2 = _mm256_extractf128_si256(r, 1);
    return combine(r1, r2);
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
int32<16> i_to_int32(const float32<16>& a)
{
    return _mm512_cvttps_epi32(a.native());
}
#endif

template<unsigned N> SIMDPP_INL
int32<N> i_to_int32(const float32<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_INSERT(int32<N>, i_to_int32, a)
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint32<4> i_to_uint32(const float32<4>& a)
{
#if SIMDPP_USE_NULL
    uint32x4 r;
    for (unsigned i = 0; i < a.length; i++) {
        r.el(i) = uint32_t(a.el(i));
    }
    return r;
#elif SIMDPP_USE_AVX512VL
    return _mm_cvttps_epu32(a.native());
#elif SIMDPP_USE_AVX512F
    __m512 a512 = _mm512_castps128_ps512(a.native());
    return _mm512_castsi512_si128(_mm512_cvttps_epu32(a512));
#elif SIMDPP_USE_NEON && !SIMDPP_USE_NEON_FLT_SP
    detail::mem_block<float32x4> mf(a);
    detail::mem_block<uint32x4> mi;
    mi[0] = unsigned(mf[0]);
    mi[1] = unsigned(mf[1]);
    mi[2] = unsigned(mf[2]);
    mi[3] = unsigned(mf[3]);
    return mi;
#elif SIMDPP_USE_NEON_FLT_SP
    return vcvtq_u32_f32(a.native());
#elif SIMDPP_USE_MSA
    return __msa_ftrunc_u_w(a.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_ctu(a.native(), 0);
#else
    // Smaller than 0x80000000 numbers can be represented as int32, so we can
    // use i_to_int32 which is available as instruction on all supported
    // architectures. Values >= 0x80000000 are biased into the range -0x80000000..0xffffffff.
    // These conveniently convert through i_to_int32() to 0x80000000..0xffffffff. No further
    // unbiasing is required. No attempt is made to produce a reliable overflow value for
    // values outside the range 0 .. 0xffffffff.
    mask_float32<4> is_large = cmp_ge(a, 0x80000000);
	return uint32<4>( i_to_int32(sub(a, bit_and(is_large, splat<float32<4>>(0x100000000)))) );
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
uint32<8> i_to_uint32(const float32<8>& a)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_cvttps_epu32(a.native());
#elif SIMDPP_USE_AVX512F
    __m512 a512 = _mm512_castps256_ps512(a.native());
    return _mm512_castsi512_si256(_mm512_cvttps_epu32(a512));
#else
    mask_float32<8> is_large = cmp_ge(a, 0x80000000);
	return uint32<8>( i_to_int32(sub(a, bit_and(is_large, splat<float32<8>>(0x100000000)))) );
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint32<16> i_to_uint32(const float32<16>& a)
{
    return _mm512_cvttps_epu32(a.native());
}
#endif

template<unsigned N> SIMDPP_INL
uint32<N> i_to_uint32(const float32<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_INSERT(uint32<N>, i_to_uint32, a)
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint32<4> i_to_uint32(const float64<4>& a)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_cvttpd_epu32(a.native());
#elif SIMDPP_USE_AVX512F
    __m512d a512 = _mm512_castpd256_pd512(a.native());
    return _mm256_castsi256_si128(_mm512_cvttpd_epu32(a512));
#elif SIMDPP_USE_NEON64
    uint64x2_t r1, r2;
    r1 = vcvtq_u64_f64(a.vec(0).native());
    r2 = vcvtq_u64_f64(a.vec(1).native());
    // FIXME: saturation
    uint32<4> r = vcombine_u32(vqmovn_u64(r1), vqmovn_u64(r2));
    return r;
#elif SIMDPP_USE_VSX_206
    uint32<4> r, r1, r2;
    r1 = (__vector uint32_t) vec_ctu(a.vec(0).native(), 0);
    r2 = (__vector uint32_t) vec_ctu(a.vec(1).native(), 0);
    r = unzip4_lo(r1, r2);
    return r;
#elif SIMDPP_USE_MSA
    uint64<2> r1, r2;
    r1 = __msa_ftrunc_u_d(a.vec(0).native());
    r2 = __msa_ftrunc_u_d(a.vec(1).native());
    return unzip4_lo((uint32<4>)r1, (uint32<4>)r2);
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC
    detail::mem_block<uint32x4> r;
    r[0] = uint32_t(a.vec(0).el(0));
    r[1] = uint32_t(a.vec(0).el(1));
    r[2] = uint32_t(a.vec(1).el(0));
    r[3] = uint32_t(a.vec(1).el(1));
    return r;
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
uint32<8> i_to_uint32(const float64<8>& a)
{
#if SIMDPP_USE_AVX512F
    return _mm512_cvttpd_epu32(a.native());
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint32<16> i_to_uint32(const float64<16>& a)
{
    uint32<8> r0, r1;
    r0 = _mm512_cvttpd_epu32(a.vec(0).native());
    r1 = _mm512_cvttpd_epu32(a.vec(1).native());
    return combine(r0, r1);
}
#endif

template<unsigned N> SIMDPP_INL
uint32<N> i_to_uint32(const float64<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_EXTRACT(uint32<N>, i_to_uint32, a)
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
int32x4 i_to_int32(const float64x4& a)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_cvttpd_epi32(a.native());
#elif SIMDPP_USE_SSE2
    int32x4 r, r1, r2;
    float64x2 a1, a2;
    split(a, a1, a2);
    r1 = _mm_cvttpd_epi32(a1.native());
    r2 = _mm_cvttpd_epi32(a2.native());
    r = zip2_lo(int64<2>(r1), int64<2>(r2));
    return r;
#elif SIMDPP_USE_NEON64
    int64x2_t r1, r2;
    r1 = vcvtq_s64_f64(a.vec(0).native());
    r2 = vcvtq_s64_f64(a.vec(1).native());
    // FIXME: saturation
    int32<4> r = vcombine_s32(vqmovn_s64(r1), vqmovn_s64(r2));
    return r;
#elif SIMDPP_USE_VSX_206
    int32<4> r, r1, r2;
    r1 = (__vector int32_t) vec_cts(a.vec(0).native(), 0);
    r2 = (__vector int32_t) vec_cts(a.vec(1).native(), 0);
    r = unzip4_lo(r1, r2);
    return r;
#elif SIMDPP_USE_MSA
    int64<2> r1, r2;
    r1 = __msa_ftrunc_s_d(a.vec(0).native());
    r2 = __msa_ftrunc_s_d(a.vec(1).native());
    return unzip4_lo((int32<4>)r1, (int32<4>)r2);
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC
    detail::mem_block<int32x4> r;
    r[0] = int32_t(a.vec(0).el(0));
    r[1] = int32_t(a.vec(0).el(1));
    r[2] = int32_t(a.vec(1).el(0));
    r[3] = int32_t(a.vec(1).el(1));
    return r;
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
int32<8> i_to_int32(const float64<8>& a)
{
#if SIMDPP_USE_AVX512F
    return _mm512_cvttpd_epi32(a.native());
#else
    int32<4> r1, r2;
    r1 = _mm256_cvttpd_epi32(a.vec(0).native());
    r2 = _mm256_cvttpd_epi32(a.vec(1).native());
    return combine(r1, r2);
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
int32<16> i_to_int32(const float64<16>& a)
{
    int32<8> r0, r1;
    r0 = _mm512_cvttpd_epi32(a.vec(0).native());
    r1 = _mm512_cvttpd_epi32(a.vec(1).native());
    return combine(r0, r1);
}
#endif

template<unsigned N> SIMDPP_INL
int32<N> i_to_int32(const float64<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_EXTRACT(int32<N>, i_to_int32, a)
}


} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif


