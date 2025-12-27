/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_BIT_XOR_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_BIT_XOR_H

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
uint8x16 i_bit_xor(const uint8x16& a, const uint8x16& b)
{
#if SIMDPP_USE_NULL
    return detail::null::bit_xor(a, uint8x16(b));
#elif SIMDPP_USE_SSE2
    return _mm_xor_si128(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return veorq_u8(a.native(), b.native());
#elif SIMDPP_USE_ALTIVEC
    return vec_xor(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return __msa_xor_v(a.native(), b.native());
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint8x32 i_bit_xor(const uint8x32& a, const uint8x32& b)
{
    return _mm256_xor_si256(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint8<64> i_bit_xor(const uint8<64>& a, const uint8<64>& b)
{
    return _mm512_xor_si512(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------
// mask_int8, mask_int8
static SIMDPP_INL
mask_int8x16 i_bit_xor(const mask_int8x16& a, const mask_int8x16& b)
{
#if SIMDPP_USE_NULL
    return detail::null::bit_xor_mm(a, b);
#elif SIMDPP_USE_AVX512VL
    return a.native() ^ b.native();
#else
    return to_mask(i_bit_xor(uint8x16(a), uint8x16(b)));
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
mask_int8x32 i_bit_xor(const mask_int8x32& a, const mask_int8x32& b)
{
#if SIMDPP_USE_AVX512VL
    return a.native() ^ b.native();
#else
    return _mm256_xor_si256(a.native(), b.native());
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL mask_int8<64> i_bit_xor(const mask_int8<64>& a, const mask_int8<64>& b)
{
    return a.native() ^ b.native();
}
#endif

// -----------------------------------------------------------------------------
// uint16, uint16
static SIMDPP_INL
uint16<8> i_bit_xor(const uint16<8>& a, const uint16<8>& b)
{
    return (uint16<8>) i_bit_xor(uint8<16>(a), uint8<16>(b));
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint16<16> i_bit_xor(const uint16<16>& a, const uint16<16>& b)
{
    return _mm256_xor_si256(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint16<32> i_bit_xor(const uint16<32>& a, const uint16<32>& b)
{
    return _mm512_xor_si512(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------
// mask_int16, mask_int16
static SIMDPP_INL
mask_int16<8> i_bit_xor(const mask_int16<8>& a, const mask_int16<8>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::bit_xor_mm(a, b);
#elif SIMDPP_USE_AVX512VL
    return a.native() ^ b.native();
#else
    return to_mask((uint16<8>) i_bit_xor(uint8<16>(a), uint8<16>(b)));
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
mask_int16<16> i_bit_xor(const mask_int16<16>& a, const mask_int16<16>& b)
{
#if SIMDPP_USE_AVX512VL
    return a.native() ^ b.native();
#else
    return to_mask((uint16<16>) i_bit_xor(uint16<16>(a), uint16<16>(b)));
#endif
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL mask_int16<32> i_bit_xor(const mask_int16<32>& a, const mask_int16<32>& b)
{
    return a.native() ^ b.native();
}
#endif

// -----------------------------------------------------------------------------
// uint32, uint32
static SIMDPP_INL
uint32<4> i_bit_xor(const uint32<4>& a, const uint32<4>& b)
{
    return (uint32<4>) i_bit_xor(uint8<16>(a), uint8<16>(b));
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint32<8> i_bit_xor(const uint32<8>& a, const uint32<8>& b)
{
    return _mm256_xor_si256(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint32<16> i_bit_xor(const uint32<16>& a, const uint32<16>& b)
{
    return _mm512_xor_epi32(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------
// mask_int32, mask_int32
static SIMDPP_INL
mask_int32<4> i_bit_xor(const mask_int32<4>& a, const mask_int32<4>& b)
{
#if SIMDPP_USE_NULL
    return detail::null::bit_xor_mm(a, b);
#elif SIMDPP_USE_AVX512VL
    return a.native() ^ b.native();
#else
    return to_mask((uint32<4>) i_bit_xor(uint8<16>(a), uint8<16>(b)));
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
mask_int32<8> i_bit_xor(const mask_int32<8>& a, const mask_int32<8>& b)
{
#if SIMDPP_USE_AVX512VL
    return a.native() ^ b.native();
#else
    return to_mask((uint32<8>) i_bit_xor(uint32<8>(a), uint32<8>(b)));
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
mask_int32<16> i_bit_xor(const mask_int32<16>& a, const mask_int32<16>& b)
{
    return _mm512_kxor(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------
// uint64, uint64
static SIMDPP_INL
uint64<2> i_bit_xor(const uint64<2>& a, const uint64<2>& b)
{
#if SIMDPP_USE_NULL || (SIMDPP_USE_ALTIVEC && !SIMDPP_USE_VSX_207)
    return detail::null::bit_xor(a, b);
#else
    return (uint64<2>) i_bit_xor(uint8<16>(a), uint8<16>(b));
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint64<4> i_bit_xor(const uint64<4>& a, const uint64<4>& b)
{
    return _mm256_xor_si256(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint64<8> i_bit_xor(const uint64<8>& a, const uint64<8>& b)
{
    return _mm512_xor_epi64(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------
// mask_int64, mask_int64
static SIMDPP_INL
mask_int64<2> i_bit_xor(const mask_int64<2>& a, const mask_int64<2>& b)
{
#if SIMDPP_USE_NULL || (SIMDPP_USE_ALTIVEC && !SIMDPP_USE_VSX_207)
    return detail::null::bit_xor_mm(a, b);
#elif SIMDPP_USE_AVX512VL
    return a.native() ^ b.native();
#else
    return to_mask((uint64<2>) i_bit_xor(uint8<16>(a), uint8<16>(b)));
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
mask_int64<4> i_bit_xor(const mask_int64<4>& a, const mask_int64<4>& b)
{
#if SIMDPP_USE_AVX512VL
    return a.native() ^ b.native();
#else
    return to_mask((uint64<4>) i_bit_xor(uint64<4>(a), uint64<4>(b)));
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
mask_int64<8> i_bit_xor(const mask_int64<8>& a, const mask_int64<8>& b)
{
    return _mm512_kxor(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

static SIMDPP_INL
float32x4 i_bit_xor(const float32x4& a, const float32x4& b)
{
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    return detail::null::bit_xor(a, b);
#elif SIMDPP_USE_SSE2
    return _mm_xor_ps(a.native(), b.native());
#elif SIMDPP_USE_NEON
    return vreinterpretq_f32_s32(veorq_s32(vreinterpretq_s32_f32(a.native()),
                                           vreinterpretq_s32_f32(b.native())));
#elif SIMDPP_USE_MSA
    return (float32<4>) i_bit_xor(uint8<16>(a), uint8<16>(b));
#elif SIMDPP_USE_ALTIVEC
    return vec_xor(a.native(), b.native());
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float32x8 i_bit_xor(const float32x8& a, const float32x8& b)
{
    return _mm256_xor_ps(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float32<16> i_bit_xor(const float32<16>& a, const float32<16>& b)
{
#if SIMDPP_USE_AVX512DQ
    return _mm512_xor_ps(a.native(), b.native());
#else
    return (float32<16>) i_bit_xor(uint32<16>(a), uint32<16>(b));
#endif
}
#endif

// -----------------------------------------------------------------------------
// mask_float32, mask_float32

static SIMDPP_INL
mask_float32x4 i_bit_xor(const mask_float32x4& a, const mask_float32x4& b)
{
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    return detail::null::bit_xor_mm(a, b);
#elif SIMDPP_USE_AVX512VL
    return a.native() ^ b.native();
#else
    return to_mask(i_bit_xor(float32x4(a), float32x4(b)));
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
mask_float32x8 i_bit_xor(const mask_float32x8& a, const mask_float32x8& b)
{
#if SIMDPP_USE_AVX512VL
    return a.native() ^ b.native();
#else
    return to_mask(i_bit_xor(float32x8(a), float32x8(b)));
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
mask_float32<16> i_bit_xor(const mask_float32<16>& a, const mask_float32<16>& b)
{
    return _mm512_kxor(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------
// float64, float64

static SIMDPP_INL
float64x2 i_bit_xor(const float64x2& a, const float64x2& b)
{
#if SIMDPP_USE_SSE2
    return _mm_xor_pd(a.native(), b.native());
#elif SIMDPP_USE_NEON64
    return vreinterpretq_f64_u64(veorq_u64(vreinterpretq_u64_f64(a.native()),
                                           vreinterpretq_u64_f64(b.native())));
#elif SIMDPP_USE_VSX_206
    return vec_xor(a.native(), b.native());
#elif SIMDPP_USE_MSA
    return (float64<2>) i_bit_xor(uint8<16>(a), uint8<16>(b));
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC
    return detail::null::bit_xor(a, b);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
float64x4 i_bit_xor(const float64x4& a, const float64x4& b)
{
    return _mm256_xor_pd(a.native(), b.native());
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
float64<8> i_bit_xor(const float64<8>& a, const float64<8>& b)
{
#if SIMDPP_USE_AVX512DQ
    return _mm512_xor_pd(a.native(), b.native());
#else
    return (float64<8>) i_bit_xor(uint64<8>(a), uint64<8>(b));
#endif
}
#endif

// -----------------------------------------------------------------------------
// mask_float64, mask_float64

static SIMDPP_INL
mask_float64x2 i_bit_xor(const mask_float64x2& a, const mask_float64x2& b)
{
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || (SIMDPP_USE_ALTIVEC && !SIMDPP_USE_VSX_206)
    return detail::null::bit_xor_mm(a, b);
#elif SIMDPP_USE_AVX512VL
    return a.native() ^ b.native();
#else
    return to_mask(i_bit_xor(float64x2(a), float64x2(b)));
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
mask_float64x4 i_bit_xor(const mask_float64x4& a, const mask_float64x4& b)
{
#if SIMDPP_USE_AVX512VL
    return a.native() ^ b.native();
#else
    return to_mask(i_bit_xor(float64x4(a), float64x4(b)));
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
mask_float64<8> i_bit_xor(const mask_float64<8>& a, const mask_float64<8>& b)
{
    return _mm512_kxor(a.native(), b.native());
}
#endif

// -----------------------------------------------------------------------------

template<class V, class VM> SIMDPP_INL
V i_bit_xor(const V& a, const VM& b)
{
    SIMDPP_VEC_ARRAY_IMPL2(V, i_bit_xor, a, b)
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif


