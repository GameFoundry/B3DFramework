/*  Copyright (C) 2012-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_SHUFFLE_BYTES_MASK_H
#define LIBSIMDPP_SIMDPP_SHUFFLE_BYTES_MASK_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <cstdint>
#include <simdpp/types.h>
#include <simdpp/core/make_uint.h>
#include <simdpp/detail/array.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

/// s - selector, u - number of elements per group
template<int s, unsigned u> SIMDPP_INL
void assert_selector_range()
{
    static_assert(-1 <= s && s < u*2, "Selector out of range");
}

template<int s0, int s1, int u> SIMDPP_INL
void assert_selector_range()
{
    static_assert(-1 <= s0 && s0 < u*2, "Selector out of range");
    static_assert(-1 <= s1 && s1 < u*2, "Selector out of range");
}

template<int s0, int s1, int s2, int s3, int u> SIMDPP_INL
void assert_selector_range()
{
    static_assert(-1 <= s0 && s0 < u*2, "Selector out of range");
    static_assert(-1 <= s1 && s1 < u*2, "Selector out of range");
    static_assert(-1 <= s2 && s2 < u*2, "Selector out of range");
    static_assert(-1 <= s3 && s3 < u*2, "Selector out of range");
}

/// s - selector, u - the number of elements per group
template<int s, int u>
struct get_shuffle_bytex1_16 {
    static const unsigned r0 = (s == -1) ? 0x80 : (s < u ? s : (s-u)+16);
};

/// s - selector, u - the number of elements per group
template<int s, int u>
struct get_shuffle_bytex2_16 {
    static const unsigned r0 = (s == -1) ? 0x80 : (s < u ? s*2   : (s-u)*2+16);
    static const unsigned r1 = (s == -1) ? 0x80 : r0+1;
};

/// s - selector, u - the number of elements per group
template<int s, int u>
struct get_shuffle_bytex4_16 {
    static const unsigned r0 = (s == -1) ? 0x80 : (s < u ? s*4   : (s-u)*4+16);
    static const unsigned r1 = (s == -1) ? 0x80 : r0+1;
    static const unsigned r2 = (s == -1) ? 0x80 : r0+2;
    static const unsigned r3 = (s == -1) ? 0x80 : r0+3;
};

/// s - selector, u - the number of elements per group
template<int s, int u>
struct get_shuffle_bytex8_16 {
    static const unsigned r0 = (s == -1) ? 0x80 : (s < u ? s*8   : (s-u)*8+16);
    static const unsigned r1 = (s == -1) ? 0x80 : r0+1;
    static const unsigned r2 = (s == -1) ? 0x80 : r0+2;
    static const unsigned r3 = (s == -1) ? 0x80 : r0+3;
    static const unsigned r4 = (s == -1) ? 0x80 : r0+4;
    static const unsigned r5 = (s == -1) ? 0x80 : r0+5;
    static const unsigned r6 = (s == -1) ? 0x80 : r0+6;
    static const unsigned r7 = (s == -1) ? 0x80 : r0+7;
};

} // namespace detail

/** Makes a mask to shuffle an int8x16 vector using @c permute_bytes16,
    @c shuffle_bytes16, @c permute_zbytes16 or @c shuffle_zbytes16 functions.

    All elements within vectors are grouped into sets of two adjacent elements.
    Elements within each set of the resulting vector can be selected only from
    corresponding sets of the source vectors.

    The template arguments define which elements to select from each element
    group:
     * Values [0,1] select elements from the first vector.
     * Values [2,3] select elements from the second vector. The mask can only be
       used in @c shuffle_bytes16 or @c shuffle_zbytes16
     * Value [-1] sets the corresponding element to zero. The mask can only be
       used in @c permute_zbytes16 or @c shuffle_zbytes16

    @par 128-bit version:

    The created mask will cause @c shuffle_bytes16 to perform as follows:
    @code
    r0 = (s0 == -1) ? 0 : (s0 < 2 ? a[s0] : b[s0-2])
    r1 = (s1 == -1) ? 0 : (s1 < 2 ? a[s1] : b[s1-2])
    r2 = (s0 == -1) ? 0 : (s0 < 2 ? a[s0+2] : b[s0])
    r3 = (s1 == -1) ? 0 : (s1 < 2 ? a[s1+2] : b[s1])
    ...
    r14 = (s0 == -1) ? 0 : (s0 < 2 ? a[s0+14] : b[s0+12])
    r15 = (s1 == -1) ? 0 : (s1 < 2 ? a[s1+14] : b[s1+12])
    @endcode

    @par 256-bit version:

    The vectors will be shuffled as if the 128-bit version was applied to the
    lower and higher halves of the vectors separately.
*/
template<int s0, int s1, unsigned N> SIMDPP_INL
uint8<N> make_shuffle_bytes16_mask(uint8<N> &mask)
{
    detail::assert_selector_range<s0,s1,2>();
    const unsigned b0 = detail::get_shuffle_bytex1_16<s0,2>::r0;
    const unsigned b1 = detail::get_shuffle_bytex1_16<s1,2>::r0;
    mask = make_uint(b0,   b1,   b0+2, b1+2,
                     b0+4, b1+4, b0+6, b1+6,
                     b0+8, b1+8, b0+10,b1+10,
                     b0+12,b1+12,b0+14,b1+14);
    return mask;
}

