/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_BLEND_H
#define LIBSIMDPP_SIMDPP_CORE_BLEND_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/blend.h>
#include <simdpp/detail/get_expr.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

namespace detail {

/*  Blend is a special function in that the type of the returned expression
    depends on three arguments.

    As always, we want to reduce the number of overloads that need to be
    created in order to match a specific case of an expression tree containing
    'blend' nodes. In this case we do the following in an attempt to achieve
    that:

     * the first and the second types have the same type as the expression
        itself, except that signed integer vectors are converted to unsigned
     * the third type is the same as the expression itself, except when it is
        a mask. In that case it is converted to floating-point mask if the
        expression is floating-point expression and to integer mask otherwise
     * TODO

     So, as a result, the following tuples of types will appear as the arguments
     of the returned expression:

      * mask_int8, mask_int8, mask_int8
      * uint8, uint8, uint8
      * uint8, uint8, mask_int8
      * mask_int16, mask_int16, mask_int16
      * uint16, uint16, uint16
      * uint16, uint16, mask_uint16
      * mask_int32, mask_int32, mask_int32
      * mask_float32, mask_float32, mask_float32
      * uint32, uint32, uint32
      * uint32, uint32, mask_int32
      * float32, float32, float32
      * float32, float32, mask_float32
      * mask_int64, mask_int64, mask_int64
      * mask_float64, mask_float64, mask_float64
      * uint64, uint64, uint64
      * uint64, uint64, mask_int64
      * float64, float64, float64
      * float64, float64, mask_float64

    The type of the returned expression is governed by the usual rules
    (see simdpp/types/tag.h)
*/

template<class V1, class V2, class V3>
class get_expr_blend {

    // (size_tag) get the size tag of the resulting expression
    static const unsigned size_tag_t1 = V1::size_tag > V2::size_tag ? V1::size_tag : V2::size_tag;
    static const unsigned size_tag = size_tag_t1 > V3::size_tag ? size_tag_t1 : V3::size_tag;

    // (type_tag_t2) get the type tag of the first pair of parameters. We
    // compute it by applying the promotion rules to the first two parameters,
    // i.e. type_tag_t2 == get_expr2<V1,V2>::type::type_tag
    static const unsigned type_tag_t1 = V1::type_tag > V2::type_tag ? V1::type_tag : V2::type_tag;
    static const bool is_mask_op1 = type_tag_t1 == SIMDPP_TAG_MASK_INT ||
                                    type_tag_t1 == SIMDPP_TAG_MASK_FLOAT;
    static const unsigned type_tag_t2 = (is_mask_op1 && V1::size_tag != V2::size_tag)
                                    ? SIMDPP_TAG_UINT : type_tag_t1;

    // (type_tag) get the type tag of the expression. We compute it by applying
    // the promotion rules to the pair that includes the third parameter and
    // the result of the first promotion.
    // I.e. type_tag == get_expr2<get_expr2<V1,V2>::type, V3>::type::type_tag
    static const unsigned type_tag_t3 = type_tag_t2 > V3::type_tag ? type_tag_t2 : V3::type_tag;
    static const bool is_mask_op2 = type_tag_t3 == SIMDPP_TAG_MASK_INT ||
                                    type_tag_t3 == SIMDPP_TAG_MASK_FLOAT;
    static const unsigned type_tag = (is_mask_op2 && V3::size_tag != size_tag_t1)
                                    ? SIMDPP_TAG_UINT : type_tag_t3;

    // strip signed types
    static const unsigned v12_type_tag = type_tag == SIMDPP_TAG_INT ? SIMDPP_TAG_UINT : type_tag;


    static const bool is_v3_mask = V3::type_tag == SIMDPP_TAG_MASK_INT ||
                                   V3::type_tag == SIMDPP_TAG_MASK_FLOAT;
    static const bool is_v12_float = v12_type_tag == SIMDPP_TAG_FLOAT ||
                                     v12_type_tag == SIMDPP_TAG_MASK_FLOAT;

    // if third parameter is a mask and its size tag matches the size tag of the
    // first two parameters, then convert the mask to float mask if the
    // expression is float and to integer mask otherwise
    static const unsigned v3_type_tag = (!is_v3_mask || size_tag != V3::size_tag) ? v12_type_tag :
                                        is_v12_float ? SIMDPP_TAG_MASK_FLOAT :
                                        SIMDPP_TAG_MASK_INT;


public:
    using v1_final_type = typename type_of_tag<v12_type_tag + size_tag,
                                               V1::length_bytes, void>::type;

    using v2_final_type = typename type_of_tag<v12_type_tag + size_tag,
                                               V1::length_bytes, void>::type;

    using v3_final_type = typename type_of_tag<v3_type_tag + size_tag,
                                               V1::length_bytes, void>::type;

    using type = typename type_of_tag<type_tag + size_tag, V1::length_bytes,
                                      expr_blend<V1, V2, V3>>::type;
};

} // namespace detail

/** Composes a vector from two sources according to a mask. Each element within
    the mask must have either all bits set or all bits unset.

    @code
    r0 = (mask0 == 0xff ) ? on0 : off0
    ...
    rN = (maskN == 0xff ) ? onN : offN
    @endcode

    @todo icost

    @par int16

    @par 128-bit version:
    @icost{SSE2-AVX, 3}

    @par 256-bit version:
    @icost{SSE2-AVX, 6}
    @icost{NEON, ALTIVEC, 2}

    @par int32

    @par 128-bit version:
    @icost{SSE2-AVX, 3}

    @par 256-bit version:
    @icost{SSE2-AVX, 6}
    @icost{NEON, ALTIVEC, 2}

    @par int64

    @par 128-bit version:
    @icost{SSE2-AVX, 3}

    @par 256-bit version:
    @icost{SSE2-AVX, 6}
    @icost{NEON, ALTIVEC, 2}

    @par float32

    @par 128-bit version:
    @icost{SSE2-SSE4.1, 3}

    @par 256-bit version:
    @icost{SSE2-SSE4.1, 6}
    @icost{NEON, ALTIVEC, 2}

    @par float64

    @par 128-bit version:
    @icost{SSE2-SSE4.1, 3}
    @novec{NEON, ALTIVEC}

    @par 256-bit version:
    @icost{SSE2-SSE4.1, 6}
    @novec{NEON, ALTIVEC}
*/
template<unsigned N, class V1, class V2, class V3> SIMDPP_INL
typename detail::get_expr_blend<V1, V2, V3>::type
        blend(const any_vec<N,V1>& on, const any_vec<N,V2>& off,
              const any_vec<N,V3>& mask)
{
    return { { on.wrapped(), off.wrapped(), mask.wrapped() } };
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

