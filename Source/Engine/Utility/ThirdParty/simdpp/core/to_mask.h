/*  Copyright (C) 2017  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_CORE_TO_MASK_H
#define LIBSIMDPP_SIMDPP_CORE_TO_MASK_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/insn/conv_to_mask.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Converts a vector to mask.

    The values of the elements in the source vector must be either all ones
    or all zeros. Otherwise the behavior is undefined.
*/
template<unsigned N, class E> SIMDPP_INL
mask_int8<N,expr_empty> to_mask(const int8<N,E>& a)
{
    return detail::insn::i_to_mask(uint8<N>(a.eval()));
}
template<unsigned N, class E> SIMDPP_INL
mask_int8<N,expr_empty> to_mask(const uint8<N,E>& a)
{
    return detail::insn::i_to_mask(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
mask_int16<N,expr_empty> to_mask(const int16<N,E>& a)
{
    return detail::insn::i_to_mask(uint16<N>(a.eval()));
}
template<unsigned N, class E> SIMDPP_INL
mask_int16<N,expr_empty> to_mask(const uint16<N,E>& a)
{
    return detail::insn::i_to_mask(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
mask_int32<N,expr_empty> to_mask(const int32<N,E>& a)
{
    return detail::insn::i_to_mask(uint32<N>(a.eval()));
}
template<unsigned N, class E> SIMDPP_INL
mask_int32<N,expr_empty> to_mask(const uint32<N,E>& a)
{
    return detail::insn::i_to_mask(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
mask_int64<N,expr_empty> to_mask(const int64<N,E>& a)
{
    return detail::insn::i_to_mask(uint64<N>(a.eval()));
}
template<unsigned N, class E> SIMDPP_INL
mask_int64<N,expr_empty> to_mask(const uint64<N,E>& a)
{
    return detail::insn::i_to_mask(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
mask_float32<N,expr_empty> to_mask(const float32<N,E>& a)
{
    return detail::insn::i_to_mask(a.eval());
}
template<unsigned N, class E> SIMDPP_INL
mask_float64<N,expr_empty> to_mask(const float64<N,E>& a)
{
    return detail::insn::i_to_mask(a.eval());
}

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

