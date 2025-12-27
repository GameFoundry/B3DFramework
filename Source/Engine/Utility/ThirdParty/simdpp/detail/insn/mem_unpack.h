/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_MEM_UNPACK_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_MEM_UNPACK_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/width.h>
#include <simdpp/detail/insn/shuffle128.h>
#include <simdpp/detail/insn/zip128.h>
#include <simdpp/core/align.h>
#include <simdpp/core/splat_n.h>
#include <simdpp/core/make_shuffle_bytes_mask.h>
#include <simdpp/core/shuffle1.h>
#include <simdpp/core/shuffle2.h>
#include <simdpp/core/transpose.h>
#include <simdpp/core/unzip_hi.h>
#include <simdpp/core/unzip_lo.h>
#include <simdpp/core/zip_hi.h>
#include <simdpp/core/zip_lo.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

/** Concatenates @a a and @a b and stores the elements of the resulting array
    as follows:
     * every (2n)-th element is stored to @a a
     * every (2n+1)-th element is stored to @a b

    n = [0, <number of elements in vector> - 1]
*/
template<class V> SIMDPP_INL
void mem_unpack2(any_vec<16,V>& qa, any_vec<16,V>& qb)
{
    V a = qa.wrapped();
    V b = qb.wrapped();

    qa.wrapped() = unzip128_lo(a, b);
    qb.wrapped() = unzip128_hi(a, b);
}

template<class V> SIMDPP_INL
void mem_unpack2(any_vec<32,V>& qa, any_vec<32,V>& qb)
{
    V a = qa.wrapped();
    V b = qb.wrapped();

    V c1 = shuffle1_128<0,0>(a, b);
    V c2 = shuffle1_128<1,1>(a, b);
    qa.wrapped() = unzip128_lo(c1, c2);
    qb.wrapped() = unzip128_hi(c1, c2);
}

#if SIMDPP_USE_AVX512F
template<class V> SIMDPP_INL
void mem_unpack2(any_vec<64,V>& qa, any_vec<64,V>& qb)
{
    V a = qa.wrapped();
    V b = qb.wrapped();

    V c1 = shuffle2_128<0,2,0,2>(a, b);
    V c2 = shuffle2_128<1,3,1,3>(a, b);
    qa.wrapped() = unzip128_lo(c1, c2);
    qb.wrapped() = unzip128_hi(c1, c2);
}
#endif

/** Generic implementation of mem_unpack3. The 128-bit lanes are processed
    independently
*/
template<class T> SIMDPP_INL
void v_mem_unpack3_impl8_128(T& a, T& b, T& c)
{
#if SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    // [a0, b0, c0, a1, b1, c1, a2, b2, c2, a3, b3, c3, a4, b4, c4, a5 ]
    // [b5, c5, a6, b6, c6, a7, b7, c7, a8, b8, c8, a9, b9, c9, a10,b10]
    // [c10,a11,b11,c11,a12,b12,c12,a13,b13,c13,a14,b14,c14,a15,b15,c15]
    T mask1 = make_shuffle_bytes16_mask<   1,    4,    7, 10, 13,16+0,16+3,16+6,
                                        16+9,16+12,16+15,  2,  5,   8,  11,  14>(mask1);
    T a1, b1, c1;
    a1 = shuffle_bytes16(c, a, mask1);
    b1 = shuffle_bytes16(a, b, mask1);
    c1 = shuffle_bytes16(b, c, mask1);
    // [a11,a12,a13,a14,a15,a0, a1, a2, a3, a4, a5, b11,b12,b13,b14,b15]
    // [b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10,c0, c1, c2, c3, c4 ]
    // [c5, c6, c7, c8, c9, c10,c11,c12,c13,c14,c15,a6, a7, a8, a9, a10]
    T a2, b2, c2;
    T mask2 = make_uint(0xff);
    mask2 = move16_l<5>(mask2);

    a2 = blend(a1, c1, mask2);
    b2 = blend(b1, a1, mask2);
    c2 = blend(c1, b1, mask2);
    // [a11..a15,a0..a10]
    // [b0..b15]
    // [c5..c15,c0..c5]
    a = align16<5>(a2, a2);
    b = b2;
    c = align16<11>(c2, c2);
#else
    typename same_width<T>::u8 t0, t1, t2, t3;
    t0 = a;
    t1 = align16<12>(a, b);
    t2 = align16<8>(b, c);
    t3 = move16_l<4>(c);
    // [a0, b0, c0, a1, b1, c1, a2, b2, c2, a3, b3, c3, ...]
    // [a4, b4, c4, a5, b5, c5, a6, b6, c6, a7, b7, c7, ...]
    // [a8, b8, c8, a9, b9, c9, a10,b10,c10,a11,b11,c11, ...]
    // [a12,b12,c12,a13,b13,c13,a14,b14,c14,a15,b15,c15, ...]
    typename same_width<T>::u16 b0, b1, b2, b3;
    b0 = zip16_lo(t0, t1);
    b1 = zip16_lo(t2, t3);
    b2 = zip16_hi(t0, t1);
    b3 = zip16_hi(t2, t3);
    // [a0, a4, b0, b4, c0, c4, a1, a5, b1, b5, c1, c5, a2, a6, b2, b6 ]
    // [a8, a12,b8, b12,c9, c13,a9, a13,b9, b13,c9, c13,a10,a14,b10,b14,]
    // [c2, c6, a3, a7, b3, b7, c3, c7, ... ]
    // [c10,c14,a11,a15,b11,b15,c11,c15,... ]
    typename same_width<T>::u8 u0, u1, u2;
    u0 = zip8_lo(b0, b1);
    u1 = zip8_hi(b0, b1);
    u2 = zip8_lo(b2, b3);
    // [a0, a4, a8, a12,b0, b4, b8, b12, c0, c4, c8, c12, a1, a5, a9, a13 ]
    // [b1, b5, b9, b13,c1, c5, c9, c13, a2, a6, a10,a14, b2, b6, b10,b14 ]
    // [c2, c6, c10,c14,a3, a7, a11,a15, b3, b7, b11,b15, c3, c7, c11,c15 ]
    t0 = u0;
    t1 = align16<12>(u0, u1);
    t2 = align16<8>(u1, u2);
    t3 = move16_l<4>(u2);
    // [a0, a4, a8, a12,b0, b4, b8, b12, c0, c4, c8, c12, ...]
    // [a1, a5, a9, a13,b1, b5, b9, b13, c1, c5, c9, c13, ...]
    // [a2, a6, a10,a14,b2, b6, b10,b13, c2, c6, c10,c14, ...]
    // [a3, a7, a11,a15,b3, b7, b11,b13, c3, c7, c11,c15, ...]
    b0 = zip16_lo(t0, t1);
    b1 = zip16_lo(t2, t3);
    b2 = zip16_hi(t0, t1);
    b3 = zip16_hi(t2, t3);
    // [a0, a1, a4, a5, a8, a9, a12,a13,b0, b1, b4, b5, b8, b9, b12,b13 ]
    // [a2, a3, a6, a7, a10,a11,a14,a15,b2, b3, b6, b7, b10,b11,b14,b15 ]
    // [c0, c1, c4, c5, c8, c9, c12,c13, ... ]
    // [c2, c3, c6, c7, c10,c11,c14,c15, ... ]
    a = zip8_lo(b0, b1);
    b = zip8_hi(b0, b1);
    c = zip8_lo(b2, b3);
#endif
}

