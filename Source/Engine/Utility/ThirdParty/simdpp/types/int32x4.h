/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_TYPES_INT32X4_H
#define LIBSIMDPP_SIMDPP_TYPES_INT32X4_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/setup_arch.h>
#include <simdpp/types/fwd.h>
#include <simdpp/types/any.h>
#include <simdpp/core/cast.h>
#include <simdpp/detail/construct_eval.h>
#include <simdpp/detail/array.h>
#include <simdpp/detail/null/mask.h>
#include <cstdint>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Class representing 4x 32-bit signed integer vector
*/
template<>
class int32<4, void> : public any_int32<4, int32<4,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_INT;
    using element_type = int32_t;
    using base_vector_type = int32<4,void>;
    using expr_type = void;

#if SIMDPP_USE_SSE2
    using native_type = __m128i;
#elif SIMDPP_USE_NEON
    using native_type = int32x4_t;
#elif SIMDPP_USE_ALTIVEC
    using native_type = __vector int32_t;
#elif SIMDPP_USE_MSA
    using native_type = v4i32;
#else
    using native_type = detail::vararray<int32_t,4>;
#endif

    SIMDPP_INL int32<4>() = default;
    SIMDPP_INL int32<4>(const int32<4> &) = default;
    SIMDPP_INL int32<4> &operator=(const int32<4> &) = default;

    template<class E> SIMDPP_INL int32<4>(const int32<4,E>& d) { *this = d.eval(); }
    template<class E> SIMDPP_INL int32<4>(const uint32<4,E>& d) { *this = d.eval(); }
    template<class V> SIMDPP_INL explicit int32<4>(const any_vec<16,V>& d)
    {
        *this = bit_cast<int32<4>>(d.wrapped().eval());
    }
    template<class V> SIMDPP_INL int32<4>& operator=(const any_vec<16,V>& d)
    {
        *this = bit_cast<int32<4>>(d.wrapped().eval()); return *this;
    }

    /// Construct from the underlying vector type
    SIMDPP_INL int32<4>(const native_type& d) : d_(d) {}
    SIMDPP_INL int32<4>& operator=(const native_type& d) { d_ = d; return *this; }

    /// Convert to the underlying vector type
#if !SIMDPP_DISABLE_DEPRECATED_CONVERSION_OPERATOR_TO_NATIVE_TYPES
    SIMDPP_INL operator native_type() const SIMDPP_IMPLICIT_CONVERSION_DEPRECATION_MSG
    { return d_; }
#endif
    SIMDPP_INL native_type native() const { return d_; }

    template<class E> SIMDPP_INL int32<4>(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr());
    }
    template<class E> SIMDPP_INL int32<4>& operator=(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr()); return *this;
    }

    /// Access base vectors
    SIMDPP_INL const int32<4>& vec(unsigned) const { return *this; }
    SIMDPP_INL int32<4>& vec(unsigned)       { return *this; }

    SIMDPP_INL int32<4> eval() const { return *this; }

#if SIMDPP_USE_NULL
    /// For internal use only
    SIMDPP_INL const int32_t& el(unsigned i) const  { return d_[i]; }
    SIMDPP_INL int32_t& el(unsigned i) { return d_[i]; }
#endif

private:
    native_type d_;
};

/** Class representing 4x 32-bit unsigned integer vector
*/
template<>
class uint32<4, void> : public any_int32<4, uint32<4,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_UINT;
    using element_type = uint32_t;
    using base_vector_type = uint32<4,void>;
    using expr_type = void;

#if SIMDPP_USE_SSE2
    using native_type = __m128i;
#elif SIMDPP_USE_NEON
    using native_type = uint32x4_t;
#elif SIMDPP_USE_ALTIVEC
    using native_type = __vector uint32_t;
#elif SIMDPP_USE_MSA
    using native_type = v4u32;
#else
    using native_type = detail::vararray<uint32_t,4>;
#endif

    SIMDPP_INL uint32<4>() = default;
    SIMDPP_INL uint32<4>(const uint32<4> &) = default;
    SIMDPP_INL uint32<4> &operator=(const uint32<4> &) = default;

    template<class E> SIMDPP_INL uint32<4>(const uint32<4,E>& d) { *this = d.eval(); }
    template<class E> SIMDPP_INL uint32<4>(const int32<4,E>& d) { *this = d.eval(); }
    template<class V> SIMDPP_INL explicit uint32<4>(const any_vec<16,V>& d)
    {
        *this = bit_cast<uint32<4>>(d.wrapped().eval());
    }
    template<class V> SIMDPP_INL uint32<4>& operator=(const any_vec<16,V>& d)
    {
        *this = bit_cast<uint32<4>>(d.wrapped().eval()); return *this;
    }

    /// Construct from the underlying vector type
    SIMDPP_INL uint32<4>(const native_type& d) : d_(d) {}
    SIMDPP_INL uint32<4>& operator=(const native_type& d) { d_ = d; return *this; }

    /// Convert to the underlying vector type
