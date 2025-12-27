/*  Copyright (C) 2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMD_TYPES_GENERIC_H
#define LIBSIMDPP_SIMD_TYPES_GENERIC_H

#include <simdpp/setup_arch.h>
#include <simdpp/types.h>
#include <type_traits>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

// -----------------------------------------------------------------------------

/*
    Note: the name of the expression vector type only identifies the target
    type. In various cases we want to change the target type, thus the arbitrary
    conversions have been added.

    Expression types are not meant to be used by the user, thus all constructors
    are implicit.
*/

// -----------------------------------------------------------------------------
// float32

template<unsigned N, class E>
class float32 : public any_float32<N, float32<N,E>>{
public:
    static const unsigned type_tag = SIMDPP_TAG_FLOAT;
    using expr_type = E;
    E e;

    SIMDPP_INL float32(const E& a) : e(a) {}
    template<template<unsigned, class> class V, unsigned N2> SIMDPP_INL
    float32(const any_vec<N*4,V<N2,E>>& a) : e(a.wrapped().e) {}

    SIMDPP_INL operator float32<N>() const { return eval(); }
    SIMDPP_INL float32<N> eval() const
    { return detail::expr_eval_wrapper<float32<N>, expr_type>::eval(e); }
};

template<unsigned N, class E>
class mask_float32 : public any_float32<N, mask_float32<N,E>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_FLOAT;
    using expr_type = E;
    E e;

    SIMDPP_INL mask_float32(const E& a) : e(a) {}

    SIMDPP_INL operator mask_float32<N>() const { return eval(); }
    SIMDPP_INL operator float32<N>() const { return eval(); }
    SIMDPP_INL mask_float32<N> eval() const
    { return detail::expr_eval_wrapper<mask_float32<N>, expr_type>::eval(e); }
};

// -----------------------------------------------------------------------------
// float64

template<unsigned N, class E>
class float64 : public any_float64<N, float64<N,E>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_FLOAT;
    using expr_type = E;
    E e;

    SIMDPP_INL float64(const E& a) : e(a) {}
    template<template<unsigned, class> class V, unsigned N2> SIMDPP_INL
    float64(const any_vec<N*8,V<N2,E>>& a) : e(a.wrapped().e) {}

    SIMDPP_INL operator float64<N>() const { return eval(); }
    SIMDPP_INL float64<N> eval() const
    { return detail::expr_eval_wrapper<float64<N>, expr_type>::eval(e); }
};

template<unsigned N, class E>
class mask_float64 : public any_float64<N, mask_float64<N,E>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_FLOAT;
    using expr_type = E;
    E e;

    SIMDPP_INL mask_float64(const E& a) : e(a) {}

    SIMDPP_INL operator mask_float64<N>() const { return eval(); }
    SIMDPP_INL operator float64<N>() const { return eval(); }
    SIMDPP_INL mask_float64<N> eval() const
    { return detail::expr_eval_wrapper<mask_float64<N>, expr_type>::eval(e); }
};

// -----------------------------------------------------------------------------
// uint8<N>
template<unsigned N, class E>
class int8 : public any_int8<N, int8<N,E>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_INT;
    using expr_type = E;
    E e;

    SIMDPP_INL int8(const E& a) : e(a) {}

    template<template<unsigned, class> class V, unsigned N2> SIMDPP_INL
    int8(const any_vec<N,V<N2,E>>& a) : e(a.wrapped().e) {}

    SIMDPP_INL operator int8<N>() const { return eval(); }
    SIMDPP_INL int8<N> eval() const
    { return detail::expr_eval_wrapper<int8<N>, expr_type>::eval(e); }
};


template<unsigned N, class E>
class uint8 : public any_int8<N, uint8<N,E>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_UINT;
    using expr_type = E;
    E e;

    SIMDPP_INL uint8(const E& a) : e(a) {}

    template<template<unsigned, class> class V, unsigned N2> SIMDPP_INL
    uint8(const any_vec<N,V<N2,E>>& a) : e(a.wrapped().e) {}

    SIMDPP_INL operator uint8<N>() const { return eval(); }
    SIMDPP_INL uint8<N> eval() const
    { return detail::expr_eval_wrapper<uint8<N>, expr_type>::eval(e); }
};

template<unsigned N, class E>
class mask_int8 : public any_int8<N, mask_int8<N,E>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_INT;
    using expr_type = E;
    E e;

    SIMDPP_INL mask_int8(const E& a) : e(a) {}

    SIMDPP_INL operator mask_int8<N>() const { return eval(); }
    SIMDPP_INL operator uint8<N>() const { return eval(); }
    SIMDPP_INL mask_int8<N> eval() const
    { return detail::expr_eval_wrapper<mask_int8<N>, expr_type>::eval(e); }
};

