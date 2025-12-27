/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_SHUFFLE_ZBYTES16_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_SHUFFLE_ZBYTES16_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/bit_andnot.h>
#include <simdpp/core/bit_or.h>
#include <simdpp/core/blend.h>
#include <simdpp/core/cmp_lt.h>
#include <simdpp/core/shuffle_bytes16.h>
#include <simdpp/core/i_add.h>
#include <simdpp/detail/not_implemented.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

#if _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800)
#endif

static SIMDPP_INL
uint8x16 i_shuffle_zbytes16(const uint8x16& a, const uint8x16& b, const uint8x16& mask)
{
#if SIMDPP_USE_NULL
    uint8x16 r;

    for (unsigned i = 0; i < 16; i++) {
        unsigned j = mask.el(i) & 0x0f;
        unsigned which = mask.el(i) < 0x10;
        bool zero = mask.el(i) & 0x80;
        r.el(i) = zero ? 0 : (which ? a.el(j) : b.el(j));
    }
    return r;
#elif SIMDPP_USE_XOP
    return _mm_perm_epi8(a.native(), b.native(), mask.native());
#elif SIMDPP_USE_SSE4_1
    uint8<16> sel = _mm_slli_epi16(mask.native(), 3);

    uint8<16> ai = _mm_shuffle_epi8(a.native(), mask.native());
    uint8<16> bi = _mm_shuffle_epi8(b.native(), mask.native());
    return _mm_blendv_epi8(ai.native(), bi.native(), sel.native());
#elif SIMDPP_USE_SSSE3
    mask_int8<16> select_a = cmp_lt((int8<16>) bit_and(mask, 0x1f), 0x10);

    uint8<16> ai = _mm_shuffle_epi8(a.native(), mask.native());
    uint8<16> bi = _mm_shuffle_epi8(b.native(), mask.native());
    return blend(ai, bi, select_a);
#elif SIMDPP_USE_NEON
    return shuffle_bytes16(a, b, mask);
#elif SIMDPP_USE_ALTIVEC
    int8x16 a0 = a, b0 = b;
    int8x16 zero_mask = mask;
    zero_mask = shift_r<7>(zero_mask); // shift in the sign bit
    a0 = i_shuffle_bytes16(a0, b0, mask);
    a0 = bit_andnot(a0, zero_mask);
    return a0;
#elif SIMDPP_USE_MSA
    return (v16u8) __msa_vshf_b((v16i8) mask.native(),
                                (v16i8) b.native(),
                                (v16i8) a.native());
#else
    return SIMDPP_NOT_IMPLEMENTED3(a, b, mask);
#endif
}

#if _MSC_VER
#pragma warning(pop)
#endif

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint8x32 i_shuffle_zbytes16(const uint8x32& a, const uint8x32& b, const uint8x32& mask)
{
    int8x32 sel, ai, bi, r;
    sel = mask;
    sel = _mm256_slli_epi16(sel.native(), 3);

    ai = _mm256_shuffle_epi8(a.native(), mask.native());
    bi = _mm256_shuffle_epi8(b.native(), mask.native());
    r = _mm256_blendv_epi8(ai.native(), bi.native(), sel.native());
    return r;
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL uint8<64> i_shuffle_zbytes16(const uint8<64>& a, const uint8<64>& b, const uint8<64>& mask)
{
    uint8<64> sel_mask, ai, bi, r;
    sel_mask = make_uint(0x10);
    __mmask64 sel = _mm512_test_epi8_mask(mask.native(), sel_mask.native());

    ai = _mm512_shuffle_epi8(a.native(), mask.native());
    bi = _mm512_shuffle_epi8(b.native(), mask.native());
    r = _mm512_mask_blend_epi8(sel, ai.native(), bi.native());
    return r;
}
#endif

template<unsigned N> SIMDPP_INL
uint8<N> i_shuffle_zbytes16(const uint8<N>& a, const uint8<N>& b, const uint8<N>& mask)
{
    SIMDPP_VEC_ARRAY_IMPL3(uint8<N>, i_shuffle_zbytes16, a, b, mask);
}

template<unsigned N> SIMDPP_INL
uint16<N> i_shuffle_zbytes16(const uint16<N>& a, const uint16<N>& b, const uint16<N>& mask)
{
    return (uint16<N>) i_shuffle_zbytes16(uint8<N*2>(a), uint8<N*2>(b), uint8<N*2>(mask));
}
template<unsigned N> SIMDPP_INL
uint32<N> i_shuffle_zbytes16(const uint32<N>& a, const uint32<N>& b, const uint32<N>& mask)
{
    return (uint32<N>) i_shuffle_zbytes16(uint8<N*4>(a), uint8<N*4>(b), uint8<N*4>(mask));
}
template<unsigned N> SIMDPP_INL
uint64<N> i_shuffle_zbytes16(const uint64<N>& a, const uint64<N>& b, const uint64<N>& mask)
{
    return (uint64<N>) i_shuffle_zbytes16(uint8<N*8>(a), uint8<N*8>(b), uint8<N*8>(mask));
}
template<unsigned N> SIMDPP_INL
float32<N> i_shuffle_zbytes16(const float32<N>& a, const float32<N>& b, const uint32<N>& mask)
{
    return float32<N>(i_shuffle_zbytes16(uint32<N>(a), uint32<N>(b), mask));
}
template<unsigned N> SIMDPP_INL
float64<N> i_shuffle_zbytes16(const float64<N>& a, const float64<N>& b, const uint64<N>& mask)
{
    return float64<N>(i_shuffle_zbytes16(uint64<N>(a), uint64<N>(b), mask));
}


} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

