/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_CMP_GT_H
#define LIBSIMDPP_SIMDPP_CORE_CMP_GT_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/cmp_gt.h>
#include <simdpp/core/detail/scalar_arg_impl.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Compares the values of two signed int16x8 vectors for greater-than

    @code
    r0 = (a0 > b0) ? 0xff : 0x0
    ...
    rN = (aN > bN) ? 0xff : 0x0
    @endcode

    @par 256-bit version:
    @icost{SSE2-AVX, NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
mask_int8<N,expr_empty> cmp_gt(const int8<N,E1>& a,
                               const int8<N,E2>& b)
{
    return detail::insn::i_cmp_gt(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(cmp_gt, mask_int8, int8)


/** Compares the values of two unsigned int16x8 vectors for greater-than

    @code
    r0 = (a0 > b0) ? 0xff : 0x0
    ...
    rN = (aN > bN) ? 0xff : 0x0
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 3-4}
    @icost{XOP, 1}

    @par 256-bit version:
    @icost{SSE2-AVX, 6-7}
    @icost{AVX2, 3-4}
    @icost{XOP, 2}
    @icost{NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
mask_int8<N,expr_empty> cmp_gt(const uint8<N,E1>& a,
                               const uint8<N,E2>& b)
{
    return detail::insn::i_cmp_gt(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(cmp_gt, mask_int8, uint8)

/** Compares the values of two signed int16x8 vectors for greater-than

    @code
    r0 = (a0 > b0) ? 0xffff : 0x0
    ...
    rN = (aN > bN) ? 0xffff : 0x0
    @endcode

    @par 256-bit version:
    @icost{SSE2-AVX, NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
mask_int16<N,expr_empty> cmp_gt(const int16<N,E1>& a,
                                const int16<N,E2>& b)
{
    return detail::insn::i_cmp_gt(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(cmp_gt, mask_int16, int16)

/** Compares the values of two unsigned int16x8 vectors for greater-than

    @code
    r0 = (a0 > b0) ? 0xffff : 0x0
    ...
    rN = (aN > bN) ? 0xffff : 0x0
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 3-4}
    @icost{XOP, 1}

    @par 256-bit version:
    @icost{SSE2-AVX, 6-7}
    @icost{AVX2, 3-4}
    @icost{XOP, NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
mask_int16<N,expr_empty> cmp_gt(const uint16<N,E1>& a,
                                const uint16<N,E2>& b)
{
    return detail::insn::i_cmp_gt(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(cmp_gt, mask_int16, uint16)

/** Compares the values of two signed int32x4 vectors for greater-than

    @code
    r0 = (a0 > b0) ? 0xffffffff : 0x0
    ...
    rN = (aN > bN) ? 0xffffffff : 0x0
    @endcode

    @par 256-bit version:
    @icost{SSE2-AVX, NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
mask_int32<N,expr_empty> cmp_gt(const int32<N,E1>& a,
                                const int32<N,E2>& b)
{
    return detail::insn::i_cmp_gt(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(cmp_gt, mask_int32, int32)

/** Compares the values of two unsigned int32x4 vectors for greater-than

    @code
    r0 = (a0 > b0) ? 0xffffffff : 0x0
    ...
    rN = (aN > bN) ? 0xffffffff : 0x0
    @endcode

    @par 128-bit version:
    @icost{SSE2-AVX2, 3-4}
    @icost{XOP, 1}

    @par 256-bit version:
    @icost{SSE2-AVX, 6-7}
    @icost{AVX2, 3-4}
    @icost{XOP, NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
mask_int32<N,expr_empty> cmp_gt(const uint32<N,E1>& a,
                                const uint32<N,E2>& b)
{
    return detail::insn::i_cmp_gt(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(cmp_gt, mask_int32, uint32)

/** Compares the values of two signed int64 vectors for greater-than

    @code
    r0 = (a0 > b0) ? 0xffffffffffff : 0x0
    ...
    rN = (aN > bN) ? 0xffffffffffff : 0x0
    @endcode

    Supported since AVX2, NEON64. Not supported on ALTIVEC.
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
mask_int64<N,expr_empty> cmp_gt(const int64<N,E1>& a,
                                const int64<N,E2>& b)
{
    return detail::insn::i_cmp_gt(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(cmp_gt, mask_int64, int64)

/** Compares the values of two unsigned int64 vectors for greater-than

    @code
    r0 = (a0 > b0) ? 0xffffffffffff : 0x0
    ...
    rN = (aN > bN) ? 0xffffffffffff : 0x0
    @endcode

    Supported since AVX2, NEON64. Not supported on ALTIVEC.
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
mask_int64<N,expr_empty> cmp_gt(const uint64<N,E1>& a,
                                const uint64<N,E2>& b)
{
    return detail::insn::i_cmp_gt(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(cmp_gt, mask_int64, uint64)

/** Compares the values of two float32x4 vectors for greater-than

    @code
    r0 = (a0 > b0) ? 0xffffffff : 0x0
    ...
    rN = (aN > bN) ? 0xffffffff : 0x0
    @endcode

    @par 256-bit version:
    @icost{SSE2-SSE4.1, NEON, ALTIVEC, 2}
*/
template<unsigned N, class E1, class E2>
mask_float32<N,expr_empty> cmp_gt(const float32<N,E1>& a,
                                  const float32<N,E2>& b)
{
    return detail::insn::i_cmp_gt(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(cmp_gt, mask_float32, float32)

/** Compares the values of two float64x2 vectors for greater-than

    @code
    r0 = (a0 > b0) ? 0xffffffffffffffff : 0x0
    ...
    rN = (aN > bN) ? 0xffffffffffffffff : 0x0
    @endcode

    @par 128-bit version:
    @novec{NEON, ALTIVEC}

    @par 256-bit version:
    @novec{NEON, ALTIVEC}
    @icost{SSE2-SSE4.1, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
mask_float64<N,expr_empty> cmp_gt(const float64<N,E1>& a,
                                  const float64<N,E2>& b)
{
    return detail::insn::i_cmp_gt(a.eval(), b.eval());
}

SIMDPP_SCALAR_ARG_IMPL_VEC(cmp_gt, mask_float64, float64)

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

