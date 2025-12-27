/*  Copyright (C) 2011-2012  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_DETAIL_NULL_MATH_H
#define LIBSIMDPP_DETAIL_NULL_MATH_H
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/core/cast.h>

#include <cmath>
#include <cstdlib>
#include <limits>

// On certain versions of MSVC min and max are defined as macros.
#if _MSC_VER
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif
#endif

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace null {

template<class V> SIMDPP_INL
typename V::mask_vector_type isnan(const V& a)
{
    typename V::mask_vector_type r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = std::isnan(a.el(i)) ? 1 : 0;
    }
    return r;
}

template<class V> SIMDPP_INL
typename V::mask_vector_type isnan2(const V& a, const V& b)
{
    typename V::mask_vector_type r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = (std::isnan(a.el(i)) || std::isnan(b.el(i))) ? 1 : 0;
    }
    return r;
}

template<class V> SIMDPP_INL
V abs(const V& a)
{
    V r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = std::abs(a.el(i));
    }
    return r;
}

template<class T, class U> SIMDPP_INL
T saturate(U t)
{
    U min = std::numeric_limits<T>::min();
    U max = std::numeric_limits<T>::max();
    t = t < min ? min : t;
    t = t > max ? max : t;
    return t;
}

template<class V> SIMDPP_INL
V add(const V& a, const V& b)
{
    V r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = a.el(i) + b.el(i);
    }
    return r;
}

template<class V> SIMDPP_INL
V add_sat(const V& a, const V& b)
{
    V r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = saturate<typename V::element_type>(int32_t(a.el(i)) + b.el(i));
    }
    return r;
}

template<class V> SIMDPP_INL
V sub(const V& a, const V& b)
{
    V r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = a.el(i) - b.el(i);
    }
    return r;
}

template<class V> SIMDPP_INL
V sub_sat(const V& a, const V& b)
{
    V r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = saturate<typename V::element_type>(int32_t(a.el(i)) - b.el(i));
    }
    return r;
}

template<class V> SIMDPP_INL
V neg(const V& a)
{
    V r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = -a.el(i);
    }
    return r;
}

template<class V> SIMDPP_INL
V mul(const V& a, const V& b)
{
    V r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = a.el(i) * b.el(i);
    }
    return r;
}

template<class V> SIMDPP_INL
V fmadd(const V& a, const V& b, const V& c)
{
    V r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = std::fma(a.el(i), b.el(i), c.el(i));
    }
    return r;
}

template<class V> SIMDPP_INL
V fmsub(const V& a, const V& b, const V& c)
{
    V r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = std::fma(a.el(i), b.el(i), -c.el(i));
    }
    return r;
}

template<unsigned P, class V> SIMDPP_INL
V div_p(const V& a, const V& b)
{
    // the least P significant bits of the mask are set.
    uint64_t mask = 0;
    mask = ~mask;
    mask <<= P;
    mask = ~mask;

    V r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = (a.el(i) & mask) / (b.el(i) & mask);
        r.el(i) &= mask;
    }
    return r;
}


template<class V> SIMDPP_INL
V shift_r(const V& a, unsigned shift)
{
    V r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = a.el(i) >> shift;
    }
    return r;
}

template<class V, class S> SIMDPP_INL
V shift_r_v(const V& a, const S& shift)
{
    V r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = a.el(i) >> shift.el(i);
    }
    return r;
}

template<class V> SIMDPP_INL
V shift_l(const V& a, unsigned shift)
{
    V r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = a.el(i) << shift;
    }
    return r;
}

template<class V, class S> SIMDPP_INL
V shift_l_v(const V& a, const S& shift)
{
    V r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = a.el(i) << shift.el(i);
    }
    return r;
}

template<class V> SIMDPP_INL
V min(const V& a, const V& b)
{
    V r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = a.el(i) <= b.el(i) ? a.el(i) : b.el(i);
    }
    return r;
}

template<class V> SIMDPP_INL
V max(const V& a, const V& b)
{
    V r;
    for (unsigned i = 0; i < V::length; i++) {
        r.el(i) = a.el(i) >= b.el(i) ? a.el(i) : b.el(i);
    }
    return r;
}

} // namespace null
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
#endif