/** Makes a mask to shuffle an int8x16 vector using @c permute_bytes16,
    @c shuffle_bytes16, @c permute_zbytes16 or @c shuffle_zbytes16 functions.

    All elements within vectors are grouped into sets of four adjacent
    elements. Elements within each set of the resulting vector can be selected
    only from corresponding sets of the source vectors.

    The template arguments define which elements to select from each element
    group:
     * Values [0,3] select elements from the first vector.
     * Values [4,7] select elements from the second vector. The mask can only be
       used in @c shuffle_bytes16 or @c shuffle_zbytes16
     * Value [-1] sets the corresponding element to zero. The mask can only be
       used in @c permute_zbytes16 or @c shuffle_zbytes16

    @par 128-bit version:

    The created mask will cause @c shuffle_bytes16 to perform as follows:
    @code
    r0 = (s0 == -1) ? 0 : (s0 < 4 ? a[s0] : b[s0-4])
    r1 = (s1 == -1) ? 0 : (s1 < 4 ? a[s1] : b[s1-4])
    r2 = (s2 == -1) ? 0 : (s2 < 4 ? a[s2] : b[s2-4])
    r3 = (s3 == -1) ? 0 : (s3 < 4 ? a[s3] : b[s3-4])
    ...
    r12 = (s0 == -1) ? 0 : (s0 < 4 ? a[s0+12] : b[s0+8])
    r13 = (s1 == -1) ? 0 : (s1 < 4 ? a[s1+12] : b[s1+8])
    r14 = (s2 == -1) ? 0 : (s2 < 4 ? a[s2+12] : b[s2+8])
    r15 = (s3 == -1) ? 0 : (s3 < 4 ? a[s3+12] : b[s3+8])
    @endcode

    @par 256-bit version:

    The vectors will be shuffled as if the 128-bit version was applied to the
    lower and higher halves of the vectors separately.
*/
template<int s0, int s1, int s2, int s3, unsigned N> SIMDPP_INL
uint8<N> make_shuffle_bytes16_mask(uint8<N> &mask)
{
    detail::assert_selector_range<s0,s1,s2,s3,4>();
    const unsigned b0 = detail::get_shuffle_bytex1_16<s0,4>::r0;
    const unsigned b1 = detail::get_shuffle_bytex1_16<s1,4>::r0;
    const unsigned b2 = detail::get_shuffle_bytex1_16<s2,4>::r0;
    const unsigned b3 = detail::get_shuffle_bytex1_16<s3,4>::r0;
    mask = make_uint(b0,   b1,   b2,   b3,
                     b0+4, b1+4, b2+4, b3+4,
                     b0+8, b1+8, b2+8, b3+8,
                     b0+12,b1+12,b2+12,b3+12);
    return mask;
}

