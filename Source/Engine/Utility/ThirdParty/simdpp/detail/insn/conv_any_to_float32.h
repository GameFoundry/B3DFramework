/*  Copyright (C) 2013-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_ANY_TO_FLOAT32_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_CONV_ANY_TO_FLOAT32_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/combine.h>
#include <simdpp/core/zip_hi.h>
#include <simdpp/core/zip_lo.h>
#include <simdpp/core/insert.h>
#include <simdpp/core/f_mul.h>
#include <simdpp/core/make_shuffle_bytes_mask.h>
#include <simdpp/core/shuffle_bytes16.h>
#include <simdpp/core/detail/subvec_extract.h>
#include <simdpp/detail/mem_block.h>
#include <simdpp/detail/insn/conv_extend_to_int32.h>
#include <simdpp/detail/insn/conv_any_to_float64.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {


static SIMDPP_INL
float32<4> i_to_float32(const float64<4>& a)
{
#if SIMDPP_USE_AVX
    return _mm256_cvtpd_ps(a.native());
#elif SIMDPP_USE_SSE2
    float32x4 r1, r2;
    r1 = _mm_cvtpd_ps(a.vec(0).native());
    r2 = _mm_cvtpd_ps(a.vec(1).native());
    return _mm_movelh_ps(r1.native(), r2.native());
#elif SIMDPP_USE_NEON64
    float32<4> r;
    r = vcvt_high_f32_f64(vcvt_f32_f64(a.vec(0).native()),
                          a.vec(1).native());
    return r;
#elif SIMDPP_USE_VSX_206
    float32<4> lo, hi;
    uint32<4> shuffle_mask;
    lo = __builtin_vsx_xvcvdpsp(a.vec(0).native());
    hi = __builtin_vsx_xvcvdpsp(a.vec(1).native());
    shuffle_mask = make_shuffle_bytes16_mask<0,2,4,6>(shuffle_mask);
    return shuffle_bytes16(lo, hi, shuffle_mask);
#elif SIMDPP_USE_MSA
    return __msa_fexdo_w(a.vec(0).native(), a.vec(1).native());
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC
    detail::mem_block<float32x4> r;
    r[0] = float(a.vec(0).el(0));
    r[1] = float(a.vec(0).el(1));
    r[2] = float(a.vec(1).el(0));
    r[3] = float(a.vec(1).el(1));
    return r;
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float32<8> i_to_float32(const float64<8>& a)
{
#if SIMDPP_USE_AVX512F
    return _mm512_cvt_roundpd_ps(a.native(), (_MM_FROUND_TO_ZERO |_MM_FROUND_NO_EXC));
#else
    float32x4 r1, r2;
    r1 = i_to_float32(a.vec(0));
    r2 = i_to_float32(a.vec(1));
    return combine(r1, r2);
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float32<16> i_to_float32(const float64<16>& a)
{
    float32<8> r1, r2;
    r1 = i_to_float32(a.vec(0));
    r2 = i_to_float32(a.vec(1));
    return combine(r1, r2);
}
#endif

template<unsigned N> SIMDPP_INL
float32<N> i_to_float32(const float64<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_EXTRACT(float32<N>, i_to_float32, a)
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
float32<4> i_to_float32(const int64<4>& a)
{
#if SIMDPP_USE_NULL
    float32<4> r;
    for (unsigned i = 0; i < a.length; i++) {
        r.el(i) = float(a.vec(i/2).el(i%2));
    }
    return r;
#elif SIMDPP_USE_AVX512VL
    return _mm256_cvtepi64_ps(a.native());
#else
    return i_to_float32(i_to_float64(a));
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float32<8> i_to_float32(const int64<8>& a)
{
#if SIMDPP_USE_AVX512DQ
    return _mm512_cvtepi64_ps(a.native());
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float32<16> i_to_float32(const int64<16>& a)
{
#if SIMDPP_USE_AVX512DQ
    float32<8> r0 = _mm512_cvtepi64_ps(a.vec(0).native());
    float32<8> r1 = _mm512_cvtepi64_ps(a.vec(1).native());
    return combine(r0, r1);
#else
    return i_to_float32(i_to_float64(a));
#endif
}
#endif

template<unsigned N> SIMDPP_INL
float32<N> i_to_float32(const int64<N>& a)
{
    return i_to_float32(i_to_float64(a));
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
float32<4> i_to_float32(const uint64<4>& a)
{
#if SIMDPP_USE_NULL
    float32<4> r;
    for (unsigned i = 0; i < a.length; i++) {
        r.el(i) = float(a.vec(i/2).el(i%2));
    }
    return r;
#elif SIMDPP_USE_AVX512VL
    return _mm256_cvtepu64_ps(a.native());
#else
    return i_to_float32(i_to_float64(a));
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float32<8> i_to_float32(const uint64<8>& a)
{
#if SIMDPP_USE_AVX512DQ
    return _mm512_cvtepu64_ps(a.native());
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float32<16> i_to_float32(const uint64<16>& a)
{
#if SIMDPP_USE_AVX512DQ
    float32<8> r0 = _mm512_cvtepu64_ps(a.vec(0).native());
    float32<8> r1 = _mm512_cvtepu64_ps(a.vec(1).native());
    return combine(r0, r1);
#else
    return i_to_float32(i_to_float64(a));
#endif
}
#endif

template<unsigned N> SIMDPP_INL
float32<N> i_to_float32(const uint64<N>& a)
{
    return i_to_float32(i_to_float64(a));
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
float32<4> i_to_float32(const int32<4>& a)
{
#if SIMDPP_USE_NULL
    float32<4> r;
    for (unsigned i = 0; i < a.length; i++) {
        r.el(i) = float(a.el(i));
    }
    return r;
#elif SIMDPP_USE_SSE2
    return _mm_cvtepi32_ps(a.native());
#elif SIMDPP_USE_NEON && !SIMDPP_USE_NEON_FLT_SP
    detail::mem_block<int32<4>> mi(a);
    float32<4> r;
    r.el(0) = float(mi[0]);
    r.el(1) = float(mi[1]);
    r.el(2) = float(mi[2]);
    r.el(3) = float(mi[3]);
    return r;
#elif SIMDPP_USE_NEON_FLT_SP
    return vcvtq_f32_s32(a.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_ctf(a.native(), 0);
#elif SIMDPP_USE_MSA
    return __msa_ffint_s_w(a.native());
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float32x8 i_to_float32(const int32x8& a)
{
#if SIMDPP_USE_AVX2
    return _mm256_cvtepi32_ps(a.native());
#else
    __m256i a1;
    a1 = _mm256_castsi128_si256(a.vec(0).native());
    a1 = _mm256_insertf128_si256(a1, a.vec(1).native(), 1);
    return _mm256_cvtepi32_ps(a1);
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float32<16> i_to_float32(const int32<16>& a)
{
    return _mm512_cvtepi32_ps(a.native());
}
#endif

template<unsigned N> SIMDPP_INL
float32<N> i_to_float32(const int32<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_EXTRACT(float32<N>, i_to_float32, a)
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
float32<4> i_to_float32(const uint32<4>& a)
{
#if SIMDPP_USE_NULL
    float32<4> r;
    for (unsigned i = 0; i < a.length; i++) {
        r.el(i) = float(a.el(i));
    }
    return r;
#elif SIMDPP_USE_SSE2
    // true when a is in the range [0x80000000, 0xffffffff)
    mask_float32<4> is_large = mask_float32<4>(cmp_lt(int32<4>(a), 0));

    float32<4> f_a = _mm_cvtepi32_ps(a.native());
    // f_a has values in the range [0x80000000, 0xffffffff) wrapped around to
    // negative values. Conditionally bias the result to fix that. Note, that
    // the result is in sufficient precision even for large argument values.
    // The result has lowest precision around 0x80000000, and the precision
    // increases going towards 0xffffffff. The final result after bias will
    // have lower precision in this whole range.
    return add(f_a, bit_and(is_large, splat<float32<4>>(0x100000000)));
#elif SIMDPP_USE_NEON && !SIMDPP_USE_NEON_FLT_SP
    detail::mem_block<uint32<4>> mi(a);
    detail::mem_block<float32<4>> mf;
    mf[0] = float(mi[0]);
    mf[1] = float(mi[1]);
    mf[2] = float(mi[2]);
    mf[3] = float(mi[3]);
    return mf;
#elif SIMDPP_USE_NEON_FLT_SP
    return vcvtq_f32_u32(a.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_ctf(a.native(), 0);
#elif SIMDPP_USE_MSA
    return __msa_ffint_u_w(a.native());
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float32x8 i_to_float32(const uint32x8& a)
{
#if SIMDPP_USE_AVX512F
    __m512i a512 = _mm512_castsi256_si512(a.native());
    return _mm512_castps512_ps256(_mm512_cvtepu32_ps(a512));
#elif SIMDPP_USE_AVX2
    // true when a is in the range [0x80000000, 0xffffffff)
    mask_float32<8> is_large = mask_float32<8>(cmp_lt(int32<8>(a), 0));

    float32<8> f_a = _mm256_cvtepi32_ps(a.native());
    return add(f_a, bit_and(is_large, splat<float32<8>>(0x100000000)));
#else
    return combine(i_to_float32(a.vec(0)), i_to_float32(a.vec(1)));
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float32<16> i_to_float32(const uint32<16>& a)
{
    return _mm512_cvtepu32_ps(a.native());
}
#endif

template<unsigned N> SIMDPP_INL
float32<N> i_to_float32(const uint32<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL_CONV_EXTRACT(float32<N>, i_to_float32, a)
}

// -----------------------------------------------------------------------------

template<unsigned N> SIMDPP_INL
float32<N> i_to_float32(const uint16<N>& a)
{
    return i_to_float32(i_to_uint32(a));
}

template<unsigned N> SIMDPP_INL
float32<N> i_to_float32(const int16<N>& a)
{
    return i_to_float32(i_to_int32(a));
}

// -----------------------------------------------------------------------------

template<unsigned N> SIMDPP_INL
float32<N> i_to_float32(const uint8<N>& a)
{
    return i_to_float32(i_to_uint32(a));
}

template<unsigned N> SIMDPP_INL
float32<N> i_to_float32(const int8<N>& a)
{
    return i_to_float32(i_to_int32(a));
}

// -----------------------------------------------------------------------------

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif


