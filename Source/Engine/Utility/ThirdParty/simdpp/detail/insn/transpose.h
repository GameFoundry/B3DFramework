/*  Copyright (C) 2012-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_TRANSPOSE_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_TRANSPOSE_H

#include <simdpp/types.h>
#include <simdpp/detail/not_implemented.h>
#include <simdpp/detail/width.h>
#include <simdpp/core/permute_bytes16.h>
#include <simdpp/core/zip_lo.h>
#include <simdpp/core/zip_hi.h>
#include <simdpp/detail/null/transpose.h>
#include <simdpp/detail/neon/shuffle.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {


template<class V8, class V16, class V32> SIMDPP_INL
void v_sse_transpose8x4(V8& a0, V8& a1, V8& a2, V8& a3);
template<class V16, class V32, class V64> SIMDPP_INL
void v_sse_transpose16x4(V16& a0, V16& a1, V16& a2, V16& a3);
template<class V, class D> SIMDPP_INL
void v_sse_transpose32x4(V& a0, V& a1, V& a2, V& a3);

/** Transposes eight 2x2 8-bit matrices within two int8x16 vectors

    @code
    r0 = [ a0_0; a1_0 ; ... ; a0_14; a1_14 ]
    r1 = [ a1_1; a1_1 ; ... ; a0_15; a0_15 ]
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 4}
    @icost{ALTIVEC, 2-4}

    @par 256-bit version:
    @icost{SSE2-AVX, 8}
    @icost{AVX2, 4}
    @icost{ALTIVEC, 4-6}

    The lower and higher 128-bit halves are processed as if 128-bit instruction
    was applied to each of them separately.
*/
static SIMDPP_INL
void i_transpose2(uint8x16& a0, uint8x16& a1)
{
#if SIMDPP_USE_NULL
    detail::null::transpose2(a0, a1);
#elif SIMDPP_USE_NEON
    auto r = vtrnq_u8(a0.native(), a1.native());
    a0 = r.val[0];
    a1 = r.val[1];
#elif SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    uint8x16 m0 = make_shuffle_bytes16_mask<0,16+0, 2,16+2, 4,16+4, 6,16+6,
                                            8,16+8, 10,16+10, 12,16+12, 14,16+14>(m0);
    uint8x16 m1 = make_shuffle_bytes16_mask<1,16+1, 3,16+3, 5,16+5, 7,16+7,
                                            9,16+9, 11,16+11, 13,16+13, 15,16+15>(m1);
    uint16x8 b0, b1;
    b0 = shuffle_bytes16(a0, a1, m0);
    b1 = shuffle_bytes16(a0, a1, m1);
    a0 = b0;  a1 = b1;
#else
    SIMDPP_NOT_IMPLEMENTED2(a0, a1);
#endif
}

/** Helper function.

    @code
    r = [a0,a4,a8,a12,a1,a5,a9,a13,a2,a6,a10,a14,a3,a7,a11,a15]
    @endcode

    The 256-bit version applies the 128 bit operation to the two halves.

    Needs SSSE3
*/
static SIMDPP_INL
uint8x16 transpose_inplace(const uint8x16& a)
{
#if SIMDPP_USE_SSSE3 || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    // the compiler will take this out of any loops automatically
    uint8x16 idx = make_uint(0, 4, 8, 12, 1, 5, 9, 13,
                            2, 6, 10,14, 3, 7, 11,15);
    return permute_bytes16(a, idx);
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}

static SIMDPP_INL
uint8x32 transpose_inplace(const uint8x32& a)
{
#if SIMDPP_USE_AVX2
    uint8x32 idx = make_uint(0, 4, 8, 12, 1, 5, 9, 13,
                             2, 6, 10,14, 3, 7, 11,15);
    return permute_bytes16(a, idx);
#elif SIMDPP_USE_SSSE3 || SIMDPP_USE_ALTIVEC
    SIMDPP_VEC_ARRAY_IMPL1(uint8x32, transpose_inplace, a);
#else
    return SIMDPP_NOT_IMPLEMENTED1(a);
#endif
}

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
uint8<64> transpose_inplace(const uint8<64>& a)
{
    uint8<64> idx = make_uint(0, 4, 8, 12, 1, 5, 9, 13,
                              2, 6, 10,14, 3, 7, 11,15);
    return permute_bytes16(a, idx);
}
#endif

static SIMDPP_INL
void i_transpose2(uint16x8& a0, uint16x8& a1)
{
#if SIMDPP_USE_NULL
    detail::null::transpose2(a0, a1);
#elif SIMDPP_USE_SSE2
    uint32x4 b0, b1;
    b0 = zip8_lo(a0, a1);
    b1 = zip8_hi(a0, a1);
    a0 = shuffle2<0,2,0,2>(b0, b1);
    a1 = shuffle2<1,3,1,3>(b0, b1);
#elif SIMDPP_USE_NEON
    auto r = vtrnq_u16(a0.native(), a1.native());
    a0 = r.val[0];
    a1 = r.val[1];
#elif SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    uint16x8 m0 = make_shuffle_bytes16_mask<0,8+0, 2,8+2, 4,8+4, 6,8+6>(m0);
    uint16x8 m1 = make_shuffle_bytes16_mask<1,8+1, 3,8+3, 5,8+5, 7,8+7>(m1);
    uint16x8 b0, b1;
    b0 = shuffle_bytes16(a0, a1, m0);
    b1 = shuffle_bytes16(a0, a1, m1);
    a0 = b0;  a1 = b1;
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_transpose2(uint16x16& a0, uint16x16& a1)
{
    uint32x8 b0, b1;
    b0 = zip8_lo(a0, a1);
    b1 = zip8_hi(a0, a1);
    a0 = shuffle2<0,2,0,2>(b0, b1);
    a1 = shuffle2<1,3,1,3>(b0, b1);
}
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL void i_transpose2(uint16<32>& a0, uint16<32>& a1)
{
    uint32<16> b0, b1;
    b0 = zip8_lo(a0, a1);
    b1 = zip8_hi(a0, a1);
    a0 = shuffle2<0,2,0,2>(b0, b1);
    a1 = shuffle2<1,3,1,3>(b0, b1);
}
#endif

template<unsigned N> SIMDPP_INL
void i_transpose2(uint16<N>& a0, uint16<N>& a1)
{
    SIMDPP_VEC_ARRAY_IMPL_REF2(uint16<N>, i_transpose2, a0, a1);
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_transpose2(uint32x4& a0, uint32x4& a1)
{
#if SIMDPP_USE_NULL
    detail::null::transpose2(a0, a1);
#elif SIMDPP_USE_SSE2
    uint64x2 b0, b1;
    b0 = zip4_lo(a0, a1);
    b1 = zip4_hi(a0, a1);
    a0 = zip2_lo(b0, b1);
    a1 = zip2_hi(b0, b1);
#elif SIMDPP_USE_NEON
    auto r = vtrnq_u32(a0.native(), a1.native());
    a0 = r.val[0];
    a1 = r.val[1];
#elif SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    uint32x4 m0 = make_shuffle_bytes16_mask<0,4+0, 2,4+2>(m0);
    uint32x4 m1 = make_shuffle_bytes16_mask<1,4+1, 3,4+3>(m1);
    uint32x4 b0, b1;
    b0 = shuffle_bytes16(a0, a1, m0);
    b1 = shuffle_bytes16(a0, a1, m1);
    a0 = b0;  a1 = b1;
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_transpose2(uint32x8& a0, uint32x8& a1)
{
    uint64x4 b0, b1;
    b0 = zip4_lo(a0, a1);
    b1 = zip4_hi(a0, a1);
    a0 = zip2_lo(b0, b1);
    a1 = zip2_hi(b0, b1);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_transpose2(uint32<16>& a0, uint32<16>& a1)
{
    uint64<8> b0, b1;
    b0 = zip4_lo(a0, a1);
    b1 = zip4_hi(a0, a1);
    a0 = zip2_lo(b0, b1);
    a1 = zip2_hi(b0, b1);
}
#endif

template<unsigned N> SIMDPP_INL
void i_transpose2(uint32<N>& a0, uint32<N>& a1)
{
    SIMDPP_VEC_ARRAY_IMPL_REF2(uint32<N>, i_transpose2, a0, a1);
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_transpose2(uint64x2& a0, uint64x2& a1)
{
#if SIMDPP_USE_SSE2 || SIMDPP_USE_VSX_207 || SIMDPP_USE_MSA
    uint64x2 b0;
    b0 = zip2_lo(a0, a1);
    a1 = zip2_hi(a0, a1);
    a0 = b0;
#elif SIMDPP_USE_NEON
    neon::transpose2(a0, a1);
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC
    detail::null::transpose2(a0, a1);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_transpose2(uint64x4& a0, uint64x4& a1)
{
    uint64x4 b0;
    b0 = zip2_lo(a0, a1);
    a1 = zip2_hi(a0, a1);
    a0 = b0;
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_transpose2(uint64<8>& a0, uint64<8>& a1)
{
    uint64<8> b0;
    b0 = zip2_lo(a0, a1);
    a1 = zip2_hi(a0, a1);
    a0 = b0;
}
#endif

template<unsigned N> SIMDPP_INL
void i_transpose2(uint64<N>& a0, uint64<N>& a1)
{
    SIMDPP_VEC_ARRAY_IMPL_REF2(uint64<N>, i_transpose2, a0, a1);
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_transpose2(float32x4& a0, float32x4& a1)
{
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    detail::null::transpose2(a0, a1);
#elif SIMDPP_USE_SSE2
    float64x2 b0, b1;
    b0 = bit_cast<float64x2>(zip4_lo(a0, a1));
    b1 = bit_cast<float64x2>(zip4_hi(a0, a1));
    a0 = bit_cast<float32x4>(zip2_lo(b0, b1));
    a1 = bit_cast<float32x4>(zip2_hi(b0, b1));
#elif SIMDPP_USE_NEON
    auto r = vtrnq_f32(a0.native(), a1.native());
    a0 = r.val[0];
    a1 = r.val[1];
#elif SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    uint32x4 m0 = make_shuffle_bytes16_mask<0,4+0, 2,4+2>(m0);
    uint32x4 m1 = make_shuffle_bytes16_mask<1,4+1, 3,4+3>(m1);
    float32x4 b0, b1;
    b0 = shuffle_bytes16(a0, a1, m0);
    b1 = shuffle_bytes16(a0, a1, m1);
    a0 = b0;  a1 = b1;
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
void i_transpose2(float32x8& a0, float32x8& a1)
{
    float64x4 b0, b1;
    b0 = zip4_lo(a0, a1);
    b1 = zip4_hi(a0, a1);
    a0 = zip2_lo(b0, b1);
    a1 = zip2_hi(b0, b1);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_transpose2(float32<16>& a0, float32<16>& a1)
{
    float64<8> b0, b1;
    b0 = zip4_lo(a0, a1);
    b1 = zip4_hi(a0, a1);
    a0 = zip2_lo(b0, b1);
    a1 = zip2_hi(b0, b1);
}
#endif

template<unsigned N> SIMDPP_INL
void i_transpose2(float32<N>& a0, float32<N>& a1)
{
    SIMDPP_VEC_ARRAY_IMPL_REF2(float32<N>, i_transpose2, a0, a1);
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_transpose2(float64x2& a0, float64x2& a1)
{
#if SIMDPP_USE_SSE2 || SIMDPP_USE_VSX_206 || SIMDPP_USE_MSA
    float64x2 b0;
    b0 = zip2_lo(a0, a1);
    a1 = zip2_hi(a0, a1);
    a0 = b0;
#elif SIMDPP_USE_NEON64
    uint64x2 b0, b1;
    b0 = a0;  b1 = a1;
    i_transpose2(b0, b1);
    a0 = b0;  a1 = b1;
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC
    detail::null::transpose2(a0, a1);
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
void i_transpose2(float64x4& a0, float64x4& a1)
{
    float64x4 b0;
    b0 = zip2_lo(a0, a1);
    a1 = zip2_hi(a0, a1);
    a0 = b0;
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_transpose2(float64<8>& a0, float64<8>& a1)
{
    float64<8> b0;
    b0 = zip2_lo(a0, a1);
    a1 = zip2_hi(a0, a1);
    a0 = b0;
}
#endif

template<unsigned N> SIMDPP_INL
void i_transpose2(float64<N>& a0, float64<N>& a1)
{
    SIMDPP_VEC_ARRAY_IMPL_REF2(float64<N>, i_transpose2, a0, a1);
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_transpose4(uint32x4& a0, uint32x4& a1,
                  uint32x4& a2, uint32x4& a3);

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_transpose4(uint32x8& a0, uint32x8& a1,
                  uint32x8& a2, uint32x8& a3);
#endif

static SIMDPP_INL
void i_transpose4(uint8x16& a0, uint8x16& a1,
                  uint8x16& a2, uint8x16& a3)
{
    // [a0,a1,a2,a3 ... ]
    // [b0,b1,b2,b3 ... ]
    // [c0,c1,c2,c3 ... ]
    // [d0,d1,d2,d3 ... ]
#if SIMDPP_USE_NULL
    detail::null::transpose4(a0, a1, a2, a3);
#elif SIMDPP_USE_SSE2
    v_sse_transpose8x4<uint8<16>, uint16<8>, uint32<4>>(a0, a1, a2, a3);
#elif SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    uint16x8 b0, b1, b2, b3;
    i_transpose2(a0, a1);  // 8-bit transpose
    i_transpose2(a2, a3);
    b0 = a0;  b1 = a1;  b2 = a2;  b3 = a3;
    i_transpose2(b0, b2);  // 16-bit transpose
    i_transpose2(b1, b3);
    a0 = b0;  a1 = b1;  a2 = b2;  a3 = b3;
#endif
}


#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_transpose4(uint8x32& a0, uint8x32& a1,
                  uint8x32& a2, uint8x32& a3)
{
    v_sse_transpose8x4<uint8<32>, uint16<16>, uint32<8>>(a0, a1, a2, a3);
}
#endif

#if SIMDPP_USE_AVX512BW
static SIMDPP_INL
void i_transpose4(uint8<64>& a0, uint8<64>& a1,
                  uint8<64>& a2, uint8<64>& a3)
{
    v_sse_transpose8x4<uint8<64>, uint16<32>, uint32<16>>(a0, a1, a2, a3);
}
#endif

template<unsigned N> SIMDPP_INL
void i_transpose4(uint8<N>& a0, uint8<N>& a1, uint8<N>& a2, uint8<N>& a3)
{
    SIMDPP_VEC_ARRAY_IMPL_REF4(uint8<N>, i_transpose4, a0, a1, a2, a3);
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_transpose4(uint16x8& a0, uint16x8& a1,
                  uint16x8& a2, uint16x8& a3)
{
#if SIMDPP_USE_NULL
    detail::null::transpose4(a0, a1, a2, a3);
#elif SIMDPP_USE_SSE2
    v_sse_transpose16x4<uint16<8>, uint32<4>, uint64<2>>(a0, a1, a2, a3);
#elif SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    uint32x4 b0, b1, b2, b3;
    i_transpose2(a0, a1);  // 16-bit transpose
    i_transpose2(a2, a3);
    b0 = a0;  b1 = a1;  b2 = a2;  b3 = a3;
    i_transpose2(b0, b2);  // 32-bit transpose
    i_transpose2(b1, b3);
    a0 = b0;  a1 = b1;  a2 = b2;  a3 = b3;
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_transpose4(uint16x16& a0, uint16x16& a1,
                             uint16x16& a2, uint16x16& a3)
{
    v_sse_transpose16x4<uint16<16>, uint32<8>, uint64<4>>(a0, a1, a2, a3);
}
#endif

#if SIMDPP_USE_AVX2
SIMDPP_INL void i_transpose4(uint16<32>& a0, uint16<32>& a1,
                             uint16<32>& a2, uint16<32>& a3)
{
    v_sse_transpose16x4<uint16<32>, uint32<16>, uint64<8>>(a0, a1, a2, a3);
}
#endif

template<unsigned N> SIMDPP_INL
void i_transpose4(uint16<N>& a0, uint16<N>& a1, uint16<N>& a2, uint16<N>& a3)
{
    SIMDPP_VEC_ARRAY_IMPL_REF4(uint16<N>, i_transpose4, a0, a1, a2, a3);
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_transpose4(uint32x4& a0, uint32x4& a1,
                  uint32x4& a2, uint32x4& a3)
{
#if SIMDPP_USE_NULL
    detail::null::transpose4(a0, a1, a2, a3);
#elif SIMDPP_USE_SSE2
    v_sse_transpose32x4<uint32<4>, uint64<2>>(a0, a1, a2, a3);
#elif SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC || SIMDPP_USE_MSA
    uint64x2 b0, b1, b2, b3;
    i_transpose2(a0, a1);  // 32-bit transpose
    i_transpose2(a2, a3);
    b0 = a0;  b1 = a1;  b2 = a2;  b3 = a3;
    i_transpose2(b0, b2);  // 64-bit transpose
    i_transpose2(b1, b3);
    a0 = b0;  a1 = b1;  a2 = b2;  a3 = b3;
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_transpose4(uint32x8& a0, uint32x8& a1,
                  uint32x8& a2, uint32x8& a3)
{
    v_sse_transpose32x4<uint32<8>, uint64<4>>(a0, a1, a2, a3);
}
#endif

#if SIMDPP_USE_AVX2
static SIMDPP_INL
void i_transpose4(uint32<16>& a0, uint32<16>& a1,
                  uint32<16>& a2, uint32<16>& a3)
{
    v_sse_transpose32x4<uint32<16>, uint64<8>>(a0, a1, a2, a3);
}
#endif

template<unsigned N> SIMDPP_INL
void i_transpose4(uint32<N>& a0, uint32<N>& a1, uint32<N>& a2, uint32<N>& a3)
{
    SIMDPP_VEC_ARRAY_IMPL_REF4(uint32<N>, i_transpose4, a0, a1, a2, a3);
}

// -----------------------------------------------------------------------------

static SIMDPP_INL
void i_transpose4(float32x4& a0, float32x4& a1,
                  float32x4& a2, float32x4& a3)
{
#if SIMDPP_USE_SSE2
    v_sse_transpose32x4<float32<4>, float64<2>>(a0, a1, a2, a3);
#else
    uint32x4 b0, b1, b2, b3;
    b0 = a0;  b1 = a1;  b2 = a2;  b3 = a3;
    i_transpose4(b0, b1, b2, b3);
    a0 = b0;  a1 = b1;  a2 = b2;  a3 = b3;
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
void i_transpose4(float32x8& a0, float32x8& a1,
                  float32x8& a2, float32x8& a3)
{
    v_sse_transpose32x4<float32<8>, float64<4>>(a0, a1, a2, a3);
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
void i_transpose4(float32<16>& a0, float32<16>& a1,
                  float32<16>& a2, float32<16>& a3)
{
    v_sse_transpose32x4<float32<16>, float64<8>>(a0, a1, a2, a3);
}
#endif

template<unsigned N> SIMDPP_INL
void i_transpose4(float32<N>& a0, float32<N>& a1, float32<N>& a2, float32<N>& a3)
{
    SIMDPP_VEC_ARRAY_IMPL_REF4(float32<N>, i_transpose4, a0, a1, a2, a3);
}

// -----------------------------------------------------------------------------

template<class V, class D> SIMDPP_INL
void v_sse_transpose32x4(V& a0, V& a1, V& a2, V& a3)
{
    D b0, b1, b2, b3;
    // [a0,a1,a2,a3]
    // [b0,b1,b2,b3]
    // [c0,c1,c2,c3]
    // [d0,d1,d2,d3]
    b0 = zip4_lo(a0, a1);
    b1 = zip4_hi(a0, a1);
    b2 = zip4_lo(a2, a3);
    b3 = zip4_hi(a2, a3);
    // [a0,b0,a1,b1]
    // [a2,b2,a3,b3]
    // [c0,d0,c1,d1]
    // [c2,d2,c3,d3]
    a0 = zip2_lo(b0, b2);
    a1 = zip2_hi(b0, b2);
    a2 = zip2_lo(b1, b3);
    a3 = zip2_hi(b1, b3);
}

template<class V16, class V32, class V64> SIMDPP_INL
void v_sse_transpose16x4(V16& a0, V16& a1, V16& a2, V16& a3)
{
    V32 b0, b1, b2, b3;
    V64 c0, c1, c2, c3;
    b0 = zip8_lo(a0, a1);
    b1 = zip8_hi(a0, a1);
    b2 = zip8_lo(a2, a3);
    b3 = zip8_hi(a2, a3);
    // [a0,b0,a1,b1,a2,b2,a3,b3]
    // [a4,b4,a5,b5,a6,b6,a7,b7]
    // [c0,d0,c1,d1,c2,d2,c3,d3]
    // [c4,d4,c5,d5,c6,d6,c7,d7]
    c0 = zip4_lo(b0, b2);
    c1 = zip4_hi(b0, b2);
    c2 = zip4_lo(b1, b3);
    c3 = zip4_hi(b1, b3);
    // [a0,b0,c0,d0,a1,b1,c1,d1]
    // [a2,b2,c2,d2,a3,b3,c3,d3]
    // [a4,b4,c4,d4,a5,b5,c5,d5]
    // [a6,b6,c6,d6,a7,b7,c7,d7]
    a0 = zip2_lo(c0, c2);
    a1 = zip2_hi(c0, c2);
    a2 = zip2_lo(c1, c3);
    a3 = zip2_hi(c1, c3);
    // [a0,b0,c0,d0,a4,b4,c4,d4]
    // [a1,b1,c1,d1,a5,b5,c5,d5]
    // [a2,b2,c2,d2,a6,b6,c6,d6]
    // [a3,b3,c3,d3,a7,b7,c7,d7]
}

template<class V8, class V16, class V32> SIMDPP_INL
void v_sse_transpose8x4(V8& a0, V8& a1, V8& a2, V8& a3)
{
    V16 b0, b1, b2, b3;
    b0 = zip16_lo(a0, a1);
    b1 = zip16_lo(a2, a3);
    b2 = zip16_hi(a0, a1);
    b3 = zip16_hi(a2, a3);
    // [a0,b0,a1,b1,a2,b2,a3,b3 ... b7]
    // [c0,d0,c1,d1,c2,d2,c3,d3 ... d7]
    // [a8 ... b15]
    // [c8 ... d15]
    V32 c0, c1, c2, c3;
    c0 = zip8_lo(b0, b1);
    c1 = zip8_hi(b0, b1);
    c2 = zip8_lo(b2, b3);
    c3 = zip8_hi(b2, b3);
    // [a0,b0,c0,d0,[a..d]1, [a..d]2,  [a..d]3]
    // [[a..d]4,    [a..d]5, [a..d]6,  [a..d]7]
    // [[a..d]8,    [a..d]9, [a..d]10, [a..d]11]
    // [[a..d]12,   [a..d]13,[a..d]14, [a..d]15]
    i_transpose4(c0, c1, c2, c3); // 32-bit transpose
    a0 = c0;
    a1 = c1;
    a2 = c2;
    a3 = c3;
}


} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
