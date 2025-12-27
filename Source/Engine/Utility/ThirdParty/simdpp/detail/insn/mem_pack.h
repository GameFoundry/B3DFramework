/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_MEM_PACK_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_MEM_PACK_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/width.h>
#include <simdpp/detail/insn/shuffle128.h>
#include <simdpp/core/bit_andnot.h>
#include <simdpp/core/bit_or.h>
#include <simdpp/core/move_l.h>
#include <simdpp/core/move_r.h>
#include <simdpp/core/permute4.h>
#include <simdpp/core/shuffle2.h>
#include <simdpp/core/transpose.h>
#include <simdpp/core/zip_hi.h>
#include <simdpp/core/zip_lo.h>
#include <simdpp/detail/insn/zip128.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

/** Interleaves the elements of @a a and @a b in such way that:
     * every (2n)-th element comes from @a a
     * every (2n+1)-th element comes from @a b

    n = [0, <number of elements in vector> - 1]
*/
template<class V> SIMDPP_INL
void mem_pack2(any_vec<16,V>& qa, any_vec<16,V>& qb)
{
    V a = qa.wrapped();
    V b = qb.wrapped();

    qa.wrapped() = zip128_lo(a, b);
    qb.wrapped() = zip128_hi(a, b);
}

template<class V> SIMDPP_INL
void mem_pack2(any_vec<32,V>& qa, any_vec<32,V>& qb)
{
    V a = qa.wrapped();
    V b = qb.wrapped();

    V c1, c2;
    c1 = zip128_lo(a, b);
    c2 = zip128_hi(a, b);
    qa.wrapped() = shuffle1_128<0,0>(c1, c2);
    qb.wrapped() = shuffle1_128<1,1>(c1, c2);
}

#if SIMDPP_USE_AVX512F || SIMDPP_USE_AVX512BW
template<class V> SIMDPP_INL
void mem_pack2(any_vec<64,V>& qa, any_vec<64,V>& qb)
{
    V a = qa.wrapped();
    V b = qb.wrapped();

    V c1, c2, d1, d2;
    c1 = zip128_lo(a, b);
    c2 = zip128_hi(a, b);
    d1 = shuffle2_128<0,1,0,1>(c1, c2);
    d2 = shuffle2_128<2,3,2,3>(c1, c2);
    qa.wrapped() = permute4_128<0,2,1,3>(d1); // FIXME: optimize
    qb.wrapped() = permute4_128<0,2,1,3>(d2);
}
#endif

/** Generic implementation of mem_pack3. The 256-bit version applies 128-bit
    operations to each half of each vector separately.
*/
template<class T> SIMDPP_INL
void v_mem_pack3_impl8_128(T& a, T& b, T& c)
{
#if SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    using U = typename T::uint_vector_type;

    T a1, b1, c1;
    a1 = align16<11>(a, a);
    b1 = b;
    c1 = align16<5>(c, c);

    // [a11..a15,a0..a10]
    // [b0..b15]
    // [c5..c15,c0..c4]
    U mask1 = make_uint(0xff);
    mask1 = move16_l<5>(mask1);

    T a2, b2, c2;
    a2 = blend(a1, b1, mask1);
    b2 = blend(b1, c1, mask1);
    c2 = blend(c1, a1, mask1);
    // [a11,a12,a13,a14,a15,a0, a1, a2, a3, a4, a5, b11,b12,b13,b14,b15]
    // [b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10,c0, c1, c2, c3, c4 ]
    // [c5, c6, c7, c8, c9, c10,c11,c12,c13,c14,c15,a6, a7, a8, a9, a10]
    U mask2 = make_shuffle_bytes16_mask<5, 16+0, 16+11,
                                        6, 16+1, 16+12,
                                        7, 16+2, 16+13,
                                        8, 16+3, 16+14,
                                        9, 16+4, 16+15,
                                        10>(mask2);
    a = shuffle_bytes16(a2, b2, mask2);
    b = shuffle_bytes16(b2, c2, mask2);
    c = shuffle_bytes16(c2, a2, mask2);

    // [a0, b0, c0, a1, b1, c1, a2, b2, c2, a3, b3, c3, a4, b4, c4, a5 ]
    // [b5, c5, a6, b6, c6, a7, b7, c7, a8, b8, c8, a9, b9, c9, a10,b10]
    // [c10,a11,b11,c11,a12,b12,c12,a13,b13,c13,a14,b14,c14,a15,b15,c15]
#else
    // either uint16x8 or uint16x16, other entries likewise
    using w_b16 = typename same_width<T>::u16;
    using w_b32 = typename same_width<T>::u32;
    using w_b8 = T;

    w_b16 t0, t1, t2, t3;
    t0 = zip16_lo(a, b);
    t1 = zip16_hi(a, b);
    t2 = zip16_lo(c, (w_b8) make_zero());
    t3 = zip16_hi(c, (w_b8) make_zero());

    w_b8 u0, u1, u2, u3;
    u0 = zip8_lo(t0, t2);
    u1 = zip8_hi(t0, t2);
    u2 = zip8_lo(t1, t3);
    u3 = zip8_hi(t1, t3);

    // [a0, b0, c0, 0, a1, b1, c1, 0, a2, b2, c2, 0, a3, b3, c3, 0]
    // [a4, b4, c4, 0, a5, b5, c5, 0, a6, b6, c6, 0, a7, b7, c7, 0]
    // [a8, b8, c8, 0, a9, b9, c9, 0, a10,b10,c10,0, a11,b11,c11,0]
    // [a12,b12,c12,0, a13,b13,c13,0, a14,b14,c14,0, a15,b15,c15,0]
#if SIMDPP_USE_SSSE3
    // it's not worth to use 4 different index vectors to shuffle the vectors
    // properly and use only bit_or later
    w_b8 idx = make_uint(0,   1,  2,  4,    5,    6,    8,    9,
                         10, 12, 13, 14, 0xff, 0xff, 0xff, 0xff);
    u0 = permute_bytes16(u0, idx);
    u1 = permute_bytes16(u1, idx);
    u2 = permute_bytes16(u2, idx);
    u3 = permute_bytes16(u3, idx);
#else
    using w_u64 = typename same_width<T>::u64;

    // the following is still faster than non-SIMD implementation
    w_b8 mask1 = make_uint(0xff, 0xff, 0xff, 0, 0, 0, 0, 0,
                           0xff, 0xff, 0xff, 0, 0, 0, 0, 0);
    w_u64 w0, w1, w2, w3;
    w0 = u0;  w1 = u1;  w2 = u2;  w3 = u3;
    w0 = shift_r(w0, 8);
    w1 = shift_r(w1, 8);
    w2 = shift_r(w2, 8);
    w3 = shift_r(w3, 8);

    u0 = blend(u0, w0, mask1);
    u1 = blend(u1, w1, mask1);
    u2 = blend(u2, w2, mask1);
    u3 = blend(u3, w3, mask1);

    w_b8 mask2 = make_uint(0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 0);
    w_b8 x0, x1, x2, x3;
    x0 = move16_l<2>(u0);
    x1 = move16_l<2>(u1);
    x2 = move16_l<2>(u2);
    x3 = move16_l<2>(u3);

    u0 = blend(u0, x0, mask2);
    u1 = blend(u1, x1, mask2);
    u2 = blend(u2, x2, mask2);
    u3 = blend(u3, x3, mask2);
#endif
    // [a0, b0, c0, a1, b1, c1, a2, b2, c2, a3, b3, c3, 0,0,0,0]
    // [a4, b4, c4, a5, b5, c5, a6, b6, c6, a7, b7, c7, 0,0,0,0]
    // [a8, b8, c8, a9, b9, c9, a10,b10,c10,a11,b11,c11,0,0,0,0]
    // [a12,b12,c12,a13,b13,c13,a14,b14,c14,a15,b15,c15,0,0,0,0]
    w_b32 k0, k1, k2, k3, l0, l3;
    k0 = u0;
    k1 = u1;
    k2 = u2;
    k3 = u3;
    l0 = move4_r<3>(k1);
    l3 = move4_l<2>(k2);
    k3 = move4_r<1>(k3);
    a = bit_or(k0, l0);
    b = shuffle2<1,2,0,1>(k1, k2);
    c = bit_or(k3, l3);
#endif
}

template<class T> SIMDPP_INL
void v_mem_pack3_impl16_128(T& a, T& b, T& c)
{
#if SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    using U = typename T::uint_vector_type;

    // [a0..a7]
    // [b0..b7]
    // [c0..c7]
    T a1, b1, c1;
    a1 = a;
    b1 = align8<5>(b, b);
    c1 = align8<2>(c, c);

    // [a0..a7]
    // [b5..b7,b0..b4]
    // [c2..c7,c0,c1]
    T a2, b2, c2;
    U mask2 = make_uint(0xffff);
    mask2 = move8_l<2>(mask2);

    a2 = blend(a1, b1, mask2);
    b2 = blend(b1, c1, mask2);
    c2 = blend(c1, a1, mask2);

    // [a0,a1,a2,a3,a4,a5,b3,b4]
    // [b5,b6,b7,b0,b1,b2,c0,c1]
    // [c2,c3,c4,c5,c6,c7,a6,a7]
    U mask1 = make_shuffle_bytes16_mask<0, 8+3, 8+6,
                                        1, 8+4, 8+7,
                                        2, 8+5>(mask1);
    a = shuffle_bytes16(a2, b2, mask1);
    b = shuffle_bytes16(c2, a2, mask1);
    c = shuffle_bytes16(b2, c2, mask1);

    // [a0,b0,c0,a1,b1,c1,a2,b2]
    // [c2,a3,b3,c3,a4,b4,c4,a5]
    // [b5,c5,a6,b6,c6,a7,b7,c7]

#else
    // either uint8x16 or uint8x32, other entries likewise
    using w_b16 = T;
    using w_b32 = typename same_width<T>::u32;

    w_b32 t0, t1, t2, t3;
    t0 = zip8_lo(a, b);
    t1 = zip8_hi(a, b);
    t2 = zip8_lo(c, (w_b16) make_zero());
    t3 = zip8_hi(c, (w_b16) make_zero());

    w_b16 u0, u1, u2, u3;
    u0 = zip4_lo(t0, t2);
    u1 = zip4_hi(t0, t2);
    u2 = zip4_lo(t1, t3);
    u3 = zip4_hi(t1, t3);

    // [a0, b0, c0, 0, a1, b1, c1, 0 ]
    // [a2, b2, c2, 0, a3, b3, c3, 0 ]
    // [a4, b4, c4, 0, a5, b5, c5, 0 ]
    // [a6, b6, c6, 0, a7, b7, c7, 0 ]

#if SIMDPP_USE_SSSE3
    // it's not worth to use 4 different index vectors to shuffle the vectors
    // properly and use only bit_or later
    w_b16 idx = make_shuffle_bytes16_mask<0,1,2,4,5,6,-1,-1>(idx);
    u0 = permute_bytes16(u0, idx);
    u1 = permute_bytes16(u1, idx);
    u2 = permute_bytes16(u2, idx);
    u3 = permute_bytes16(u3, idx);

#else
    // the following is still faster than non-SIMD implementation
    w_b16 mask2 = make_uint(0xffff, 0xffff, 0xffff, 0,
                            0, 0, 0, 0);
    u0 = blend(u0, move8_l<1>(u0), mask2);
    u1 = blend(u1, move8_l<1>(u1), mask2);
    u2 = blend(u2, move8_l<1>(u2), mask2);
    u3 = blend(u3, move8_l<1>(u3), mask2);
#endif
    // [a0, b0, c0, a1, b1, c1, 0, 0]
    // [a2, b2, c2, a3, b3, c3, 0, 0]
    // [a4, b4, c4, a5, b5, c5, 0, 0]
    // [a6, b6, c6, a7, b7, c7, 0, 0]
    w_b32 k0, k1, k2, k3, l0, l3;
    k0 = u0;
    k1 = u1;
    k2 = u2;
    k3 = u3;
    l0 = move4_r<3>(k1);
    l3 = move4_l<2>(k2);
    k3 = move4_r<1>(k3);
    a = bit_or(k0, l0);
    b = shuffle2<1,2,0,1>(k1, k2);
    c = bit_or(k3, l3);
#endif
}

template<class T> SIMDPP_INL
void v_mem_pack3_impl32_128(T& a, T& b, T& c)
{
#if SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    using U = typename T::uint_vector_type;

    // [a0,a1,a2,a3]
    // [b0,b1,b2,b3]
    // [c0,c1,c2,c3]
    T a1, b1, c1;
    a1 = a;
    b1 = align4<1>(b, b);
    c1 = align4<2>(c, c);

    // [a0,a1,a2,a3]
    // [b1,b2,b3,b0]
    // [c2,c3,c0,c1]
    T a2, b2, c2;
    U mask2 = make_uint(0xffffffff);
    mask2 = move4_l<1>(mask2);

    a2 = blend(a1, c1, mask2);
    b2 = blend(b1, a1, mask2);
    c2 = blend(c1, b1, mask2);
    // [a0,a1,a2,c1]
    // [b1,b2,b3,a3]
    // [c2,c3,c0,b0]
    U mask1 = make_shuffle_bytes16_mask<0,4+3,4+2,1>(mask1);
    a = shuffle_bytes16(a2, c2, mask1);
    b = shuffle_bytes16(b2, a2, mask1);
    c = shuffle_bytes16(c2, b2, mask1);
    // [a0,b0,c0,a1]
    // [b1,c1,a2,b2]
    // [c2,a3,b3,c3]
#else
    T t0, t1, t2;
    t0 = shuffle2<0,2,0,2>(a, b);
    t1 = shuffle2<0,2,1,3>(c, a);
    t2 = shuffle2<1,3,1,3>(b, c);
    // [a0,a2,b0,b2]
    // [c0,c2,a1,a3]
    // [b1,b3,c1,c3]
    t0 = permute4<0,2,1,3>(t0);
    t1 = permute4<0,2,1,3>(t1);
    t2 = permute4<0,2,1,3>(t2);
    // [a0,b0,a2,b2]
    // [c0,a1,c2,a3]
    // [b1,c1,b3,c3]
    a = shuffle2<0,1,0,1>(t0, t1);
    b = shuffle2<0,1,2,3>(t2, t0);
    c = shuffle2<2,3,2,3>(t1, t2);
#endif
}

template<class T> SIMDPP_INL
void v_mem_pack3_impl64_128(T& a, T& b, T& c)
{
    T d0, d1, d2;
    d0 = shuffle1<0,0>(a, b);
    d1 = shuffle1<0,1>(c, a);
    d2 = shuffle1<1,1>(b, c);
    a = d0;  b = d1;  c = d2;
}

template<class V> SIMDPP_INL
void v_mem_pack3_shuffle128(any_vec<16,V>& qa, any_vec<16,V>& qb, any_vec<16,V>& qc)
{
    (void) qa; (void) qb; (void) qc;
}

template<class V> SIMDPP_INL
void v_mem_pack3_shuffle128(any_vec<32,V>& qa, any_vec<32,V>& qb, any_vec<32,V>& qc)
{
    // shuffle the vectors so that the lower halves contain the first 3 128-bit
    // items (a and lower half of b) and the higher halves contain the rest

    V a0, b0, c0, a1, b1, c1;

    a0 = qa.wrapped();  b0 = qb.wrapped();  c0 = qc.wrapped();

    a1 = shuffle1_128<0,0>(a0, b0);
    b1 = shuffle1_128<0,1>(c0, a0);
    c1 = shuffle1_128<1,1>(b0, c0);

    qa.wrapped() = a1;  qb.wrapped() = b1;  qc.wrapped() = c1;
}