/** Makes a mask to shuffle an int8x16 vector using @c permute_bytes16,
    @c shuffle_bytes16, @c permute_zbytes16 or @c shuffle_zbytes16 functions.

    All elements within vectors are grouped into sets of eight adjacent
    elements. Elements within each set of the resulting vector can be selected
    only from corresponding sets of the source vectors.

    The template arguments define which elements to select from each element
    group:
     * Values [0,7] select elements from the first vector.
     * Values [8,15] select elements from the second vector. The mask can only be
       used in @c shuffle_bytes16 or @c shuffle_zbytes16
     * Value [-1] sets the corresponding element to zero. The mask can only be
       used in @c permute_zbytes16 or @c shuffle_zbytes16

    @par 128-bit version:

    The created mask will cause @c shuffle_bytes16 to perform as follows:
    @code
    r0 = (s0 == -1) ? 0 : (s0 < 8 ? a[s0] : b[s0-8])
    ...
    r7 = (s7 == -1) ? 0 : (s7 < 8 ? a[s7] : b[s7-8])
    r8 = (s0 == -1) ? 0 : (s0 < 8 ? a[s0+8] : b[s0])
    ...
    r15 = (s7 == -1) ? 0 : (s7 < 8 ? a[s7+8] : b[s7])
    @endcode

    @par 256-bit version:

    The vectors will be shuffled as if the 128-bit version was applied to the
    lower and higher halves of the vectors separately.
*/
template<int s0, int s1, int s2, int s3, int s4, int s5, int s6, int s7, unsigned N> SIMDPP_INL
uint8<N> make_shuffle_bytes16_mask(uint8<N> &mask)
{
    detail::assert_selector_range<s0,s1,s2,s3,8>();
    detail::assert_selector_range<s4,s5,s6,s7,8>();
    const unsigned b0 = detail::get_shuffle_bytex1_16<s0,8>::r0;
    const unsigned b1 = detail::get_shuffle_bytex1_16<s1,8>::r0;
    const unsigned b2 = detail::get_shuffle_bytex1_16<s2,8>::r0;
    const unsigned b3 = detail::get_shuffle_bytex1_16<s3,8>::r0;
    const unsigned b4 = detail::get_shuffle_bytex1_16<s4,8>::r0;
    const unsigned b5 = detail::get_shuffle_bytex1_16<s5,8>::r0;
    const unsigned b6 = detail::get_shuffle_bytex1_16<s6,8>::r0;
    const unsigned b7 = detail::get_shuffle_bytex1_16<s7,8>::r0;
    mask = make_uint(b0,   b1,   b2,   b3,
                     b4,   b5,   b6,   b7,
                     b0+8, b1+8, b2+8, b3+8,
                     b4+8, b5+8, b6+8, b7+8);
    return mask;
}

/** Makes a mask to shuffle an int8x16 vector using @c permute_bytes16,
    @c shuffle_bytes16, @c permute_zbytes16 or @c shuffle_zbytes16 functions.

    The template arguments define which elements to select from each element
    group:
     * Values [0,15] select elements from the first vector.
     * Values [16,32] select elements from the second vector. The mask can only be
       used in @c shuffle_bytes16 or @c shuffle_zbytes16
     * Value [-1] sets the corresponding element to zero. The mask can only be
       used in @c permute_zbytes16 or @c shuffle_zbytes16

    @par 128-bit version:

    The created mask will cause @c shuffle_bytes16 to perform as follows:
    @code
    r0 = (s0 == -1) ? 0 : (s0 < 16 ? a[s0] : b[s0-16])
    r1 = (s1 == -1) ? 0 : (s0 < 16 ? a[s1] : b[s1-16])
    ...
    r15 = (s15 == -1) ? 0 : (s15 < 16 ? a[s15] : b[s15-16])
    @endcode

    @par 256-bit version:

    The vectors will be shuffled as if the 128-bit version was applied to the
    lower and higher halves of the vectors separately.
*/
template<int s0, int s1, int s2, int s3, int s4, int s5, int s6, int s7,
         int s8, int s9, int s10, int s11, int s12, int s13, int s14, int s15, unsigned N> SIMDPP_INL
