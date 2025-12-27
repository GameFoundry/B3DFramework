/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_TYPES_INT32X16_H
#define LIBSIMDPP_SIMDPP_TYPES_INT32X16_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/setup_arch.h>
#include <simdpp/types/fwd.h>
#include <simdpp/types/any.h>
#include <simdpp/detail/construct_eval.h>
#include <cstdint>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

#if SIMDPP_USE_AVX512F

/** Class representing 8x 32-bit signed integer vector
*/
template<>
class int32<16, void> : public any_int32<16, int32<16,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_INT;
    using element_type = int32_t;
    using base_vector_type = int32<16,void>;
    using expr_type = void;

#if SIMDPP_USE_AVX512F
    using native_type = __m512i;
#endif

    SIMDPP_INL int32<16>() = default;
    SIMDPP_INL int32<16>(const int32<16> &) = default;
    SIMDPP_INL int32<16> &operator=(const int32<16> &) = default;

    template<class E> SIMDPP_INL int32<16>(const int32<16,E>& d) { *this = d.eval(); }
    template<class E> SIMDPP_INL int32<16>(const uint32<16,E>& d) { *this = d.eval(); }
    template<class V> SIMDPP_INL explicit int32<16>(const any_vec<64,V>& d)
    {
        *this = bit_cast<int32<16>>(d.wrapped().eval());
    }
    template<class V> SIMDPP_INL int32<16>& operator=(const any_vec<64,V>& d)
    {
        *this = bit_cast<int32<16>>(d.wrapped().eval()); return *this;
    }

    /// Construct from the underlying vector type
    SIMDPP_INL int32<16>(const native_type& d) : d_(d) {}
    SIMDPP_INL int32<16>& operator=(const native_type& d) { d_ = d; return *this; }

    /// Convert to the underlying vector type
#if !SIMDPP_DISABLE_DEPRECATED_CONVERSION_OPERATOR_TO_NATIVE_TYPES
    SIMDPP_INL operator native_type() const SIMDPP_IMPLICIT_CONVERSION_DEPRECATION_MSG
    { return d_; }
#endif
    SIMDPP_INL native_type native() const { return d_; }

    template<class E> SIMDPP_INL int32<16>(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr());
    }
    template<class E> SIMDPP_INL int32<16>& operator=(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr()); return *this;
    }

    /// Access base vectors
    SIMDPP_INL const int32<16>& vec(unsigned) const { return *this; }
    SIMDPP_INL int32<16>& vec(unsigned)       { return *this; }

    SIMDPP_INL int32<16> eval() const { return *this; }

private:
    native_type d_;
};

/** Class representing 8x 32-bit unsigned integer vector
*/
template<>
class uint32<16, void> : public any_int32<16, uint32<16,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_UINT;
    using element_type = uint32_t;
    using base_vector_type = uint32<16,void>;
    using expr_type = void;

#if SIMDPP_USE_AVX512F
    using native_type = __m512i;
#endif

    SIMDPP_INL uint32<16>() = default;
    SIMDPP_INL uint32<16>(const uint32<16> &) = default;
    SIMDPP_INL uint32<16> &operator=(const uint32<16> &) = default;

    template<class E> SIMDPP_INL uint32<16>(const uint32<16,E>& d) { *this = d.eval(); }
    template<class E> SIMDPP_INL uint32<16>(const int32<16,E>& d) { *this = d.eval(); }
    template<class V> SIMDPP_INL explicit uint32<16>(const any_vec<64,V>& d)
    {
        *this = bit_cast<uint32<16>>(d.wrapped().eval());
    }
    template<class V> SIMDPP_INL uint32<16>& operator=(const any_vec<64,V>& d)
    {
        *this = bit_cast<uint32<16>>(d.wrapped().eval()); return *this;
    }

    /// Construct from the underlying vector type
    SIMDPP_INL uint32<16>(const native_type& d) : d_(d) {}
    SIMDPP_INL uint32<16>& operator=(const native_type& d) { d_ = d; return *this; }

    /// Convert to the underlying vector type
#if !SIMDPP_DISABLE_DEPRECATED_CONVERSION_OPERATOR_TO_NATIVE_TYPES
    SIMDPP_INL operator native_type() const SIMDPP_IMPLICIT_CONVERSION_DEPRECATION_MSG
    { return d_; }
#endif
    SIMDPP_INL native_type native() const { return d_; }

    template<class E> SIMDPP_INL uint32<16>(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr());
    }
    template<class E> SIMDPP_INL uint32<16>& operator=(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr()); return *this;
    }

    /// Access base vectors
    SIMDPP_INL const uint32<16>& vec(unsigned) const { return *this; }
    SIMDPP_INL uint32<16>& vec(unsigned)       { return *this; }

    SIMDPP_INL uint32<16> eval() const { return *this; }

private:
    native_type d_;
};

/// Class representing possibly optimized mask data for 4x 32-bit integer
/// vector
template<>
class mask_int32<16, void> : public any_int32<16, mask_int32<16,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_INT;
    using base_vector_type = mask_int32<16,void>;
    using expr_type = void;

#if SIMDPP_USE_AVX512F
    using native_type = __mmask16;
#endif

    SIMDPP_INL mask_int32<16>() = default;
    SIMDPP_INL mask_int32<16>(const mask_int32<16> &) = default;
    SIMDPP_INL mask_int32<16> &operator=(const mask_int32<16> &) = default;

    SIMDPP_INL mask_int32<16>(const native_type& d) : d_(d) {}

    template<class E> SIMDPP_INL explicit mask_int32<16>(const mask_float32<16,E>& d)
    {
        *this = bit_cast<mask_int32<16>>(d.eval());
    }
    template<class E> SIMDPP_INL mask_int32<16>& operator=(const mask_float32<16,E>& d)
    {
        *this = bit_cast<mask_int32<16>>(d.eval()); return *this;
    }

    /// Convert to the underlying vector type
#if !SIMDPP_DISABLE_DEPRECATED_CONVERSION_OPERATOR_TO_NATIVE_TYPES
    SIMDPP_INL operator native_type() const SIMDPP_IMPLICIT_CONVERSION_DEPRECATION_MSG
    { return d_; }
#endif
    SIMDPP_INL native_type native() const { return d_; }

    /// Access the underlying type
    SIMDPP_INL uint32<16> unmask() const
    {
    #if SIMDPP_USE_AVX512DQ
        return _mm512_movm_epi32(d_);
    #elif SIMDPP_USE_AVX512F
        return _mm512_maskz_set1_epi32(d_, 0xffffffff);
    #endif
    }

    SIMDPP_INL const mask_int32<16>& vec(unsigned) const { return *this; }
    SIMDPP_INL mask_int32<16>& vec(unsigned)       { return *this; }

    SIMDPP_INL mask_int32<16> eval() const { return *this; }

private:
    native_type d_;
};

#endif // SIMDPP_USE_AVX512F

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
