/*  Copyright (C) 2016  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_TEST_BITS_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_TEST_BITS_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/expr.h>
#include <simdpp/core/bit_and.h>
#include <simdpp/core/extract.h>
#include <simdpp/core/move_l.h>
#include <simdpp/detail/insn/test_bits.h>
#include <simdpp/detail/null/bitwise.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

static SIMDPP_INL
bool i_test_bits_any(const uint32<4>& a)
{
#if SIMDPP_USE_NULL
    return null::test_bits_any(a);
#elif SIMDPP_USE_SSE4_1
    return !_mm_testz_si128(a.native(), a.native());
#elif SIMDPP_USE_SSE2
    uint32<4> r = _mm_cmpeq_epi32(a.native(), _mm_setzero_si128());
    return _mm_movemask_epi8(r.native()) != 0xffff;
#elif SIMDPP_USE_NEON
#if SIMDPP_64_BITS
    uint64<2> r;
    r = a;
    r = bit_or(r, move2_l<1>(r));
    return extract<0>(r) != 0;
#else
    uint32x4 r = bit_or(a, move4_l<2>(a));
    r = bit_or(r, move4_l<1>(r));
    return extract<0>(r) != 0;
#endif
#elif SIMDPP_USE_ALTIVEC
    uint32<4> z = make_uint(0);
    return vec_any_gt(a.native(), z.native());
#elif SIMDPP_USE_MSA
    return __msa_test_bnz_v((v16u8) a.native());
#endif
}

static SIMDPP_INL
bool i_test_bits_any(const uint16<8>& a) { return i_test_bits_any(uint32<4>(a)); }
static SIMDPP_INL
bool i_test_bits_any(const uint8<16>& a) { return i_test_bits_any(uint32<4>(a)); }

static SIMDPP_INL
bool i_test_bits_any(const uint64<2>& a)
{
#if SIMDPP_USE_VSX_207
    uint64<2> z = make_zero();
    return vec_any_gt(a.native(), z.native());
#elif SIMDPP_USE_NULL || SIMDPP_USE_ALTIVEC
    return null::test_bits_any(a);
#else
    return i_test_bits_any(uint32<4>(a));
#endif
}

static SIMDPP_INL
bool i_test_bits_any(const float32<4>& a)
{
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    return null::test_bits_any(a);
#else
    return i_test_bits_any(uint32<4>(a));
#endif
}

static SIMDPP_INL
bool i_test_bits_any(const float64<2>& a)
{
#if SIMDPP_USE_VSX_206
    return i_test_bits_any(uint32<4>(a));
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON32 || SIMDPP_USE_ALTIVEC
    return null::test_bits_any(a);
#else
    return i_test_bits_any(uint32<4>(a));
#endif
}

#if SIMDPP_USE_AVX
static SIMDPP_INL
bool i_test_bits_any(const float32<8>& a)
{
    return !_mm256_testz_si256(_mm256_castps_si256(a.native()),
                               _mm256_castps_si256(a.native()));
}
static SIMDPP_INL
bool i_test_bits_any(const float64<4>& a)
{
    return !_mm256_testz_si256(_mm256_castpd_si256(a.native()),
                               _mm256_castpd_si256(a.native()));
}
#endif

#if SIMDPP_USE_AVX2
static SIMDPP_INL
bool i_test_bits_any(const uint8<32>& a) { return !_mm256_testz_si256(a.native(), a.native()); }
static SIMDPP_INL
bool i_test_bits_any(const uint16<16>& a) { return !_mm256_testz_si256(a.native(), a.native()); }
static SIMDPP_INL
bool i_test_bits_any(const uint32<8>& a) { return !_mm256_testz_si256(a.native(), a.native()); }
static SIMDPP_INL
bool i_test_bits_any(const uint64<4>& a) { return !_mm256_testz_si256(a.native(), a.native()); }
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
bool i_test_bits_any(const uint32<16>& a)
{
    return _mm512_test_epi64_mask(a.native(), a.native()) != 0;
}
static SIMDPP_INL
bool i_test_bits_any(const uint64<8>& a) { return i_test_bits_any(uint32<16>(a)); }
static SIMDPP_INL
bool i_test_bits_any(const float32<16>& a) { return i_test_bits_any(uint32<16>(a)); }
static SIMDPP_INL
bool i_test_bits_any(const float64<8>& a) { return i_test_bits_any(uint32<16>(a)); }
#endif

#if SIMDPP_USE_AVX512BW
SIMDPP_INL bool i_test_bits_any(const uint8<64>& a) { return i_test_bits_any(uint32<16>(a)); }
SIMDPP_INL bool i_test_bits_any(const uint16<32>& a) { return i_test_bits_any(uint32<16>(a)); }
#endif

template<unsigned N, class V>
SIMDPP_INL bool i_test_bits_any(const any_vec<N,V>& a)
{
    const V& wa = a.wrapped();
    typename V::base_vector_type r = wa.vec(0);
    for (unsigned i = 1; i < wa.vec_length; ++i)
        r = bit_or(r, wa.vec(i));
    return i_test_bits_any(r);
}

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