uint8<N> make_shuffle_bytes16_mask(uint8<N> &mask)
{
    detail::assert_selector_range<s0,s1,s2,s3,16>();
    detail::assert_selector_range<s4,s5,s6,s7,16>();
    detail::assert_selector_range<s8,s9,s10,s11,16>();
    detail::assert_selector_range<s12,s13,s14,s15,16>();
    const unsigned b0 = detail::get_shuffle_bytex1_16<s0,16>::r0;
    const unsigned b1 = detail::get_shuffle_bytex1_16<s1,16>::r0;
    const unsigned b2 = detail::get_shuffle_bytex1_16<s2,16>::r0;
    const unsigned b3 = detail::get_shuffle_bytex1_16<s3,16>::r0;
    const unsigned b4 = detail::get_shuffle_bytex1_16<s4,16>::r0;
    const unsigned b5 = detail::get_shuffle_bytex1_16<s5,16>::r0;
    const unsigned b6 = detail::get_shuffle_bytex1_16<s6,16>::r0;
    const unsigned b7 = detail::get_shuffle_bytex1_16<s7,16>::r0;
    const unsigned b8 = detail::get_shuffle_bytex1_16<s8,16>::r0;
    const unsigned b9 = detail::get_shuffle_bytex1_16<s9,16>::r0;
    const unsigned b10 = detail::get_shuffle_bytex1_16<s10,16>::r0;
    const unsigned b11 = detail::get_shuffle_bytex1_16<s11,16>::r0;
    const unsigned b12 = detail::get_shuffle_bytex1_16<s12,16>::r0;
    const unsigned b13 = detail::get_shuffle_bytex1_16<s13,16>::r0;
    const unsigned b14 = detail::get_shuffle_bytex1_16<s14,16>::r0;
    const unsigned b15 = detail::get_shuffle_bytex1_16<s15,16>::r0;
    mask = make_uint(b0,  b1,  b2,  b3,
                     b4,  b5,  b6,  b7,
                     b8,  b9,  b10, b11,
                     b12, b13, b14, b15);
    return mask;
}

/** Makes a mask to shuffle an int16x8 vector using @c permute_bytes16,
    @c shuffle_bytes16, @c permute_zbytes16 or @c shuffle_zbytes16 functions.

    All elements within vectors are grouped into sets of two adjacent elements.
    Elements within each set of the resulting vector can be selected only from
    corresponding sets of the source vectors.

    The template arguments define which elements to select from each element
    group:
     * Values [0,1] select elements from the first vector.
     * Values [2,3] select elements from the second vector. The mask can only be
       used in @c shuffle_bytes16 or @c shuffle_zbytes16
     * Value [-1] sets the corresponding element to zero. The mask can only be
       used in @c permute_zbytes16 or @c shuffle_zbytes16

    @par 128-bit version:

    The created mask will cause @c shuffle_bytes16 to perform as follows:
    @code
    r0 = (s0 == -1) ? 0 : (s0 < 2 ? a[s0] : b[s0-2])
    r1 = (s1 == -1) ? 0 : (s1 < 2 ? a[s1] : b[s1-2])
    r2 = (s0 == -1) ? 0 : (s0 < 2 ? a[s0+2] : b[s0])
    r3 = (s1 == -1) ? 0 : (s1 < 2 ? a[s1+2] : b[s1])
    ...
    r6 = (s0 == -1) ? 0 : (s0 < 2 ? a[s0+6] : b[s0+4])
    r7 = (s1 == -1) ? 0 : (s1 < 2 ? a[s1+6] : b[s1+4])
    @endcode

    @par 256-bit version:

    The vectors will be shuffled as if the 128-bit version was applied to the
    lower and higher halves of the vectors separately.
*/
template<int s0, int s1, unsigned N> SIMDPP_INL
uint16<N> make_shuffle_bytes16_mask(uint16<N> &mask)
{
    detail::assert_selector_range<s0,s1,2>();
    using b0 = typename detail::get_shuffle_bytex2_16<s0,2>;
    using b1 = typename detail::get_shuffle_bytex2_16<s1,2>;
    mask = (uint8<N*2>) make_uint(b0::r0,   b0::r1,   b1::r0,   b1::r1,
                                  b0::r0+4, b0::r1+4, b1::r0+4, b1::r1+4,
                                  b0::r0+8, b0::r1+8, b1::r0+8, b1::r1+8,
                                  b0::r0+12,b0::r1+12,b1::r0+12,b1::r1+12);
    return mask;
}