template<class T> SIMDPP_INL
void v_mem_unpack3_impl16_128(T& a, T& b, T& c)
{
#if SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    // [a0,b0,c0,a1,b1,c1,a2,b2]
    // [c2,a3,b3,c3,a4,b4,c4,a5]
    // [b5,c5,a6,b6,c6,a7,b7,c7]
    T mask1 = make_shuffle_bytes16_mask<0,3,6,8+1,8+4,8+7,8+2,8+5>(mask1);
    T a1, b1, c1;
    a1 = shuffle_bytes16(a, b, mask1);
    c1 = shuffle_bytes16(b, c, mask1);
    b1 = shuffle_bytes16(c, a, mask1);
    // [a0,a1,a2,a3,a4,a5,b3,b4]
    // [c2,c3,c4,c5,c6,c7,a6,a7]
    // [b5,b6,b7,b0,b1,b2,c0,c1]
    T a2, b2, c2;
    T mask2 = make_uint(0xffff);
    mask2 = move8_l<2>(mask2);

    a2 = blend(a1, c1, mask2);
    b2 = blend(b1, a1, mask2);
    c2 = blend(c1, b1, mask2);
    // [a0..a7]
    // [b5..b7,b0..b4]
    // [c2..c7,c0,c1]
    a = a2;
    b = align8<3>(b2, b2);
    c = align8<6>(c2, c2);
#else
    T t0, t1, t2, t3;
    t0 = a;
    t1 = align8<6>(a, b);
    t2 = align8<4>(b, c);
    t3 = move8_l<2>(c);
    // [a0,b0,c0,a1,b1,c1, ... ]
    // [a2,b2,c2,a3,b3,c3, ... ]
    // [a4,b4,c4,a5,b5,c5, ... ]
    // [a6,b6,c6,a7,b7,c7, ... ]
    typename same_width<T>::u32 b0, b1, b2, b3;
    b0 = zip8_lo(t0, t1);
    b1 = zip8_lo(t2, t3);
    b2 = zip8_hi(t0, t1);
    b3 = zip8_hi(t2, t3);
    // [a0,a2,b0,b2,c0,c2,a1,a3]
    // [a4,a6,b4,b6,c4,c6,a5,a7]
    // [b1,b3,c1,c3, ... ]
    // [b5,b7,c5,c7, ... ]
    typename same_width<T>::u64 c0, c1, c2;
    c0 = zip4_lo(b0, b1);
    c1 = zip4_hi(b0, b1);
    c2 = zip4_lo(b2, b3);
    // [a0,a2,a4,a6,b0,b2,b4,b6]
    // [c0,c2,c4,c6,a1,a3,a5,a7]
    // [b1,b3,b5,b7,c1,c3,c5,c7]
    t0 = c0;
    t1 = shuffle1<1,0>(c0, c1);
    t2 = splat2<1>(c1);
    t3 = c2;
    // [a0,a2,a4,a6,b0,b2,b4,b6]
    // [b0,b2,b4,b6,c0,c2,c4,c6]
    // [a1,a3,a5,a7,a1,a3,a5,a7]
    // [b1,b3,b5,b7,c1,c3,c5,c7]
    a = zip8_lo(t0, t2);
    b = zip8_lo(t1, t3);
    c = zip8_hi(t1, t3);
#endif
}

template<class T> SIMDPP_INL
void v_mem_unpack3_impl32_128(T& a, T& b, T& c)
{
#if SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    using U = typename T::uint_vector_type;

    // [a0,b0,c0,a1]
    // [b1,c1,a2,b2]
    // [c2,a3,b3,c3]
    U mask1 = make_shuffle_bytes16_mask<0,3,4+2,4+1>(mask1);
    T a1, b1, c1;
    a1 = shuffle_bytes16(a, b, mask1);
    b1 = shuffle_bytes16(b, c, mask1);
    c1 = shuffle_bytes16(c, a, mask1);
    // [a0,a1,a2,c1]
    // [b1,b2,b3,a3]
    // [c2,c3,c0,b0]
    T a2, b2, c2;
    U mask2 = make_uint(0xffffffff);
    mask2 = move4_l<1>(mask2);

    a2 = blend(a1, b1, mask2);
    b2 = blend(b1, c1, mask2);
    c2 = blend(c1, a1, mask2);
    // [a0,a1,a2,a3]
    // [b1,b2,b3,b0]
    // [c2,c3,c0,c1]
    a = a2;
    b = align4<3>(b2, b2);
    c = align4<2>(c2, c2);
#else
    T t11, t12, t21, t22, t31, t32;
    // [a0,b0,c0,a1]
    // [b1,c1,a2,b2]
    // [c2,a3,b3,c3]
    t11 = a;
    t12 = shuffle2<0,1,2,3>(c, b);
    t21 = shuffle2<0,1,0,1>(a, b);
    t22 = shuffle2<2,3,2,3>(b, c);
    t31 = shuffle2<2,3,0,1>(a, b);
    t32 = c;
    // [a0,b0,c0,a1]
    // [c2,a3,a2,b2]
    // [a0,b0,b1,c1]
    // [a2,b2,b3,c3]
    // [c0,a1,b1,c1]
    // [c2,a3,b3,c3]
    a = shuffle2<0,3,2,1>(t11, t12);
    b = shuffle2<1,2,1,2>(t21, t22);
    c = shuffle2<0,3,0,3>(t31, t32);
#endif
}

template<class T> SIMDPP_INL
void v_mem_unpack3_impl64_128(T& a, T& b, T& c)
{
    T d0, d1, d2;
    d0 = shuffle1<0,1>(a, b);
    d1 = shuffle1<1,0>(a, c);
    d2 = shuffle1<0,1>(b, c);
    a = d0; b = d1; c = d2;
}

template<class V> SIMDPP_INL
void v_mem_unpack3_shuffle128(any_vec<16,V>& qa, any_vec<16,V>& qb, any_vec<16,V>& qc)
{
    (void) qa; (void) qb; (void) qc;
}

template<class V> SIMDPP_INL
void v_mem_unpack3_shuffle128(any_vec<32,V>& qa, any_vec<32,V>& qb, any_vec<32,V>& qc)
{
    // shuffle the vectors so that the lower halves contain the first 3 128-bit
    // items (a and lower half of b) and the higher halves contain the rest

    V a0, b0, c0, a1, b1, c1;

    a0 = qa.wrapped();  b0 = qb.wrapped();  c0 = qc.wrapped();

    a1 = shuffle1_128<0,1>(a0, b0);
    b1 = shuffle1_128<1,0>(a0, c0);
    c1 = shuffle1_128<0,1>(b0, c0);

    qa.wrapped() = a1;  qb.wrapped() = b1;  qc.wrapped() = c1;
}

#if SIMDPP_USE_AVX512F
template<class V> SIMDPP_INL
void v_mem_unpack3_shuffle128(any_vec<64,V>& qa, any_vec<64,V>& qb, any_vec<64,V>& qc)
{
    V a, b, c; // TODO: optimize. Using full-vector shuffle may be faster
    a = qa.wrapped();  b = qb.wrapped();  c = qc.wrapped();

    V t11, t12, t21, t22, t31, t32;
    // [a0,b0,c0,a1]
    // [b1,c1,a2,b2]
    // [c2,a3,b3,c3]
    t11 = a;
    t12 = shuffle2_128<0,1,2,3>(c, b);
    t21 = shuffle2_128<0,1,0,1>(a, b);
    t22 = shuffle2_128<2,3,2,3>(b, c);
    t31 = shuffle2_128<2,3,0,1>(a, b);
    t32 = c;
    // [a0,b0,c0,a1]
    // [c2,a3,a2,b2]
    // [a0,b0,b1,c1]
    // [a2,b2,b3,c3]
    // [c0,a1,b1,c1]
    // [c2,a3,b3,c3]
    a = shuffle2_128<0,3,2,1>(t11, t12);
    b = shuffle2_128<1,2,1,2>(t21, t22);
    c = shuffle2_128<0,3,0,3>(t31, t32);

    qa.wrapped() = a;  qb.wrapped() = b;  qc.wrapped() = c;
}
#endif

/** Concatenates @a a, @a b and @a c and stores the elements of the resulting
    array as follows:
     * every (3n)-th element is stored to @a a
     * every (3n+1)-th element is stored to @a b
     * every (3n+2)-th element is stored to @a c

    n = [0, <number of elements in vector> - 1]
*/
template<unsigned N> SIMDPP_INL
void mem_unpack3(uint8<N>& a, uint8<N>& b, uint8<N>& c)
{
    v_mem_unpack3_shuffle128(a, b, c);
    v_mem_unpack3_impl8_128(a, b, c);
}

template<unsigned N> SIMDPP_INL
void mem_unpack3(uint16<N>& a, uint16<N>& b, uint16<N>& c)
{
    v_mem_unpack3_shuffle128(a, b, c);
    v_mem_unpack3_impl16_128(a, b, c);
}

template<unsigned N> SIMDPP_INL
void mem_unpack3(uint32<N>& a, uint32<N>& b, uint32<N>& c)
{
    v_mem_unpack3_shuffle128(a, b, c);
    v_mem_unpack3_impl32_128(a, b, c);
}

template<unsigned N> SIMDPP_INL
void mem_unpack3(uint64<N>& a, uint64<N>& b, uint64<N>& c)
{
    v_mem_unpack3_shuffle128(a, b, c);
    v_mem_unpack3_impl64_128(a, b, c);
}

template<unsigned N> SIMDPP_INL
void mem_unpack3(float32<N>& a, float32<N>& b, float32<N>& c)
{
    v_mem_unpack3_shuffle128(a, b, c);
    v_mem_unpack3_impl32_128(a, b, c);
}