// -----------------------------------------------------------------------------
// uint16<N>
template<unsigned N, class E>
class int16 : public any_int16<N, int16<N,E>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_INT;
    using expr_type = E;
    E e;

    SIMDPP_INL int16(const E& a) : e(a) {}
    template<template<unsigned, class> class V, unsigned N2> SIMDPP_INL
    int16(const any_vec<N*2,V<N2,E>>& a) : e(a.wrapped().e) {}

    SIMDPP_INL operator int16<N>() const { return eval(); }
    SIMDPP_INL int16<N> eval() const
    { return detail::expr_eval_wrapper<int16<N>, expr_type>::eval(e); }
};


template<unsigned N, class E>
class uint16 : public any_int16<N, uint16<N,E>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_UINT;
    using expr_type = E;
    E e;

    SIMDPP_INL uint16(const E& a) : e(a) {}
    template<template<unsigned, class> class V, unsigned N2> SIMDPP_INL
    uint16(const any_vec<N*2,V<N2,E>>& a) : e(a.wrapped().e) {}

    SIMDPP_INL operator uint16<N>() const { return eval(); }
    SIMDPP_INL uint16<N> eval() const
    { return detail::expr_eval_wrapper<uint16<N>, expr_type>::eval(e); }
};

template<unsigned N, class E>
class mask_int16 : public any_int16<N, mask_int16<N,E>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_INT;
    using expr_type = E;
    E e;

    SIMDPP_INL mask_int16(const E& a) : e(a) {}

    SIMDPP_INL operator mask_int16<N>() const { return eval(); }
    SIMDPP_INL operator uint16<N>() const { return eval(); }
    SIMDPP_INL mask_int16<N> eval() const
    { return detail::expr_eval_wrapper<mask_int16<N>, expr_type>::eval(e); }
};

// -----------------------------------------------------------------------------
// uint32<N>
template<unsigned N, class E>
class int32 : public any_int32<N, int32<N,E>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_INT;
    using expr_type = E;
    E e;

    SIMDPP_INL int32(const E& a) : e(a) {}
    template<template<unsigned, class> class V, unsigned N2> SIMDPP_INL
    int32(const any_vec<N*4,V<N2,E>>& a) : e(a.wrapped().e) {}

    SIMDPP_INL operator int32<N>() const { return eval(); }
    SIMDPP_INL int32<N> eval() const
    { return detail::expr_eval_wrapper<int32<N>, expr_type>::eval(e); }
};


template<unsigned N, class E>
class uint32 : public any_int32<N, uint32<N,E>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_UINT;
    using expr_type = E;
    E e;

    SIMDPP_INL uint32(const E& a) : e(a) {}
    template<template<unsigned, class> class V, unsigned N2> SIMDPP_INL
    uint32(const any_vec<N*4,V<N2,E>>& a) : e(a.wrapped().e) {}

    SIMDPP_INL operator uint32<N>() const { return eval(); }
    SIMDPP_INL uint32<N> eval() const
    { return detail::expr_eval_wrapper<uint32<N>, expr_type>::eval(e); }
};

template<unsigned N, class E>
class mask_int32 : public any_int32<N, mask_int32<N,E>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_INT;
    using expr_type = E;
    E e;

    SIMDPP_INL mask_int32(const E& a) : e(a) {}

    SIMDPP_INL operator mask_int32<N>() const { return eval(); }
    SIMDPP_INL operator uint32<N>() const { return eval(); }
    SIMDPP_INL mask_int32<N> eval() const
    { return detail::expr_eval_wrapper<mask_int32<N>, expr_type>::eval(e); }
};

// -----------------------------------------------------------------------------
// int64<N>

template<unsigned N, class E>
class int64 : public any_int64<N, int64<N,E>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_INT;
    using expr_type = E;
    E e;

    SIMDPP_INL int64(const E& a) : e(a) {}
    template<template<unsigned, class> class V, unsigned N2> SIMDPP_INL
    int64(const any_vec<N*8,V<N2,E>>& a) : e(a.wrapped().e) {}

    SIMDPP_INL operator int64<N>() const { return eval(); }
    SIMDPP_INL int64<N> eval() const
    { return detail::expr_eval_wrapper<int64<N>, expr_type>::eval(e); }
};


template<unsigned N, class E>
class uint64 : public any_int64<N, uint64<N,E>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_UINT;
    using expr_type = E;
    E e;

    SIMDPP_INL uint64(const E& a) : e(a) {}
    template<template<unsigned, class> class V, unsigned N2> SIMDPP_INL
    uint64(const any_vec<N*8,V<N2,E>>& a) : e(a.wrapped().e) {}

    SIMDPP_INL operator uint64<N>() const { return eval(); }
    SIMDPP_INL uint64<N> eval() const
    { return detail::expr_eval_wrapper<uint64<N>, expr_type>::eval(e); }
};


template<unsigned N, class E>
class mask_int64 : public any_int64<N, mask_int64<N,E>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_INT;
    using expr_type = E;
    E e;

    SIMDPP_INL mask_int64(const E& a) : e(a) {}

    SIMDPP_INL operator mask_int64<N>() const { return eval(); }
    SIMDPP_INL operator uint64<N>() const { return eval(); }
    SIMDPP_INL mask_int64<N> eval() const
    { return detail::expr_eval_wrapper<mask_int64<N>, expr_type>::eval(e); }
};

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

