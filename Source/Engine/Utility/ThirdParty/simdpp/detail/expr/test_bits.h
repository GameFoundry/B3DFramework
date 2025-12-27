/*  Copyright (C) 2016  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_EXPR_TEST_BITS_H
#define LIBSIMDPP_SIMDPP_DETAIL_EXPR_TEST_BITS_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/expr.h>
#include <simdpp/detail/insn/test_bits.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

template<unsigned N, class V> SIMDPP_INL
bool e_test_bits_any(const any_vec<N,V>& a)
{
    typename detail::get_expr_nosign<V>::type ra;
    ra = a.wrapped().eval();
    return insn::i_test_bits_any(ra);
}

#if SIMDPP_USE_SSE4_1
template<unsigned N, template<unsigned, typename> class V,
         class V1, class V2> SIMDPP_INL
bool e_test_bits_any(const any_vec<16, V<N, expr_bit_and<V1,V2>>>& e)
{
    uint8<16> a, b;
    a = e.wrapped().e.a.eval();
    b = e.wrapped().e.b.eval();
    return !_mm_testz_si128(b.native(), a.native());
}

template<unsigned N, template<unsigned, typename> class V,
         class V1, class V2> SIMDPP_INL
bool e_test_bits_any(const any_vec<16, V<N, expr_bit_andnot<V1,V2>>>& e)
{
    uint8<16> a, b;
    a = e.wrapped().e.a.eval();
    b = e.wrapped().e.b.eval();
    return !_mm_testc_si128(b.native(), a.native());
}
#endif

#if SIMDPP_USE_AVX2
template<unsigned N, template<unsigned, typename> class V,
         class V1, class V2> SIMDPP_INL
bool e_test_bits_any(const any_vec<32, V<N, expr_bit_and<V1,V2>>>& e)
{
    uint8<32> a, b;
    a = e.wrapped().e.a.eval();
    b = e.wrapped().e.b.eval();
    return !_mm256_testz_si256(b.native(), a.native());
}

template<unsigned N, template<unsigned, typename> class V,
         class V1, class V2>
bool e_test_bits_any(const any_vec<32, V<N, expr_bit_andnot<V1,V2>>>& e)
{
    uint8<32> a, b;
    a = e.wrapped().e.a.eval();
    b = e.wrapped().e.b.eval();
    return !_mm256_testc_si256(b.native(), a.native());
}
#endif

#if SIMDPP_USE_AVX512F
template<class V1, class V2> SIMDPP_INL
bool e_test_bits_any(const uint32<16, expr_bit_and<V1,V2>>& e)
{
    uint32<16> a, b;
    a = e.wrapped().e.a.eval();
    b = e.wrapped().e.b.eval();
    return _mm512_test_epi64_mask(a.native(), b.native()) != 0;
}

template<class V1, class V2> SIMDPP_INL
bool e_test_bits_any(const uint64<8, expr_bit_and<V1,V2>>& e)
{
    uint64<8> a, b;
    a = e.wrapped().e.a.eval();
    b = e.wrapped().e.b.eval();
    return _mm512_test_epi64_mask(a.native(), b.native()) != 0;
}
#endif


} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