#if SIMDPP_USE_AVX512F
template<class V> SIMDPP_INL
void v_mem_pack3_shuffle128(any_vec<64,V>& qa, any_vec<64,V>& qb, any_vec<64,V>& qc)
{
    V a, b, c; // TODO: optimize. Using full-vector shuffle may be faster
    a = qa.wrapped();  b = qb.wrapped();  c = qc.wrapped();

    V t0, t1, t2;
    t0 = shuffle2_128<0,2,0,2>(a, b);
    t1 = shuffle2_128<0,2,1,3>(c, a);
    t2 = shuffle2_128<1,3,1,3>(b, c);
    // [a0,a2,b0,b2]
    // [c0,c2,a1,a3]
    // [b1,b3,c1,c3]
    t0 = permute4_128<0,2,1,3>(t0);
    t1 = permute4_128<0,2,1,3>(t1);
    t2 = permute4_128<0,2,1,3>(t2);
    // [a0,b0,a2,b2]
    // [c0,a1,c2,a3]
    // [b1,c1,b3,c3]
    a = shuffle2_128<0,1,0,1>(t0, t1);
    b = shuffle2_128<0,1,2,3>(t2, t0);
    c = shuffle2_128<2,3,2,3>(t1, t2);

    qa.wrapped() = a;  qb.wrapped() = b;  qc.wrapped() = c;
}
#endif

/** Interleaves the elements of @a a, @a b and @a c in such way that:
     * every (3n)-th element comes from @a a
     * every (3n+1)-th element comes from @a b
     * every (3n+2)-th element comes from @a c

    n = [0, <number of elements in vector> - 1]
*/
template<unsigned N> SIMDPP_INL
void mem_pack3(uint8<N>& a, uint8<N>& b, uint8<N>& c)
{
    v_mem_pack3_impl8_128(a, b, c);
    v_mem_pack3_shuffle128(a, b, c);
}

template<unsigned N> SIMDPP_INL
void mem_pack3(uint16<N>& a, uint16<N>& b, uint16<N>& c)
{
    v_mem_pack3_impl16_128(a, b, c);
    v_mem_pack3_shuffle128(a, b, c);
}

template<unsigned N> SIMDPP_INL
void mem_pack3(uint32<N>& a, uint32<N>& b, uint32<N>& c)
{
    v_mem_pack3_impl32_128(a, b, c);
    v_mem_pack3_shuffle128(a, b, c);
}

template<unsigned N> SIMDPP_INL
void mem_pack3(uint64<N>& a, uint64<N>& b, uint64<N>& c)
{
    v_mem_pack3_impl64_128(a, b, c);
    v_mem_pack3_shuffle128(a, b, c);
}

template<unsigned N> SIMDPP_INL
void mem_pack3(float32<N>& a, float32<N>& b, float32<N>& c)
{
    v_mem_pack3_impl32_128(a, b, c);
    v_mem_pack3_shuffle128(a, b, c);
}

template<unsigned N> SIMDPP_INL
void mem_pack3(float64<N>& a, float64<N>& b, float64<N>& c)
{
    v_mem_pack3_impl64_128(a, b, c);
    v_mem_pack3_shuffle128(a, b, c);
}

/** Generic implementation of mem_pack4. The 256-bit version applies 128-bit
    operations to each half of each vector separately.
*/
template<class T> SIMDPP_INL
void v_mem_pack4_impl8_128(T& a, T& b, T& c, T& d)
{
    // either uint16x8 or uint16x16, other entries likewise
#if SIMDPP_USE_SSSE3 || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    // TODO: optimize for altivec
    using w_b32 = typename same_width<T>::u32;

    w_b32 b0, b1, b2, b3;
    b0 = a;  b1 = b;  b2 = c;  b3 = d;
    transpose4(b0, b1, b2, b3);
    a = b0;  b = b1;  c = b2;  d = b3;

    a = transpose_inplace(a);
    b = transpose_inplace(b);
    c = transpose_inplace(c);
    d = transpose_inplace(d);
#else
    using w_b8 = T;
    using w_b16 = typename same_width<T>::u16;
    using w_b64 = typename same_width<T>::u64;

    w_b8 e0, e1, e2, e3;
    w_b64 d0, d1, d2, d3;
    d0 = a;  d1 = b;  d2 = c;  d3 = d;
    e0 = zip2_lo(d0, d2);
    e1 = zip2_lo(d1, d3);
    e2 = zip2_hi(d0, d2);
    e3 = zip2_hi(d1, d3);
    // [a0 .. a7,  c0 .. c7  ]
    // [b0 .. b7,  d0 .. d7  ]
    // [a8 .. a15, c8 .. c15 ]
    // [b8 .. b15, d8 .. d15 ]
    w_b16 f0, f1, f2, f3;
    f0 = zip16_lo(e0, e1);
    f1 = zip16_hi(e0, e1);
    f2 = zip16_lo(e2, e3);
    f3 = zip16_hi(e2, e3);
    // [a0, b0, a1, b1, a2, b2, a3, b3, a4, b4, a5, b5, a6, b6, a7, b7 ]
    // [c0, d0, c1, d1, c2, d2, c3, d3, c4, d4, c5, d5, c6, d6, c7, d7 ]
    // [a8, b8, a9, b9, a10,b10,a11,b11,a12,b12,a13,b13,a14,b14,a15,b15]
    // [c8, d8, c9, d9, c10,d10,c11,d11,c12,d12,c13,d13,c14,d14,c15,d15]
    a = zip8_lo(f0, f1);
    b = zip8_hi(f0, f1);
    c = zip8_lo(f2, f3);
    d = zip8_hi(f2, f3);
#endif
}

