/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_I_SHIFT_R_H
#define LIBSIMDPP_SIMDPP_CORE_I_SHIFT_R_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/capabilities.h>
#include <simdpp/detail/insn/i_shift_r.h>
#include <simdpp/detail/insn/i_shift_r_v.h>
#include <simdpp/detail/not_implemented.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

// -----------------------------------------------------------------------------
// shift by scalar

/** Shifts signed 8-bit values right by @a count bits while shifting in the
    sign bit.

    @code
    r0 = a0 >> count
    ...
    rN = aN >> count
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
int8<N,expr_empty> shift_r(const int8<N,E>& a, unsigned count)
{
    return detail::insn::i_shift_r(a.eval(), count);
}

/** Shifts unsigned 8-bit values right by @a count bits while shifting in zeros.

    @code
    r0 = a0 >> count
    ...
    rN = aN >> count
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
uint8<N,expr_empty> shift_r(const uint8<N,E>& a, unsigned count)
{
    return detail::insn::i_shift_r(a.eval(), count);
}

/** Shifts signed 16-bit values right by @a count bits while shifting in the
    sign bit.

    @code
    r0 = a0 >> count
    ...
    rN = aN >> count
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
int16<N,expr_empty> shift_r(const int16<N,E>& a, unsigned count)
{
    return detail::insn::i_shift_r(a.eval(), count);
}

/** Shifts unsigned 16-bit values right by @a count bits while shifting in
    zeros.

    @code
    r0 = a0 >> count
    ...
    rN = aN >> count
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
uint16<N,expr_empty> shift_r(const uint16<N,E>& a, unsigned count)
{
    return detail::insn::i_shift_r(a.eval(), count);
}

/** Shifts signed 32-bit values right by @a count bits while shifting in the
    sign bit.

    @code
    r0 = a0 >> count
    ...
    rN = aN >> count
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
int32<N,expr_empty> shift_r(const int32<N,E>& a, unsigned count)
{
    return detail::insn::i_shift_r(a.eval(), count);
}

/** Shifts unsigned 32-bit values right by @a count bits while shifting in
    zeros.

    @code
    r0 = a0 >> count
    ...
    rN = aN >> count
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
uint32<N,expr_empty> shift_r(const uint32<N,E>& a, unsigned count)
{
    return detail::insn::i_shift_r(a.eval(), count);
}

/** Shifts signed 64-bit values right by @a count bits while shifting in the
    sign bit.

    @code
    r0 = a0 >> count
    ...
    rN = aN >> count
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
int64<N,expr_empty> shift_r(const int64<N,E>& a, unsigned count)
{
    return detail::insn::i_shift_r(a.eval(), count);
}

/** Shifts unsigned 64-bit values right by @a count bits while shifting in
    zeros.

    @code
    r0 = a0 >> count
    ...
    rN = aN >> count
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
uint64<N,expr_empty> shift_r(const uint64<N,E>& a, unsigned count)
{
    return detail::insn::i_shift_r(a.eval(), count);
}

// -----------------------------------------------------------------------------
// shift by vector

/** Shifts signed 8-bit values right by the number of bits in corresponding
    element in the given count vector. Sign bits are shifted in.

    @code
    r0 = a0 >> count0
    ...
    rN = aN >> countN
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
int8<N,expr_empty> shift_r(const int8<N,E>& a, const uint8<N,E>& count)
{
#if SIMDPP_HAS_INT8_SHIFT_R_BY_VECTOR
    return detail::insn::i_shift_r_v(a.eval(), count.eval());
#else
    return SIMDPP_NOT_IMPLEMENTED_TEMPLATE2(E, a, count);
#endif
}

/** Shifts unsigned 8-bit values right by the number of bits in corresponding
    element in the given count vector. Zero bits are shifted in.

    @code
    r0 = a0 >> count0
    ...
    rN = aN >> countN
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
uint8<N,expr_empty> shift_r(const uint8<N,E>& a, const uint8<N,E>& count)
{
#if SIMDPP_HAS_UINT8_SHIFT_R_BY_VECTOR
    return detail::insn::i_shift_r_v(a.eval(), count.eval());
#else
    return SIMDPP_NOT_IMPLEMENTED_TEMPLATE2(E, a, count);
#endif
}

/** Shifts signed 16-bit values right by the number of bits in corresponding
    element in the given count vector. Sign bits are shifted in.

    @code
    r0 = a0 >> count0
    ...
    rN = aN >> countN
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
int16<N,expr_empty> shift_r(const int16<N,E>& a, const uint16<N,E>& count)
{
#if SIMDPP_HAS_INT16_SHIFT_R_BY_VECTOR
    return detail::insn::i_shift_r_v(a.eval(), count.eval());
#else
    return SIMDPP_NOT_IMPLEMENTED_TEMPLATE2(E, a, count);
#endif
}

/** Shifts unsigned 16-bit values right by the number of bits in corresponding
    element in the given count vector. Zero bits are shifted in.

    @code
    r0 = a0 >> count0
    ...
    rN = aN >> countN
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
uint16<N,expr_empty> shift_r(const uint16<N,E>& a, const uint16<N,E>& count)
{
#if SIMDPP_HAS_UINT16_SHIFT_R_BY_VECTOR
    return detail::insn::i_shift_r_v(a.eval(), count.eval());
#else
    return SIMDPP_NOT_IMPLEMENTED_TEMPLATE2(E, a, count);
#endif
}

/** Shifts signed 32-bit values right by the number of bits in corresponding
    element in the given count vector. Sign bits are shifted in.

    @code
    r0 = a0 >> count0
    ...
    rN = aN >> countN
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
int32<N,expr_empty> shift_r(const int32<N,E>& a, const uint32<N,E>& count)
{
#if SIMDPP_HAS_INT32_SHIFT_R_BY_VECTOR
    return detail::insn::i_shift_r_v(a.eval(), count.eval());
#else
    return SIMDPP_NOT_IMPLEMENTED_TEMPLATE2(E, a, count);
#endif
}

/** Shifts unsigned 32-bit values right by the number of bits in corresponding
    element in the given count vector. Zero bits are shifted in.

    @code
    r0 = a0 >> count0
    ...
    rN = aN >> countN
    @endcode
*/
template<unsigned N, class E> SIMDPP_INL
uint32<N,expr_empty> shift_r(const uint32<N,E>& a, const uint32<N,E>& count)
{
#if SIMDPP_HAS_UINT32_SHIFT_R_BY_VECTOR
    return detail::insn::i_shift_r_v(a.eval(), count.eval());
#else
    return SIMDPP_NOT_IMPLEMENTED_TEMPLATE2(E, a, count);
#endif
}

