/*  Copyright (C) 2011-2012  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_DETAIL_SHUFFLE_NEON_INT64x2_H
#define LIBSIMDPP_DETAIL_SHUFFLE_NEON_INT64x2_H
#if SIMDPP_USE_NEON

#include <type_traits>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace neon_shuffle_int64x2 {

#if SIMDPP_USE_NEON32

/*
    The code below implements generalized permutations of elements within
    int64x2 vectors using half-vector move instructions available on NEON.
*/
using T = uint64x2;    // full vector
using H = uint64x1_t;       // half vector


/// Returns the lower/higher part of a vector. Cost: 0
SIMDPP_INL H lo(T a)   { return vget_low_u64(a.native()); }
SIMDPP_INL H hi(T a)   { return vget_high_u64(a.native()); }

/// Combines two half vectors. Cost: 0
SIMDPP_INL T co(H lo, H hi) { return vcombine_u64(lo, hi); }

// 2-element permutation
template<unsigned s0, unsigned s1> SIMDPP_INL
T permute2(T a)
{
    const unsigned sel = s0*2 + s1;
    switch (sel) {
    default:
    case 0:  /*00*/ return co(lo(a), lo(a));
    case 1:  /*01*/ return a;
    case 2:  /*10*/ return co(hi(a), lo(a));
    case 3:  /*11*/ return co(hi(a), hi(a));
    }
}

// 2-element shuffle: the first element must come from a, the second - from b
template<unsigned s0, unsigned s1> SIMDPP_INL
T shuffle1(T a, T b)
{
    const unsigned sel = s0*2 + s1;
    switch (sel) {
    default:
    case 0:  /*00*/ return co(lo(a), lo(b));
    case 1:  /*01*/ return co(lo(a), hi(b));
    case 2:  /*10*/ return co(hi(a), lo(b));
    case 3:  /*11*/ return co(hi(a), hi(b));
    }
}

template<unsigned s0, unsigned s1> SIMDPP_INL
T shuffle2x2(const T& a, const T& b)
{
    const unsigned sel = s0*4 + s1;
    switch (sel) {
    default:
    case 0:  /*00*/ return co(lo(a), lo(a));
    case 1:  /*01*/ return a;
    case 2:  /*02*/ return co(lo(a), lo(b));
    case 3:  /*03*/ return co(lo(a), hi(b));
    case 4:  /*10*/ return co(hi(a), lo(a));
    case 5:  /*11*/ return co(hi(a), hi(a));
    case 6:  /*12*/ return co(hi(a), lo(b));
    case 7:  /*13*/ return co(hi(a), hi(b));
    case 8:  /*20*/ return co(lo(b), lo(a));
    case 9:  /*21*/ return co(lo(b), hi(a));
    case 10: /*22*/ return co(lo(b), lo(b));
    case 11: /*23*/ return b;
    case 12: /*30*/ return co(hi(b), lo(a));
    case 13: /*31*/ return co(hi(b), hi(a));
    case 14: /*32*/ return co(hi(b), lo(b));
    case 15: /*33*/ return co(hi(b), hi(b));
    }
}

#else // SIMDPP_USE_NEON64

using T = uint64x2;    // full vector

// Moves the high half of b onto high half of a
SIMDPP_INL T move_hi(const T& a, const T& b)
{
    T mask = make_uint(0xffffffffffffffff, 0x0);
    return vbslq_u64(mask.native(), a.native(), b.native());
}

// 2-element permutation
template<unsigned s0, unsigned s1> SIMDPP_INL
T permute2(const T& a)
{
    const unsigned sel = s0*2 + s1;
    switch (sel) {
    default:
    case 0:  /*00*/ return vzip1q_u64(a.native(), a.native());
    case 1:  /*01*/ return a;
    case 2:  /*10*/ return vextq_u64(a.native(), a.native(), 1);
    case 3:  /*11*/ return vzip2q_u64(a.native(), a.native());
    }
}

// 2-element shuffle: the first element must come from a, the second - from b
template<unsigned s0, unsigned s1> SIMDPP_INL
T shuffle1(const T& a, const T& b)
{
    const unsigned sel = s0*2 + s1;
    switch (sel) {
    default:
    case 0:  /*00*/ return vzip1q_u64(a.native(), b.native());
    case 1:  /*01*/ return move_hi(a, b);
    case 2:  /*10*/ return vextq_u64(a.native(), b.native(), 1);
    case 3:  /*11*/ return vzip2q_u64(a.native(), b.native());
    }
}

template<unsigned s0, unsigned s1> SIMDPP_INL
T shuffle2x2(const T& a, const T& b)
{
    const unsigned sel = s0*4 + s1;
    switch (sel) {
    default:
    case 0:  /*00*/ return vzip1q_u64(a.native(), a.native());
    case 1:  /*01*/ return a;
    case 2:  /*02*/ return vzip1q_u64(a.native(), b.native());
    case 3:  /*03*/ return move_hi(a, b);
    case 4:  /*10*/ return vextq_u64(a.native(), a.native(), 1);
    case 5:  /*11*/ return vzip2q_u64(a.native(), a.native());
    case 6:  /*12*/ return vextq_u64(a.native(), b.native(), 1);
    case 7:  /*13*/ return vzip2q_u64(a.native(), b.native());
    case 8:  /*20*/ return vzip1q_u64(b.native(), a.native());
    case 9:  /*21*/ return move_hi(b, a);
    case 10: /*22*/ return vzip1q_u64(b.native(), b.native());
    case 11: /*23*/ return b;
    case 12: /*30*/ return vextq_u64(b.native(), a.native(), 1);
    case 13: /*31*/ return vzip2q_u64(b.native(), a.native());
    case 14: /*32*/ return vextq_u64(b.native(), b.native(), 1);
    case 15: /*33*/ return vzip2q_u64(b.native(), b.native());
    }
}

#endif

} // namespace neon_shuffle_int64x2
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
#endif

