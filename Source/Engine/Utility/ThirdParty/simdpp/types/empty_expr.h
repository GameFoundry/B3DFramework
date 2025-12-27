/*  Copyright (C) 2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMD_TYPES_EMPTY_EXPR_H
#define LIBSIMDPP_SIMD_TYPES_EMPTY_EXPR_H

#include <simdpp/setup_arch.h>
#include <simdpp/types.h>
#include <simdpp/expr.h>
#include <type_traits>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

// -----------------------------------------------------------------------------
// float32

template<unsigned N>
class float32<N, expr_empty> : public any_float32<N, float32<N,expr_empty>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_FLOAT;
    using expr_type = expr_empty;
    float32<N> e;

    SIMDPP_INL float32(const float32<N>& a) : e(a) {}
    template<template<unsigned, class> class V, unsigned N2> SIMDPP_INL
    float32(const any_vec<N*4,V<N2,void>>& a) : e(a.wrapped()) {}

    SIMDPP_INL operator float32<N>() const { return e; }
    SIMDPP_INL float32<N> eval() const { return e; }
};

template<unsigned N>
class mask_float32<N, expr_empty> : public any_float32<N, mask_float32<N,expr_empty>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_FLOAT;
    using expr_type = expr_empty;
    mask_float32<N> e;

    SIMDPP_INL mask_float32(const mask_float32<N>& a) : e(a) {}

    SIMDPP_INL operator mask_float32<N>() const { return e; }
    SIMDPP_INL operator float32<N>() const { return e; }
    SIMDPP_INL mask_float32<N> eval() const { return e; }
};

// -----------------------------------------------------------------------------
// float64

template<unsigned N>
class float64<N, expr_empty> : public any_float64<N, float64<N,expr_empty>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_FLOAT;
    using expr_type = expr_empty;
    float64<N> e;

    SIMDPP_INL float64(const float64<N>& a) : e(a) {}
    template<template<unsigned, class> class V, unsigned N2> SIMDPP_INL
    float64(const any_vec<N*8,V<N2,void>>& a) : e(a.wrapped()) {}

    SIMDPP_INL operator float64<N>() const { return e; }
    SIMDPP_INL float64<N> eval() const { return e; }
};

template<unsigned N>
class mask_float64<N, expr_empty> : public any_float64<N, mask_float64<N,expr_empty>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_FLOAT;
    using expr_type = expr_empty;
    mask_float64<N> e;

    SIMDPP_INL mask_float64(const mask_float64<N>& a) : e(a) {}

    SIMDPP_INL operator mask_float64<N>() const { return e; }
    SIMDPP_INL operator float64<N>() const { return e; }
    SIMDPP_INL mask_float64<N> eval() const { return e; }
};

// -----------------------------------------------------------------------------
// uint8<N>
template<unsigned N>
class int8<N, expr_empty> : public any_int8<N, int8<N,expr_empty>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_INT;
    using expr_type = expr_empty;
    int8<N> e;

    SIMDPP_INL int8(const int8<N>& a) : e(a) {}
    template<template<unsigned, class> class V, unsigned N2> SIMDPP_INL
    int8(const any_vec<N,V<N2,void>>& a) : e(a.wrapped()) {}

    SIMDPP_INL operator int8<N>() const { return e; }
    SIMDPP_INL int8<N> eval() const { return e; }
};


template<unsigned N>
class uint8<N, expr_empty> : public any_int8<N, uint8<N,expr_empty>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_UINT;
    using expr_type = expr_empty;
    uint8<N> e;

    SIMDPP_INL uint8(const uint8<N>& a) : e(a) {}
    template<template<unsigned, class> class V, unsigned N2> SIMDPP_INL
    uint8(const any_vec<N,V<N2,void>>& a) : e(a.wrapped()) {}

    SIMDPP_INL operator uint8<N>() const { return e; }
    SIMDPP_INL uint8<N> eval() const { return e; }
};

template<unsigned N>
class mask_int8<N, expr_empty> : public any_int8<N, mask_int8<N,expr_empty>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_INT;
    using expr_type = expr_empty;
    mask_int8<N> e;

    SIMDPP_INL mask_int8(const mask_int8<N>& a) : e(a) {}

    SIMDPP_INL operator mask_int8<N>() const { return e; }
    SIMDPP_INL operator uint8<N>() const { return e; }
    SIMDPP_INL mask_int8<N> eval() const { return e; }
};

// -----------------------------------------------------------------------------
// uint16<N>
template<unsigned N>
class int16<N, expr_empty> : public any_int16<N, int16<N,expr_empty>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_INT;
    using expr_type = expr_empty;
    int16<N> e;

    SIMDPP_INL int16(const int16<N>& a) : e(a) {}
    template<template<unsigned, class> class V, unsigned N2> SIMDPP_INL
    int16(const any_vec<N*2,V<N2,void>>& a) : e(a.wrapped()) {}

    SIMDPP_INL operator int16<N>() const { return e; }
    SIMDPP_INL int16<N> eval() const { return e; }
};


template<unsigned N>
class uint16<N, expr_empty> : public any_int16<N, uint16<N,expr_empty>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_UINT;
    using expr_type = expr_empty;
    uint16<N> e;

    SIMDPP_INL uint16(const uint16<N>& a) : e(a) {}
    template<template<unsigned, class> class V, unsigned N2> SIMDPP_INL
    uint16(const any_vec<N*2,V<N2,void>>& a) : e(a.wrapped()) {}

    SIMDPP_INL operator uint16<N>() const { return e; }
    SIMDPP_INL uint16<N> eval() const { return e; }
};

template<unsigned N>
class mask_int16<N, expr_empty> : public any_int16<N, mask_int16<N,expr_empty>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_INT;
    using expr_type = expr_empty;
    mask_int16<N> e;

    SIMDPP_INL mask_int16(const mask_int16<N>& a) : e(a) {}

    SIMDPP_INL operator mask_int16<N>() const { return e; }
    SIMDPP_INL operator uint16<N>() const { return e; }
    SIMDPP_INL mask_int16<N> eval() const { return e; }
};

// -----------------------------------------------------------------------------
// uint32<N>
template<unsigned N>
class int32<N, expr_empty> : public any_int32<N, int32<N,expr_empty>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_INT;
    using expr_type = expr_empty;
    int32<N> e;

    SIMDPP_INL int32(const int32<N>& a) : e(a) {}
    template<template<unsigned, class> class V, unsigned N2> SIMDPP_INL
    int32(const any_vec<N*4,V<N2,void>>& a) : e(a.wrapped()) {}

    SIMDPP_INL operator int32<N>() const { return e; }
    SIMDPP_INL int32<N> eval() const { return e; }
};


template<unsigned N>
class uint32<N, expr_empty> : public any_int32<N, uint32<N,expr_empty>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_UINT;
    using expr_type = expr_empty;
    uint32<N> e;

    SIMDPP_INL uint32(const uint32<N>& a) : e(a) {}
    template<template<unsigned, class> class V, unsigned N2> SIMDPP_INL
    uint32(const any_vec<N*4,V<N2,void>>& a) : e(a.wrapped()) {}

    SIMDPP_INL operator uint32<N>() const { return e; }
    SIMDPP_INL uint32<N> eval() const { return e; }
};

template<unsigned N>
class mask_int32<N, expr_empty> : public any_int32<N, mask_int32<N,expr_empty>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_INT;
    using expr_type = expr_empty;
    mask_int32<N> e;

    SIMDPP_INL mask_int32(const mask_int32<N>& a) : e(a) {}

    SIMDPP_INL operator mask_int32<N>() const { return e; }
    SIMDPP_INL operator uint32<N>() const { return e; }
    SIMDPP_INL mask_int32<N> eval() const { return e; }
};

// -----------------------------------------------------------------------------
// int64<N>

template<unsigned N>
class int64<N, expr_empty> : public any_int64<N, int64<N,expr_empty>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_INT;
    using expr_type = expr_empty;
    int64<N> e;

    SIMDPP_INL int64(const int64<N>& a) : e(a) {}
    template<template<unsigned, class> class V, unsigned N2> SIMDPP_INL
    int64(const any_vec<N*8,V<N2,void>>& a) : e(a.wrapped()) {}

    SIMDPP_INL operator int64<N>() const { return e; }
    SIMDPP_INL int64<N> eval() const { return e; }
};


template<unsigned N>
class uint64<N, expr_empty> : public any_int64<N, uint64<N,expr_empty>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_UINT;
    using expr_type = expr_empty;
    uint64<N> e;

    SIMDPP_INL uint64(const uint64<N>& a) : e(a) {}
    template<template<unsigned, class> class V, unsigned N2> SIMDPP_INL
    uint64(const any_vec<N*8,V<N2,void>>& a) : e(a.wrapped()) {}

    SIMDPP_INL operator uint64<N>() const { return e; }
    SIMDPP_INL uint64<N> eval() const { return e; }
};


template<unsigned N>
class mask_int64<N, expr_empty> : public any_int64<N, mask_int64<N,expr_empty>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_INT;
    using expr_type = expr_empty;
    mask_int64<N> e;

    SIMDPP_INL mask_int64(const mask_int64<N>& a) : e(a) {}

    SIMDPP_INL operator mask_int64<N>() const { return e; }
    SIMDPP_INL operator uint64<N>() const { return e; }
    SIMDPP_INL mask_int64<N> eval() const { return e; }
};

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

