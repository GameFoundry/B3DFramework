/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_I_MAX_H
#define LIBSIMDPP_SIMDPP_CORE_I_MAX_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/i_max.h>
#include <simdpp/core/detail/scalar_arg_impl.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {


/** Computes maximum of the signed 8-bit values.

    @code
    r0 = max(a0, b0)
    ...
    rN = max(aN, bN)
    @endcode

    @par 128-bit version:
    @icost{SSE2-SSSE3, 4}

    @par 256-bit version:
    @icost{SSE2-SSSE3, 8}
    @icost{SSE4.1-AVX, NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
int8<N,expr_empty> max(const int8<N,E1>& a, const int8<N,E2>& b)
{
    return detail::insn::i_max(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(max, int8, int8)

/** Computes maximum of the unsigned 8-bit values.

    @code
    r0 = max(a0, b0)
    ...
    rN = max(aN, bN)
    @endcode

    @par 256-bit version:
    @icost{SSE2-AVX, NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
uint8<N,expr_empty> max(const uint8<N,E1>& a, const uint8<N,E2>& b)
{
    return detail::insn::i_max(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(max, uint8, uint8)

/** Computes maximum of the signed 16-bit values.

    @code
    r0 = max(a0, b0)
    ...
    rN = max(aN, bN)
    @endcode

    @par 256-bit version:
    @icost{SSE2-AVX, NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
int16<N,expr_empty> max(const int16<N,E1>& a, const int16<N,E2>& b)
{
    return detail::insn::i_max(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(max, int16, int16)

/** Computes maximum of the unsigned 16-bit values.

    @code
    r0 = max(a0, b0)
    ...
    rN = max(aN, bN)
    @endcode

    @par 128-bit version:
    @icost{SSE2-SSSE3, 6-7}

    @par 256-bit version:
    @icost{SSE2-SSSE3, 12-13}
    @icost{SSE4.1-AVX, NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
uint16<N,expr_empty> max(const uint16<N,E1>& a, const uint16<N,E2>& b)
{
    return detail::insn::i_max(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(max, uint16, uint16)

/** Computes maximum of the signed 32-bit values.

    @code
    r0 = max(a0, b0)
    ...
    rN = max(aN, bN)
    @endcode

    @par 128-bit version:
    @icost{SSE2-SSSE3, 4}

    @par 256-bit version:
    @icost{SSE2-SSSE3, 8}
    @icost{SSE4.1-AVX, NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
int32<N,expr_empty> max(const int32<N,E1>& a, const int32<N,E2>& b)
{
    return detail::insn::i_max(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(max, int32, int32)

/** Computes maximum of the unsigned 32-bit values.

    @code
    r0 = max(a0, b0)
    ...
    rN = max(aN, bN)
    @endcode

    @par 128-bit version:
    @icost{SSE2-SSSE3, 6-7}

    @par 256-bit version:
    @icost{SSE2-SSSE3, 12-13}
    @icost{SSE4.1-AVX, NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
uint32<N,expr_empty> max(const uint32<N,E1>& a, const uint32<N,E2>& b)
{
    return detail::insn::i_max(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(max, uint32, uint32)

/** Computes maximum of the signed 64-bit values.

    @code
    r0 = max(a0, b0)
    ...
    rN = max(aN, bN)
    @endcode

    Supported since AVX2, NEON64. Not supported on ALTIVEC.
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
int64<N,expr_empty> max(const int64<N,E1>& a, const int64<N,E2>& b)
{
    return detail::insn::i_max(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(max, int64, int64)

/** Computes maximum of the unsigned 64-bit values.

    @code
    r0 = max(a0, b0)
    ...
    rN = max(aN, bN)
    @endcode

    Supported since AVX2, NEON64. Not supported on ALTIVEC.
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
uint64<N,expr_empty> max(const uint64<N,E1>& a, const uint64<N,E2>& b)
{
    return detail::insn::i_max(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(max, uint64, uint64)

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

