/*  Copyright (C) 2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_EXPR_VEC_H
#define LIBSIMDPP_SIMDPP_DETAIL_EXPR_VEC_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

template<class R, unsigned N>
struct expr_eval<R, uint8<N>> {
    static SIMDPP_INL R eval(const uint8<N>& e) { return (R) e; }
};
template<class R, unsigned N>
struct expr_eval<R, int8<N>> {
    static SIMDPP_INL R eval(const int8<N>& e) { return (R) e; }
};
template<class R, unsigned N>
struct expr_eval<R, mask_int8<N>> {
    static SIMDPP_INL R eval(const mask_int8<N>& e) { return (R) e; }
};

template<class R, unsigned N>
struct expr_eval<R, uint16<N>> {
    static SIMDPP_INL R eval(const uint16<N>& e) { return (R) e; }
};
template<class R, unsigned N>
struct expr_eval<R, int16<N>> {
    static SIMDPP_INL R eval(const int16<N>& e) { return (R) e; }
};
template<class R, unsigned N>
struct expr_eval<R, mask_int16<N>> {
    static SIMDPP_INL R eval(const mask_int16<N>& e) { return (R) e; }
};

template<class R, unsigned N>
struct expr_eval<R, uint32<N>> {
    static SIMDPP_INL R eval(const uint32<N>& e) { return (R) e; }
};
template<class R, unsigned N>
struct expr_eval<R, int32<N>> {
    static SIMDPP_INL R eval(const int32<N>& e) { return (R) e; }
};
template<class R, unsigned N>
struct expr_eval<R, mask_int32<N>> {
    static SIMDPP_INL R eval(const mask_int32<N>& e) { return (R) e; }
};

template<class R, unsigned N>
struct expr_eval<R, uint64<N>> {
    static SIMDPP_INL R eval(const uint64<N>& e) { return (R) e; }
};
template<class R, unsigned N>
struct expr_eval<R, int64<N>> {
    static SIMDPP_INL R eval(const int64<N>& e) { return (R) e; }
};
template<class R, unsigned N>
struct expr_eval<R, mask_int64<N>> {
    static SIMDPP_INL R eval(const mask_int64<N>& e) { return (R) e; }
};

template<class R, unsigned N>
struct expr_eval<R, float32<N>> {
    static SIMDPP_INL R eval(const float32<N>& e) { return (R) e; }
};
template<class R, unsigned N>
struct expr_eval<R, mask_float32<N>> {
    static SIMDPP_INL R eval(const mask_float32<N>& e) { return (R) e; }
};

template<class R, unsigned N>
struct expr_eval<R, float64<N>> {
    static SIMDPP_INL R eval(const float64<N>& e) { return (R) e; }
};
template<class R, unsigned N>
struct expr_eval<R, mask_float64<N>> {
    static SIMDPP_INL R eval(const mask_float64<N>& e) { return (R) e; }
};

template<class R, unsigned N>
struct expr_eval<R, uint8<N, expr_empty>> {
    static SIMDPP_INL R eval(const uint8<N, expr_empty>& e) { return (R) e.e; }
};
template<class R, unsigned N>
struct expr_eval<R, int8<N, expr_empty>> {
    static SIMDPP_INL R eval(const int8<N, expr_empty>& e) { return (R) e.e; }
};
template<class R, unsigned N>
struct expr_eval<R, mask_int8<N, expr_empty>> {
    static SIMDPP_INL R eval(const mask_int8<N, expr_empty>& e) { return (R) e.e; }
};

template<class R, unsigned N>
struct expr_eval<R, uint16<N, expr_empty>> {
    static SIMDPP_INL R eval(const uint16<N, expr_empty>& e) { return (R) e.e; }
};
template<class R, unsigned N>
struct expr_eval<R, int16<N, expr_empty>> {
    static SIMDPP_INL R eval(const int16<N, expr_empty>& e) { return (R) e.e; }
};
template<class R, unsigned N>
struct expr_eval<R, mask_int16<N, expr_empty>> {
    static SIMDPP_INL R eval(const mask_int16<N, expr_empty>& e) { return (R) e.e; }
};

template<class R, unsigned N>
struct expr_eval<R, uint32<N, expr_empty>> {
    static SIMDPP_INL R eval(const uint32<N, expr_empty>& e) { return (R) e.e; }
};
template<class R, unsigned N>
struct expr_eval<R, int32<N, expr_empty>> {
    static SIMDPP_INL R eval(const int32<N, expr_empty>& e) { return (R) e.e; }
};
template<class R, unsigned N>
struct expr_eval<R, mask_int32<N, expr_empty>> {
    static SIMDPP_INL R eval(const mask_int32<N, expr_empty>& e) { return (R) e.e; }
};

template<class R, unsigned N>
struct expr_eval<R, uint64<N, expr_empty>> {
    static SIMDPP_INL R eval(const uint64<N, expr_empty>& e) { return (R) e.e; }
};
template<class R, unsigned N>
struct expr_eval<R, int64<N, expr_empty>> {
    static SIMDPP_INL R eval(const int64<N, expr_empty>& e) { return (R) e.e; }
};
template<class R, unsigned N>
struct expr_eval<R, mask_int64<N, expr_empty>> {
    static SIMDPP_INL R eval(const mask_int64<N, expr_empty>& e) { return (R) e.e; }
};

template<class R, unsigned N>
struct expr_eval<R, float32<N, expr_empty>> {
    static SIMDPP_INL R eval(const float32<N, expr_empty>& e) { return (R) e.e; }
};
template<class R, unsigned N>
struct expr_eval<R, mask_float32<N, expr_empty>> {
    static SIMDPP_INL R eval(const mask_float32<N, expr_empty>& e) { return (R) e.e; }
};

template<class R, unsigned N>
struct expr_eval<R, float64<N, expr_empty>> {
    static SIMDPP_INL R eval(const float64<N, expr_empty>& e) { return (R) e.e; }
};
template<class R, unsigned N>
struct expr_eval<R, mask_float64<N, expr_empty>> {
    static SIMDPP_INL R eval(const mask_float64<N, expr_empty>& e) { return (R) e.e; }
};


} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
