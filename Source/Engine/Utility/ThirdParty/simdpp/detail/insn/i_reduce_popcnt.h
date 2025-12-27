/*  Copyright (C) 2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_I_REDUCE_POPCNT_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_I_REDUCE_POPCNT_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/null/bitwise.h>
#include <simdpp/core/i_popcnt.h>
#include <simdpp/core/i_reduce_add.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {

static SIMDPP_INL
uint32_t i_reduce_popcnt(const uint32<4>& a)
{
#if SIMDPP_USE_NULL
    uint32_t r = 0;
    for (unsigned i = 0; i < a.length; i++) {
        r += detail::null::el_popcnt32(a.el(i));
    }
    return r;
#elif SIMDPP_USE_X86_POPCNT_INSN
    uint32_t r = 0;
#if SIMDPP_64_BITS
    uint64<2> a64; a64 = a;
    r += _mm_popcnt_u64(extract<0>(a64));
    r += _mm_popcnt_u64(extract<1>(a64));
#else
    r += _mm_popcnt_u32(extract<0>(a));
    r += _mm_popcnt_u32(extract<1>(a));
    r += _mm_popcnt_u32(extract<2>(a));
    r += _mm_popcnt_u32(extract<3>(a));
#endif
    return r;
#elif SIMDPP_USE_NEON
    uint8<16> r = vcntq_u8(vreinterpretq_u8_u32(a.native()));
    return reduce_add(r);
#elif SIMDPP_USE_VSX_207 || SIMDPP_USE_MSA
    uint64<2> a64; a64 = a;
    a64 = popcnt(a64);
    return reduce_add(a64);
#elif SIMDPP_USE_SSE2
    uint64<2> r = popcnt((uint64<2>)a);
    return (uint32_t) reduce_add(r);
#else
    uint32<4> r = popcnt(a);
    return reduce_add(r);
#endif
}

#if SIMDPP_USE_AVX2
static SIMDPP_INL
uint32_t i_reduce_popcnt(const uint32<8>& a)
{
#if SIMDPP_USE_X86_POPCNT_INSN && SIMDPP_64_BITS
    uint32<4> a0, a1;
    split(a, a0, a1);
    return i_reduce_popcnt(a0) + i_reduce_popcnt(a1);
#else
    uint64<4> r = popcnt((uint64<4>)a);
    return (uint32_t) reduce_add(r);
#endif
}
#endif

#if SIMDPP_USE_AVX512F
static SIMDPP_INL
uint32_t i_reduce_popcnt(const uint32<16>& a)
{
#if SIMDPP_USE_X86_POPCNT_INSN && SIMDPP_64_BITS
    uint32<8> a0, a1;
    split(a, a0, a1);
    return i_reduce_popcnt(a0) + i_reduce_popcnt(a1);
#else
    // TODO: support AVX512VPOPCNTDQ
    uint64<8> r = popcnt((uint64<8>)a);
    return reduce_add(r);
#endif
}
#endif

template<unsigned N> SIMDPP_INL
uint32_t i_reduce_popcnt(const uint32<N>& a)
{
    uint32_t r = 0;
    for (unsigned j = 0; j < a.vec_length; ++j) {
        r += i_reduce_popcnt(a.vec(j));
    }
    return r;
}

// -----------------------------------------------------------------------------

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