/** Makes a mask to shuffle an int16x8 vector using @c permute_bytes16,
    @c shuffle_bytes16, @c permute_zbytes16 or @c shuffle_zbytes16 functions.

    All elements within vectors are grouped into sets of four adjacent
    elements. Elements within each set of the resulting vector can be selected
    only from corresponding sets of the source vectors.

    The template arguments define which elements to select from each element
    group:
     * Values [0,3] select elements from the first vector.
     * Values [4,7] select elements from the second vector. The mask can only be
       used in @c shuffle_bytes16 or @c shuffle_zbytes16
     * Value [-1] sets the corresponding element to zero. The mask can only be
       used in @c permute_zbytes16 or @c shuffle_zbytes16

    @par 128-bit version:

    The created mask will cause @c shuffle_bytes16 to perform as follows:
    @code
    r0 = (s0 == -1) ? 0 : (s0 < 4 ? a[s0] : b[s0-4])
    r1 = (s1 == -1) ? 0 : (s1 < 4 ? a[s1] : b[s1-4])
    r2 = (s2 == -1) ? 0 : (s2 < 4 ? a[s2] : b[s2-4])
    r3 = (s3 == -1) ? 0 : (s3 < 4 ? a[s3] : b[s3-4])
    ...
    r12 = (s0 == -1) ? 0 : (s0 < 4 ? a[s0+12] : b[s0+8])
    r13 = (s1 == -1) ? 0 : (s1 < 4 ? a[s1+12] : b[s1+8])
    r14 = (s2 == -1) ? 0 : (s2 < 4 ? a[s2+12] : b[s2+8])
    r15 = (s3 == -1) ? 0 : (s3 < 4 ? a[s3+12] : b[s3+8])
    @endcode

    @par 256-bit version:

    The vectors will be shuffled as if the 128-bit version was applied to the
    lower and higher halves of the vectors separately.
*/
template<int s0, int s1, int s2, int s3, unsigned N> SIMDPP_INL
uint16<N> make_shuffle_bytes16_mask(uint16<N> &mask)
{
    detail::assert_selector_range<s0,s1,s2,s3,4>();
    using b0 = typename detail::get_shuffle_bytex2_16<s0,4>;
    using b1 = typename detail::get_shuffle_bytex2_16<s1,4>;
    using b2 = typename detail::get_shuffle_bytex2_16<s2,4>;
    using b3 = typename detail::get_shuffle_bytex2_16<s3,4>;
    mask = (uint8<N*2>) make_uint(b0::r0,   b0::r1,   b1::r0,   b1::r1,
                                  b2::r0,   b2::r1,   b3::r0,   b3::r1,
                                  b0::r0+8, b0::r1+8, b1::r0+8, b1::r1+8,
                                  b2::r0+8, b2::r1+8, b3::r0+8, b3::r1+8);
    return mask;
}

