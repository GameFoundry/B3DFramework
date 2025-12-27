/*  Copyright (C) 2011-2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_EXTRACT_BITS_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_EXTRACT_BITS_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/bit_and.h>
#include <simdpp/core/bit_or.h>
#include <simdpp/core/extract.h>
#include <simdpp/core/i_shift_l.h>
#include <simdpp/core/i_sub.h>
#include <simdpp/core/make_uint.h>
#include <simdpp/core/move_l.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

SIMDPP_INL uint16_t i_extract_bits_any(const uint8<16>& ca)
{
    uint8<16> a = ca;
#if SIMDPP_USE_NULL
    uint16_t r = 0;
    for (unsigned i = 0; i < a.length; i++) {
        uint8_t x = ca.el(i);
        x = x & 1;
        r = (r >> 1) | (uint16_t(x) << 15);
    }
    return r;
#elif SIMDPP_USE_SSE2
    // Note that i_extract_bits depends on the exact implementation of this
    // function.
    return _mm_movemask_epi8(a.native());
#elif SIMDPP_USE_NEON
    uint8x16 mask = make_uint(0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80);

    a = bit_and(a, mask);
    uint16<8> a16 = vpaddlq_u8(a.native());
    uint32<4> a32 = vpaddlq_u16(a16.native());
    uint8<16> a8 = vreinterpretq_u8_u64(vpaddlq_u32(a32.native()));
    uint8x8_t r = vzip_u8(vget_low_u8(a8.native()), vget_high_u8(a8.native())).val[0];
    return vget_lane_u16(vreinterpret_u16_u8(r), 0);
#elif SIMDPP_USE_ALTIVEC
    uint8x16 mask = make_uint(0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80);
    a = bit_and(a, mask);
    uint32<4> zero = make_zero();
    uint32x4 s = vec_sum4s(a.native(), zero.native());
    uint32x4 shifts = make_uint(0, 0, 8, 8);
    s = (__vector uint32_t) vec_sl(s.native(), shifts.native());
    s = (int32x4)vec_sums((__vector int32_t)s.native(),
                          (__vector int32_t)zero.native());
#if SIMDPP_BIG_ENDIAN
    return extract<7>(uint16x8(s));
#else
    return extract<6>(uint16x8(s));
#endif
#elif SIMDPP_USE_MSA
    // Note: the implementation of extract_bits depends of the exact behavior
    // of this function
    uint8x16 mask = make_uint(0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80);

    a = bit_and(a, mask);
    uint16<8> a16 = __msa_hadd_u_h(a.native(), a.native());
    uint32<4> a32 = __msa_hadd_u_w(a16.native(), a16.native());
    a = (v16u8) __msa_hadd_u_d(a32.native(), a32.native());
    a = bit_or(a, move16_l<7>(a));
    return extract<0>((uint16<8>)a);
#endif
}

SIMDPP_INL uint32_t i_extract_bits_any(const uint8<32>& ca)
{
    uint8<32> a = ca;
#if SIMDPP_USE_AVX2
    return _mm256_movemask_epi8(a.native());
#else
    uint8<16> lo, hi;
    split(a, lo, hi);
    return i_extract_bits_any(lo) | (i_extract_bits_any(hi) << 16);
#endif
}

template<unsigned id> SIMDPP_INL
uint16_t i_extract_bits(const uint8<16>& ca)
{
    uint8<16> a = ca;
#if SIMDPP_USE_NULL
    uint16_t r = 0;
    for (unsigned i = 0; i < a.length; i++) {
        uint8_t x = ca.el(i);
        x = (x >> id) & 1;
        r = (r >> 1) | (uint16_t(x) << 15);
    }
    return r;
#elif SIMDPP_USE_SSE2
    a = shift_l<7-id>((uint16x8) a);
    return i_extract_bits_any(a);
#elif SIMDPP_USE_NEON
    int8x16 shift_mask = make_int(0-int(id), 1-int(id), 2-int(id), 3-int(id),
                                  4-int(id), 5-int(id), 6-int(id), 7-int(id));

    a = vshlq_u8(a.native(), shift_mask.native());
    return i_extract_bits_any(a);
#elif SIMDPP_USE_ALTIVEC
    uint8x16 rot_mask = make_int(0-int(id), 1-int(id), 2-int(id), 3-int(id),
                                 4-int(id), 5-int(id), 6-int(id), 7-int(id));
    a = vec_rl(a.native(), rot_mask.native());
    return i_extract_bits_any(a);
#elif SIMDPP_USE_MSA
    int8x16 shifts = make_int(0-int(id), 1-int(id), 2-int(id), 3-int(id),
                              4-int(id), 5-int(id), 6-int(id), 7-int(id));
    uint8<16> a_l = (v16u8) __msa_sll_b((v16i8) a.native(), shifts.native());
    shifts = sub((int8<16>) make_zero(), shifts);
    uint8<16> a_r = (v16u8) __msa_srl_b((v16i8) a.native(), shifts.native());
    a = bit_or(a_l, a_r);
    return i_extract_bits_any(a);
#endif
}

template<unsigned id> SIMDPP_INL
uint32_t i_extract_bits(const uint8<32>& ca)
{
    uint8<32> a = ca;
#if SIMDPP_USE_AVX2
    a = shift_l<7-id>((uint16<16>) a);
    return i_extract_bits_any(a);
#else
    uint8<16> lo, hi;
    split(a, lo, hi);
    return i_extract_bits<id>(lo) | (i_extract_bits<id>(hi) << 16);
#endif
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif


