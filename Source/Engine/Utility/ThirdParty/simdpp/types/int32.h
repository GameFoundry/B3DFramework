/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_TYPES_INT32_H
#define LIBSIMDPP_SIMDPP_TYPES_INT32_H

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

/** Class representing an signed @a int64 vector of arbitrary length. The vector
    always contains at least one native vector.
*/
template<unsigned N>
class int32<N, void> : public any_int32<N, int32<N,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_INT;
    using element_type = int32_t;
    using base_vector_type = int32v;
    using expr_type = void;

    SIMDPP_INL int32<N>() = default;
    SIMDPP_INL int32<N>(const int32<N>&) = default;
    SIMDPP_INL int32<N>& operator=(const int32<N>&) = default;

    template<class E> SIMDPP_INL int32<N>(const int32<N,E>& d) { *this = d.eval(); }
    template<class E> SIMDPP_INL int32<N>(const uint32<N,E>& d) { *this = d.eval(); }
    template<class V> SIMDPP_INL explicit int32<N>(const any_vec<N*4,V>& d)
    {
        *this = bit_cast<int32<N>>(d.wrapped().eval());
    }
    template<class V> SIMDPP_INL int32<N>& operator=(const any_vec<N*4,V>& d)
    {
        *this = bit_cast<int32<N>>(d.wrapped().eval()); return *this;
    }

    template<class E> SIMDPP_INL int32<N>(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr());
    }
    template<class E> SIMDPP_INL int32<N>& operator=(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr()); return *this;
    }

    SIMDPP_INL const int32v& vec(unsigned i) const { return d_[i]; }
    SIMDPP_INL int32v& vec(unsigned i)             { return d_[i]; }

    SIMDPP_INL int32<N> eval() const { return *this; }

private:
    /// Creates a signed int32 vector with the contents set to copy of native
    /// register
    static SIMDPP_INL int32<N> set_vec(const int32v& a)
    {
        int32<N> r;
        for (auto& v : r.d_) {
            v = a;
        }
        return r;
    }

    detail::vararray<int32v, int32::vec_length> d_;
};

/** Class representing an unsigned @a int32 vector of arbitrary length. The vector
    always contains at least one native vector.
*/
template<unsigned N>
class uint32<N, void> : public any_int32<N, uint32<N,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_UINT;
    using element_type = uint32_t;
    using base_vector_type = uint32v;
    using expr_type = void;

    SIMDPP_INL uint32<N>() = default;
    SIMDPP_INL uint32<N>(const uint32<N>&) = default;
    SIMDPP_INL uint32<N>& operator=(const uint32<N>&) = default;

    template<class E> SIMDPP_INL uint32<N>(const uint32<N,E>& d) { *this = d.eval(); }
    template<class E> SIMDPP_INL uint32<N>(const int32<N,E>& d) { *this = d.eval(); }
    template<class V> SIMDPP_INL explicit uint32<N>(const any_vec<N*4,V>& d)
    {
        *this = bit_cast<uint32<N>>(d.wrapped().eval());
    }
    template<class V> SIMDPP_INL uint32<N>& operator=(const any_vec<N*4,V>& d)
    {
        *this = bit_cast<uint32<N>>(d.wrapped().eval()); return *this;
    }

    template<class E> SIMDPP_INL uint32<N>(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr());
    }
    template<class E> SIMDPP_INL uint32<N>& operator=(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr()); return *this;
    }

    SIMDPP_INL const uint32v& vec(unsigned i) const { return d_[i]; }
    SIMDPP_INL uint32v& vec(unsigned i)             { return d_[i]; }

    SIMDPP_INL uint32<N> eval() const { return *this; }

private:
    /// Creates a unsigned int32 vector with the contents set to copy of native
    /// register
    static SIMDPP_INL uint32<N> set_vec(const uint32v& a)
    {
        uint32<N> r;
        for (auto& v : r.d_) {
            v = a;
        }
        return r;
    }

    detail::vararray<uint32v, uint32::vec_length> d_;
};


/// Class representing a mask for 32-bit integer vector of arbitrary
/// length.
template<unsigned N>
class mask_int32<N, void> : public any_int32<N, mask_int32<N,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_INT;
    using base_vector_type = mask_int32v;
    using expr_type = void;

    SIMDPP_INL mask_int32<N>() = default;
    SIMDPP_INL mask_int32<N>(const mask_int32<N> &) = default;
    SIMDPP_INL mask_int32<N> &operator=(const mask_int32<N> &) = default;

    template<class E> SIMDPP_INL explicit mask_int32<N>(const mask_float32<N,E>& d)
    {
        *this = bit_cast<mask_int32<N>>(d.eval());
    }
    template<class E> SIMDPP_INL mask_int32<N>& operator=(const mask_float32<N,E>& d)
    {
        *this = bit_cast<mask_int32<N>>(d.eval()); return *this;
    }

    /// Access the underlying type
    SIMDPP_INL uint32<N> unmask() const
    {
        uint32<N> r;
        for (unsigned i = 0; i < mask_int32::vec_length; ++i) {
            r.vec(i) = d_[i].unmask();
        }
        return r;
    }

    SIMDPP_INL const mask_int32v& vec(unsigned i) const { return d_[i]; }
    SIMDPP_INL mask_int32v& vec(unsigned i)       { return d_[i]; }

    SIMDPP_INL mask_int32<N> eval() const { return *this; }

private:
    detail::vararray<mask_int32v, mask_int32::vec_length> d_;
};

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
