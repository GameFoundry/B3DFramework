/*  Copyright (C) 2011-2012  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_DETAIL_NULL_MEMORY_H
#define LIBSIMDPP_DETAIL_NULL_MEMORY_H
#if SIMDPP_USE_NULL || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {
namespace null {

template<class V> SIMDPP_INL
void load(V& a, const char* p)
{
    using T = typename V::element_type;
    const T* pt = reinterpret_cast<const T*>(p);
    for (unsigned i = 0; i < V::length; i++) {
        a.el(i) = *pt++;
    }
}

template<class V> SIMDPP_INL
void load_packed2(V& a, V& b, const char* p)
{
    using T = typename V::element_type;
    const T* pt = reinterpret_cast<const T*>(p);
    for (unsigned i = 0; i < V::length; i++) {
        a.el(i) = *pt++;
        b.el(i) = *pt++;
    }
}

template<class V> SIMDPP_INL
void load_packed3(V& a, V& b, V& c, const char* p)
{
    using T = typename V::element_type;
    const T* pt = reinterpret_cast<const T*>(p);
    for (unsigned i = 0; i < V::length; i++) {
        a.el(i) = *pt++;
        b.el(i) = *pt++;
        c.el(i) = *pt++;
    }
}

template<class V> SIMDPP_INL
void load_packed4(V& a, V& b, V& c, V& d, const char* p)
{
    using T = typename V::element_type;
    const T* pt = reinterpret_cast<const T*>(p);
    for (unsigned i = 0; i < V::length; i++) {
        a.el(i) = *pt++;
        b.el(i) = *pt++;
        c.el(i) = *pt++;
        d.el(i) = *pt++;
    }
}

template<class V> SIMDPP_INL
void store(char* p, const V& a)
{
    using T = typename V::element_type;
    T* pt = reinterpret_cast<T*>(p);
    for (unsigned i = 0; i < V::length; i++) {
        *pt++ = a.el(i);
    }
}

template<class V, class M> SIMDPP_INL
void store_masked(char* p, const V& a, const M& mask)
{
    using T = typename V::element_type;
    T* pt = reinterpret_cast<T*>(p);
    for (unsigned i = 0; i < a.length; ++i) {
        if (mask.el(i))
            *pt = a.el(i);
        pt++;
    }
}

template<class V> SIMDPP_INL
void store_first(char* p, const V& a, unsigned n)
{
    using T = typename V::element_type;
    T* pt = reinterpret_cast<T*>(p);
    for (unsigned i = 0; i < V::length && i < n; i++) {
        *pt++ = a.el(i);
    }
}

template<class V> SIMDPP_INL
void store_last(char* p, const V& a, unsigned n)
{
    using T = typename V::element_type;
    T* pt = reinterpret_cast<T*>(p);
    pt += V::length - n;
    for (unsigned i = V::length - n; i < V::length; i++) {
        *pt++ = a.el(i);
    }
}

template<class V> SIMDPP_INL
void store_packed2(char* p, const V& a, const V& b)
{
    using T = typename V::element_type;
    T* pt = reinterpret_cast<T*>(p);
    for (unsigned i = 0; i < V::length; i++) {
        *pt++ = a.el(i);
        *pt++ = b.el(i);
    }
}

template<class V> SIMDPP_INL
void store_packed3(char* p, const V& a, const V& b, const V& c)
{
    using T = typename V::element_type;
    T* pt = reinterpret_cast<T*>(p);
    for (unsigned i = 0; i < V::length; i++) {
        *pt++ = a.el(i);
        *pt++ = b.el(i);
        *pt++ = c.el(i);
    }
}

template<class V> SIMDPP_INL
void store_packed4(char* p, const V& a, const V& b, const V& c, const V& d)
{
    using T = typename V::element_type;
    T* pt = reinterpret_cast<T*>(p);
    for (unsigned i = 0; i < V::length; i++) {
        *pt++ = a.el(i);
        *pt++ = b.el(i);
        *pt++ = c.el(i);
        *pt++ = d.el(i);
    }
}

} // namespace null
} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
#endif
