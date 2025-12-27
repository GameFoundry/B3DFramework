/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_TYPES_FLOAT32X8_H
#define LIBSIMDPP_SIMDPP_TYPES_FLOAT32X8_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/setup_arch.h>
#include <simdpp/types/fwd.h>
#include <simdpp/types/any.h>
#include <simdpp/types/int32x8.h>
#include <simdpp/types/float32x4.h>
#include <simdpp/detail/construct_eval.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

#if SIMDPP_USE_AVX

/// Class representing float32x8 vector
template<>
class float32<8, void> : public any_float32<8, float32<8,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_FLOAT;
    using base_vector_type = float32<8,void>;
    using expr_type = void;

#if SIMDPP_USE_AVX
    using native_type = __m256;
#endif

    SIMDPP_INL float32<8>() = default;
    SIMDPP_INL float32<8>(const float32<8> &) = default;
    SIMDPP_INL float32<8> &operator=(const float32<8> &) = default;

    template<class E> SIMDPP_INL float32<8>(const float32<8,E>& d) { *this = d.eval(); }
    template<class V> SIMDPP_INL explicit float32<8>(const any_vec<32,V>& d)
    {
        *this = bit_cast<float32<8>>(d.wrapped().eval());
    }
    template<class V> SIMDPP_INL float32<8>& operator=(const any_vec<32,V>& d)
    {
        *this = bit_cast<float32<8>>(d.wrapped().eval()); return *this;
    }

    /// Construct from the underlying vector type
    SIMDPP_INL float32<8>(const native_type& d) : d_(d) {}
    SIMDPP_INL float32<8>& operator=(const native_type& d) { d_ = d; return *this; }

    /// Convert to the underlying vector type
#if !SIMDPP_DISABLE_DEPRECATED_CONVERSION_OPERATOR_TO_NATIVE_TYPES
    SIMDPP_INL operator native_type() const SIMDPP_IMPLICIT_CONVERSION_DEPRECATION_MSG
    { return d_; }
#endif
    SIMDPP_INL native_type native() const { return d_; }

    template<class E> SIMDPP_INL float32<8>(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr());
    }
    template<class E> SIMDPP_INL float32<8>& operator=(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr()); return *this;
    }

    /// Access base vectors
    SIMDPP_INL const float32<8>& vec(unsigned) const { return *this; }
    SIMDPP_INL float32<8>& vec(unsigned)       { return *this; }

    SIMDPP_INL float32<8> eval() const { return *this; }

private:
    native_type d_;
};

/// Class representing possibly optimized mask data for 4x 32-bit floating-point
/// vector
template<>
class mask_float32<8, void> : public any_float32<8, mask_float32<8,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_FLOAT;
    using base_vector_type = mask_float32<8,void>;
    using expr_type = void;

#if SIMDPP_USE_AVX512VL
    using native_type = __mmask8;
#elif SIMDPP_USE_AVX
    using native_type = __m256;
#endif

    SIMDPP_INL mask_float32<8>() = default;
    SIMDPP_INL mask_float32<8>(const mask_float32<8> &) = default;
    SIMDPP_INL mask_float32<8> &operator=(const mask_float32<8> &) = default;

    SIMDPP_INL mask_float32<8>(const native_type& d) : d_(d) {}

#if SIMDPP_USE_AVX && !SIMDPP_USE_AVX512VL
    SIMDPP_INL mask_float32<8>(const float32<8>& d) : d_(d.native()) {}
#endif

    template<class E> SIMDPP_INL explicit mask_float32<8>(const mask_int32<8,E>& d)
    {
        *this = bit_cast<mask_float32<8>>(d.eval());
    }
    template<class E> SIMDPP_INL mask_float32<8>& operator=(const mask_int32<8,E>& d)
    {
        *this = bit_cast<mask_float32<8>>(d.eval()); return *this;
    }

    /// Convert to the underlying vector type
#if !SIMDPP_DISABLE_DEPRECATED_CONVERSION_OPERATOR_TO_NATIVE_TYPES
    SIMDPP_INL operator native_type() const SIMDPP_IMPLICIT_CONVERSION_DEPRECATION_MSG
    { return d_; }
#endif
    SIMDPP_INL native_type native() const { return d_; }

    /// Access the underlying type
    SIMDPP_INL float32<8> unmask() const
    {
#if SIMDPP_USE_AVX512VL
        return _mm256_castsi256_ps(_mm256_movm_epi32(d_));
#else
        return float32<8>(d_);
#endif
    }

    SIMDPP_INL const mask_float32<8>& vec(unsigned) const { return *this; }
    SIMDPP_INL mask_float32<8>& vec(unsigned)       { return *this; }

    SIMDPP_INL mask_float32<8> eval() const { return *this; }

private:
    native_type d_;
};

#endif // SIMDPP_USE_AVX

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
