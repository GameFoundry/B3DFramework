/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_MOVE_R_H
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_MOVE_R_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/shuffle/shuffle_mask.h>
#include <simdpp/detail/vector_array_macros.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace insn {


template<unsigned shift> SIMDPP_INL
uint8x16 i_move16_r(const uint8x16& a)
{
    static_assert(shift <= 16, "Selector out of range");

#if SIMDPP_USE_NULL
    return detail::null::move_n_r<shift>(a);
#elif SIMDPP_USE_SSE2
    return _mm_slli_si128(a.native(), shift);
#elif SIMDPP_USE_NEON
    uint8x16 zero = make_zero();
    return vextq_u8(zero.native(), a.native(), 16-shift);
#elif SIMDPP_USE_ALTIVEC
    // return align<16-shift>((uint8x16) make_zero(), a);
    return vec_sld_biendian<16-shift>((uint8<16>)make_zero(), a);
#elif SIMDPP_USE_MSA
    uint8x16 zero = make_zero();
    return (v16u8) __msa_sldi_b((v16i8)a.native(), (v16i8)zero.native(), 16-shift);
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned shift> SIMDPP_INL
uint8x32 i_move16_r(const uint8x32& a)
{
    static_assert(shift <= 16, "Selector out of range");
    return _mm256_slli_si256(a.native(), shift);
}
#endif

#if SIMDPP_USE_AVX512BW
template<unsigned shift> SIMDPP_INL
uint8<64> i_move16_r(const uint8<64>& a)
{
    static_assert(shift <= 16, "Selector out of range");
    return _mm512_bslli_epi128(a.native(), shift);
}
#endif

template<unsigned shift, unsigned N> SIMDPP_INL
uint8<N> i_move16_r(const uint8<N>& a)
{
    static_assert(shift <= 16, "Selector out of range");
    SIMDPP_VEC_ARRAY_IMPL1(uint8<N>, i_move16_r<shift>, a);
}

// -----------------------------------------------------------------------------

template<unsigned shift> SIMDPP_INL
uint16<8> i_move8_r(const uint16<8>& a)
{
#if SIMDPP_USE_NULL
    return detail::null::move_n_r<shift>(a);
#else
    return (uint16<8>) i_move16_r<shift*2>(uint8<16>(a));
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned shift> SIMDPP_INL
uint16<16> i_move8_r(const uint16<16>& a)
{
    static_assert(shift <= 8, "Selector out of range");
    return _mm256_slli_si256(a.native(), shift*2);
}
#endif

#if SIMDPP_USE_AVX512BW
template<unsigned shift> SIMDPP_INL
uint16<32> i_move8_r(const uint16<32>& a)
{
    static_assert(shift <= 8, "Selector out of range");
    return _mm512_bslli_epi128(a.native(), shift*2);
}
#endif

template<unsigned shift, unsigned N> SIMDPP_INL
uint16<N> i_move8_r(const uint16<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL1(uint16<N>, i_move8_r<shift>, a);
}

// -----------------------------------------------------------------------------

template<unsigned shift> SIMDPP_INL
uint32<4> i_move4_r(const uint32<4>& a)
{
#if SIMDPP_USE_NULL
    return detail::null::move_n_r<shift>(a);
#else
    return (uint32<4>) i_move16_r<shift*4>(uint8<16>(a));
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned shift> SIMDPP_INL
uint32<8> i_move4_r(const uint32<8>& a)
{
    static_assert(shift <= 4, "Selector out of range");
    return _mm256_slli_si256(a.native(), shift*4);
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned shift> SIMDPP_INL
uint32<16> i_move4_r(const uint32<16>& a)
{
    static_assert(shift <= 4, "Selector out of range");
    switch (shift) {
    default:
    case 0: return a;
    case 1: return _mm512_maskz_shuffle_epi32(0xeeee, a.native(),
                                              _MM_PERM_ENUM(_MM_SHUFFLE(2, 1, 0, 0)));
    case 2: return _mm512_maskz_shuffle_epi32(0xcccc, a.native(),
                                              _MM_PERM_ENUM(_MM_SHUFFLE(1, 0, 0, 0)));
    case 3: return _mm512_maskz_shuffle_epi32(0x8888, a.native(),
                                              _MM_PERM_ENUM(_MM_SHUFFLE(0, 0, 0, 0)));
    case 4: return make_zero();
    }
}
#endif

template<unsigned shift, unsigned N> SIMDPP_INL
uint32<N> i_move4_r(const uint32<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL1(uint32<N>, i_move4_r<shift>, a);
}

// -----------------------------------------------------------------------------

template<unsigned shift> SIMDPP_INL
uint64<2> i_move2_r(const uint64<2>& a)
{
#if SIMDPP_USE_NULL || (SIMDPP_USE_ALTIVEC && !SIMDPP_USE_VSX_207)
    return detail::null::move_n_r<shift>(a);
#else
    return (uint64<2>) i_move16_r<shift*8>(uint8<16>(a));
#endif
}

#if SIMDPP_USE_AVX2
template<unsigned shift> SIMDPP_INL
uint64<4> i_move2_r(const uint64<4>& a)
{
    static_assert(shift <= 2, "Selector out of range");
    return _mm256_slli_si256(a.native(), shift*8);
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned shift> SIMDPP_INL
uint64<8> i_move2_r(const uint64<8>& a)
{
    static_assert(shift <= 4, "Selector out of range");
    return (uint64<8>) i_move4_r<shift*2>(uint32<16>(a));
}
#endif

template<unsigned shift, unsigned N> SIMDPP_INL
uint64<N> i_move2_r(const uint64<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL1(uint64<N>, i_move2_r<shift>, a);
}

// -----------------------------------------------------------------------------

template<unsigned shift> SIMDPP_INL
float32<4> i_move4_r(const float32<4>& a)
{
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    return detail::null::move_n_r<shift>(a);
#else
    return (float32<4>) i_move16_r<shift*4>(uint8<16>(a));
#endif
}

#if SIMDPP_USE_AVX
template<unsigned shift> SIMDPP_INL
float32<8> i_move4_r(const float32<8>& a)
{
    static_assert(shift <= 4, "Selector out of range");
    return (float32<8>) i_move16_r<shift*4>(uint8<32>(a));
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned shift> SIMDPP_INL
float32<16> i_move4_r(const float32<16>& a)
{
    static_assert(shift <= 4, "Selector out of range");
    switch (shift) {
    default:
    case 0: return a;
    case 1: return _mm512_maskz_shuffle_ps(0xeeee, a.native(), a.native(),
                                           _MM_SHUFFLE(2, 1, 0, 0));
    case 2: return _mm512_maskz_shuffle_ps(0xcccc, a.native(), a.native(),
                                           _MM_SHUFFLE(1, 0, 0, 0));
    case 3: return _mm512_maskz_shuffle_ps(0x8888, a.native(), a.native(),
                                           _MM_SHUFFLE(0, 0, 0, 0));
    case 4: return make_zero();
    }
}
#endif

template<unsigned shift, unsigned N> SIMDPP_INL
float32<N> i_move4_r(const float32<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL1(float32<N>, i_move4_r<shift>, a);
}

// -----------------------------------------------------------------------------

template<unsigned shift> SIMDPP_INL
float64<2> i_move2_r(const float64<2>& a)
{
#if SIMDPP_USE_SSE2 || SIMDPP_USE_NEON64 || SIMDPP_USE_VSX_206 || SIMDPP_USE_MSA
    return (float64<2>) i_move16_r<shift*8>(uint8<16>(a));
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC
    return detail::null::move_n_r<shift>(a);
#endif
}

#if SIMDPP_USE_AVX
template<unsigned shift> SIMDPP_INL
float64<4> i_move2_r(const float64<4>& a)
{
    static_assert(shift <= 2, "Selector out of range");
    return (float64<4>) i_move16_r<shift*8>(uint8<32>(a));
}
#endif

#if SIMDPP_USE_AVX512F
template<unsigned shift> SIMDPP_INL
float64<8> i_move2_r(const float64<8>& a)
{
    static_assert(shift <= 2, "Selector out of range");
    switch (shift) {
    default:
    case 0: return a;
    case 1: return _mm512_maskz_shuffle_pd(0xaa, a.native(), a.native(), SIMDPP_SHUFFLE_MASK_2x2_4(0, 0));
    case 2: return make_zero();
    }
}
#endif

template<unsigned shift, unsigned N> SIMDPP_INL
float64<N> i_move2_r(const float64<N>& a)
{
    SIMDPP_VEC_ARRAY_IMPL1(float64<N>, i_move2_r<shift>, a);
}

// -----------------------------------------------------------------------------
// Certain compilers don't like zero or full vector width moves. The templates
// below offer a warkaround

template<unsigned count>
struct i_move2_r_wrapper {
    template<class V>
    static SIMDPP_INL V run(const V& arg) { return i_move2_r<count>(arg); }
};
template<>
struct i_move2_r_wrapper<0> {
    template<class V>
    static SIMDPP_INL V run(const V& arg) { return arg; }
};
template<>
struct i_move2_r_wrapper<2> {
    template<class V>
    static SIMDPP_INL V run(const V&) { return (V) make_zero(); }
};

template<unsigned count>
struct i_move4_r_wrapper {
    template<class V>
    static SIMDPP_INL V run(const V& arg) { return i_move4_r<count>(arg); }
};
template<>
struct i_move4_r_wrapper<0> {
    template<class V>
    static SIMDPP_INL V run(const V& arg) { return arg; }
};
template<>
struct i_move4_r_wrapper<4> {
    template<class V>
    static SIMDPP_INL V run(const V&) { return (V) make_zero(); }
};

template<unsigned count>
struct i_move8_r_wrapper {
    template<class V>
    static SIMDPP_INL V run(const V& arg) { return i_move8_r<count>(arg); }
};
template<>
struct i_move8_r_wrapper<0> {
    template<class V>
    static SIMDPP_INL V run(const V& arg) { return arg; }
};
template<>
struct i_move8_r_wrapper<8> {
    template<class V>
    static SIMDPP_INL V run(const V&) { return (V) make_zero(); }
};

template<unsigned count>
struct i_move16_r_wrapper {
    template<class V>
    static SIMDPP_INL V run(const V& arg) { return i_move16_r<count>(arg); }
};
template<>
struct i_move16_r_wrapper<0> {
    template<class V>
    static SIMDPP_INL V run(const V& arg) { return arg; }
};
template<>
struct i_move16_r_wrapper<16> {
    template<class V>
    static SIMDPP_INL V run(const V&) { return (V) make_zero(); }
};

} // namespace insn
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

