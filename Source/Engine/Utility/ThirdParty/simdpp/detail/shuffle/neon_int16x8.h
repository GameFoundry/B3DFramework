/*  Copyright (C) 2011-2012  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_DETAIL_SHUFFLE_NEON_INT16x8_H
#define LIBSIMDPP_DETAIL_SHUFFLE_NEON_INT16x8_H
#if SIMDPP_USE_NEON

#include <simdpp/core/make_shuffle_bytes_mask.h>
#include <simdpp/core/permute_bytes16.h>
#include <type_traits>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace neon_shuffle_int16x8 {

/*
    The code below implements generalized permutations for 4 elements sets
    within int16x8 vectors using various shuffling instructions available on
    NEON. If no straightforward permutation is available, TBL instruction is
    used.

    Note: the compiler should optimize all masks into one VMOV #imm instruction
*/

using _0 = std::integral_constant<unsigned, 0>;
using _1 = std::integral_constant<unsigned, 1>;
using _2 = std::integral_constant<unsigned, 2>;
using _3 = std::integral_constant<unsigned, 3>;
using T = uint16x8;    // full vector
using H = uint16x4_t;       // half vector

/// Cost: 2
template<unsigned n> SIMDPP_INL
T bcast(T a)
{
    H h1 = vget_low_u16(a.native());
    H h2 = vget_high_u16(a.native());
    h1 = vdup_lane_u16(h1, n);
    h2 = vdup_lane_u16(h2, n);
    return vcombine_u16(h1, h2);
}

/** Selects the elements from two vectors according to selectors. 0 selects
    the value from the first vector, 1 selects from the second
    Cost: 2
*/
template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
T sel(T a, T b)
{
    const uint64_t um =
        (s0 > 0 ? 0xffffLL : 0) |
        (s1 > 0 ? 0xffffLL << 16 : 0) |
        (s2 > 0 ? 0xffffLL << 32 : 0) |
        (s3 > 0 ? 0xffffLL << 48 : 0);
    uint16x8_t mask = vreinterpretq_u16_u64(vmovq_n_u64(um));

    return vbslq_u16(mask, b.native(), a.native());
}

/** Within each 4-element set moves the elements to the left or right. The
    shifted-in values are undefined.
    Cost: 1
*/
template<unsigned shift> SIMDPP_INL
T mov_r(T a)
{
    return vreinterpretq_u16_u64(vshlq_n_u64(vreinterpretq_u64_u16(a.native()), shift*16));
}

template<unsigned shift> SIMDPP_INL
T mov_l(T a)
{
    return vreinterpretq_u16_u64(vshrq_n_u64(vreinterpretq_u64_u16(a.native()), shift*16));
}

/// Within each 4-element set: r0 = a3; r1 = a2; r2 = a1; r3 = a0; - 3210
/// Cost: 1
SIMDPP_INL T rev41(T a)
{
    return vrev64q_u16(a.native());
}

/// Within each 4-element set: r0 = a2; r1 = a3; r2 = a0; r3 = a1; - 2301
/// Cost: 1
SIMDPP_INL T rev42(T a)
{
    return vreinterpretq_u16_u32(vrev64q_u32(vreinterpretq_u32_u16(a.native())));
}

/// Within each 4-element set: r0 = a1; r1 = a0; r2 = a3; r3 = a2; - 1032
/// Cost: 1
SIMDPP_INL T rev21(T a)
{
    return vrev32q_u16(a.native());
}

/// Within each 4-element set: r0 = a0; r1 = a0; r2 = a2; r3 = a2; - 0022
/// Cost: 2
SIMDPP_INL T dup_lo(T a)
{
    T b = a;
    return vtrnq_u16(a.native(), b.native()).val[0];
}

/// Within each 4-element set: r0 = a1; r1 = a1; r2 = a3; r3 = a3; - 1133
/// Cost: 2
SIMDPP_INL T dup_hi(T a)
{
    T b = a;
    return vtrnq_u16(a.native(), b.native()).val[1];
}

/// Within each 4-element set: r0 = a0; r1 = a1; r2 = a0; r3 = a1; - 0101
/// Cost: 2
SIMDPP_INL T dup2_lo(T a)
{
    uint32x4_t i;
    uint32x2_t lo, hi;
    i = vreinterpretq_u32_u16(a.native());
    lo = vget_low_u32(i);
    hi = vget_high_u32(i);
    lo = vdup_lane_u32(lo, 0);
    hi = vdup_lane_u32(hi, 0);
    return vreinterpretq_u16_u32(vcombine_u32(lo, hi));
}

