/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_I_AVG_TRUNC_H
#define LIBSIMDPP_SIMDPP_CORE_I_AVG_TRUNC_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/i_avg_trunc.h>
#include <simdpp/core/detail/scalar_arg_impl.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Computes truncated average of the unsigned 8-bit values.

    @code
    r0 = (a0 + b0) / 2
    ...
    rN = (aN + bN) / 2
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 4}
    @icost{NEON, 1}

    @par 256-bit version:
    @icost{SSE2-AVX, 8}
    @icost{AVX2, 4}
    @icost{NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
uint8<N,expr_empty> avg_trunc(const uint8<N,E1>& a, const uint8<N,E2>& b)
{
    return detail::insn::i_avg_trunc(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(avg_trunc, uint8, uint8)

/** Computes truncated average of signed 8-bit values.

    @code
    r0 = (a0 + b0) / 2
    ...
    rN = (aN + bN) / 2
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 7-8}

    @par 256-bit version:
    @icost{SSE2-AVX, 14-15}
    @icost{AVX2, 7-8}
    @icost{NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
int8<N,expr_empty> avg_trunc(const int8<N,E1>& a, const int8<N,E2>& b)
{
    return detail::insn::i_avg_trunc(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(avg_trunc, int8, int8)

/** Computes truncated average of unsigned 16-bit values.

    @code
    r0 = (a0 + b0) / 2
    ...
    rN = (aN + bN) / 2
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 4}
    @icost{NEON, 1}

    @par 256-bit version:
    @icost{SSE2-AVX, 8}
    @icost{AVX2, 4}
    @icost{NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
uint16<N,expr_empty> avg_trunc(const uint16<N,E1>& a, const uint16<N,E2>& b)
{
    return detail::insn::i_avg_trunc(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(avg_trunc, uint16, uint16)

/** Computes truncated average of signed 16-bit values.

    @code
    r0 = (a0 + b0) / 2
    ...
    rN = (aN + bN) / 2
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 7-8}
    @icost{NEON, 1}

    @par 256-bit version:
    @icost{SSE2-AVX, 14-15}
    @icost{AVX2, 7-8}
    @icost{NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
int16<N,expr_empty> avg_trunc(const int16<N,E1>& a, const int16<N,E2>& b)
{
    return detail::insn::i_avg_trunc(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(avg_trunc, int16, int16)

/** Computes truncated average of unsigned 32-bit values.

    @code
    r0 = (a0 + b0) / 2
    ...
    rN = (aN + bN) / 2
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 4}
    @icost{NEON, 1}

    @par 256-bit version:
    @icost{SSE2-AVX, 8}
    @icost{AVX2, 4}
    @icost{NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
uint32<N,expr_empty> avg_trunc(const uint32<N,E1>& a, const uint32<N,E2>& b)
{
    return detail::insn::i_avg_trunc(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(avg_trunc, uint32, uint32)

/** Computes truncated average of signed 32-bit values.

    @code
    r0 = (a0 + b0) / 2
    ...
    rN = (aN + bN) / 2
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 7-8}
    @icost{ALTIVEC, 4}

    @par 256-bit version:
    @icost{SSE2-AVX, 14-15}
    @icost{AVX2, 7-8}
    @icost{ALTIVEC, 8}
    @icost{NEON, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
int32<N,expr_empty> avg_trunc(const int32<N,E1>& a, const int32<N,E2>& b)
{
    return detail::insn::i_avg_trunc(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(avg_trunc, int32, int32)


} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