template<class T> SIMDPP_INL
void v_mem_pack4_impl16_128(T& a, T& b, T& c, T& d)
{
    using w_b16 = T;
    using w_b32 = typename same_width<T>::u32;
    using w_b64 = typename same_width<T>::u64;

    w_b16 e0, e1, e2, e3;
    w_b64 d0, d1, d2, d3;
    d0 = a;  d1 = b;  d2 = c;  d3 = d;
    e0 = zip2_lo(d0, d2);
    e1 = zip2_lo(d1, d3);
    e2 = zip2_hi(d0, d2);
    e3 = zip2_hi(d1, d3);
    // [a0,a1,a2,a3,c0,c1,c2,c3]
    // [b0,b1,b2,b3,d0,d1,d2,d3]
    // [a4,a5,a6,a7,c4,c5,c6,c7]
    // [b4,b5,b6,b7,d4,d5,d6,d7]
    w_b32 f0, f1, f2, f3;
    f0 = zip8_lo(e0, e1);
    f1 = zip8_hi(e0, e1);
    f2 = zip8_lo(e2, e3);
    f3 = zip8_hi(e2, e3);
    // [a0,b0,a1,b1,a2,b2,a3,b3]
    // [c0,d0,c1,d1,c2,d2,c3,d3]
    // [a4,b4,a5,b5,a6,b6,a7,b7]
    // [c4,d4,c5,d5,c6,d6,c7,d7]
    a = zip4_lo(f0, f1);
    b = zip4_hi(f0, f1);
    c = zip4_lo(f2, f3);
    d = zip4_hi(f2, f3);
}

template<class T> SIMDPP_INL
void v_mem_pack4_impl32_128(T& a, T& b, T& c, T& d)
{
    transpose4(a, b, c, d);
}

template<class T> SIMDPP_INL
void v_mem_pack4_impl64_128(T& a, T& b, T& c, T& d)
{
    transpose2(a, b);
    transpose2(c, d);
    T t;
    t = b;
    b = c;
    c = t;
}

template<class V> SIMDPP_INL
void v_mem_pack4_shuffle128(any_vec<16,V>& qa, any_vec<16,V>& qb,
                            any_vec<16,V>& qc, any_vec<16,V>& qd)
{
    (void) qa; (void) qb; (void) qc; (void) qd;
}

template<class V> SIMDPP_INL
void v_mem_pack4_shuffle128(any_vec<32,V>& qa, any_vec<32,V>& qb,
                            any_vec<32,V>& qc, any_vec<32,V>& qd)
{
    // shuffle the vectors with the lower halves containing the first 4 128-bit
    // items and the higher halves contain the rest
    V a0, b0, c0, d0, a1, b1, c1, d1;

    a0 = qa.wrapped();  b0 = qb.wrapped();  c0 = qc.wrapped();  d0 = qd.wrapped();

    a1 = shuffle1_128<0,0>(a0, b0);
    b1 = shuffle1_128<0,0>(c0, d0);
    c1 = shuffle1_128<1,1>(a0, b0);
    d1 = shuffle1_128<1,1>(c0, d0);

    qa.wrapped() = a1;  qb.wrapped() = b1;  qc.wrapped() = c1;  qd.wrapped() = d1;
}

