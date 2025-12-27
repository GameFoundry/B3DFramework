/*  Copyright (C) 2015

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_STORE_U_H
#define LIBSIMDPP_SIMDPP_CORE_STORE_U_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/store_u.h>

namespace simdpp {
#ifndef SIMDPP_DOXYGEN
namespace SIMDPP_ARCH_NAMESPACE {
#endif

/** Stores a 128-bit or 256-bit integer to an unaligned memory location.

    @par 128-bit version:

    @code
    *(p) = a[0..127]
    @endcode

    @par 256-bit version:

    @code
    *(p) = a[0..255]
    @endcode
*/
template<class T, unsigned N, class V> SIMDPP_INL
void store_u(T* p, const any_vec<N,V>& a)
{
    static_assert(!is_mask<V>::value, "Masks can not be stored"); // FIXME: automatically convert
    detail::insn::v_store_u(reinterpret_cast<char*>(p), a.wrapped().eval());
}

#ifndef SIMDPP_DOXYGEN
} // namespace SIMDPP_ARCH_NAMESPACE
#endif
} // namespace simdpp

#endif

