/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_TYPES_INT16_H
#define LIBSIMDPP_SIMDPP_TYPES_INT16_H

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

/** Class representing an signed @a int16 vector of arbitrary length. The vector
    always contains at least one native vector.
*/
template<unsigned N>
class int16<N, void> : public any_int16<N, int16<N,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_INT;
    using element_type = int16_t;
    using base_vector_type = int16v;
    using expr_type = void;

    SIMDPP_INL int16<N>() = default;
    SIMDPP_INL int16<N>(const int16<N>&) = default;
    SIMDPP_INL int16<N>& operator=(const int16<N>&) = default;

    template<class E> SIMDPP_INL int16<N>(const int16<N,E>& d) { *this = d.eval(); }
    template<class E> SIMDPP_INL int16<N>(const uint16<N,E>& d) { *this = d.eval(); }
    template<class V> SIMDPP_INL explicit int16<N>(const any_vec<N*2,V>& d)
    {
        *this = bit_cast<int16<N>>(d.wrapped().eval());
    }
    template<class V> SIMDPP_INL int16<N>& operator=(const any_vec<N*2,V>& d)
    {
        *this = bit_cast<int16<N>>(d.wrapped().eval()); return *this;
    }

    template<class E> SIMDPP_INL int16<N>(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr());
    }
    template<class E> SIMDPP_INL int16<N>& operator=(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr()); return *this;
    }

    SIMDPP_INL const int16v& vec(unsigned i) const { return d_[i]; }
    SIMDPP_INL int16v& vec(unsigned i)             { return d_[i]; }

    SIMDPP_INL int16<N> eval() const { return *this; }

private:
    /// Creates a signed int16 vector with the contents set to copy of native
    /// register
    static SIMDPP_INL int16<N> set_vec(const int16v& a)
    {
        int16<N> r;
        for (auto& v : r.d_) {
            v = a;
        }
        return r;
    }

    detail::vararray<int16v, int16::vec_length> d_;
};

/** Class representing an unsigned @a int16 vector of arbitrary length. The vector
    always contains at least one native vector.
*/
template<unsigned N>
class uint16<N, void> : public any_int16<N, uint16<N,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_UINT;
    using element_type = uint16_t;
    using base_vector_type = uint16v;
    using expr_type = void;

    SIMDPP_INL uint16<N>() = default;
    SIMDPP_INL uint16<N>(const uint16<N>&) = default;
    SIMDPP_INL uint16<N>& operator=(const uint16<N>&) = default;

    template<class E> SIMDPP_INL uint16<N>(const uint16<N,E>& d) { *this = d.eval(); }
    template<class E> SIMDPP_INL uint16<N>(const int16<N,E>& d) { *this = d.eval(); }
    template<class V> SIMDPP_INL explicit uint16<N>(const any_vec<N*2,V>& d)
    {
        *this = bit_cast<uint16<N>>(d.wrapped().eval());
    }
    template<class V> SIMDPP_INL uint16<N>& operator=(const any_vec<N*2,V>& d)
    {
        *this = bit_cast<uint16<N>>(d.wrapped().eval()); return *this;
    }

    SIMDPP_INL const uint16v& vec(unsigned i) const { return d_[i]; }
    SIMDPP_INL uint16v& vec(unsigned i)             { return d_[i]; }

    template<class E> SIMDPP_INL uint16<N>(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr());
    }
    template<class E> SIMDPP_INL uint16<N>& operator=(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr()); return *this;
    }

    SIMDPP_INL uint16<N> eval() const { return *this; }

private:
    /// Creates a unsigned int16 vector with the contents set to copy of native
    /// register
    static SIMDPP_INL uint16<N> set_vec(const uint16v& a)
    {
        uint16<N> r;
        for (auto& v : r.d_) {
            v = a;
        }
        return r;
    }

    detail::vararray<uint16v, uint16::vec_length> d_;
};

/// Class representing a mask for 16-bit integer vector of arbitrary
/// length.
template<unsigned N>
class mask_int16<N, void> : public any_int16<N, mask_int16<N,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_INT;
    using base_vector_type = mask_int16v;
    using expr_type = void;

    SIMDPP_INL mask_int16<N>() = default;
    SIMDPP_INL mask_int16<N>(const mask_int16<N> &) = default;
    SIMDPP_INL mask_int16<N> &operator=(const mask_int16<N> &) = default;

    /// Access the underlying type
    SIMDPP_INL uint16<N> unmask() const
    {
        uint16<N> r;
        for (unsigned i = 0; i < mask_int16::vec_length; ++i) {
            r.vec(i) = d_[i].unmask();
        }
        return r;
    }

    SIMDPP_INL const mask_int16v& vec(unsigned i) const { return d_[i]; }
    SIMDPP_INL mask_int16v& vec(unsigned i)             { return d_[i]; }

    SIMDPP_INL mask_int16<N> eval() const { return *this; }

private:
    detail::vararray<mask_int16v, mask_int16::vec_length> d_;
};

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
