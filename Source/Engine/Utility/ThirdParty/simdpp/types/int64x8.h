/*  Copyright (C) 2012-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_TYPES_INT64X8_H
#define LIBSIMDPP_SIMDPP_TYPES_INT64X8_H

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

/** Class representing 4x 64-bit signed integer vector
*/
template<>
class int64<8, void> : public any_int64<8, int64<8,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_INT;
    using element_type = int64_t;
    using base_vector_type = int64<8,void>;
    using expr_type = void;

#if SIMDPP_USE_AVX512F
    using native_type = __m512i;
#endif

    SIMDPP_INL int64<8>() = default;
    SIMDPP_INL int64<8>(const int64<8> &) = default;
    SIMDPP_INL int64<8> &operator=(const int64<8> &) = default;

    template<class E> SIMDPP_INL int64<8>(const int64<8,E>& d) { *this = d.eval(); }
    template<class E> SIMDPP_INL int64<8>(const uint64<8,E>& d) { *this = d.eval(); }
    template<class V> SIMDPP_INL explicit int64<8>(const any_vec<64,V>& d)
    {
        *this = bit_cast<int64<8>>(d.wrapped().eval());
    }
    template<class V> SIMDPP_INL int64<8>& operator=(const any_vec<64,V>& d)
    {
        *this = bit_cast<int64<8>>(d.wrapped().eval()); return *this;
    }

    /// Construct from the underlying vector type
    SIMDPP_INL int64<8>(const native_type& d) : d_(d) {}
    SIMDPP_INL int64<8>& operator=(const native_type& d) { d_ = d; return *this; }

    /// Convert to the underlying vector type
#if !SIMDPP_DISABLE_DEPRECATED_CONVERSION_OPERATOR_TO_NATIVE_TYPES
    SIMDPP_INL operator native_type() const SIMDPP_IMPLICIT_CONVERSION_DEPRECATION_MSG
    { return d_; }
#endif
    SIMDPP_INL native_type native() const { return d_; }

    template<class E> SIMDPP_INL int64<8>(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr());
    }
    template<class E> SIMDPP_INL int64<8>& operator=(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr()); return *this;
    }

    /// Access base vectors
    SIMDPP_INL const int64<8>& vec(unsigned) const { return *this; }
    SIMDPP_INL int64<8>& vec(unsigned)       { return *this; }

    SIMDPP_INL int64<8> eval() const { return *this; }

private:
    native_type d_;
};

/** Class representing 4x 64-bit unsigned integer vector
*/
template<>
class uint64<8, void> : public any_int64<8, uint64<8,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_UINT;
    using element_type = uint64_t;
    using base_vector_type = uint64<8,void>;
    using expr_type = void;

#if SIMDPP_USE_AVX512F
    using native_type = __m512i;
#endif

    SIMDPP_INL uint64<8>() = default;
    SIMDPP_INL uint64<8>(const uint64<8> &) = default;
    SIMDPP_INL uint64<8> &operator=(const uint64<8> &) = default;

    template<class E> SIMDPP_INL uint64<8>(const uint64<8,E>& d) { *this = d.eval(); }
    template<class E> SIMDPP_INL uint64<8>(const int64<8,E>& d) { *this = d.eval(); }
    template<class V> SIMDPP_INL explicit uint64<8>(const any_vec<64,V>& d)
    {
        *this = bit_cast<uint64<8>>(d.wrapped().eval());
    }
    template<class V> SIMDPP_INL uint64<8>& operator=(const any_vec<64,V>& d)
    {
        *this = bit_cast<uint64<8>>(d.wrapped().eval()); return *this;
    }

    /// Construct from the underlying vector type
    SIMDPP_INL uint64<8>(const native_type& d) : d_(d) {}
    SIMDPP_INL uint64<8>& operator=(const native_type& d) { d_ = d; return *this; }

    /// Convert to the underlying vector type
#if !SIMDPP_DISABLE_DEPRECATED_CONVERSION_OPERATOR_TO_NATIVE_TYPES
    SIMDPP_INL operator native_type() const SIMDPP_IMPLICIT_CONVERSION_DEPRECATION_MSG
    { return d_; }
#endif
    SIMDPP_INL native_type native() const { return d_; }

    template<class E> SIMDPP_INL uint64<8>(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr());
    }
    template<class E> SIMDPP_INL uint64<8>& operator=(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr()); return *this;
    }

    /// Access base vectors
    SIMDPP_INL const uint64<8>& vec(unsigned) const { return *this; }
    SIMDPP_INL uint64<8>& vec(unsigned)       { return *this; }

    SIMDPP_INL uint64<8> eval() const { return *this; }

private:
    native_type d_;
};


/// Class representing possibly optimized mask data for 4x 64-bit integer
/// vector
template<>
class mask_int64<8, void> : public any_int64<8, mask_int64<8,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_INT;
    using base_vector_type = mask_int64<8,void>;
    using expr_type = void;

#if SIMDPP_USE_AVX512F
    using native_type = __mmask8;
#endif

    SIMDPP_INL mask_int64<8>() = default;
    SIMDPP_INL mask_int64<8>(const mask_int64<8> &) = default;
    SIMDPP_INL mask_int64<8> &operator=(const mask_int64<8> &) = default;

    SIMDPP_INL mask_int64<8>(const native_type& d) : d_(d) {}

    template<class E> SIMDPP_INL explicit mask_int64<8>(const mask_float64<8,E>& d)
    {
        *this = bit_cast<mask_int64<8>>(d.eval());
    }
    template<class E> SIMDPP_INL mask_int64<8>& operator=(const mask_float64<8,E>& d)
    {
        *this = bit_cast<mask_int64<8>>(d.eval()); return *this;
    }

    /// Convert to the underlying vector type
#if !SIMDPP_DISABLE_DEPRECATED_CONVERSION_OPERATOR_TO_NATIVE_TYPES
    SIMDPP_INL operator native_type() const SIMDPP_IMPLICIT_CONVERSION_DEPRECATION_MSG
    { return d_; }
#endif
    SIMDPP_INL native_type native() const { return d_; }

    /// Access the underlying type
    SIMDPP_INL uint64<8> unmask() const
    {
    #if SIMDPP_USE_AVX512DQ
        return _mm512_movm_epi64(d_);
    #elif SIMDPP_USE_AVX512F
        return _mm512_maskz_set1_epi64(d_, 0xffffffffffffffff);
    #endif
    }

    SIMDPP_INL const mask_int64<8>& vec(unsigned) const { return *this; }
    SIMDPP_INL mask_int64<8>& vec(unsigned)       { return *this; }

    SIMDPP_INL mask_int64<8> eval() const { return *this; }

private:
    native_type d_;
};

#endif // SIMDPP_USE_AVX512F

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
