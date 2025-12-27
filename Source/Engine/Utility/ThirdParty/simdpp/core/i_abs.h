/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_I_ABS_H
#define LIBSIMDPP_SIMDPP_CORE_I_ABS_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/expr/i_abs.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Computes absolute value of 8-bit integer values.

    @code
    r0 = abs(a0)
    ...
    rN = abs(aN)
    @endcode

    @par 128-bit version:
    @icost{SSE2-SSE3, 3}
    @icost{ALTIVEC, 1-3}

    @par 256-bit version:
    @icost{SSE2-SSE3, 6}
    @icost{SSSE3-AVX, NEON, 2}
    @icost{ALTIVEC, 2-4}
*/
template<unsigned N, class E> SIMDPP_INL
uint8<N, expr_iabs<int8<N,E>>> abs(const int8<N,E>& a)
{
    return { { a } };
}


/** Computes absolute value of 16-bit integer values.

    @code
    r0 = abs(a0)
    ...
    rN = abs(aN)
    @endcode
    @par 128-bit version:
    @icost{SSE2-SSE3, 3}
    @icost{ALTIVEC, 1-3}

    @par 256-bit version:
    @icost{SSE2-SSE3, 6}
    @icost{SSSE3-AVX, NEON, 2}
    @icost{ALTIVEC, 2-5}
*/
template<unsigned N, class E> SIMDPP_INL
uint16<N, expr_iabs<int16<N,E>>> abs(const int16<N,E>& a)
{
    return { { a } };
}

/** Computes absolute value of 32-bit integer values.

    @code
    r0 = abs(a0)
    ...
    rN = abs(aN)
    @endcode
    @par 128-bit version:
    @icost{SSE2-SSE3, 3}
    @icost{ALTIVEC, 1-3}

    @par 256-bit version:
    @icost{SSE2-SSE3, 6}
    @icost{SSSE3-AVX, NEON, 2}
    @icost{ALTIVEC, 2-4}
*/
template<unsigned N, class E> SIMDPP_INL
uint32<N, expr_iabs<int32<N,E>>> abs(const int32<N,E>& a)
{
    return { { a } };
}

/** Computes absolute value of 64-bit integer values.

    @code
    r0 = abs(a0)
    ...
    rN = abs(aN)
    @endcode
    @par 128-bit version:
    @icost{SSE2-AVX, 5}
    @icost{NEON, 6}
    @novec{ALTIVEC}

    @par 256-bit version:
    @icost{SSE2-AVX, 10}
    @icost{NEON, 12}
    @icost{AVX2, 4}
    @novec{ALTIVEC}
*/
template<unsigned N, class E> SIMDPP_INL
uint64<N, expr_iabs<int64<N,E>>> abs(const int64<N,E>& a)
{
    return { { a } };
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