/** Makes a mask to shuffle an int16x8 vector using @c permute_bytes16,
    @c shuffle_bytes16, @c permute_zbytes16 or @c shuffle_zbytes16 functions.

    The template arguments define which elements to select from each element
    group:
     * Values [0,7] select elements from the first vector.
     * Values [8,15] select elements from the second vector. The mask can only be
       used in @c shuffle_bytes16 or @c shuffle_zbytes16
     * Value [-1] sets the corresponding element to zero. The mask can only be
       used in @c permute_zbytes16 or @c shuffle_zbytes16

    @par 128-bit version:

    The created mask will cause @c shuffle_bytes16 to perform as follows:
    @code
    r0 = (s0 == -1) ? 0 : (s0 < 8 ? a[s0] : b[s0-8])
    ...
    r7 = (s7 == -1) ? 0 : (s7 < 8 ? a[s7] : b[s7-8])
    r8 = (s0 == -1) ? 0 : (s0 < 8 ? a[s0+8] : b[s0])
    ...
    r15 = (s7 == -1) ? 0 : (s7 < 8 ? a[s7+8] : b[s7])
    @endcode

    @par 256-bit version:

    The vectors will be shuffled as if the 128-bit version was applied to the
    lower and higher halves of the vectors separately.
*/
template<int s0, int s1, int s2, int s3, int s4, int s5, int s6, int s7,
         unsigned N> SIMDPP_INL
uint16<N> make_shuffle_bytes16_mask(uint16<N> &mask)
{
    detail::assert_selector_range<s0,s1,s2,s3,8>();
    detail::assert_selector_range<s4,s5,s6,s7,8>();

    using b0 = typename detail::get_shuffle_bytex2_16<s0,8>;
    using b1 = typename detail::get_shuffle_bytex2_16<s1,8>;
    using b2 = typename detail::get_shuffle_bytex2_16<s2,8>;
    using b3 = typename detail::get_shuffle_bytex2_16<s3,8>;
    using b4 = typename detail::get_shuffle_bytex2_16<s4,8>;
    using b5 = typename detail::get_shuffle_bytex2_16<s5,8>;
    using b6 = typename detail::get_shuffle_bytex2_16<s6,8>;
    using b7 = typename detail::get_shuffle_bytex2_16<s7,8>;
    mask = (uint8<N*2>) make_uint(b0::r0, b0::r1, b1::r0, b1::r1,
                                  b2::r0, b2::r1, b3::r0, b3::r1,
                                  b4::r0, b4::r1, b5::r0, b5::r1,
                                  b6::r0, b6::r1, b7::r0, b7::r1);
    return mask;
}

/** Makes a mask to shuffle an int32x4 vector using @c permute_bytes16,
    @c shuffle_bytes16, @c permute_zbytes16 or @c shuffle_zbytes16 functions.

    All elements within vectors are grouped into sets of two adjacent elements.
    Elements within each set of the resulting vector can be selected only from
    corresponding sets of the source vectors.

    The template arguments define which elements to select from each element
    group:
     * Values [0,1] select elements from the first vector.
     * Values [2,3] select elements from the second vector. The mask can only be
       used in @c shuffle_bytes16 or @c shuffle_zbytes16
     * Value [-1] sets the corresponding element to zero. The mask can only be
       used in @c permute_zbytes16 or @c shuffle_zbytes16

    @par 128-bit version:

    The created mask will cause @c shuffle_bytes16 to perform as follows:
    @code
    r0 = (s0 == -1) ? 0 : (s0 < 2 ? a[s0] : b[s0-2])
    r1 = (s1 == -1) ? 0 : (s1 < 2 ? a[s1] : b[s1-2])
    r2 = (s0 == -1) ? 0 : (s0 < 2 ? a[s0+2] : b[s0])
    r3 = (s1 == -1) ? 0 : (s1 < 2 ? a[s1+2] : b[s1])
    @endcode

    @par 256-bit version:

    The vectors will be shuffled as if the 128-bit version was applied to the
    lower and higher halves of the vectors separately.
*/
template<int s0, int s1, unsigned N> SIMDPP_INL
uint32<N> make_shuffle_bytes16_mask(uint32<N> &mask)
{
    detail::assert_selector_range<s0,s1,2>();
    using b0 = typename detail::get_shuffle_bytex4_16<s0,2>;
    using b1 = typename detail::get_shuffle_bytex4_16<s1,2>;
    mask = (uint8<N*4>) make_uint(b0::r0,   b0::r1,   b0::r2,   b0::r3,
                                  b1::r0,   b1::r1,   b1::r2,   b1::r3,
                                  b0::r0+8, b0::r1+8, b0::r2+8, b0::r3+8,
                                  b1::r0+8, b1::r1+8, b1::r2+8, b1::r3+8);
    return mask;
}