// -----------------------------------------------------------------------------
// shift by compile-time constant

/** Shifts signed 8-bit values right by @a count bits while shifting in the
    sign bit.

    @code
    r0 = a0 >> count
    ...
    rN = aN >> count
    @endcode
*/
template<unsigned count, unsigned N, class E> SIMDPP_INL
int8<N,expr_empty> shift_r(const int8<N,E>& a)
{
    static_assert(count < 8, "Shift out of bounds");
    return detail::insn::i_shift_r_wrapper<count == 0>::template run<count>(a.eval());
}

/** Shifts unsigned 8-bit values right by @a count bits while shifting in
    zeros.

    @code
    r0 = a0 >> count
    ...
    rN = aN >> count
    @endcode
*/
template<unsigned count, unsigned N, class E> SIMDPP_INL
uint8<N,expr_empty> shift_r(const uint8<N,E>& a)
{
    static_assert(count < 8, "Shift out of bounds");
    return detail::insn::i_shift_r_wrapper<count == 0>::template run<count>(a.eval());
}

/** Shifts signed 16-bit values right by @a count bits while shifting in the
    sign bit.

    @code
    r0 = a0 >> count
    ...
    rN = aN >> count
    @endcode
*/
template<unsigned count, unsigned N, class E> SIMDPP_INL
int16<N,expr_empty> shift_r(const int16<N,E>& a)
{
    static_assert(count < 16, "Shift out of bounds");
    return detail::insn::i_shift_r_wrapper<count == 0>::template run<count>(a.eval());
}

/** Shifts unsigned 16-bit values right by @a count bits while shifting in
    zeros.

    @code
    r0 = a0 >> count
    ...
    rN = aN >> count
    @endcode
*/
template<unsigned count, unsigned N, class E> SIMDPP_INL
uint16<N,expr_empty> shift_r(const uint16<N,E>& a)
{
    static_assert(count < 16, "Shift out of bounds");
    return detail::insn::i_shift_r_wrapper<count == 0>::template run<count>(a.eval());
}

/** Shifts signed 32-bit values right by @a count bits while shifting in the
    sign bit.

    @code
    r0 = a0 >> count
    ...
    rN = aN >> count
    @endcode
*/
template<unsigned count, unsigned N, class E> SIMDPP_INL
int32<N,expr_empty> shift_r(const int32<N,E>& a)
{
    static_assert(count < 32, "Shift out of bounds");
    return detail::insn::i_shift_r_wrapper<count == 0>::template run<count>(a.eval());
}

/** Shifts unsigned 32-bit values right by @a count bits while shifting in
    zeros.

    @code
    r0 = a0 >> count
    ...
    rN = aN >> count
    @endcode
*/
template<unsigned count, unsigned N, class E> SIMDPP_INL
uint32<N,expr_empty> shift_r(const uint32<N,E>& a)
{
    static_assert(count < 32, "Shift out of bounds");
    return detail::insn::i_shift_r_wrapper<count == 0>::template run<count>(a.eval());
}

/** Shifts signed 64-bit values right by @a count bits while shifting in the
    sign bit.

    @code
    r0 = a0 >> count
    ...
    rN = aN >> count
    @endcode
*/
template<unsigned count, unsigned N, class E> SIMDPP_INL
int64<N,expr_empty> shift_r(const int64<N,E>& a)
{
    static_assert(count < 64, "Shift out of bounds");
    return detail::insn::i_shift_r_wrapper<count == 0>::template run<count>(a.eval());
}

/** Shifts unsigned 64-bit values right by @a count bits while shifting in
    zeros.

    @code
    r0 = a0 >> count
    ...
    rN = aN >> count
    @endcode
*/
template<unsigned count, unsigned N, class E> SIMDPP_INL
uint64<N,expr_empty> shift_r(const uint64<N,E>& a)
{
    static_assert(count < 64, "Shift out of bounds");
    return detail::insn::i_shift_r_wrapper<count == 0>::template run<count>(a.eval());
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

