/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_ZIP_LO_H
#define LIBSIMDPP_SIMDPP_CORE_ZIP_LO_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/zip_lo.h>
#include <simdpp/detail/get_expr.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Interleaves the lower halves of two vectors.

    @code
        | 0  1  2  3  4  5  ... N-2      N-1      |
    r = [ a0 b0 a1 b1 a2 b2 ... a(N/2-1) b(N/2-1) ]
    @endcode

    @par 256-bit version:
    The lower and higher 128-bit halves are processed as if 128-bit instruction
    was applied to each of them separately.

    @icost{SSE2-AVX, NEON, ALTIVEC, 2}
*/
template<unsigned N, class V1, class V2> SIMDPP_INL
typename detail::get_expr2_nomask<V1, V2>::empty
        zip16_lo(const any_vec8<N,V1>& a, const any_vec8<N,V2>& b)
{
    typename detail::get_expr2_nomask_nosign<V1, V2>::type ra, rb;
    ra = a.wrapped().eval();
    rb = b.wrapped().eval();
    return detail::insn::i_zip16_lo(ra, rb);
}

template<unsigned N, class V1, class V2> SIMDPP_INL
typename detail::get_expr2_nomask<V1, V2>::empty
        zip8_lo(const any_vec16<N,V1>& a, const any_vec16<N,V2>& b)
{
    typename detail::get_expr2_nomask_nosign<V1, V2>::type ra, rb;
    ra = a.wrapped().eval();
    rb = b.wrapped().eval();
    return detail::insn::i_zip8_lo(ra, rb);
}

template<unsigned N, class V1, class V2> SIMDPP_INL
typename detail::get_expr2_nomask<V1, V2>::empty
        zip4_lo(const any_vec32<N,V1>& a, const any_vec32<N,V2>& b)
{
    typename detail::get_expr2_nomask_nosign<V1, V2>::type ra, rb;
    ra = a.wrapped().eval();
    rb = b.wrapped().eval();
    return detail::insn::i_zip4_lo(ra, rb);
}

template<unsigned N, class V1, class V2> SIMDPP_INL
typename detail::get_expr2_nomask<V1, V2>::empty
        zip2_lo(const any_vec64<N,V1>& a, const any_vec64<N,V2>& b)
{
    typename detail::get_expr2_nomask_nosign<V1, V2>::type ra, rb;
    ra = a.wrapped().eval();
    rb = b.wrapped().eval();
    return detail::insn::i_zip2_lo(ra, rb);
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
