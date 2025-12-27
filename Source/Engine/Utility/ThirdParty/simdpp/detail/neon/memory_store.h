/*  Copyright (C) 2012  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_NEON_MEMORY_STORE_H
#define LIBSIMDPP_NEON_MEMORY_STORE_H
#if SIMDPP_USE_NEON

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace neon {

/** Stores @a n elements of a 128-bit vector starting from the @a pos-th one.
    @a p must be aligned to 16 bytes.

    @a n must be a power of 2. @a pos must be a multiple of @a n. The behavior is
    undefined if @a pos+n exceeds the number of elements in the specified vector.
*/
template<unsigned pos, unsigned n> SIMDPP_INL
void store_lane(char* p, const uint8x16& a)
{
    static_assert(n == 1 || n == 2 || n == 4 || n == 8, "Size not supported");
    static_assert(pos%n == 0, "pos must be a multiple of n");
    static_assert(pos+n <= 16, "Index out of bounds");
    switch (n) {
    case 1:
        vst1q_lane_u8(reinterpret_cast<uint8_t*>(p), a.native(), pos);
        return;
    case 2:
        vst1q_lane_u16(reinterpret_cast<uint16_t*>(p), vreinterpretq_u16_u8(a.native()), pos/2);
        return;
    case 4:
        vst1q_lane_u32(reinterpret_cast<uint32_t*>(p), vreinterpretq_u32_u8(a.native()), pos/4);
        return;
    case 8:
        vst1q_lane_u64(reinterpret_cast<uint64_t*>(p), vreinterpretq_u64_u8(a.native()), pos/8);
        return;
    }
}

template<unsigned pos, unsigned n> SIMDPP_INL
void store_lane(char* p, const uint16x8& a)
{
    static_assert(n == 1 || n == 2 || n == 4, "Size not supported");
    static_assert(pos%n == 0, "pos must be a multiple of n");
    static_assert(pos+n <= 8, "Index out of bounds");
    switch (n) {
    case 1:
        vst1q_lane_u16(reinterpret_cast<uint16_t*>(p), a.native(), pos);
        return;
    case 2:
        vst1q_lane_u32(reinterpret_cast<uint32_t*>(p),
                       vreinterpretq_u32_u16(a.native()), pos/2);
        return;
    case 4:
        vst1q_lane_u64(reinterpret_cast<uint64_t*>(p),
                       vreinterpretq_u64_u16(a.native()), pos/4);
        return;
    }
}

template<unsigned pos, unsigned n> SIMDPP_INL
void store_lane(char* p, const uint32x4& a)
{
    static_assert(n == 1 || n == 2, "Size not supported");
    static_assert(pos%n == 0, "pos must be a multiple of n");
    static_assert(pos+n <= 4, "Index out of bounds");
    switch (n) {
    case 1:
        vst1q_lane_u32(reinterpret_cast<uint32_t*>(p), a.native(), pos);
        return;
    case 2:
        vst1q_lane_u64(reinterpret_cast<uint64_t*>(p),
                       vreinterpretq_u64_u32(a.native()), pos/2);
        return;
    }
}

template<unsigned pos, unsigned n> SIMDPP_INL
void store_lane(char* p, const uint64x2& a)
{
    static_assert(n == 1, "Size not supported");
    static_assert(pos%n == 0, "pos must be a multiple of n");
    static_assert(pos+n <= 2, "Index out of bounds");
    vst1q_lane_u64(reinterpret_cast<uint64_t*>(p), a.native(), pos);
}

template<unsigned pos, unsigned n> SIMDPP_INL
void store_lane(char* p, const float32x4& a)
{
    store_lane<pos,n>(p, uint32x4(a));
}

} // namespace neon
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
#endif
