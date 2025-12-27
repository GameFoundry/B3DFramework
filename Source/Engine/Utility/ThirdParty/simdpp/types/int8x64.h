/*  Copyright (C) 2012-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_TYPES_INT8X64_H
#define LIBSIMDPP_SIMDPP_TYPES_INT8X64_H

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

#if SIMDPP_USE_AVX512BW

/// @ingroup simd_vec_int
/// @{

/** Class representing 64x 8-bit signed integer vector
*/
template<>
class int8<64, void> : public any_int8<64, int8<64,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_INT;
    using element_type = int8_t;
    using base_vector_type = int8<64>;
    using expr_type = void;

#if SIMDPP_USE_AVX512BW
    using native_type = __m512i;
#endif

    SIMDPP_INL int8<64>() = default;
    SIMDPP_INL int8<64>(const int8<64> &) = default;
    SIMDPP_INL int8<64> &operator=(const int8<64> &) = default;

    template<class E> SIMDPP_INL int8<64>(const int8<64,E>& d) { *this = d.eval(); }
    template<class E> SIMDPP_INL int8<64>(const uint8<64,E>& d) { *this = d.eval(); }
    template<class V> SIMDPP_INL explicit int8<64>(const any_vec<64,V>& d)
    {
        *this = bit_cast<int8<64>>(d.wrapped().eval());
    }
    template<class V> SIMDPP_INL int8<64>& operator=(const any_vec<64,V>& d)
    {
        *this = bit_cast<int8<64>>(d.wrapped().eval()); return *this;
    }

    /// @{
    /// Construct from the underlying vector type
    SIMDPP_INL int8<64>(const native_type& d) : d_(d) {}
    SIMDPP_INL int8<64>& operator=(const native_type& d) { d_ = d; return *this; }
    /// @}

    /// Convert to the underlying vector type
#if !SIMDPP_DISABLE_DEPRECATED_CONVERSION_OPERATOR_TO_NATIVE_TYPES
    SIMDPP_INL operator native_type() const SIMDPP_IMPLICIT_CONVERSION_DEPRECATION_MSG
    { return d_; }
#endif
    SIMDPP_INL native_type native() const { return d_; }

    template<class E> SIMDPP_INL int8<64>(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr());
    }
    template<class E> SIMDPP_INL int8<64>& operator=(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr()); return *this;
    }

    /// @{
    /// Access base vectors
    SIMDPP_INL const int8<64>& vec(unsigned) const { return *this; }
    SIMDPP_INL int8<64>& vec(unsigned)       { return *this; }
    /// @}

    SIMDPP_INL int8<64> eval() const { return *this; }

private:
    native_type d_;
};

/** Class representing 64x 8-bit unsigned integer vector
*/
template<>
class uint8<64, void> : public any_int8<64, uint8<64,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_UINT;
    using element_type = uint8_t;
    using base_vector_type = uint8<64>;
    using expr_type = void;

#if SIMDPP_USE_AVX512BW
    using native_type = __m512i;
#endif

    SIMDPP_INL uint8<64>() = default;
    SIMDPP_INL uint8<64>(const uint8<64> &) = default;
    SIMDPP_INL uint8<64> &operator=(const uint8<64> &) = default;

    template<class E> SIMDPP_INL uint8<64>(const uint8<64,E>& d) { *this = d.eval(); }
    template<class E> SIMDPP_INL uint8<64>(const int8<64,E>& d) { *this = d.eval(); }
    template<class V> SIMDPP_INL explicit uint8<64>(const any_vec<64,V>& d)
    {
        *this = bit_cast<uint8<64>>(d.wrapped().eval());
    }
    template<class V> SIMDPP_INL uint8<64>& operator=(const any_vec<64,V>& d)
    {
        *this = bit_cast<uint8<64>>(d.wrapped().eval()); return *this;
    }

    /// @{
    /// Construct from the underlying vector type
    SIMDPP_INL uint8<64>(const native_type& d) : d_(d) {}
    SIMDPP_INL uint8<64>& operator=(const native_type& d) { d_ = d; return *this; }
    /// @}

    /// Convert to the underlying vector type
#if !SIMDPP_DISABLE_DEPRECATED_CONVERSION_OPERATOR_TO_NATIVE_TYPES
    SIMDPP_INL operator native_type() const SIMDPP_IMPLICIT_CONVERSION_DEPRECATION_MSG
    { return d_; }
#endif
    SIMDPP_INL native_type native() const { return d_; }

    template<class E> SIMDPP_INL uint8<64>(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr());
    }
    template<class E> SIMDPP_INL uint8<64>& operator=(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr()); return *this;
    }

    /// @{
    /// Access base vectors
    SIMDPP_INL const uint8<64>& vec(unsigned) const { return *this; }
    SIMDPP_INL uint8<64>& vec(unsigned)       { return *this; }
    /// @}

    SIMDPP_INL uint8<64> eval() const { return *this; }

private:
    native_type d_;
};

/// Class representing possibly optimized mask data for 64x 8-bit integer
/// vector
template<>
class mask_int8<64, void> : public any_int8<64, mask_int8<64,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_INT;
    using base_vector_type = mask_int16v;
    using expr_type = void;

#if SIMDPP_USE_AVX512BW
    using native_type = __mmask64;
#endif

    SIMDPP_INL mask_int8<64>() = default;
    SIMDPP_INL mask_int8<64>(const mask_int8<64> &) = default;
    SIMDPP_INL mask_int8<64> &operator=(const mask_int8<64> &) = default;

    SIMDPP_INL mask_int8<64>(const native_type& d) : d_(d) {}

    /// Convert to the underlying vector type
#if !SIMDPP_DISABLE_DEPRECATED_CONVERSION_OPERATOR_TO_NATIVE_TYPES
    SIMDPP_INL operator native_type() const SIMDPP_IMPLICIT_CONVERSION_DEPRECATION_MSG
    { return d_; }
#endif
    SIMDPP_INL native_type native() const { return d_; }

    /// Access the underlying type
    SIMDPP_INL uint8<64> unmask() const
    {
    #if SIMDPP_USE_AVX512BW
        return _mm512_movm_epi8(d_);
    #endif
    }

    SIMDPP_INL const mask_int8<64>& vec(unsigned) const { return *this; }
    SIMDPP_INL mask_int8<64>& vec(unsigned)       { return *this; }

    SIMDPP_INL mask_int8<64> eval() const { return *this; }

private:
    native_type d_;
};

/// @} -- end ingroup

#endif // SIMDPP_USE_AVX512BW

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