/** Makes a mask to shuffle an int32x4 vector using @c permute_bytes16,
    @c shuffle_bytes16, @c permute_zbytes16 or @c shuffle_zbytes16 functions.

    The template arguments define which elements to select from each element
    group:
     * Values [0,3] select elements from the first vector.
     * Values [4,7] select elements from the second vector. The mask can only be
       used in @c shuffle_bytes16 or @c shuffle_zbytes16
     * Value [-1] sets the corresponding element to zero. The mask can only be
       used in @c permute_zbytes16 or @c shuffle_zbytes16

    @par 128-bit version:

    The created mask will cause @c shuffle_bytes16 to perform as follows:
    @code
    r0 = (s0 == -1) ? 0 : (s0 < 4 ? a[s0] : b[s0-4])
    r1 = (s1 == -1) ? 0 : (s1 < 4 ? a[s1] : b[s1-4])
    r2 = (s2 == -1) ? 0 : (s2 < 4 ? a[s2] : b[s2-4])
    r3 = (s3 == -1) ? 0 : (s3 < 4 ? a[s3] : b[s3-4])
    @endcode

    @par 256-bit version:

    The vectors will be shuffled as if the 128-bit version was applied to the
    lower and higher halves of the vectors separately.
*/
template<int s0, int s1, int s2, int s3, unsigned N> SIMDPP_INL
uint32<N> make_shuffle_bytes16_mask(uint32<N> &mask)
{
    detail::assert_selector_range<s0,s1,s2,s3,4>();
    using b0 = typename detail::get_shuffle_bytex4_16<s0,4>;
    using b1 = typename detail::get_shuffle_bytex4_16<s1,4>;
    using b2 = typename detail::get_shuffle_bytex4_16<s2,4>;
    using b3 = typename detail::get_shuffle_bytex4_16<s3,4>;
    mask = (uint8<N*4>) make_uint(b0::r0, b0::r1, b0::r2, b0::r3,
                                  b1::r0, b1::r1, b1::r2, b1::r3,
                                  b2::r0, b2::r1, b2::r2, b2::r3,
                                  b3::r0, b3::r1, b3::r2, b3::r3);
    return mask;
}

/** Makes a mask to shuffle an int64x2 vector using @c permute_bytes16,
    @c shuffle_bytes16, @c permute_zbytes16 or @c shuffle_zbytes16 functions.

    The template arguments define which elements to select from each element
    group:
     * Values [0,1] select elements from the first vector.
     * Values [2,3] select elements from the second vector. The mask can only be
       used in @c shuffle_bytes16 or @c shuffle_zbytes16
     * Value [-1] sets the corresponding element to zero. The mask can only be
       used in @c permute_zbytes16 or @c shuffle_zbytes16

    @par 128-bit version:

    The created mask will cause @c shuffle_bytes16 to perform as follows:
    @code
    r0 = (s0 == -1) ? 0 : (s0 < 2 ? a[s0] : b[s0])
    r1 = (s1 == -1) ? 0 : (s1 < 2 ? a[s1] : b[s1])
    @endcode

    @par 256-bit version:

    The vectors will be shuffled as if the 128-bit version was applied to the
    lower and higher halves of the vectors separately.
*/
template<int s0, int s1, unsigned N> SIMDPP_INL
uint64<N> make_shuffle_bytes16_mask(uint64<N> &mask)
{
    detail::assert_selector_range<s0,s1,2>();
    using b0 = typename detail::get_shuffle_bytex8_16<s0,2>;
    using b1 = typename detail::get_shuffle_bytex8_16<s1,2>;
    mask = (uint8<N*8>) make_uint(b0::r0, b0::r1, b0::r2, b0::r3,
                                  b0::r4, b0::r5, b0::r6, b0::r7,
                                  b1::r0, b1::r1, b1::r2, b1::r3,
                                  b1::r4, b1::r5, b1::r6, b1::r7);
    return mask;
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