template<unsigned N> SIMDPP_INL
void mem_unpack3(float64<N>& a, float64<N>& b, float64<N>& c)
{
    v_mem_unpack3_shuffle128(a, b, c);
    v_mem_unpack3_impl64_128(a, b, c);
}

/** Generic implementation of mem_unpack4. The 256-bit version applies 128-bit
    operations to each half of each vector separately.
*/
template<class T> SIMDPP_INL
void v_mem_unpack4_impl8_128(T& a, T& b, T& c, T& d)
{
#if SIMDPP_USE_SSSE3 || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    // TODO: optimize for Altivec and MSA
    typename same_width<T>::u32 b0, b1, b2, b3;
    b0 = transpose_inplace(a);
    b1 = transpose_inplace(b);
    b2 = transpose_inplace(c);
    b3 = transpose_inplace(d);

    transpose4(b0, b1, b2, b3);
    a = b0;  b = b1;  c = b2;  d = b3;
#else
    // [a0, b0, c0, d0, a1, b1, c1, d1, a2, b2, c2, d2, a3, b3, c3, d3 ]
    // [a4, b4, c4, d4, a5, b5, c5, d5, a6, b6, c6, d6, a7, b7, c7, d7 ]
    // [a8, b8, c8, d8, a9, b9, c9, d9, a10,b10,c10,d10,a11,b11,c11,d11]
    // [a12,b12,c12,d12,a13,b13,c13,d13,a14,b14,c14,d14,a15,b15,c15,d15]
    T b0, b1, b2, b3, c0, c1, c2, c3;
    b0 = zip16_lo(a, b);
    b1 = zip16_hi(a, b);
    b2 = zip16_lo(c, d);
    b3 = zip16_hi(c, d);
    // [a0, a4, b0, b4, c0, c4, d0, d4, a1, a5, b1, b5, c1, c5, d1, d5 ]
    // [a2, a6, b2, b6, c2, c6, d2, d6, a3, a7, b3, b7, c3, c7, d3, d7 ]
    // [a8, a12,b8, b12,c8, c12,d8, d12,a9, a13,b9, b13,c9, c13,d9, d13]
    // [a10,a14,b10,b14,c10,c14,d10,d14,a11,a15,b11,b15,c11,c15,d11,d15]
    c0 = zip16_lo(b0, b1);
    c1 = zip16_hi(b0, b1);
    c2 = zip16_lo(b2, b3);
    c3 = zip16_hi(b2, b3);
    // [a0, a2, a4, a6, b0, b2, b4, b6, c0, c2, c4, c6, d0, d2, d4, d6 ]
    // [a1, a3, a5, a7, b1, b3, b5, b7, c1, c3, c5, c7, d1, d3, d5, d7 ]
    // [a8, a10,a12,a14,b8, b10,b12,b14,c8, c10,c12,c14,d8, d10,d12,d14]
    // [a9, a11,a13,a15,b9, b11,b13,b15,c9, c11,c13,c15,d9, d11,d13,d15]
    typename same_width<T>::u64 d0, d1, d2, d3;
    d0 = zip16_lo(c0, c1);
    d1 = zip16_hi(c0, c1);
    d2 = zip16_lo(c2, c3);
    d3 = zip16_hi(c2, c3);
    // [a0 .. a7, b0 .. b7 ]
    // [c0 .. c7, d0 .. d7 ]
    // [a8 .. a15, b8 .. b15 ]
    // [b8 .. b15, d8 .. d15 ]
    a = zip2_lo(d0, d2);
    b = zip2_hi(d0, d2);
    c = zip2_lo(d1, d3);
    d = zip2_hi(d1, d3);
#endif
}