/// Within each 4-element set: r0 = a2; r1 = a3; r2 = a2; r3 = a3; - 2323
/// Cost: 2
SIMDPP_INL T dup2_hi(T a)
{
    uint32x4_t i;
    uint32x2_t lo, hi;
    i = vreinterpretq_u32_u16(a.native());
    lo = vget_low_u32(i);
    hi = vget_high_u32(i);
    lo = vdup_lane_u32(lo, 1);
    hi = vdup_lane_u32(hi, 1);
    return vreinterpretq_u16_u32(vcombine_u32(lo, hi));
}

/// Within each 4-element set: r0 = a0; r1 = a0; r2 = a1; r3 = a1; - 0011
/// Cost: 3
SIMDPP_INL T dup_unpack_lo(T a)
{
    H lo = vget_low_u16(a.native());
    H hi = vget_high_u16(a.native());
    H rlo = vzip_u16(lo, lo).val[0];
    H rhi = vzip_u16(hi, hi).val[0];
    return vcombine_u16(rlo, rhi);
}

/// Within each 4-element set: r0 = a2; r1 = a2; r2 = a3; r3 = a3; - 2233
/// Cost: 3
SIMDPP_INL T dup_unpack_hi(T a)
{
    H lo = vget_low_u16(a.native());
    H hi = vget_high_u16(a.native());
    H rlo = vzip_u16(lo, lo).val[1];
    H rhi = vzip_u16(hi, hi).val[1];
    return vcombine_u16(rlo, rhi);
}

/// Swaps the n1-th element with the element in n2-th position. The values of
/// other elements are undefined. Cost: 1
template<unsigned n1, unsigned n2> SIMDPP_INL
T swap1(T a)
{
    const unsigned sel = n1*4 + n2;
    switch (sel) {
    case 0: //00
    case 5: //11
    case 10: //22
    case 15: //33
        return a;
    case 1: //01
    case 4: //10
    case 11: //23
    case 14: //32
        return rev21(a);
    case 2: //02
    case 8: //20
    case 7:  //13
    case 13: //31
        return rev42(a);
    case 3:  //03
    case 12: //30
    case 6: //12
    case 9: //21
        return rev41(a);
    }
    return a;
}

/// Cost: 3 (includes one 16-byte load)
template<unsigned N0, unsigned N1, unsigned N2, unsigned N3> SIMDPP_INL
T fallback(T a)
{
    T mask;
    mask = make_shuffle_bytes16_mask<N0,N1,N2,N3>(mask);
    return permute_bytes16(a, mask);
}

