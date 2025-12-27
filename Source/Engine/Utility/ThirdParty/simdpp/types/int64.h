/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_TYPES_INT64_H
#define LIBSIMDPP_SIMDPP_TYPES_INT64_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/setup_arch.h>
#include <simdpp/types/any.h>
#include <simdpp/types/fwd.h>
#include <simdpp/detail/array.h>
#include <simdpp/detail/construct_eval.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** Class representing an signed @a int64 vector of arbitrary length. The vector
    always contains at least one native vector.
*/
template<unsigned N>
class int64<N, void> : public any_int64<N, int64<N,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_INT;
    using element_type = int64_t;
    using base_vector_type = int64v;
    using expr_type = void;

    SIMDPP_INL int64<N>() = default;
    SIMDPP_INL int64<N>(const int64<N>&) = default;
    SIMDPP_INL int64<N>& operator=(const int64<N>&) = default;

    template<class E> SIMDPP_INL int64<N>(const int64<N,E>& d) { *this = d.eval(); }
    template<class E> SIMDPP_INL int64<N>(const uint64<N,E>& d) { *this = d.eval(); }
    template<class V> SIMDPP_INL explicit int64<N>(const any_vec<N*8,V>& d)
    {
        *this = bit_cast<int64<N>>(d.wrapped().eval());
    }
    template<class V> SIMDPP_INL int64<N>& operator=(const any_vec<N*8,V>& d)
    {
        *this = bit_cast<int64<N>>(d.wrapped().eval()); return *this;
    }

    template<class E> SIMDPP_INL int64<N>(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr());
    }
    template<class E> SIMDPP_INL int64<N>& operator=(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr()); return *this;
    }

    SIMDPP_INL const int64v& vec(unsigned i) const { return d_[i]; }
    SIMDPP_INL int64v& vec(unsigned i)             { return d_[i]; }

    SIMDPP_INL int64<N> eval() const { return *this; }

private:

    /// Creates a signed int64 vector with the contents set to copy of native
    /// register
    static SIMDPP_INL int64<N> set_vec(const int64v& a)
    {
        int64<N> r;
        for (auto& v : r.d_) {
            v = a;
        }
        return r;
    }

private:
    detail::vararray<int64v, int64::vec_length> d_;
};

/** Class representing an unsigned @a int64 vector of arbitrary length. The vector
    always contains at least one native vector.
*/
template<unsigned N>
class uint64<N, void> : public any_int64<N, uint64<N,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_UINT;
    using element_type = uint64_t;
    using base_vector_type = uint64v;
    using expr_type = void;

    SIMDPP_INL uint64<N>() = default;
    SIMDPP_INL uint64<N>(const uint64<N>&) = default;
    SIMDPP_INL uint64<N>& operator=(const uint64<N>&) = default;

    template<class E> SIMDPP_INL uint64<N>(const uint64<N,E>& d) { *this = d.eval(); }
    template<class E> SIMDPP_INL uint64<N>(const int64<N,E>& d) { *this = d.eval(); }
    template<class V> SIMDPP_INL explicit uint64<N>(const any_vec<N*8,V>& d)
    {
        *this = bit_cast<uint64<N>>(d.wrapped().eval());
    }
    template<class V> SIMDPP_INL uint64<N>& operator=(const any_vec<N*8,V>& d)
    {
        *this = bit_cast<uint64<N>>(d.wrapped().eval()); return *this;
    }

    template<class E> SIMDPP_INL uint64<N>(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr());
    }
    template<class E> SIMDPP_INL uint64<N>& operator=(const expr_vec_construct<E>& e)
    {
        detail::construct_eval_wrapper(*this, e.expr()); return *this;
    }

    SIMDPP_INL const uint64v& vec(unsigned i) const { return d_[i]; }
    SIMDPP_INL uint64v& vec(unsigned i)             { return d_[i]; }

    SIMDPP_INL uint64<N> eval() const { return *this; }

private:
    /// Creates a unsigned int64 vector with the contents set to copy of native
    /// register
    static SIMDPP_INL uint64<N> set_vec(const uint64v& a)
    {
        uint64<N> r;
        for (auto& v : r.d_) {
            v = a;
        }
        return r;
    }
private:
    detail::vararray<uint64v, uint64::vec_length> d_;
};

/// Class representing a mask for 64-bit integer vector of arbitrary
/// length.
template<unsigned N>
class mask_int64<N, void> : public any_int64<N, mask_int64<N,void>> {
public:
    static const unsigned type_tag = SIMDPP_TAG_MASK_INT;
    using base_vector_type = mask_int64v;
    using expr_type = void;

    SIMDPP_INL mask_int64<N>() = default;
    SIMDPP_INL mask_int64<N>(const mask_int64<N> &) = default;
    SIMDPP_INL mask_int64<N> &operator=(const mask_int64<N> &) = default;

    template<class E> SIMDPP_INL explicit mask_int64<N>(const mask_float64<N,E>& d)
    {
        *this = bit_cast<mask_int64<N>>(d.eval());
    }
    template<class E> SIMDPP_INL mask_int64<N>& operator=(const mask_float64<N,E>& d)
    {
        *this = bit_cast<mask_int64<N>>(d.eval()); return *this;
    }

    /// Access the underlying type
    SIMDPP_INL uint64<N> unmask() const
    {
        uint64<N> r;
        for (unsigned i = 0; i < mask_int64::vec_length; ++i) {
            r.vec(i) = d_[i].unmask();
        }
        return r;
    }

    SIMDPP_INL const mask_int64v& vec(unsigned i) const { return d_[i]; }
    SIMDPP_INL mask_int64v& vec(unsigned i)       { return d_[i]; }

    SIMDPP_INL mask_int64<N> eval() const { return *this; }

private:
    detail::vararray<mask_int64v, mask_int64::vec_length> d_;
};

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