template<class T> SIMDPP_INL
void v_mem_unpack4_impl16_128(T& a, T& b, T& c, T& d)
{
    // [a0,b0,c0,d0,a1,b1,c1,d1]
    // [a2,b2,c2,d2,a3,b3,c3,d3]
    // [a4,b4,c4,d4,a5,b5,c5,d5]
    // [a6,b6,c6,d6,a7,b7,c7,d7]
    typename same_width<T>::u16 t0, t1, t2, t3;
    t0 = zip8_lo(a, b);
    t1 = zip8_hi(a, b);
    t2 = zip8_lo(c, d);
    t3 = zip8_hi(c, d);
    // [a0,a2,b0,b2,c0,c2,d0,d2]
    // [a1,a3,b1,b3,c1,c3,d1,d3]
    // [a4,a6,b4,b6,c4,c6,d4,d6]
    // [a5,a7,b5,b7,c5,c7,d5,d7]
    typename same_width<T>::u64 u0, u1, u2, u3;
    u0 = zip8_lo(t0, t1);
    u1 = zip8_hi(t0, t1);
    u2 = zip8_lo(t2, t3);
    u3 = zip8_hi(t2, t3);
    // [a0,a1,a2,a3,b0,b1,b2,b3]
    // [c0,c1,c2,c3,d0,d1,d2,d3]
    // [a4,a5,a6,a7,b4,b5,b6,b7]
    // [c4,c5,c6,c7,d4,d5,d6,d7]
    a = zip2_lo(u0, u2);
    b = zip2_hi(u0, u2);
    c = zip2_lo(u1, u3);
    d = zip2_hi(u1, u3);
}

template<class T> SIMDPP_INL
void v_mem_unpack4_impl32_128(T& a, T& b, T& c, T& d)
{
    transpose4(a, b, c, d);
}

template<class T> SIMDPP_INL
void v_mem_unpack4_impl64_128(T& a, T& b, T& c, T& d)
{
    transpose2(a, c);
    transpose2(b, d);
    T t;
    t = b;
    b = c;
    c = t;
}

template<class V> SIMDPP_INL
void v_mem_unpack4_shuffle128(any_vec<16,V>& qa, any_vec<16,V>& qb,
                              any_vec<16,V>& qc, any_vec<16,V>& qd)
{
    (void) qa; (void) qb; (void) qc; (void) qd;
}

template<class V> SIMDPP_INL
void v_mem_unpack4_shuffle128(any_vec<32,V>& qa, any_vec<32,V>& qb,
                              any_vec<32,V>& qc, any_vec<32,V>& qd)
{
    V a0, b0, c0, d0, a1, b1, c1, d1;

    a0 = qa.wrapped();  b0 = qb.wrapped();  c0 = qc.wrapped();  d0 = qd.wrapped();

    a1 = shuffle1_128<0,0>(a0, c0);
    b1 = shuffle1_128<1,1>(a0, c0);
    c1 = shuffle1_128<0,0>(b0, d0);
    d1 = shuffle1_128<1,1>(b0, d0);

    qa.wrapped() = a1;  qb.wrapped() = b1;  qc.wrapped() = c1;  qd.wrapped() = d1;
}