static SIMDPP_INL
T perm4(_0,_0,_0,_0, T a) { return bcast<0>(a); }
SIMDPP_INL T perm4(_0,_0,_0,_1, T a) { return sel<0,0,0,1>(bcast<0>(a), swap1<1,3>(a)); }
SIMDPP_INL T perm4(_0,_0,_0,_2, T a) { return sel<0,0,0,1>(bcast<0>(a), swap1<2,3>(a)); }
SIMDPP_INL T perm4(_0,_0,_0,_3, T a) { return sel<1,1,1,0>(a, bcast<0>(a)); }
SIMDPP_INL T perm4(_0,_0,_1,_0, T a) { return sel<0,0,1,0>(bcast<0>(a), swap1<1,2>(a)); }
SIMDPP_INL T perm4(_0,_0,_1,_1, T a) { return dup_unpack_lo(a); }
SIMDPP_INL T perm4(_0,_0,_1,_2, T a) { return sel<0,1,1,1>(a, mov_r<1>(a)); }
SIMDPP_INL T perm4(_0,_0,_1,_3, T a) { return sel<0,1,1,0>(a, mov_r<1>(a)); }
SIMDPP_INL T perm4(_0,_0,_2,_0, T a) { return sel<1,1,0,1>(a, bcast<0>(a));}
//SIMDPP_INL T perm4(_0,_0,_2,_1, T a) { }
SIMDPP_INL T perm4(_0,_0,_2,_2, T a) { return dup_lo(a); }
SIMDPP_INL T perm4(_0,_0,_2,_3, T a) { return sel<0,1,0,0>(a, swap1<0,1>(a)); }
SIMDPP_INL T perm4(_0,_0,_3,_0, T a) { return sel<0,0,1,0>(bcast<0>(a), swap1<3,2>(a)); }
//SIMDPP_INL T perm4(_0,_0,_3,_1, T a) { }
SIMDPP_INL T perm4(_0,_0,_3,_2, T a) { return sel<0,1,1,1>(a, rev21(a)); }
SIMDPP_INL T perm4(_0,_0,_3,_3, T a) { return sel<0,1,1,0>(a, rev21(a)); }
SIMDPP_INL T perm4(_0,_1,_0,_0, T a) { return sel<1,0,1,1>(a, bcast<0>(a)); }
SIMDPP_INL T perm4(_0,_1,_0,_1, T a) { return dup2_lo(a); }
//SIMDPP_INL T perm4(_0,_1,_0,_2, T a) { }
SIMDPP_INL T perm4(_0,_1,_0,_3, T a) { return sel<0,0,1,0>(a, swap1<0,2>(a)); }
SIMDPP_INL T perm4(_0,_1,_1,_0, T a) { return sel<0,0,1,1>(a, rev41(a)); }
SIMDPP_INL T perm4(_0,_1,_1,_1, T a) { return sel<0,0,1,1>(a, bcast<1>(a)); }
SIMDPP_INL T perm4(_0,_1,_1,_2, T a) { return sel<0,0,1,1>(a, mov_r<1>(a)); }
SIMDPP_INL T perm4(_0,_1,_1,_3, T a) { return sel<0,0,1,0>(a, swap1<1,2>(a)); }
SIMDPP_INL T perm4(_0,_1,_2,_0, T a) { return sel<0,0,0,1>(a, swap1<0,3>(a)); }
SIMDPP_INL T perm4(_0,_1,_2,_1, T a) { return sel<0,0,0,1>(a, swap1<1,3>(a)); }
SIMDPP_INL T perm4(_0,_1,_2,_2, T a) { return sel<0,0,0,1>(a, swap1<2,3>(a)); }
SIMDPP_INL T perm4(_0,_1,_2,_3, T a) { return a; }
//SIMDPP_INL T perm4(_0,_1,_3,_0, T a) { }
//SIMDPP_INL T perm4(_0,_1,_3,_1, T a) { }
SIMDPP_INL T perm4(_0,_1,_3,_2, T a) { return sel<0,0,1,1>(a, rev21(a)); }
SIMDPP_INL T perm4(_0,_1,_3,_3, T a) { return sel<0,0,1,0>(a, swap1<3,2>(a)); }
SIMDPP_INL T perm4(_0,_2,_0,_0, T a) { return sel<0,1,0,0>(bcast<0>(a), swap1<2,1>(a)); }
//SIMDPP_INL T perm4(_0,_2,_0,_1, T a) { }
//SIMDPP_INL T perm4(_0,_2,_0,_2, T a) { }
//SIMDPP_INL T perm4(_0,_2,_0,_3, T a) { }
SIMDPP_INL T perm4(_0,_2,_1,_0, T a) { return sel<0,1,1,1>(a, rev41(a)); }
//SIMDPP_INL T perm4(_0,_2,_1,_1, T a) { }
//SIMDPP_INL T perm4(_0,_2,_1,_2, T a) { }
SIMDPP_INL T perm4(_0,_2,_1,_3, T a) { return sel<0,1,1,0>(a, rev41(a)); }
SIMDPP_INL T perm4(_0,_2,_2,_0, T a) { return sel<0,1,0,1>(a, rev41(a)); }
//SIMDPP_INL T perm4(_0,_2,_2,_1, T a) { }
SIMDPP_INL T perm4(_0,_2,_2,_2, T a) { return sel<0,1,1,1>(a, bcast<2>(a)); }
SIMDPP_INL T perm4(_0,_2,_2,_3, T a) { return sel<0,1,0,0>(a, swap1<2,1>(a)); }
//SIMDPP_INL T perm4(_0,_2,_3,_0, T a) { }
//SIMDPP_INL T perm4(_0,_2,_3,_1, T a) { }
//SIMDPP_INL T perm4(_0,_2,_3,_2, T a) { }
SIMDPP_INL T perm4(_0,_2,_3,_3, T a) { return sel<0,1,1,0>(a, mov_l<1>(a)); }
SIMDPP_INL T perm4(_0,_3,_0,_0, T a) { return sel<1,0,1,1>(swap1<3,1>(a), bcast<0>(a)); }
SIMDPP_INL T perm4(_0,_3,_0,_1, T a) { return sel<0,1,1,1>(a, rev42(a)); }
//SIMDPP_INL T perm4(_0,_3,_0,_2, T a) { }
SIMDPP_INL T perm4(_0,_3,_0,_3, T a) { return sel<0,1,1,0>(a, rev42(a)); }
//SIMDPP_INL T perm4(_0,_3,_1,_0, T a) { }
//SIMDPP_INL T perm4(_0,_3,_1,_1, T a) { }
//SIMDPP_INL T perm4(_0,_3,_1,_2, T a) { }
//SIMDPP_INL T perm4(_0,_3,_1,_3, T a) { }
//SIMDPP_INL T perm4(_0,_3,_2,_0, T a) { }
SIMDPP_INL T perm4(_0,_3,_2,_1, T a) { return sel<0,1,0,1>(a, swap1<1,3>(a)); }
//SIMDPP_INL T perm4(_0,_3,_2,_2, T a) { }
SIMDPP_INL T perm4(_0,_3,_2,_3, T a) { return sel<0,1,0,0>(a, swap1<3,1>(a)); }
//SIMDPP_INL T perm4(_0,_3,_3,_0, T a) { }
//SIMDPP_INL T perm4(_0,_3,_3,_1, T a) { }
//SIMDPP_INL T perm4(_0,_3,_3,_2, T a) { }
SIMDPP_INL T perm4(_0,_3,_3,_3, T a) { return sel<0,1,1,1>(a, bcast<3>(a)); }
SIMDPP_INL T perm4(_1,_0,_0,_0, T a) { return sel<0,1,1,1>(swap1<1,0>(a), bcast<0>(a)); }
SIMDPP_INL T perm4(_1,_0,_0,_1, T a) { return rev21(sel<0,0,1,1>(a, rev41(a))); }
//SIMDPP_INL T perm4(_1,_0,_0,_2, T a) { }
//SIMDPP_INL T perm4(_1,_0,_0,_3, T a) { }
SIMDPP_INL T perm4(_1,_0,_1,_0, T a) { return rev21(dup2_lo(a)); }
SIMDPP_INL T perm4(_1,_0,_1,_1, T a) { return sel<1,0,1,1>(swap1<1,0>(a), bcast<1>(a)); }
SIMDPP_INL T perm4(_1,_0,_1,_2, T a) { return sel<0,1,1,1>(swap1<1,0>(a), mov_r<1>(a)); }
//SIMDPP_INL T perm4(_1,_0,_1,_3, T a) { }
//SIMDPP_INL T perm4(_1,_0,_2,_0, T a) { }
//SIMDPP_INL T perm4(_1,_0,_2,_1, T a) { }
SIMDPP_INL T perm4(_1,_0,_2,_2, T a) { return sel<0,0,1,1>(swap1<0,1>(a), bcast<2>(a)); }
SIMDPP_INL T perm4(_1,_0,_2,_3, T a) { return sel<1,1,0,0>(a, swap1<0,1>(a)); }
//SIMDPP_INL T perm4(_1,_0,_3,_0, T a) { }
//SIMDPP_INL T perm4(_1,_0,_3,_1, T a) { }
SIMDPP_INL T perm4(_1,_0,_3,_2, T a) { return rev21(a); }
SIMDPP_INL T perm4(_1,_0,_3,_3, T a) { return sel<0,0,1,1>(swap1<0,1>(a), bcast<3>(a)); }
SIMDPP_INL T perm4(_1,_1,_0,_0, T a) { return rev42(dup_unpack_lo(a)); }
SIMDPP_INL T perm4(_1,_1,_0,_1, T a) { return sel<1,1,0,1>(swap1<0,2>(a), bcast<1>(a));}
//SIMDPP_INL T perm4(_1,_1,_0,_2, T a) { }
//SIMDPP_INL T perm4(_1,_1,_0,_3, T a) { }
SIMDPP_INL T perm4(_1,_1,_1,_0, T a) { return sel<1,1,1,0>(swap1<0,3>(a), bcast<1>(a)); }
SIMDPP_INL T perm4(_1,_1,_1,_1, T a) { return bcast<1>(a); }
SIMDPP_INL T perm4(_1,_1,_1,_2, T a) { return sel<1,1,1,0>(swap1<2,3>(a), bcast<1>(a)); }
SIMDPP_INL T perm4(_1,_1,_1,_3, T a) { return sel<1,1,1,0>(a, bcast<1>(a)); }
//SIMDPP_INL T perm4(_1,_1,_2,_0, T a) { }
SIMDPP_INL T perm4(_1,_1,_2,_1, T a) { return sel<1,1,0,1>(a, bcast<1>(a)); }
SIMDPP_INL T perm4(_1,_1,_2,_2, T a) { return sel<1,0,0,1>(a, rev21(a)); }
SIMDPP_INL T perm4(_1,_1,_2,_3, T a) { return sel<1,0,0,0>(a, swap1<1,0>(a)); }
//SIMDPP_INL T perm4(_1,_1,_3,_0, T a) { }
SIMDPP_INL T perm4(_1,_1,_3,_1, T a) { return sel<1,1,0,1>(swap1<3,2>(a), bcast<1>(a)); }
//SIMDPP_INL T perm4(_1,_1,_3,_2, T a) { }
SIMDPP_INL T perm4(_1,_1,_3,_3, T a) { return dup_hi(a); }
//SIMDPP_INL T perm4(_1,_2,_0,_0, T a) { }
SIMDPP_INL T perm4(_1,_2,_0,_1, T a) { return sel<1,1,0,0>(rev42(a), mov_l<1>(a)); }
//SIMDPP_INL T perm4(_1,_2,_0,_2, T a) { }
//SIMDPP_INL T perm4(_1,_2,_0,_3, T a) { }
SIMDPP_INL T perm4(_1,_2,_1,_0, T a) { a = rev41(a); return sel<1,0,0,0>(a, swap1<2,0>(a)); }
SIMDPP_INL T perm4(_1,_2,_1,_1, T a) { return sel<1,0,1,1>(swap1<2,1>(a), bcast<1>(a)); }
//SIMDPP_INL T perm4(_1,_2,_1,_2, T a) { }
//SIMDPP_INL T perm4(_1,_2,_1,_3, T a) { }
//SIMDPP_INL T perm4(_1,_2,_2,_0, T a) { }
//SIMDPP_INL T perm4(_1,_2,_2,_1, T a) { }
SIMDPP_INL T perm4(_1,_2,_2,_2, T a) { return sel<0,1,1,1>(swap1<1,0>(a), bcast<2>(a)); }
//SIMDPP_INL T perm4(_1,_2,_2,_3, T a) { }
SIMDPP_INL T perm4(_1,_2,_3,_0, T a) { return sel<0,0,0,1>(mov_l<1>(a), swap1<0,3>(a)); }
SIMDPP_INL T perm4(_1,_2,_3,_1, T a) { return sel<0,0,0,1>(mov_l<1>(a), swap1<1,3>(a)); }
SIMDPP_INL T perm4(_1,_2,_3,_2, T a) { return sel<0,0,0,1>(mov_l<1>(a), swap1<2,3>(a)); }
SIMDPP_INL T perm4(_1,_2,_3,_3, T a) { return sel<0,0,0,1>(mov_l<1>(a), a); }
//SIMDPP_INL T perm4(_1,_3,_0,_0, T a) { }
SIMDPP_INL T perm4(_1,_3,_0,_1, T a) { return rev42(sel<0,0,1,0>(a, swap1<1,2>(a))); }
//SIMDPP_INL T perm4(_1,_3,_0,_2, T a) { }
//SIMDPP_INL T perm4(_1,_3,_0,_3, T a) { }
//SIMDPP_INL T perm4(_1,_3,_1,_0, T a) { }
SIMDPP_INL T perm4(_1,_3,_1,_1, T a) { return sel<1,0,1,1>(swap1<3,1>(a), bcast<1>(a)); }
//SIMDPP_INL T perm4(_1,_3,_1,_2, T a) { }
//SIMDPP_INL T perm4(_1,_3,_1,_3, T a) { }
//SIMDPP_INL T perm4(_1,_3,_2,_0, T a) { }
//SIMDPP_INL T perm4(_1,_3,_2,_1, T a) { }
//SIMDPP_INL T perm4(_1,_3,_2,_2, T a) { }
//SIMDPP_INL T perm4(_1,_3,_2,_3, T a) { }
//SIMDPP_INL T perm4(_1,_3,_3,_0, T a) { }
//SIMDPP_INL T perm4(_1,_3,_3,_1, T a) { }
//SIMDPP_INL T perm4(_1,_3,_3,_2, T a) { }
SIMDPP_INL T perm4(_1,_3,_3,_3, T a) { return sel<0,1,1,1>(swap1<1,0>(a), bcast<3>(a)); }
SIMDPP_INL T perm4(_2,_0,_0,_0, T a) { return sel<0,1,1,1>(swap1<2,0>(a), bcast<0>(a)); }
SIMDPP_INL T perm4(_2,_0,_0,_1, T a) { return rev42(sel<0,0,0,1>(a, swap1<0,3>(a))); }
//SIMDPP_INL T perm4(_2,_0,_0,_2, T a) { }
//SIMDPP_INL T perm4(_2,_0,_0,_3, T a) { }
//SIMDPP_INL T perm4(_2,_0,_1,_0, T a) { }
//SIMDPP_INL T perm4(_2,_0,_1,_1, T a) { }
SIMDPP_INL T perm4(_2,_0,_1,_2, T a) { return sel<0,1,1,1>(swap1<2,0>(a), mov_r<1>(a)); }
//SIMDPP_INL T perm4(_2,_0,_1,_3, T a) { }
//SIMDPP_INL T perm4(_2,_0,_2,_0, T a) { }
//SIMDPP_INL T perm4(_2,_0,_2,_1, T a) { }
SIMDPP_INL T perm4(_2,_0,_2,_2, T a) { return sel<1,0,1,1>(swap1<0,1>(a), bcast<2>(a)); }
//SIMDPP_INL T perm4(_2,_0,_2,_3, T a) { }
//SIMDPP_INL T perm4(_2,_0,_3,_0, T a) { }
//SIMDPP_INL T perm4(_2,_0,_3,_1, T a) { }
//SIMDPP_INL T perm4(_2,_0,_3,_2, T a) { }
//SIMDPP_INL T perm4(_2,_0,_3,_3, T a) { }
//SIMDPP_INL T perm4(_2,_1,_0,_0, T a) { }
SIMDPP_INL T perm4(_2,_1,_0,_1, T a) { return rev42(sel<0,0,0,1>(a, swap1<1,3>(a))); }
//SIMDPP_INL T perm4(_2,_1,_0,_2, T a) { }
SIMDPP_INL T perm4(_2,_1,_0,_3, T a) { return sel<1,0,1,0>(a, swap1<0,2>(a)); }
//SIMDPP_INL T perm4(_2,_1,_1,_0, T a) { }
SIMDPP_INL T perm4(_2,_1,_1,_1, T a) { return sel<0,1,1,1>(swap1<2,0>(a), bcast<1>(a)); }
//SIMDPP_INL T perm4(_2,_1,_1,_2, T a) { }
//SIMDPP_INL T perm4(_2,_1,_1,_3, T a) { }
//SIMDPP_INL T perm4(_2,_1,_2,_0, T a) { }
//SIMDPP_INL T perm4(_2,_1,_2,_1, T a) { }
SIMDPP_INL T perm4(_2,_1,_2,_2, T a) { return sel<1,0,1,1>(a, bcast<2>(a)); }
SIMDPP_INL T perm4(_2,_1,_2,_3, T a) { return sel<1,0,0,0>(a, swap1<2,0>(a)); }
//SIMDPP_INL T perm4(_2,_1,_3,_0, T a) { }
//SIMDPP_INL T perm4(_2,_1,_3,_1, T a) { }
//SIMDPP_INL T perm4(_2,_1,_3,_2, T a) { }
//SIMDPP_INL T perm4(_2,_1,_3,_3, T a) { }
SIMDPP_INL T perm4(_2,_2,_0,_0, T a) { return rev41(dup_lo(a)); }
SIMDPP_INL T perm4(_2,_2,_0,_1, T a) { return rev42(sel<0,0,0,1>(a, swap1<2,3>(a))); }
SIMDPP_INL T perm4(_2,_2,_0,_2, T a) { return sel<1,1,0,1>(swap1<0,2>(a), bcast<2>(a)); }
//SIMDPP_INL T perm4(_2,_2,_0,_3, T a) { }
SIMDPP_INL T perm4(_2,_2,_1,_0, T a) { a = rev41(a); return sel<1,0,0,0>(a, swap1<1,0>(a)); }
SIMDPP_INL T perm4(_2,_2,_1,_1, T a) { a = rev41(a); return sel<1,0,0,1>(a, rev21(a)); }
SIMDPP_INL T perm4(_2,_2,_1,_2, T a) { return sel<1,1,0,1>(swap1<1,2>(a), bcast<2>(a)); }
//SIMDPP_INL T perm4(_2,_2,_1,_3, T a) { }
SIMDPP_INL T perm4(_2,_2,_2,_0, T a) { return sel<1,1,1,0>(swap1<0,3>(a), bcast<2>(a)); }
SIMDPP_INL T perm4(_2,_2,_2,_1, T a) { return sel<1,1,1,0>(swap1<1,3>(a), bcast<2>(a)); }
SIMDPP_INL T perm4(_2,_2,_2,_2, T a) { return bcast<2>(a); }
SIMDPP_INL T perm4(_2,_2,_2,_3, T a) { return sel<1,1,1,0>(a, bcast<2>(a)); }
//SIMDPP_INL T perm4(_2,_2,_3,_0, T a) { }
//SIMDPP_INL T perm4(_2,_2,_3,_1, T a) { }
SIMDPP_INL T perm4(_2,_2,_3,_2, T a) { return sel<1,1,0,1>(swap1<3,2>(a), bcast<2>(a)); }
SIMDPP_INL T perm4(_2,_2,_3,_3, T a) { return dup_unpack_hi(a); }
SIMDPP_INL T perm4(_2,_3,_0,_0, T a) { return rev42(sel<0,1,0,0>(a, swap1<0,1>(a))); }
SIMDPP_INL T perm4(_2,_3,_0,_1, T a) { return rev42(a); }
SIMDPP_INL T perm4(_2,_3,_0,_2, T a) { return rev42(sel<0,1,0,0>(a, swap1<2,1>(a))); }
SIMDPP_INL T perm4(_2,_3,_0,_3, T a) { return rev42(sel<0,1,0,0>(a, swap1<3,1>(a))); }
SIMDPP_INL T perm4(_2,_3,_1,_0, T a) { return rev42(sel<1,1,0,0>(a, swap1<1,0>(a))); }
SIMDPP_INL T perm4(_2,_3,_1,_1, T a) { return rev42(sel<1,0,0,0>(a, swap1<1,0>(a))); }
SIMDPP_INL T perm4(_2,_3,_1,_2, T a) { return sel<0,0,1,1>(rev42(a), mov_r<1>(a)); }
//SIMDPP_INL T perm4(_2,_3,_1,_3, T a) { }
//SIMDPP_INL T perm4(_2,_3,_2,_0, T a) { }
SIMDPP_INL T perm4(_2,_3,_2,_1, T a) { return rev42(sel<1,0,0,0>(a, swap1<2,0>(a))); }
SIMDPP_INL T perm4(_2,_3,_2,_2, T a) { return sel<1,0,1,1>(swap1<3,1>(a), bcast<2>(a)); }
SIMDPP_INL T perm4(_2,_3,_2,_3, T a) { return dup2_hi(a); }
//SIMDPP_INL T perm4(_2,_3,_3,_0, T a) { }
SIMDPP_INL T perm4(_2,_3,_3,_1, T a) { return rev42(sel<1,0,0,0>(a, swap1<3,0>(a))); }
SIMDPP_INL T perm4(_2,_3,_3,_2, T a) { return rev21(sel<1,1,0,0>(a, rev41(a))); }
SIMDPP_INL T perm4(_2,_3,_3,_3, T a) { return sel<0,1,1,1>(swap1<2,0>(a), bcast<3>(a)); }
SIMDPP_INL T perm4(_3,_0,_0,_0, T a) { return sel<0,1,1,1>(swap1<3,0>(a), bcast<0>(a)); }
//SIMDPP_INL T perm4(_3,_0,_0,_1, T a) { }
//SIMDPP_INL T perm4(_3,_0,_0,_2, T a) { }
//SIMDPP_INL T perm4(_3,_0,_0,_3, T a) { }
SIMDPP_INL T perm4(_3,_0,_1,_0, T a) { a = rev41(a); return sel<0,1,0,0>(a, swap1<3,1>(a)); }
//SIMDPP_INL T perm4(_3,_0,_1,_1, T a) { }
SIMDPP_INL T perm4(_3,_0,_1,_2, T a) { return sel<0,1,1,1>(swap1<3,0>(a), mov_r<1>(a)); }
//SIMDPP_INL T perm4(_3,_0,_1,_3, T a) { }
//SIMDPP_INL T perm4(_3,_0,_2,_0, T a) { }
//SIMDPP_INL T perm4(_3,_0,_2,_1, T a) { }
//SIMDPP_INL T perm4(_3,_0,_2,_2, T a) { }
//SIMDPP_INL T perm4(_3,_0,_2,_3, T a) { }
//SIMDPP_INL T perm4(_3,_0,_3,_0, T a) { }
//SIMDPP_INL T perm4(_3,_0,_3,_1, T a) { }
//SIMDPP_INL T perm4(_3,_0,_3,_2, T a) { }
SIMDPP_INL T perm4(_3,_0,_3,_3, T a) { return sel<1,0,1,1>(swap1<0,1>(a), bcast<3>(a)); }
//SIMDPP_INL T perm4(_3,_1,_0,_0, T a) { }
//SIMDPP_INL T perm4(_3,_1,_0,_1, T a) { }
//SIMDPP_INL T perm4(_3,_1,_0,_2, T a) { }
//SIMDPP_INL T perm4(_3,_1,_0,_3, T a) { }
SIMDPP_INL T perm4(_3,_1,_1,_0, T a) { a = rev41(a); return sel<0,1,0,0>(a, swap1<2,1>(a)); }
SIMDPP_INL T perm4(_3,_1,_1,_1, T a) { return sel<0,1,1,1>(swap1<3,0>(a), bcast<1>(a)); }
//SIMDPP_INL T perm4(_3,_1,_1,_2, T a) { }
SIMDPP_INL T perm4(_3,_1,_1,_3, T a) { return sel<1,0,1,0>(a, rev41(a)); }
SIMDPP_INL T perm4(_3,_1,_2,_0, T a) { return sel<1,0,0,1>(a, swap1<3,0>(a)); }
//SIMDPP_INL T perm4(_3,_1,_2,_1, T a) { }
//SIMDPP_INL T perm4(_3,_1,_2,_2, T a) { }
SIMDPP_INL T perm4(_3,_1,_2,_3, T a) { return sel<1,0,0,0>(a, swap1<3,0>(a)); }
//SIMDPP_INL T perm4(_3,_1,_3,_0, T a) { }
//SIMDPP_INL T perm4(_3,_1,_3,_1, T a) { }
//SIMDPP_INL T perm4(_3,_1,_3,_2, T a) { }
SIMDPP_INL T perm4(_3,_1,_3,_3, T a) { return sel<1,0,1,1>(a, bcast<3>(a)); }
SIMDPP_INL T perm4(_3,_2,_0,_0, T a) { return rev41(sel<0,1,0,0>(a, swap1<0,1>(a))); }
SIMDPP_INL T perm4(_3,_2,_0,_1, T a) { return rev41(sel<1,1,0,0>(a, swap1<0,1>(a))); }
//SIMDPP_INL T perm4(_3,_2,_0,_2, T a) { }
//SIMDPP_INL T perm4(_3,_2,_0,_3, T a) { }
SIMDPP_INL T perm4(_3,_2,_1,_0, T a) { return rev41(a); }
SIMDPP_INL T perm4(_3,_2,_1,_1, T a) { a = rev41(a); return sel<0,0,0,1>(a, swap1<2,3>(a)); }
SIMDPP_INL T perm4(_3,_2,_1,_2, T a) { a = rev41(a); return sel<0,0,0,1>(a, swap1<1,3>(a)); }
SIMDPP_INL T perm4(_3,_2,_1,_3, T a) { a = rev41(a); return sel<0,0,0,1>(a, swap1<0,3>(a)); }
SIMDPP_INL T perm4(_3,_2,_2,_0, T a) { return rev41(sel<0,1,0,0>(a, swap1<2,1>(a))); }
SIMDPP_INL T perm4(_3,_2,_2,_1, T a) { a = rev41(a); return sel<0,0,1,1>(a, mov_r<1>(a)); }
SIMDPP_INL T perm4(_3,_2,_2,_2, T a) { return sel<0,1,1,1>(swap1<3,0>(a), bcast<2>(a)); }
SIMDPP_INL T perm4(_3,_2,_2,_3, T a) { return sel<1,1,0,0>(a, rev41(a)); }
SIMDPP_INL T perm4(_3,_2,_3,_0, T a) { return rev41(sel<0,1,0,0>(a, swap1<3,1>(a))); }
//SIMDPP_INL T perm4(_3,_2,_3,_1, T a) { }
SIMDPP_INL T perm4(_3,_2,_3,_2, T a) { return rev21(dup2_hi(a)); }
SIMDPP_INL T perm4(_3,_2,_3,_3, T a) { return sel<1,0,1,1>(swap1<2,1>(a), bcast<3>(a)); }
SIMDPP_INL T perm4(_3,_3,_0,_0, T a) { return rev42(sel<0,1,1,0>(a, rev21(a))); }
SIMDPP_INL T perm4(_3,_3,_0,_1, T a) { return rev42(sel<0,0,1,0>(a, swap1<3,2>(a))); }
//SIMDPP_INL T perm4(_3,_3,_0,_2, T a) { }
SIMDPP_INL T perm4(_3,_3,_0,_3, T a) { return sel<1,1,0,1>(swap1<0,2>(a), bcast<3>(a)); }
SIMDPP_INL T perm4(_3,_3,_1,_0, T a) { a = rev41(a); return sel<0,1,0,0>(a, swap1<0,1>(a)); }
SIMDPP_INL T perm4(_3,_3,_1,_1, T a) { return rev41(dup_hi(a)); }
SIMDPP_INL T perm4(_3,_3,_1,_2, T a) { return sel<1,1,0,0>(mov_r<1>(a), bcast<3>(a)); }
SIMDPP_INL T perm4(_3,_3,_1,_3, T a) { return sel<1,1,0,1>(swap1<1,2>(a), bcast<3>(a)); }
SIMDPP_INL T perm4(_3,_3,_2,_0, T a) { a = rev41(a); return sel<0,1,1,0>(a, mov_r<1>(a)); }
SIMDPP_INL T perm4(_3,_3,_2,_1, T a) { a = rev41(a); return sel<0,1,1,1>(a, mov_r<1>(a)); }
SIMDPP_INL T perm4(_3,_3,_2,_2, T a) { return rev41(dup_unpack_hi(a)); }
SIMDPP_INL T perm4(_3,_3,_2,_3, T a) { return sel<1,1,0,1>(a, bcast<3>(a)); }
SIMDPP_INL T perm4(_3,_3,_3,_0, T a) { return sel<1,1,1,0>(swap1<0,3>(a), bcast<3>(a)); }
SIMDPP_INL T perm4(_3,_3,_3,_1, T a) { return sel<1,1,1,0>(swap1<1,3>(a), bcast<3>(a)); }
SIMDPP_INL T perm4(_3,_3,_3,_2, T a) { return sel<1,1,1,0>(swap1<2,3>(a), bcast<3>(a)); }
SIMDPP_INL T perm4(_3,_3,_3,_3, T a) { return bcast<3>(a); }

template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
T perm4(std::integral_constant<unsigned, s0>,
        std::integral_constant<unsigned, s1>,
        std::integral_constant<unsigned, s2>,
        std::integral_constant<unsigned, s3>, T a)
{
    return fallback<s0,s1,s2,s3>(a);
}

template<unsigned s0, unsigned s1, unsigned s2, unsigned s3> SIMDPP_INL
T permute4(T a)
{
    return perm4(std::integral_constant<unsigned, s0>{},
                 std::integral_constant<unsigned, s1>{},
                 std::integral_constant<unsigned, s2>{},
                 std::integral_constant<unsigned, s3>{}, a);
}

} // namespace neon_shuffle_int16x8
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
#endif