#if !SIMDPP_DISABLE_DEPRECATED_CONVERSION_OPERATOR_TO_NATIVE_TYPES
    SIMDPP_INL operator native_type() const SIMDPP_IMPLICIT_CONVERSION_DEPRECATION_MSG
    { return d_; }
#endif
    SIMDPP_INL native_type native() const { return d_; }

    template<class E> SIMDPP_INL uint32<4>(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr());
    }
    template<class E> SIMDPP_INL uint32<4>& operator=(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr()); return *this;
    }

    /// Access base vectors
    SIMDPP_INL const uint32<4>& vec(unsigned) const { return *this; }
    SIMDPP_INL uint32<4>& vec(unsigned)       { return *this; }

    SIMDPP_INL uint32<4> eval() const { return *this; }

#if SIMDPP_USE_NULL
    /// For uinternal use only
    SIMDPP_INL const uint32_t& el(unsigned i) const { return d_[i]; }
    SIMDPP_INL uint32_t& el(unsigned i) { return d_[i]; }
#endif

private:
    native_type d_;
};

/// Class representing possibly optimized mask data for 4x 32-bit integer
/// vector
template<>
class mask_int32<4, void> : public any_int32<4, mask_int32<4,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_INT;
    using base_vector_type = mask_int32<4,void>;
    using expr_type = void;

#if SIMDPP_USE_AVX512VL
    using native_type = __mmask8;
#elif SIMDPP_USE_SSE2
    using native_type = __m128i;
#elif SIMDPP_USE_NEON
    using native_type = uint32x4_t;
#elif SIMDPP_USE_ALTIVEC
    using native_type = __vector uint32_t;
#elif SIMDPP_USE_MSA
    using native_type = v4u32;
#else
    using native_type = detail::vararray<uint8_t,4>;
#endif

    SIMDPP_INL mask_int32<4>() = default;
    SIMDPP_INL mask_int32<4>(const mask_int32<4> &) = default;
    SIMDPP_INL mask_int32<4> &operator=(const mask_int32<4> &) = default;

    SIMDPP_INL mask_int32<4>(const native_type& d) : d_(d) {}

#if SIMDPP_USE_ALTIVEC
    SIMDPP_INL mask_int32<4>(const __vector __bool int& d) : d_((__vector uint32_t)d) {}
#endif

#if (SIMDPP_USE_SSE2 && !SIMDPP_USE_AVX512VL) || SIMDPP_USE_NEON || SIMDPP_USE_ALTIVEC
    SIMDPP_INL mask_int32<4>(const uint32<4>& d) : d_(d.native()) {}
#endif

    template<class E> SIMDPP_INL explicit mask_int32<4>(const mask_float32<4,E>& d)
    {
        *this = bit_cast<mask_int32<4>>(d.eval());
    }
    template<class E> SIMDPP_INL mask_int32<4>& operator=(const mask_float32<4,E>& d)
    {
        *this = bit_cast<mask_int32<4>>(d.eval()); return *this;
    }

    /// Convert to the underlying vector type
#if !SIMDPP_DISABLE_DEPRECATED_CONVERSION_OPERATOR_TO_NATIVE_TYPES
    SIMDPP_INL operator native_type() const SIMDPP_IMPLICIT_CONVERSION_DEPRECATION_MSG
    { return d_; }
#endif
    SIMDPP_INL native_type native() const { return d_; }

    /// Access the underlying type
    SIMDPP_INL uint32<4> unmask() const
    {
#if SIMDPP_USE_NULL
        return detail::null::unmask_mask<uint32<4>>(*this);
#elif SIMDPP_USE_AVX512VL
        return _mm_movm_epi32(d_);
#else
        return uint32<4>(d_);
#endif
    }

#if SIMDPP_USE_NULL
    SIMDPP_INL uint8_t& el(unsigned id) { return d_[id]; }
    SIMDPP_INL const uint8_t& el(unsigned id) const { return d_[id]; }
#endif

    SIMDPP_INL const mask_int32<4>& vec(unsigned) const { return *this; }
    SIMDPP_INL mask_int32<4>& vec(unsigned)       { return *this; }

    SIMDPP_INL mask_int32<4> eval() const { return *this; }

private:
    native_type d_;
};

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
