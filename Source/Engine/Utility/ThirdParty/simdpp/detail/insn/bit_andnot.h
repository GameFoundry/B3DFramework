/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_BIT_ANDNOT_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_BIT_ANDNOT_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/to_mask.h>
#include <simdpp/detail/null/bitwise.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

// -----------------------------------------------------------------------------
// uint8, uint8
static SIMDPP_INL
uint8<16> i_bit_andnot(const uint8<16>& a, const uint8<16>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::bit_andnot(a, b);
#elif SIMDPP_USE_SSE2
    return _mm_andnot_si128(b.native(), a.native());
#elif SIMDPP_USE_NEON
    return vbicq_u8(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_andc(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_and_v(a.native(), __msa_nor_v(b.native(), b.native()));
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint8<32> i_bit_andnot(const uint8<32>& a, const uint8<32>& b)
{
    return _mm256_andnot_si256(b.native(), a.native());
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint8<64> i_bit_andnot(const uint8<64>& a, const uint8<64>& b)
{
    return _mm512_andnot_si512(b.native(), a.native());
}
#endif

// -----------------------------------------------------------------------------
// uint8, mask_int8
static SIMDPP_INL
uint8<16> i_bit_andnot(const uint8<16>& a, const mask_int8<16>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::bit_andnot_vm(a, b);
#elif SIMDPP_USE_AVX512VL
    return _mm_maskz_mov_epi8(~b.native(), a.native());
#else
    return i_bit_andnot(a, uint8<16>(b));
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint8<32> i_bit_andnot(const uint8<32>& a, const mask_int8<32>& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_maskz_mov_epi8(~b.native(), a.native());
#else
    return i_bit_andnot(a, uint8<32>(b));
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint8<64> i_bit_andnot(const uint8<64>& a, const mask_int8<64>& b)
{
    return _mm512_maskz_mov_epi8(~b.native(), a.native());
}
#endif

// -----------------------------------------------------------------------------
// mask_int8, mask_int8
static SIMDPP_INL
mask_int8<16> i_bit_andnot(const mask_int8<16>& a, const mask_int8<16>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::bit_andnot_mm(a, b);
#elif SIMDPP_USE_AVX512VL
    return a.native() & ~b.native();
#else
    return to_mask(i_bit_andnot(uint8<16>(a), uint8<16>(b)));
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
mask_int8<32> i_bit_andnot(const mask_int8<32>& a, const mask_int8<32>& b)
{
#if SIMDPP_USE_AVX512VL
    return a.native() & ~b.native();
#else
    return to_mask(i_bit_andnot(uint8<32>(a), uint8<32>(b)));
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL mask_int8<64> i_bit_andnot(const mask_int8<64>& a, const mask_int8<64>& b)
{
    return a.native() & ~b.native();
}
#endif

// -----------------------------------------------------------------------------
// uint16, uint16
static SIMDPP_INL
uint16<8> i_bit_andnot(const uint16<8>& a, const uint16<8>& b)
{
    return uint16<8>(i_bit_andnot(uint8<16>(a), uint8<16>(b)));
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint16<16> i_bit_andnot(const uint16<16>& a, const uint16<16>& b)
{
    return _mm256_andnot_si256(b.native(), a.native());
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint16<32> i_bit_andnot(const uint16<32>& a, const uint16<32>& b)
{
    return _mm512_andnot_si512(b.native(), a.native());
}
#endif

// -----------------------------------------------------------------------------
// uint16, mask_int16
static SIMDPP_INL
uint16<8> i_bit_andnot(const uint16<8>& a, const mask_int16<8>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::bit_andnot_vm(a, b);
#elif SIMDPP_USE_AVX512VL
    return _mm_maskz_mov_epi16(~b.native(), a.native());
#else
    return i_bit_andnot(a, uint16<8>(b));
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint16<16> i_bit_andnot(const uint16<16>& a, const mask_int16<16>& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_maskz_mov_epi16(~b.native(), a.native());
#else
    return i_bit_andnot(a, uint16<16>(b));
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint16<32> i_bit_andnot(const uint16<32>& a, const mask_int16<32>& b)
{
    return _mm512_maskz_mov_epi16(~b.native(), a.native());
}
#endif

// -----------------------------------------------------------------------------
// mask_int16, mask_int16
static SIMDPP_INL
mask_int16<8> i_bit_andnot(const mask_int16<8>& a, const mask_int16<8>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::bit_andnot_mm(a, b);
#elif SIMDPP_USE_AVX512VL
    return a.native() & ~b.native();
#else
    return to_mask((uint16<8>) i_bit_andnot(uint8<16>(a), uint8<16>(b)));
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
mask_int16<16> i_bit_andnot(const mask_int16<16>& a, const mask_int16<16>& b)
{
#if SIMDPP_USE_AVX512VL
    return a.native() & ~b.native();
#else
    return to_mask((uint16<16>) i_bit_andnot(uint16<16>(a), uint16<16>(b)));
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL mask_int16<32> i_bit_andnot(const mask_int16<32>& a, const mask_int16<32>& b)
{
    return a.native() & ~b.native();
}
#endif

// -----------------------------------------------------------------------------
// uint32, uint32
static SIMDPP_INL
uint32<4> i_bit_andnot(const uint32<4>& a, const uint32<4>& b)
{
    return uint32<4>(i_bit_andnot(uint8<16>(a), uint8<16>(b)));
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint32<8> i_bit_andnot(const uint32<8>& a, const uint32<8>& b)
{
    return _mm256_andnot_si256(b.native(), a.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint32<16> i_bit_andnot(const uint32<16>& a, const uint32<16>& b)
{
    return _mm512_andnot_epi32(b.native(), a.native());
}
#endif

// -----------------------------------------------------------------------------
// uint32, mask_int32
static SIMDPP_INL
uint32<4> i_bit_andnot(const uint32<4>& a, const mask_int32<4>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::bit_andnot_vm(a, b);
#elif SIMDPP_USE_AVX512VL
    return _mm_maskz_mov_epi32(~b.native(), a.native());
#else
    return i_bit_andnot(a, uint32<4>(b));
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint32<8> i_bit_andnot(const uint32<8>& a, const mask_int32<8>& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_maskz_mov_epi32(~b.native(), a.native());
#else
    return i_bit_andnot(a, uint32<8>(b));
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint32<16> i_bit_andnot(const uint32<16>& a, const mask_int32<16>& b)
{
    return _mm512_maskz_mov_epi32(_mm512_knot(b.native()), a.native());
}
#endif

// -----------------------------------------------------------------------------
// mask_int32, mask_int32
static SIMDPP_INL
mask_int32<4> i_bit_andnot(const mask_int32<4>& a, const mask_int32<4>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::bit_andnot_mm(a, b);
#elif SIMDPP_USE_AVX512VL
    return a.native() & ~b.native();
#else
    return to_mask((uint32<4>) i_bit_andnot(uint8<16>(a), uint8<16>(b)));
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
mask_int32<8> i_bit_andnot(const mask_int32<8>& a, const mask_int32<8>& b)
{
#if SIMDPP_USE_AVX512VL
    return a.native() & ~b.native();
#else
    return to_mask((uint32<8>) i_bit_andnot(uint32<8>(a), uint32<8>(b)));
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
mask_int32<16> i_bit_andnot(const mask_int32<16>& a, const mask_int32<16>& b)
{
    return _mm512_kandn(b.native(), a.native());
}
#endif

// -----------------------------------------------------------------------------
// uint64, uint64
static SIMDPP_INL
uint64<2> i_bit_andnot(const uint64<2>& a, const uint64<2>& b)
{
#if SIMDPP_USE_NULL || (SIMDPP_USE_ALTIVEC && !SIMDPP_USE_VSX_207)
    return detail::null::bit_andnot(a, b);
#else
    return uint64<2>(i_bit_andnot(uint8<16>(a), uint8<16>(b)));
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint64<4> i_bit_andnot(const uint64<4>& a, const uint64<4>& b)
{
    return _mm256_andnot_si256(b.native(), a.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint64<8> i_bit_andnot(const uint64<8>& a, const uint64<8>& b)
{
    return _mm512_andnot_epi64(b.native(), a.native());
}
#endif

// -----------------------------------------------------------------------------
// uint64, mask_int64
static SIMDPP_INL
uint64<2> i_bit_andnot(const uint64<2>& a, const mask_int64<2>& b)
{
#if SIMDPP_USE_NULL || (SIMDPP_USE_ALTIVEC && !SIMDPP_USE_VSX_207)
    return detail::null::bit_andnot_vm(a, b);
#elif SIMDPP_USE_AVX512VL
    return _mm_maskz_mov_epi64(~b.native(), a.native());
#else
    return i_bit_andnot(a, uint64<2>(b));
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint64<4> i_bit_andnot(const uint64<4>& a, const mask_int64<4>& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_maskz_mov_epi64(~b.native(), a.native());
#else
    return i_bit_andnot(a, uint64<4>(b));
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint64<8> i_bit_andnot(const uint64<8>& a, const mask_int64<8>& b)
{
    return _mm512_maskz_mov_epi64(_mm512_knot(b.native()), a.native());
}
#endif

// -----------------------------------------------------------------------------
// mask_int64, mask_int64
static SIMDPP_INL
mask_int64<2> i_bit_andnot(const mask_int64<2>& a, const mask_int64<2>& b)
{
#if SIMDPP_USE_NULL || (SIMDPP_USE_ALTIVEC && !SIMDPP_USE_VSX_207)
    return detail::null::bit_andnot_mm(a, b);
#elif SIMDPP_USE_AVX512VL
    return a.native() & ~b.native();
#else
    return to_mask((uint64<2>) i_bit_andnot(uint8<16>(a), uint8<16>(b)));
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
mask_int64<4> i_bit_andnot(const mask_int64<4>& a, const mask_int64<4>& b)
{
#if SIMDPP_USE_AVX512VL
    return a.native() & ~b.native();
#else
    return to_mask((uint64<4>) i_bit_andnot(uint64<4>(a), uint64<4>(b)));
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
mask_int64<8> i_bit_andnot(const mask_int64<8>& a, const mask_int64<8>& b)
{
    return _mm512_kandn(b.native(), a.native());
}
#endif

// -----------------------------------------------------------------------------
// float32, float32
static SIMDPP_INL
float32<4> i_bit_andnot(const float32<4>& a, const float32<4>& b)
{
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    return detail::null::bit_andnot(a, b);
#elif SIMDPP_USE_SSE2
    return _mm_andnot_ps(b.native(), a.native());
#elif SIMDPP_USE_NEON
    return vreinterpretq_f32_u32(vbicq_u32(vreinterpretq_u32_f32(a.native()),
                                           vreinterpretq_u32_f32(b.native())));
#elif SIMDPP_USE_ALTIVEC
    return vec_andc(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return (float32<4>) i_bit_andnot(uint8<16>(a), uint8<16>(b));
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float32<8> i_bit_andnot(const float32<8>& a, const float32<8>& b)
{
    return _mm256_andnot_ps(b.native(), a.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float32<16> i_bit_andnot(const float32<16>& a, const float32<16>& b)
{
#if SIMDPP_USE_AVX512DQ
    return _mm512_andnot_ps(b.native(), a.native());
#else
    return float32<16>(i_bit_andnot(uint32<16>(a), uint32<16>(b)));
#endif
}
#endif

// -----------------------------------------------------------------------------
// float32, mask_float32
static SIMDPP_INL
float32<4> i_bit_andnot(const float32<4>& a, const mask_float32<4>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::bit_andnot_vm(a, b);
#elif SIMDPP_USE_AVX512VL
    return _mm_maskz_mov_ps(~b.native(), a.native());
#else
    return i_bit_andnot(a, float32<4>(b));
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float32<8> i_bit_andnot(const float32<8>& a, const mask_float32<8>& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_maskz_mov_ps(~b.native(), a.native());
#else
    return i_bit_andnot(a, float32<8>(b));
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float32<16> i_bit_andnot(const float32<16>& a, const mask_float32<16>& b)
{
    return _mm512_maskz_mov_ps(_mm512_knot(b.native()), a.native());
}
#endif

// -----------------------------------------------------------------------------
// mask_float32, mask_float32
static SIMDPP_INL
mask_float32<4> i_bit_andnot(const mask_float32<4>& a, const mask_float32<4>& b)
{
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    return detail::null::bit_andnot_mm(a, b);
#elif SIMDPP_USE_AVX512VL
    return a.native() & ~b.native();
#else
    return to_mask(i_bit_andnot(float32<4>(a), float32<4>(b)));
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
mask_float32<8> i_bit_andnot(const mask_float32<8>& a, const mask_float32<8>& b)
{
#if SIMDPP_USE_AVX512VL
    return a.native() & ~b.native();
#else
    return to_mask(i_bit_andnot(float32<8>(a), float32<8>(b)));
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
mask_float32<16> i_bit_andnot(const mask_float32<16>& a, const mask_float32<16>& b)
{
    return _mm512_kandn(b.native(), a.native());
}
#endif

// -----------------------------------------------------------------------------
// float64, float64
static SIMDPP_INL
float64<2> i_bit_andnot(const float64<2>& a, const float64<2>& b)
{
#if SIMDPP_USE_SSE2
    return _mm_andnot_pd(b.native(), a.native());
#elif SIMDPP_USE_NEON64
    return vreinterpretq_f64_u64(vbicq_u64(vreinterpretq_u64_f64(a.native()),
                                           vreinterpretq_u64_f64(b.native())));
#elif SIMDPP_USE_VSX_206
    return vec_andc(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return (float64<2>) i_bit_andnot(uint8<16>(a), uint8<16>(b));
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC
    return detail::null::bit_andnot(a, b);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float64<4> i_bit_andnot(const float64<4>& a, const float64<4>& b)
{
    return _mm256_andnot_pd(b.native(), a.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float64<8> i_bit_andnot(const float64<8>& a, const float64<8>& b)
{
#if SIMDPP_USE_AVX512DQ
    return _mm512_andnot_pd(b.native(), a.native());
#else
    return float64<8>(i_bit_andnot(uint64<8>(a), uint64<8>(b)));
#endif
}
#endif

// -----------------------------------------------------------------------------
// float64, mask_float64
static SIMDPP_INL
float64<2> i_bit_andnot(const float64<2>& a, const mask_float64<2>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::bit_andnot_vm(a, b);
#elif SIMDPP_USE_AVX512VL
    return _mm_maskz_mov_pd(~b.native(), a.native());
#else
    return i_bit_andnot(a, float64<2>(b));
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float64<4> i_bit_andnot(const float64<4>& a, const mask_float64<4>& b)
{
#if SIMDPP_USE_AVX512VL
    return _mm256_maskz_mov_pd(~b.native(), a.native());
#else
    return i_bit_andnot(a, float64<4>(b));
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float64<8> i_bit_andnot(const float64<8>& a, const mask_float64<8>& b)
{
    return _mm512_maskz_mov_pd(_mm512_knot(b.native()), a.native());
}
#endif

// -----------------------------------------------------------------------------
// mask_float64, mask_float64
static SIMDPP_INL
mask_float64<2> i_bit_andnot(const mask_float64<2>& a, const mask_float64<2>& b)
{
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || (SIMDPP_USE_ALTIVEC && !SIMDPP_USE_VSX_206)
    return detail::null::bit_andnot_mm(a, b);
#elif SIMDPP_USE_AVX512VL
    return a.native() & ~b.native();
#else
    return to_mask(i_bit_andnot(float64<2>(a), float64<2>(b)));
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
mask_float64<4> i_bit_andnot(const mask_float64<4>& a, const mask_float64<4>& b)
{
#if SIMDPP_USE_AVX512VL
    return a.native() & ~b.native();
#else
    return to_mask(i_bit_andnot(float64<4>(a), float64<4>(b)));
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
mask_float64<8> i_bit_andnot(const mask_float64<8>& a, const mask_float64<8>& b)
{
    return _mm512_kandn(b.native(), a.native());
}
#endif

// -----------------------------------------------------------------------------

template<class V, class VM> SIMDPP_INL
V i_bit_andnot(const V& a, const VM& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(V, i_bit_andnot, a, b)
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

