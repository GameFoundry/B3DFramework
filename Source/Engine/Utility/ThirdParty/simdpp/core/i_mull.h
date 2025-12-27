/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_I_MULL_H
#define LIBSIMDPP_SIMDPP_CORE_I_MULL_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/expr/i_mull.h>
#include <simdpp/core/detail/scalar_arg_impl.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {


/*  Note: widening integer multiplication instructions are very different among
    instruction sets. The main difference is in which half of the elements are
    selected for multiplication. Trying to abstract this incurs definite
    overhead.

     - SSE2-SSE4.1 and AVX2 provide only instructions with interfaces similar
        to mul_lo and mul_hi. The result vectors must be interleaved to obtain
        contiguous result values. Multiplying 2 vectors always incurs
        overhead of at least two interleaving instructions.

     - AVX512 only provides 32-bit integer support. Widening multiplication
        can be done only by using PMULDQ, which takes odd elements and produces
        widened multiplication results. Multiplication of two whole vectors
        always incurs overhead of at least two shifts or interleaving
        instructions.

     - NEON, NEONv2 provide instructions that take elements of either the lower
        or higher halves of two 128-bit vectors and multiply them. No
        additional overhead is incurred to obtain contiguous result values.

     - ALTIVEC hav multiply odd and multiply even instructions. No additional
        overhead is incurred to obtain contiguous result values.

    The abstraction below uses the NEON model. No additional overhead is
    incurred on SSE/AVX and NEON. On ALTIVEC, a single additional permute
    instruction is needed for each vector multiplication on average.
*/

/** Multiplies signed 16-bit values and expands the results to 32 bits.

    @par 128-bit version:
    @code
    r0 = a0 * b0
    ...
    rN = aN * bN
    @endcode

    @icost{SSE2-AVX, ALTIVEC, 2-3}

    @par 256-bit version:

    @icost{SSE2-AVX, ALTIVEC, 4-6}
    @icost{AVX2, NEON, 2-3}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
int32<N, expr_mull<int16<N,E1>,
                   int16<N,E2>>> mull(const int16<N,E1>& a, const int16<N,E2>& b)
{
    return { { a, b } };
}

SIMDPP_SCALAR_ARG_IMPL_EXPR(mull, expr_mull, int32, int16)

/** Multiplies unsigned 16-bit values and expands the results to 32 bits.

    @par 128-bit version:
    @code
    r0 = a0 * b0
    ...
    rN = aN * bN
    @endcode

    @icost{SSE2-AVX2, ALTIVEC, 2-3}

    @par 256-bit version:
    @icost{SSE2-AVX, ALTIVEC, 4-6}
    @icost{AVX2, 2-3}
    @icost{NEON, 2}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
uint32<N, expr_mull<uint16<N,E1>,
                    uint16<N,E2>>> mull(const uint16<N,E1>& a, const uint16<N,E2>& b)
{
    return { { a, b } };
}

SIMDPP_SCALAR_ARG_IMPL_EXPR(mull, expr_mull, uint32, uint16)

/** Multiplies signed 32-bit values in and expands the results to 64 bits.

    @code
    r0 = a0 * b0
    ...
    rN = aN * bN
    @endcode
    @par 128-bit version:
    @icost{SSE4.1-AVX, 3}
    @unimp{SSE2-SSSE3, ALTIVEC}

    @par 256-bit version:
    @icost{SSE4.1-AVX, 6}
    @icost{AVX2, 3}
    @icost{NEON, 2}
    @unimp{SSE2-SSSE3, ALTIVEC}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
int64<N, expr_mull<int32<N,E1>,
                   int32<N,E2>>> mull(const int32<N,E1>& a, const int32<N,E2>& b)
{
    return { { a, b } };
}

SIMDPP_SCALAR_ARG_IMPL_EXPR(mull, expr_mull, int64, int32)

/** Multiplies unsigned 32-bit values in the lower halves of the vectors and
    expands the results to 64 bits.

    @par 128-bit version:
    @code
    r0 = a0 * b0
    r1 = a1 * b1
    @endcode
    @icost{SSE2-AVX, 3}
    @unimp{ALTIVEC}

    @icost{SSE2-AVX, 6}
    @icost{AVX2, 3}
    @icost{NEON, 2}
    @unimp{ALTIVEC}
*/
template<unsigned N, class E1, class E2> SIMDPP_INL
uint64<N, expr_mull<uint32<N,E1>,
                    uint32<N,E2>>> mull(const uint32<N,E1>& a, const uint32<N,E2>& b)
{
    return { { a, b } };
}

SIMDPP_SCALAR_ARG_IMPL_EXPR(mull, expr_mull, uint64, uint32)

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

