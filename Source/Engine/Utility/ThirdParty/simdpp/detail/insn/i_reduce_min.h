/*  Copyright (C) 2016  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_I_REDUCE_MIN_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_I_REDUCE_MIN_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/i_min.h>
#include <simdpp/core/extract.h>
#include <simdpp/core/move_l.h>
#include <simdpp/core/make_uint.h>
#include <simdpp/detail/mem_block.h>
#include <limits>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

static SIMDPP_INL
uint8_t i_reduce_min(const uint8x16& a)
{
#if SIMDPP_USE_NULL
    uint8_t r = a.el(0);
    for (unsigned i = 0; i < a.length; i++) {
        r = r < a.el(i) ? r : a.el(i);
    }
    return r;
#elif SIMDPP_USE_NEON64
    return vminvq_u8(a.native());
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    uint8x16 r = min(a, move16_l<8>(a));
    r = min(r, move16_l<4>(r));
    r = min(r, move16_l<2>(r));
    r = min(r, move16_l<1>(r));
    return extract<0>(r);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint8_t i_reduce_min(const uint8<32>& a)
{
    uint8x16 r = detail::extract128<0>(a);
    r = min(r, detail::extract128<1>(a));
    return i_reduce_min(r);
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint8_t i_reduce_min(const uint8<64>& a)
{
    uint8<32> r = detail::extract256<0>(a);
    r = min(r, detail::extract256<1>(a));
    return i_reduce_min(r);
}
#endif

template<unsigned N>
SIMDPP_INL uint8_t i_reduce_min(const uint8<N>& a)
{
#if SIMDPP_USE_NULL
    uint8_t r = std::numeric_limits<uint8_t>::max();
    for (unsigned j = 0; j < a.vec_length; ++j) {
        for (unsigned i = 0; i < a.base_length; i++) {
            r = r < a.vec(j).el(i) ? r : a.vec(j).el(i);
        }
    }
    return r;
#else
    uint8v r = a.vec(0);
    for (unsigned j = 1; j < a.vec_length; ++j) {
        r = min(r, a.vec(j));
    }
    return i_reduce_min(r);
#endif
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
int8_t i_reduce_min(const int8x16& a)
{
#if SIMDPP_USE_NULL
    int8_t r = a.el(0);
    for (unsigned i = 0; i < a.length; i++) {
        r = r < a.el(i) ? r : a.el(i);
    }
    return r;
#elif SIMDPP_USE_NEON64
    return vminvq_s8(a.native());
#elif SIMDPP_USE_SSE4_1 || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    int8x16 r = min(a, move16_l<8>(a));
    r = min(r, move16_l<4>(r));
    r = min(r, move16_l<2>(r));
    r = min(r, move16_l<1>(r));
    return extract<0>(r);
#elif SIMDPP_USE_SSE2
    // no instruction for int8 min available, only for uint8
    uint8x16 ca = bit_xor(a, 0x80);
    return i_reduce_min(ca) ^ 0x80;
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int8_t i_reduce_min(const int8x32& a)
{
    int8x16 r = detail::extract128<0>(a);
    r = min(r, detail::extract128<1>(a));
    return i_reduce_min(r);
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL int8_t i_reduce_min(const int8<64>& a)
{
    int8<32> r = detail::extract256<0>(a);
    r = min(r, detail::extract256<1>(a));
    return i_reduce_min(r);
}
#endif

template<unsigned N>
SIMDPP_INL int8_t i_reduce_min(const int8<N>& a)
{
#if SIMDPP_USE_NULL
    int8_t r = std::numeric_limits<int8_t>::max();
    for (unsigned j = 0; j < a.vec_length; ++j) {
        for (unsigned i = 0; i < a.base_length; i++) {
            r = r < a.vec(j).el(i) ? r : a.vec(j).el(i);
        }
    }
    return r;
#elif SIMDPP_USE_SSE2 && !SIMDPP_USE_SSE4_1
    // no instruction for int8 min available, only for uint8
    uint8x16 r = bit_xor(a.vec(0), 0x80);
    for (unsigned j = 1; j < a.vec_length; ++j) {
        uint8x16 ca = bit_xor(a.vec(j), 0x80);
        r = min(r, ca);
    }
    return i_reduce_min(r) ^ 0x80;
#else
    int8v r = a.vec(0);
    for (unsigned j = 1; j < a.vec_length; ++j) {
        r = min(r, a.vec(j));
    }
    return i_reduce_min(r);
#endif
}

// -----------------------------------------------------------------------------
static SIMDPP_INL
int16_t i_reduce_min(const int16x8& a);

static SIMDPP_INL
uint16_t i_reduce_min(const uint16x8& a)
{
#if SIMDPP_USE_NULL
    uint16_t r = a.el(0);
    for (unsigned i = 0; i < a.length; i++) {
        r = r < a.el(i) ? r : a.el(i);
    }
    return r;
#elif SIMDPP_USE_NEON64
    return vminvq_u16(a.native());
#elif SIMDPP_USE_SSE4_1 || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    uint16x8 r = min(a, move8_l<4>(a));
    r = min(r, move8_l<2>(r));
    r = min(r, move8_l<1>(r));
    return extract<0>(r);
#elif SIMDPP_USE_SSE2
    // no instruction for uint16 min available, only for int16
    int16x8 ca = bit_xor(a, 0x8000);
    return i_reduce_min(ca) ^ 0x8000;
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint16_t i_reduce_min(const uint16x16& a)
{
    uint16x8 r = detail::extract128<0>(a);
    r = min(r, detail::extract128<1>(a));
    return i_reduce_min(r);
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint16_t i_reduce_min(const uint16<32>& a)
{
    uint16<16> r = detail::extract256<0>(a);
    r = min(r, detail::extract256<1>(a));
    return i_reduce_min(r);
}
#endif

template<unsigned N>
SIMDPP_INL uint16_t i_reduce_min(const uint16<N>& a)
{
#if SIMDPP_USE_NULL
    uint16_t r = std::numeric_limits<uint16_t>::max();
    for (unsigned j = 0; j < a.vec_length; ++j) {
        for (unsigned i = 0; i < a.base_length; i++) {
            r = r < a.vec(j).el(i) ? r : a.vec(j).el(i);
        }
    }
    return r;
#elif SIMDPP_USE_SSE2 && !SIMDPP_USE_SSE4_1
    // no instruction for uint16 min available, only for int16
    int16x8 r = bit_xor(a.vec(0), 0x8000);
    for (unsigned j = 1; j < a.vec_length; ++j) {
        int16x8 ca = bit_xor(a.vec(j), 0x8000);
        r = min(r, ca);
    }
    return i_reduce_min(r) ^ 0x8000;
#else
    uint16v r = a.vec(0);
    for (unsigned j = 1; j < a.vec_length; ++j) {
        r = min(r, a.vec(j));
    }
    return i_reduce_min(r);
#endif
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
int16_t i_reduce_min(const int16x8& a)
{
#if SIMDPP_USE_NULL
    int16_t r = a.el(0);
    for (unsigned i = 0; i < a.length; i++) {
        r = r < a.el(i) ? r : a.el(i);
    }
    return r;
#elif SIMDPP_USE_NEON64
    return vminvq_s16(a.native());
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    int16x8 r = min(a, move8_l<4>(a));
    r = min(r, move8_l<2>(r));
    r = min(r, move8_l<1>(r));
    return extract<0>(r);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int16_t i_reduce_min(const int16x16& a)
{
    int16x8 r = detail::extract128<0>(a);
    r = min(r, detail::extract128<1>(a));
    return i_reduce_min(r);
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL int16_t i_reduce_min(const int16<32>& a)
{
    int16<16> r = detail::extract256<0>(a);
    r = min(r, detail::extract256<1>(a));
    return i_reduce_min(r);
}
#endif

template<unsigned N>
SIMDPP_INL int16_t i_reduce_min(const int16<N>& a)
{
#if SIMDPP_USE_NULL
    int16_t r = std::numeric_limits<int16_t>::max();
    for (unsigned j = 0; j < a.vec_length; ++j) {
        for (unsigned i = 0; i < a.base_length; i++) {
            r = r < a.vec(j).el(i) ? r : a.vec(j).el(i);
        }
    }
    return r;
#else
    int16v r = a.vec(0);
    for (unsigned j = 1; j < a.vec_length; ++j) {
        r = min(r, a.vec(j));
    }
    return i_reduce_min(r);
#endif
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint32_t i_reduce_min(const uint32x4& a)
{
#if SIMDPP_USE_NULL
    uint32_t r = a.el(0);
    for (unsigned i = 0; i < a.length; i++) {
        r = r < a.el(i) ? r : a.el(i);
    }
    return r;
#elif SIMDPP_USE_NEON64
    return vminvq_u32(a.native());
#elif SIMDPP_USE_SSE4_1 || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    uint32x4 r = min(a, move4_l<2>(a));
    r = min(r, move4_l<1>(r));
    return extract<0>(r);
#elif SIMDPP_USE_SSE2
    mem_block<uint32x4> b = a;
    uint32_t r = b[0];
    for (unsigned i = 1; i < b.length; i++) {
        r = r < b[i] ? r : b[i];
    }
    return r;
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint32_t i_reduce_min(const uint32x8& a)
{
    uint32x4 r = detail::extract128<0>(a);
    r = min(r, detail::extract128<1>(a));
    r = min(r, move4_l<2>(r));
    r = min(r, move4_l<1>(r));
    return extract<0>(r);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint32_t i_reduce_min(const uint32<16>& a)
{
    return i_reduce_min((uint32<8>)min(extract256<0>(a), extract256<1>(a)));
}
#endif

template<unsigned N>
SIMDPP_INL uint32_t i_reduce_min(const uint32<N>& a)
{
#if SIMDPP_USE_NULL
    uint32_t r = std::numeric_limits<uint32_t>::max();
    for (unsigned j = 0; j < a.vec_length; ++j) {
        for (unsigned i = 0; i < a.base_length; i++) {
            r = r < a.vec(j).el(i) ? r : a.vec(j).el(i);
        }
    }
    return r;
#else
    uint32v r = a.vec(0);
    for (unsigned j = 1; j < a.vec_length; ++j) {
        r = min(r, a.vec(j));
    }
    return i_reduce_min(r);
#endif
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
int32_t i_reduce_min(const int32x4& a)
{
#if SIMDPP_USE_NULL
    int32_t r = a.el(0);
    for (unsigned i = 1; i < a.length; i++) {
        r = r < a.el(i) ? r : a.el(i);
    }
    return r;
#elif SIMDPP_USE_NEON64
    return vminvq_s32(a.native());
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    int32x4 r = min(a, move4_l<2>(a));
    r = min(r, move4_l<1>(r));
    return extract<0>(r);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int32_t i_reduce_min(const int32x8& a)
{
    int32x4 r = detail::extract128<0>(a);
    r = min(r, detail::extract128<1>(a));
    r = min(r, move4_l<2>(r));
    r = min(r, move4_l<1>(r));
    return extract<0>(r);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
int32_t i_reduce_min(const int32<16>& a)
{
    return i_reduce_min((int32<8>)min(extract256<0>(a), extract256<1>(a)));
}
#endif

template<unsigned N>
SIMDPP_INL int32_t i_reduce_min(const int32<N>& a)
{
#if SIMDPP_USE_NULL
    int32_t r = std::numeric_limits<int32_t>::max();
    for (unsigned j = 0; j < a.vec_length; ++j) {
        for (unsigned i = 0; i < a.base_length; i++) {
            r = r < a.vec(j).el(i) ? r : a.vec(j).el(i);
        }
    }
    return r;
#else
    int32v r = a.vec(0);
    for (unsigned j = 1; j < a.vec_length; ++j) {
        r = min(r, a.vec(j));
    }
    return i_reduce_min(r);
#endif
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
uint64_t i_reduce_min(const uint64x2& a)
{
#if SIMDPP_USE_AVX2 || SIMDPP_USE_NEON64 || SIMDPP_USE_VSX_207 || SIMDPP_USE_MSA
    uint64x2 r = min(a, move2_l<1>(a));
    return extract<0>(r);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON32
    mem_block<uint64x2> b = a;
    return b[0] < b[1] ? b[0] : b[1];
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC
    uint64_t r = a.el(0);
    for (unsigned i = 0; i < a.length; i++) {
        r = r < a.el(i) ? r : a.el(i);
    }
    return r;
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint64_t i_reduce_min(const uint64x4& a)
{
    uint64x2 r = detail::extract128<0>(a);
    r = min(r, detail::extract128<1>(a));
    r = min(r, move2_l<1>(r));
    return extract<0>(r);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint64_t i_reduce_min(const uint64<8>& a)
{
    return i_reduce_min((uint64<4>)min(extract256<0>(a), extract256<1>(a)));
}
#endif

template<unsigned N>
SIMDPP_INL uint64_t i_reduce_min(const uint64<N>& a)
{
#if SIMDPP_USE_AVX2 || SIMDPP_USE_NEON64 || SIMDPP_USE_VSX_207 || SIMDPP_USE_MSA
    uint64v r = a.vec(0);
    for (unsigned j = 1; j < a.vec_length; ++j) {
        r = min(r, a.vec(j));
    }
    return i_reduce_min(r);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON32
    uint64_t r = std::numeric_limits<uint64_t>::max();
    for (unsigned j = 0; j < a.vec_length; ++j) {
        mem_block<uint64v> b = a.vec(j);
        for (unsigned i = 0; i < a.base_length; i++) {
            r = r < b[i] ? r : b[i];
        }
    }
    return r;
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC
    uint64_t r = std::numeric_limits<uint64_t>::max();
    for (unsigned j = 0; j < a.vec_length; ++j) {
        for (unsigned i = 0; i < a.base_length; i++) {
            r = r < a.vec(j).el(i) ? r : a.vec(j).el(i);
        }
    }
    return r;
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
int64_t i_reduce_min(const int64x2& a)
{
#if SIMDPP_USE_AVX2 || SIMDPP_USE_NEON64 || SIMDPP_USE_VSX_206 || SIMDPP_USE_MSA
    int64x2 r = min(a, move2_l<1>(a));
    return extract<0>(r);
#elif SIMDPP_USE_SSE2
    mem_block<int64x2> b = a;
    return b[0] < b[1] ? b[0] : b[1];
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC
    int64_t r = a.el(0);
    for (unsigned i = 0; i < a.length; i++) {
        r = r < a.el(i) ? r : a.el(i);
    }
    return r;
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
int64_t i_reduce_min(const int64x4& a)
{
    int64x2 r = detail::extract128<0>(a);
    r = min(r, detail::extract128<1>(a));
    r = min(r, move2_l<1>(r));
    return extract<0>(r);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
int64_t i_reduce_min(const int64<8>& a)
{
    return i_reduce_min((int64<4>)min(extract256<0>(a), extract256<1>(a)));
}
#endif

template<unsigned N>
SIMDPP_INL int64_t i_reduce_min(const int64<N>& a)
{
#if SIMDPP_USE_AVX2 || SIMDPP_USE_NEON64 || SIMDPP_USE_VSX_206 || SIMDPP_USE_MSA
    int64v r = a.vec(0);
    for (unsigned j = 1; j < a.vec_length; ++j) {
        r = min(r, a.vec(j));
    }
    return i_reduce_min(r);
#elif SIMDPP_USE_SSE2 || SIMDPP_USE_NEON32
    int64_t r = std::numeric_limits<int64_t>::max();
    for (unsigned j = 0; j < a.vec_length; ++j) {
        mem_block<int64v> b = a.vec(j);
        for (unsigned i = 0; i < a.base_length; i++) {
            r = r < b[i] ? r : b[i];
        }
    }
    return r;
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC
    int64_t r = std::numeric_limits<int64_t>::max();
    for (unsigned j = 0; j < a.vec_length; ++j) {
        for (unsigned i = 0; i < a.base_length; i++) {
            r = r < a.vec(j).el(i) ? r : a.vec(j).el(i);
        }
    }
    return r;
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}

// -----------------------------------------------------------------------------

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

