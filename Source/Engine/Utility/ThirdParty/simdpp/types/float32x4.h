/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_TYPES_FLOAT32X4_H
#define LIBSIMDPP_SIMDPP_TYPES_FLOAT32X4_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/setup_arch.h>
#include <simdpp/capabilities.h>
#include <simdpp/types/fwd.h>
#include <simdpp/types/any.h>
#include <simdpp/types/int32x4.h>
#include <simdpp/core/cast.h>
#include <simdpp/detail/construct_eval.h>
#include <simdpp/detail/array.h>
#include <simdpp/detail/null/mask.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/// Class representing float32x4 vector
template<>
class float32<4, void> : public any_float32<4, float32<4,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_FLOAT;
    using base_vector_type = float32<4,void>;
    using expr_type = void;

#if SIMDPP_USE_SSE2
    using native_type = __m128;
#elif SIMDPP_USE_NEON_FLT_SP
    using native_type = float32x4_t;
#elif SIMDPP_USE_ALTIVEC
    using native_type = __vector float;
#elif SIMDPP_USE_MSA
    using native_type = v4f32;
#else // NULL && (NEON && !FLT_SP)
    using native_type = detail::vararray<float,4>;
#endif

    SIMDPP_INL float32<4>() = default;
    SIMDPP_INL float32<4>(const float32<4> &) = default;
    SIMDPP_INL float32<4> &operator=(const float32<4> &) = default;

    template<class E> SIMDPP_INL float32<4>(const float32<4,E>& d) { *this = d.eval(); }
    template<class V> SIMDPP_INL explicit float32<4>(const any_vec<16,V>& d)
    {
        *this = bit_cast<float32<4>>(d.wrapped().eval());
    }
    template<class V> SIMDPP_INL float32<4>& operator=(const any_vec<16,V>& d)
    {
        *this = bit_cast<float32<4>>(d.wrapped().eval()); return *this;
    }

    /// Construct from the underlying vector type
    SIMDPP_INL float32<4>(const native_type& d) : d_(d) {}
    SIMDPP_INL float32<4>& operator=(const native_type& d) { d_ = d; return *this; }

    /// Convert to the underlying vector type
#if !SIMDPP_DISABLE_DEPRECATED_CONVERSION_OPERATOR_TO_NATIVE_TYPES
    SIMDPP_INL operator native_type() const SIMDPP_IMPLICIT_CONVERSION_DEPRECATION_MSG
    { return d_; }
#endif
    SIMDPP_INL native_type native() const { return d_; }

#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    SIMDPP_INL float& el(unsigned id) { return d_[id]; }
    SIMDPP_INL const float& el(unsigned id) const { return d_[id]; }
#endif

    template<class E> SIMDPP_INL float32<4>(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr());
    }
    template<class E> SIMDPP_INL float32<4>& operator=(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr()); return *this;
    }

    /// Access base vectors
    SIMDPP_INL const float32<4>& vec(unsigned) const { return *this; }
    SIMDPP_INL float32<4>& vec(unsigned)       { return *this; }

    SIMDPP_INL float32<4> eval() const { return *this; }

private:
#if SIMDPP_ARM && !SIMDPP_HAS_FLOAT32_SIMD
    // Force alignment to be the same as of the real vector types. Different
    // alignment causes problems when types such as int32<4> are casted to
    // float32<4> or vice-versa - GCC assumes that some data on stack has
    // higher alignment than it really has and uses aligned load or store which
    // causes crashes.
    SIMDPP_ALIGN(8) native_type d_;
#else
    native_type d_;
#endif
};

/// Class representing possibly optimized mask data for 4x 32-bit floating-point
/// vector
template<>
class mask_float32<4, void> : public any_float32<4, mask_float32<4,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_FLOAT;
    using base_vector_type = mask_float32<4,void>;
    using expr_type = void;

#if SIMDPP_USE_AVX512VL
    using native_type = __mmask8;
#elif SIMDPP_USE_SSE2
    using native_type = __m128;
#elif SIMDPP_USE_NEON_FLT_SP
    using native_type = float32x4_t;
#elif SIMDPP_USE_ALTIVEC
    using native_type = __vector float;
#elif SIMDPP_USE_MSA
    using native_type = v4f32;
#else // NULL || (NEON && !FLT_SP)
    using native_type = detail::vararray<uint8_t,4>;
#endif
    SIMDPP_INL mask_float32<4>() = default;
    SIMDPP_INL mask_float32<4>(const mask_float32<4> &) = default;
    SIMDPP_INL mask_float32<4> &operator=(const mask_float32<4> &) = default;

    SIMDPP_INL mask_float32<4>(const native_type& d) : d_(d) {}

#if SIMDPP_USE_ALTIVEC
    SIMDPP_INL mask_float32<4>(const __vector __bool int& d) : d_((__vector float)d) {}
#endif

#if SIMDPP_USE_SSE2 && !SIMDPP_USE_AVX512VL
    SIMDPP_INL mask_float32<4>(const float32<4>& d) : d_(d.native()) {}
#endif

    template<class E> SIMDPP_INL explicit mask_float32<4>(const mask_int32<4,E>& d)
    {
        *this = bit_cast<mask_float32<4>>(d.eval());
    }
    template<class E> SIMDPP_INL mask_float32<4>& operator=(const mask_int32<4,E>& d)
    {
        *this = bit_cast<mask_float32<4>>(d.eval()); return *this;
    }

    /// Convert to the underlying vector type
#if !SIMDPP_DISABLE_DEPRECATED_CONVERSION_OPERATOR_TO_NATIVE_TYPES
    SIMDPP_INL operator native_type() const SIMDPP_IMPLICIT_CONVERSION_DEPRECATION_MSG
    { return d_; }
#endif
    SIMDPP_INL native_type native() const { return d_; }

    /// Access the underlying type
    SIMDPP_INL float32<4> unmask() const
    {
#if SIMDPP_USE_AVX512VL
        return _mm_castsi128_ps(_mm_movm_epi32(d_));
#elif SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
        return detail::null::unmask_mask<float32<4>>(*this);
#else
        return float32<4>(d_);
#endif
    }

#if SIMDPP_USE_NULL || SIMDPP_USE_NEON_NO_FLT_SP
    SIMDPP_INL uint8_t& el(unsigned id) { return d_[id]; }
    SIMDPP_INL const uint8_t& el(unsigned id) const { return d_[id]; }
#endif

    SIMDPP_INL const mask_float32<4>& vec(unsigned) const { return *this; }
    SIMDPP_INL mask_float32<4>& vec(unsigned)       { return *this; }

    SIMDPP_INL mask_float32<4> eval() const { return *this; }

private:
    native_type d_;
};

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