#if SIMDPP_USE_AVX512F
template<class V> SIMDPP_INL
void v_mem_pack4_shuffle128(any_vec<64,V>& qa, any_vec<64,V>& qb,
                            any_vec<64,V>& qc, any_vec<64,V>& qd)
{
    V a, b, c, d; // TODO: optimize. Using full-vector shuffle/permute will be faster

    a = qa.wrapped();  b = qb.wrapped();  c = qc.wrapped();  d = qd.wrapped();

    V t1, t2, t3, t4;
    // [a0,a1,a2,a3]
    // [b0,b1,b2,b3]
    // [c0,c1,c2,c3]
    // [d0,d1,d2,d3]
    t1 = shuffle2_128<0,2,0,2>(a, b);
    t2 = shuffle2_128<1,3,1,3>(a, b);
    t3 = shuffle2_128<0,2,0,2>(c, d);
    t4 = shuffle2_128<1,3,1,3>(c, d);
    // [a0,a2,b0,b2]
    // [a1,a3,b1,b3]
    // [c0,c2,d0,d2]
    // [c1,c3,d1,d3]
    a = shuffle2_128<0,2,0,2>(t1, t3);
    b = shuffle2_128<0,2,0,2>(t2, t4);
    c = shuffle2_128<1,3,1,3>(t1, t3);
    d = shuffle2_128<1,3,1,3>(t2, t4);
    // [a0,b0,c0,d0]
    // [a1,b1,c1,d1]
    // [a2,b2,c2,d2]
    // [a3,b3,c3,d3]


    qa.wrapped() = a;  qb.wrapped() = b;  qc.wrapped() = c;  qd.wrapped() = d;
}
#endif

/** Interleaves the elements of @a a, @a b, @a c and @a d in such way that:
     * every (4n)-th element comes from @a a
     * every (4n+1)-th element comes from @a b
     * every (4n+2)-th element comes from @a c
     * every (4n+3)-th element comes from @a d

    n = [0, <number of elements in vector> - 1]
*/
template<unsigned N> SIMDPP_INL
void mem_pack4(uint8<N>& a, uint8<N>& b, uint8<N>& c, uint8<N>& d)
{
    v_mem_pack4_impl8_128(a, b, c, d);
    v_mem_pack4_shuffle128(a, b, c, d);
}

template<unsigned N> SIMDPP_INL
void mem_pack4(uint16<N>& a, uint16<N>& b, uint16<N>& c, uint16<N>& d)
{
    v_mem_pack4_impl16_128(a, b, c, d);
    v_mem_pack4_shuffle128(a, b, c, d);
}

template<unsigned N> SIMDPP_INL
void mem_pack4(uint32<N>& a, uint32<N>& b, uint32<N>& c, uint32<N>& d)
{
    v_mem_pack4_impl32_128(a, b, c, d);
    v_mem_pack4_shuffle128(a, b, c, d);
}

template<unsigned N> SIMDPP_INL
void mem_pack4(uint64<N>& a, uint64<N>& b, uint64<N>& c, uint64<N>& d)
{
    v_mem_pack4_impl64_128(a, b, c, d);
    v_mem_pack4_shuffle128(a, b, c, d);
}

template<unsigned N> SIMDPP_INL
void mem_pack4(float32<N>& a, float32<N>& b, float32<N>& c, float32<N>& d)
{
    v_mem_pack4_impl32_128(a, b, c, d);
    v_mem_pack4_shuffle128(a, b, c, d);
}

template<unsigned N> SIMDPP_INL
void mem_pack4(float64<N>& a, float64<N>& b, float64<N>& c, float64<N>& d)
{
    v_mem_pack4_impl64_128(a, b, c, d);
    v_mem_pack4_shuffle128(a, b, c, d);
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

