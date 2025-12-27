/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_TYPES_FLOAT64_H
#define LIBSIMDPP_SIMDPP_TYPES_FLOAT64_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/setup_arch.h>
#include <simdpp/types/any.h>
#include <simdpp/types/fwd.h>
#include <simdpp/core/cast.h>
#include <simdpp/detail/array.h>
#include <simdpp/detail/construct_eval.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Class representing a @a float64 vector of arbitrary length. The vector
    always contains at least one native vector.
*/
template<unsigned N>
class float64<N, void> : public any_float64<N, float64<N,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_FLOAT;
    using element_type = double;
    using base_vector_type = float64v;
    using expr_type = void;

    SIMDPP_INL float64<N>() = default;
    SIMDPP_INL float64<N>(const float64<N>&) = default;
    SIMDPP_INL float64<N>& operator=(const float64<N>&) = default;

    template<class E> SIMDPP_INL float64<N>(const float64<N,E>& d) { *this = d.eval(); }
    template<class V> SIMDPP_INL explicit float64<N>(const any_vec<N*8,V>& d)
    {
        *this = bit_cast<float64<N>>(d.wrapped().eval());
    }
    template<class V> SIMDPP_INL float64<N>& operator=(const any_vec<N*8,V>& d)
    {
        *this = bit_cast<float64<N>>(d.wrapped().eval()); return *this;
    }

    template<class E> SIMDPP_INL float64<N>(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr());
    }
    template<class E> SIMDPP_INL float64<N>& operator=(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr()); return *this;
    }

    SIMDPP_INL const float64v& vec(unsigned i) const { return d_[i]; }
    SIMDPP_INL float64v& vec(unsigned i)             { return d_[i]; }

    SIMDPP_INL float64<N> eval() const { return *this; }

private:
    /** Creates a float vector from a native vector

        @code
        r0 = v0
        ...
        rN = v0
        @endcode
    */
    static SIMDPP_INL float64<N> set_vec(const float64v& v0)
    {
        float64<N> r;
        for (unsigned i = 0; i < r.vec_length; i++) r.vec(i) = v0;
        return r;
    }

    detail::vararray<float64v, float64::vec_length> d_;
};

/// Class representing possibly optimized mask data for 2x 64-bit floating point
/// vector
template<unsigned N>
class mask_float64<N, void> : public any_float64<N, mask_float64<N,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_FLOAT;
    using base_vector_type = mask_float64v;
    using expr_type = void;

    SIMDPP_INL mask_float64<N>() = default;
    SIMDPP_INL mask_float64<N>(const mask_float64<N> &) = default;
    SIMDPP_INL mask_float64<N> &operator=(const mask_float64<N> &) = default;

    template<class E> SIMDPP_INL explicit mask_float64<N>(const mask_int64<N,E>& d)
    {
        *this = bit_cast<mask_float64<N>>(d.eval());
    }
    template<class E> SIMDPP_INL mask_float64<N>& operator=(const mask_int64<N,E>& d)
    {
        *this = bit_cast<mask_float64<N>>(d.eval()); return *this;
    }

    /// Access the underlying type
    SIMDPP_INL float64<N> unmask() const
    {
        float64<N> r;
        for (unsigned i = 0; i < mask_float64::vec_length; ++i) {
            r.vec(i) = d_[i].unmask();
        }
        return r;
    }

    SIMDPP_INL const mask_float64v& vec(unsigned i) const { return d_[i]; }
    SIMDPP_INL mask_float64v& vec(unsigned i)       { return d_[i]; }

    SIMDPP_INL mask_float64<N> eval() const { return *this; }

private:
    detail::vararray<mask_float64v, mask_float64::vec_length> d_;
};

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