#if SIMDPP_USE_AVX512F
template<class V> SIMDPP_INL
void v_mem_unpack4_shuffle128(any_vec<64,V>& qa, any_vec<64,V>& qb,
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

/** Concatenates @a a, @a b, @a c and @a d and stores the elements of the
    resulting array as follows:
     * every (4n)-th element is stored to @a a
     * every (4n+1)-th element is stored to @a b
     * every (4n+2)-th element is stored to @a c
     * every (4n+3)-th element is stored to @a d

    n = [0, <number of elements in vector> - 1]
*/
// @icost{SSE2, SSE3, 16}
// @icost{SSSE3, SSE4.1, 12}
template<unsigned N> SIMDPP_INL
void mem_unpack4(uint8<N>& a, uint8<N>& b, uint8<N>& c, uint8<N>& d)
{
    v_mem_unpack4_shuffle128(a, b, c, d);
    v_mem_unpack4_impl8_128(a, b, c, d);
}

template<unsigned N> SIMDPP_INL
void mem_unpack4(uint16<N>& a, uint16<N>& b, uint16<N>& c, uint16<N>& d)
{
    v_mem_unpack4_shuffle128(a, b, c, d);
    v_mem_unpack4_impl16_128(a, b, c, d);
}

template<unsigned N> SIMDPP_INL
void mem_unpack4(uint32<N>& a, uint32<N>& b, uint32<N>& c, uint32<N>& d)
{
    v_mem_unpack4_shuffle128(a, b, c, d);
    v_mem_unpack4_impl32_128(a, b, c, d);
}

template<unsigned N> SIMDPP_INL
void mem_unpack4(uint64<N>& a, uint64<N>& b, uint64<N>& c, uint64<N>& d)
{
    v_mem_unpack4_shuffle128(a, b, c, d);
    v_mem_unpack4_impl64_128(a, b, c, d);
}

template<unsigned N> SIMDPP_INL
void mem_unpack4(float32<N>& a, float32<N>& b, float32<N>& c, float32<N>& d)
{
    v_mem_unpack4_shuffle128(a, b, c, d);
    v_mem_unpack4_impl32_128(a, b, c, d);
}

template<unsigned N> SIMDPP_INL
void mem_unpack4(float64<N>& a, float64<N>& b, float64<N>& c, float64<N>& d)
{
    v_mem_unpack4_shuffle128(a, b, c, d);
    v_mem_unpack4_impl64_128(a, b, c, d);
}

/** Concatenates the given vectors and stores the elements of the resulting
    array as follows:
     * every (3n)-th element of the first 48 elements is stored to @a a
     * every (3n+1)-th element of the first 48 elements is stored to @a b
     * every (3n+2)-th element of the first 48 elements is stored to @a c
     * every (3n)-th element of the last 48 elements is stored to @a d
     * every (3n+1)-th element of the last 48 elements is stored to @a e
     * every (3n+2)-th element of the lasd 48 elements is stored to @a f

    n = [0, <number of elements in vector> - 1]
*/
static SIMDPP_INL
void mem_unpack6(uint8x16& a, uint8x16& b, uint8x16& c,
                            uint8x16& d, uint8x16& e, uint8x16& f)
{
    uint8x16 t0, t1, t2, t3, t4, t5;
    t0 = zip16_lo(a, d);
    t1 = zip16_hi(a, d);
    t2 = zip16_lo(b, e);
    t3 = zip16_hi(b, e);
    t4 = zip16_lo(c, f);
    t5 = zip16_hi(c, f);

    uint8x16 u0, u1, u2, u3, u4, u5;
    u0 = zip16_lo(t0, t3);
    u1 = zip16_hi(t0, t3);
    u2 = zip16_lo(t1, t4);
    u3 = zip16_hi(t1, t4);
    u4 = zip16_lo(t2, t5);
    u5 = zip16_hi(t2, t5);

    t0 = zip16_lo(u0, u3);
    t1 = zip16_hi(u0, u3);
    t2 = zip16_lo(u1, u4);
    t3 = zip16_hi(u1, u4);
    t4 = zip16_lo(u2, u5);
    t5 = zip16_hi(u2, u5);

    u0 = zip16_lo(t0, t3);
    u1 = zip16_hi(t0, t3);
    u2 = zip16_lo(t1, t4);
    u3 = zip16_hi(t1, t4);
    u4 = zip16_lo(t2, t5);
    u5 = zip16_hi(t2, t5);

    t0 = zip16_lo(u0, u3);
    t1 = zip16_hi(u0, u3);
    t2 = zip16_lo(u1, u4);
    t3 = zip16_hi(u1, u4);
    t4 = zip16_lo(u2, u5);
    t5 = zip16_hi(u2, u5);

    a = zip16_lo(t0, t3);
    b = zip16_hi(t0, t3);
    c = zip16_lo(t1, t4);
    d = zip16_hi(t1, t4);
    e = zip16_lo(t2, t5);
    f = zip16_hi(t2, t5);
}

static SIMDPP_INL
void mem_unpack6(uint16x8& a, uint16x8& b, uint16x8& c,
                            uint16x8& d, uint16x8& e, uint16x8& f)
{
    uint16x8 t0, t1, t2, t3, t4, t5;
    t0 = zip8_lo(a, d);
    t1 = zip8_hi(a, d);
    t2 = zip8_lo(b, e);
    t3 = zip8_hi(b, e);
    t4 = zip8_lo(c, f);
    t5 = zip8_hi(c, f);

    uint16x8 u0, u1, u2, u3, u4, u5;
    u0 = zip8_lo(t0, t3);
    u1 = zip8_hi(t0, t3);
    u2 = zip8_lo(t1, t4);
    u3 = zip8_hi(t1, t4);
    u4 = zip8_lo(t2, t5);
    u5 = zip8_hi(t2, t5);

    a = zip8_lo(u0, u3);
    b = zip8_hi(u0, u3);
    c = zip8_lo(u1, u4);
    d = zip8_hi(u1, u4);
    e = zip8_lo(u2, u5);
    f = zip8_hi(u2, u5);
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

