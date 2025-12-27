/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_UNZIP_HI_H
#define LIBSIMDPP_SIMDPP_CORE_UNZIP_HI_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/unzip_hi.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** De-interleaves the even(higher) elements of two vectors

    For example, in case of int8x16:

    @code
        | 0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  |
    r = [ a1  a3  a5  a7  a9  a11 a13 a15 b1  b3  b5  b7  b9  b11 b13 b15 ]
    @endcode

    @par int8

    @par 128-bit version:
    @icost{SSE2-AVX2, 3}

    @par 256-bit version:
    The lower and higher 128-bit halves are processed as if 128-bit instruction
    was applied to each of them separately.

    @icost{SSE2-AVX, 6}
    @icost{NEON, ALTIVEC, 2}
    @icost{AVX2, 3}

    @par int16

    @par 128-bit version:
    @icost{SSE2-AVX2, 3}

    @par 256-bit version:
    The lower and higher 128-bit halves are processed as if 128-bit instruction
    was applied to each of them separately.

    @icost{SSE2-AVX, 6}
    @icost{NEON, ALTIVEC, 2}
    @icost{AVX2, 3}

    @par int32

    @par 128-bit version:
    @icost{ALTIVEC, 1-2}

    @par 256-bit version:
    The lower and higher 128-bit halves are processed as if 128-bit instruction
    was applied to each of them separately.

    @icost{ALTIVEC, 2-3}
    @icost{SSE2-AVX, NEON, 2}

    @par int64

    @par 128-bit version:
    @icost{ALTIVEC, 1-2}

    @par 256-bit version:
    The lower and higher 128-bit halves are processed as if 128-bit instruction
    was applied to each of them separately.

    @icost{ALTIVEC, 2-3}
    @icost{SSE2-AVX, NEON, 2}

    @par float32
    @par 256-bit version:
    The lower and higher 128-bit halves are processed as if 128-bit instruction
    was applied to each of them separately.

    @icost{SSE2-SSE4.1, NEON, ALTIVEC, 2}

    @par float64
    @par 128-bit version:
    @novec{NEON, ALTIVEC}

    @par 256-bit version:
    The lower and higher 128-bit halves are processed as if 128-bit instruction
    was applied to each of them separately.

    @novec{NEON, ALTIVEC}
    @icost{SSE2-AVX, 2}
*/
template<unsigned N, class V1, class V2> SIMDPP_INL
typename detail::get_expr2_nomask<V1, V2>::empty
        unzip16_hi(const any_vec8<N,V1>& a, const any_vec8<N,V2>& b)
{
    typename detail::get_expr2_nomask_nosign<V1, V2>::type ra, rb;
    ra = a.wrapped().eval();
    rb = b.wrapped().eval();
    return detail::insn::i_unzip16_hi(ra, rb);
}

template<unsigned N, class V1, class V2> SIMDPP_INL
typename detail::get_expr2_nomask<V1, V2>::empty
        unzip8_hi(const any_vec16<N,V1>& a, const any_vec16<N,V2>& b)
{
    typename detail::get_expr2_nomask_nosign<V1, V2>::type ra, rb;
    ra = a.wrapped().eval();
    rb = b.wrapped().eval();
    return detail::insn::i_unzip8_hi(ra, rb);
}

template<unsigned N, class V1, class V2> SIMDPP_INL
typename detail::get_expr2_nomask<V1, V2>::empty
        unzip4_hi(const any_vec32<N,V1>& a, const any_vec32<N,V2>& b)
{
    typename detail::get_expr2_nomask_nosign<V1, V2>::type ra, rb;
    ra = a.wrapped().eval();
    rb = b.wrapped().eval();
    return detail::insn::i_unzip4_hi(ra, rb);
}

template<unsigned N, class V1, class V2> SIMDPP_INL
typename detail::get_expr2_nomask<V1, V2>::empty
        unzip2_hi(const any_vec64<N,V1>& a, const any_vec64<N,V2>& b)
{
    typename detail::get_expr2_nomask_nosign<V1, V2>::type ra, rb;
    ra = a.wrapped().eval();
    rb = b.wrapped().eval();
    return detail::insn::i_unzip2_hi(ra, rb);
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

