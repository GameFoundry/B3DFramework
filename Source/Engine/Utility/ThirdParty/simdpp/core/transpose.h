/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_TRANSPOSE_H
#define LIBSIMDPP_SIMDPP_CORE_TRANSPOSE_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/make_shuffle_bytes_mask.h>
#include <simdpp/core/bit_and.h>
#include <simdpp/core/shuffle2.h>
#include <simdpp/detail/insn/transpose.h>
#include <simdpp/detail/neon/shuffle.h>
#include <simdpp/detail/null/transpose.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Transposes four 2x2 16-bit matrices within two int16x8 vectors

    Mask or expression vectors are not supported.

    @code
    r0 = [ a0_0; a1_0 ; ... ; a0_6; a1_6 ]
    r1 = [ a0_1; a1_1 ; ... ; a0_7; a0_7 ]
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 4}
    @icost{ALTIVEC, 2-4}

    @par 256-bit version:
    The lower and higher 128-bit halves are processed as if 128-bit instruction
    was applied to each of them separately.

    @icost{SSE2-AVX, 8}
    @icost{AVX2, 4}
    @icost{NEON, 2}
    @icost{ALTIVEC, 4-6}
*/
template<unsigned N, class V> SIMDPP_INL
void transpose2(any_int16<N,V>& a0, any_int16<N,V>& a1)
{
    static_assert(!is_mask<V>::value, "Mask vectors are not supported");
    static_assert(is_value_vector<V>::value, "Expression vectors are not supported");
    uint16<N> qa0 = a0.wrapped();
    uint16<N> qa1 = a1.wrapped();
    detail::insn::i_transpose2(qa0, qa1);
    a0.wrapped() = qa0;
    a1.wrapped() = qa1;
}

/** Transposes two 2x2 32-bit matrices within two int32x4 vectors

    @code
    r0 = [ a0_0; a1_0 ; a0_2; a1_2 ]
    r1 = [ a0_1; a1_1 ; a1_3; a0_3 ]
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 4}
    @icost{ALTIVEC, 2-4}

    @par 256-bit version:
    The lower and higher 128-bit halves are processed as if 128-bit instruction
    was applied to each of them separately.

    @icost{SSE2-AVX, 8}
    @icost{AVX2, 4}
    @icost{NEON, 2}
    @icost{ALTIVEC, 4-6}
*/
template<unsigned N, class V> SIMDPP_INL
void transpose2(any_int32<N,V>& a0, any_int32<N,V>& a1)
{
    static_assert(!is_mask<V>::value, "Mask vectors are not supported");
    static_assert(is_value_vector<V>::value, "Expression vectors are not supported");
    uint32<N> qa0 = a0.wrapped();
    uint32<N> qa1 = a1.wrapped();
    detail::insn::i_transpose2(qa0, qa1);
    a0.wrapped() = qa0;
    a1.wrapped() = qa1;
}

/** Transposes a 2x2 64-bit matrix within two int64x2 vectors

    @code
    r0 = [ a0_0; a1_0 ]
    r1 = [ a0_1; a1_1 ]
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 2}
    @icost{ALTIVEC, 2-4}

    @par 256-bit version:
    The lower and higher 128-bit halves are processed as if 128-bit instruction
    was applied to each of them separately.

    @icost{SSE2-AVX, 4}
    @icost{AVX2, 2}
    @icost{NEON, 2}
    @icost{ALTIVEC, 4-6}
*/
template<unsigned N, class V> SIMDPP_INL
void transpose2(any_int64<N,V>& a0, any_int64<N,V>& a1)
{
    static_assert(!is_mask<V>::value, "Mask vectors are not supported");
    static_assert(is_value_vector<V>::value, "Expression vectors are not supported");
    uint64<N> qa0 = a0.wrapped();
    uint64<N> qa1 = a1.wrapped();
    detail::insn::i_transpose2(qa0, qa1);
    a0.wrapped() = qa0;
    a1.wrapped() = qa1;
}

/** Transposes two 2x2 32-bit matrices within two float32x4 vectors

    @code
    r0 = [ a0_0; a1_0 ; a0_2; a1_2 ]
    r1 = [ a0_1; a1_1 ; a0_3; a0_3 ]
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 4}
    @icost{ALTIVEC, 2-4}

    @par 256-bit version:
    The lower and higher 128-bit halves are processed as if 128-bit instruction
    was applied to each of them separately.

    @icost{SSE2-SSE4.1, 8}
    @icost{AVX-AVX2, 4}
    @icost{ALTIVEC, 4-6}
    @icost{NEON, 2}
*/
template<unsigned N> SIMDPP_INL
void transpose2(float32<N>& a0, float32<N>& a1)
{
    detail::insn::i_transpose2(a0, a1);
}

/** Transposes a 2x2 64-bit matrix within two int64x2 vectors

    @code
    r0 = [ a0_0; a1_0 ]
    r1 = [ a0_1; a1_1 ]
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 2}
    @novec{NEON, ALTIVEC}

    @par 256-bit version:
    The lower and higher 128-bit halves are processed as if 128-bit instruction
    was applied to each of them separately.

    @icost{SSE2-SSE4.1, 4}
    @icost{AVX-AVX2, 2}
    @novec{NEON, ALTIVEC}
*/
template<unsigned N> SIMDPP_INL
void transpose2(float64<N>& a0, float64<N>& a1)
{
    detail::insn::i_transpose2(a0, a1);
}

/** Transposes four 4x4 8-bit matrix within four int8x16 vectors

    Mask or expression vectors are not supported.

    @code
    r0 = [ a0_0; a1_0; a2_0; a3_0 ; a0_4; a1_4; a2_4; a3_4 ...]
    r1 = [ a0_1; a1_1; a2_1; a3_1 ; a0_5; a1_5; a2_5; a3_5 ...]
    r2 = [ a0_2; a1_2; a2_2; a3_2 ; a0_6; a1_6; a2_6; a3_6 ...]
    r3 = [ a0_3; a1_3; a2_3; a3_3 ; a0_7; a1_7; a2_7; a3_7 ...]
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 16}
    @icost{NEON, 4}
    @icost{ALTIVEC, 8-12}

    @par 256-bit version:
    The lower and higher 128-bit halves are processed as if 128-bit instruction
    was applied to each of them separately.

    @icost{SSE2-AVX, 32}
    @icost{AVX2, 16}
    @icost{NEON, 8}
    @icost{ALTIVEC, 16-20}
*/
template<unsigned N, class V> SIMDPP_INL
void transpose4(any_int8<N,V>& a0, any_int8<N,V>& a1,
                any_int8<N,V>& a2, any_int8<N,V>& a3)
{
    static_assert(!is_mask<V>::value, "Mask vectors are not supported");
    static_assert(is_value_vector<V>::value, "Expression vectors are not supported");
    uint8<N> qa0, qa1, qa2, qa3;
    qa0 = a0.wrapped();  qa1 = a1.wrapped();  qa2 = a2.wrapped();  qa3 = a3.wrapped();
    detail::insn::i_transpose4(qa0, qa1, qa2, qa3);
    a0.wrapped() = qa0;  a1.wrapped() = qa1;  a2.wrapped() = qa2;  a3.wrapped() = qa3;
}

/** Transposes two 4x4 16-bit matrices within four int16x8 vectors

    Mask or expression vectors are not supported.

    @code
    r0 = [ a0_0; a1_0; a2_0; a3_0 ; a0_4; a1_4; a2_4; a3_4 ]
    r1 = [ a0_1; a1_1; a2_1; a3_1 ; a0_5; a1_5; a2_5; a3_5 ]
    r2 = [ a0_2; a1_2; a2_2; a3_2 ; a0_6; a1_6; a2_6; a3_6 ]
    r3 = [ a0_3; a1_3; a2_3; a3_3 ; a0_7; a1_7; a2_7; a3_7 ]
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 12}
    @icost{NEON, 4}
    @icost{ALTIVEC, 8-12}

    @par 256-bit version:
    The lower and higher 128-bit halves are processed as if 128-bit instruction
    was applied to each of them separately.

    @icost{SSE2-AVX, 24}
    @icost{AVX2, 12}
    @icost{NEON, 8}
    @icost{ALTIVEC, 16-20}
*/
template<unsigned N, class V> SIMDPP_INL
void transpose4(any_int16<N,V>& a0, any_int16<N,V>& a1,
                any_int16<N,V>& a2, any_int16<N,V>& a3)
{
    static_assert(!is_mask<V>::value, "Mask vectors are not supported");
    static_assert(is_value_vector<V>::value, "Expression vectors are not supported");
    uint16<N> qa0, qa1, qa2, qa3;
    qa0 = a0.wrapped();  qa1 = a1.wrapped();  qa2 = a2.wrapped();  qa3 = a3.wrapped();
    detail::insn::i_transpose4(qa0, qa1, qa2, qa3);
    a0.wrapped() = qa0;  a1.wrapped() = qa1;  a2.wrapped() = qa2;  a3.wrapped() = qa3;
}

/** Transposes a 4x4 32-bit matrix within four int32x4 vectors

    Mask or expression vectors are not supported.

    @code
    r0 = [ a0_0; a1_0; a2_0; a3_0 ]
    r1 = [ a0_1; a1_1; a2_1; a3_1 ]
    r2 = [ a0_2; a1_2; a2_2; a3_2 ]
    r3 = [ a0_3; a1_3; a2_3; a3_3 ]
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 12}
    @icost{NEON, 4}
    @icost{ALTIVEC, 8-12}

    @par 256-bit version:
    @icost{SSE2-AVX, 24}
    @icost{AVX2, 12}
    @icost{NEON, 8}
    @icost{ALTIVEC, 16-20}

    The lower and higher 128-bit halves are processed as if 128-bit instruction
    was applied to each of them separately.
*/
template<unsigned N, class V> SIMDPP_INL
void transpose4(any_int32<N,V>& a0, any_int32<N,V>& a1,
                any_int32<N,V>& a2, any_int32<N,V>& a3)
{
    static_assert(!is_mask<V>::value, "Mask vectors are not supported");
    static_assert(is_value_vector<V>::value, "Expression vectors are not supported");
    uint32<N> qa0, qa1, qa2, qa3;
    qa0 = a0.wrapped();  qa1 = a1.wrapped();  qa2 = a2.wrapped();  qa3 = a3.wrapped();
    detail::insn::i_transpose4(qa0, qa1, qa2, qa3);
    a0.wrapped() = qa0;  a1.wrapped() = qa1;  a2.wrapped() = qa2;  a3.wrapped() = qa3;
}

/** Transposes 4x4 32-bit matrix within four float32x4 vectors

    @code
    r0 = [ a0_0; a1_0; a2_0; a3_0 ]
    r1 = [ a0_1; a1_1; a2_1; a3_1 ]
    r2 = [ a0_2; a1_2; a2_2; a3_2 ]
    r3 = [ a0_3; a1_3; a2_3; a3_3 ]
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 12}
    @icost{NEON, 4}
    @icost{ALTIVEC, 8-12}

    @par 256-bit version:
    @icost{SSE2-SSE4.1, 24}
    @icost{AVX-AVX2, 12}
    @icost{NEON, 8}
    @icost{ALTIVEC, 16-20}

    The lower and higher 128-bit halves are processed as if 128-bit instruction
    was applied to each of them separately.
*/
template<unsigned N> SIMDPP_INL
void transpose4(float32<N>& a0, float32<N>& a1,
                float32<N>& a2, float32<N>& a3)
{
    detail::insn::i_transpose4(a0, a1, a2, a3);
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

