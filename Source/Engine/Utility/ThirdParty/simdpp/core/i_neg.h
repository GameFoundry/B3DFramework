/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_I_NEG_H
#define LIBSIMDPP_SIMDPP_CORE_I_NEG_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/expr/i_neg.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Negates signed 8-bit values.

    @code
    r0 = -a0
    ...
    rN = -aN
    @endcode

    @par 256-bit version:
    @icost{SSE2-AVX, NEON, ALTIVEC, 2}
*/
template<unsigned N, class E> SIMDPP_INL
int8<N, expr_ineg<int8<N,E>>> neg(const int8<N,E>& a)
{
    return { { a } };
}

/** Negates signed 16-bit values.

    @code
    r0 = -a0
    ...
    rN = -aN
    @endcode

    @par 256-bit version:
    @icost{SSE2-AVX, NEON, ALTIVEC, 2}
*/
template<unsigned N, class E> SIMDPP_INL
int16<N, expr_ineg<int16<N,E>>> neg(const int16<N,E>& a)
{
    return { { a } };
}

/** Negates signed 32-bit values.

    @code
    r0 = -a0
    ...
    rN = -aN
    @endcode

    @par 256-bit version:
    @icost{SSE2-AVX, NEON, ALTIVEC, 2}
*/
template<unsigned N, class E> SIMDPP_INL
int32<N, expr_ineg<int32<N,E>>> neg(const int32<N,E>& a)
{
    return { { a } };
}

/** Negates signed 64-bit values.

    @code
    r0 = -a0
    ...
    rN = -aN
    @endcode

    @par 128-bit version:
    @icost{ALTIVEC, 4-5}

    @par 256-bit version:
    @icost{SSE2-AVX, NEON, 2}
    @icost{ALTIVEC, 8-9}
*/
template<unsigned N, class E> SIMDPP_INL
int64<N, expr_ineg<int64<N,E>>> neg(const int64<N,E>& a)
{
    return { { a } };
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

