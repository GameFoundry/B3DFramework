/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_I_AVG_H
#define LIBSIMDPP_SIMDPP_CORE_I_AVG_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/i_avg.h>
#include <simdpp/core/detail/scalar_arg_impl.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {


/** Computes rounded average of the unsigned 8-bit values.

    @code
    r0 = (a0 + b0 + 1) / 2
    ...
    rN = (aN + bN + 1) / 2
    @endcode

    @par 256-bit version:
    @icost{SSE2-AVX, NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
uint8<N,expr_empty> avg(const uint8<N,E1>& a, const uint8<N,E2>& b)
{
    return detail::insn::i_avg(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(avg, uint8, uint8)

/** Computes rounded average of signed 8-bit values.

    @code
    r0 = (a0 + b0 + 1) / 2
    ...
    rN = (aN + bN + 1) / 2
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 4-5}

    @par 256-bit version:
    @icost{SSE2-AVX, 8-9}
    @icost{AVX2, 4-5}
    @icost{NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
int8<N,expr_empty> avg(const int8<N,E1>& a, const int8<N,E2>& b)
{
    return detail::insn::i_avg(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(avg, int8, int8)

/** Computes rounded average of unsigned 16-bit values.

    @code
    r0 = (a0 + b0 + 1) / 2
    ...
    rN = (aN + bN + 1) / 2
    @endcode

    @par 256-bit version:
    @icost{SSE2-AVX, NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
uint16<N,expr_empty> avg(const uint16<N,E1>& a, const uint16<N,E2>& b)
{
    return detail::insn::i_avg(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(avg, uint16, uint16)

/** Computes rounded average of signed 16-bit values.

    @code
    r0 = (a0 + b0 + 1) / 2
    ...
    rN = (aN + bN + 1) / 2
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 4-5}

    @par 256-bit version:
    @icost{SSE2-AVX, 8-9}
    @icost{AVX2, 4-5}
    @icost{NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
int16<N,expr_empty> avg(const int16<N,E1>& a, const int16<N,E2>& b)
{
    return detail::insn::i_avg(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(avg, int16, int16)

/** Computes rounded average of unsigned 32-bit values.

    @code
    r0 = (a0 + b0 + 1) / 2
    ...
    rN = (aN + bN + 1) / 2
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 6-7}

    @par 256-bit version:
    @icost{SSE2-AVX, 12-13}
    @icost{AVX2, 6-7}
    @icost{NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
uint32<N,expr_empty> avg(const uint32<N,E1>& a, const uint32<N,E2>& b)
{
    return detail::insn::i_avg(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(avg, uint32, uint32)

/** Computes rounded average of signed 32-bit values.

    @code
    r0 = (a0 + b0 + 1) / 2
    ...
    rN = (aN + bN + 1) / 2
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 9-10}
    @icost{NEON, 1}

    @par 256-bit version:
    @icost{SSE2-AVX, 18-19}
    @icost{AVX2, 9-10}
    @icost{NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
int32<N,expr_empty> avg(const int32<N,E1>& a, const int32<N,E2>& b)
{
    return detail::insn::i_avg(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(avg, int32, int32)

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

