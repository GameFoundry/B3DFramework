/*  Copyright (C) 2013-2017 Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_INSN_DETAIL_CAST_INL
#define LIBSIMDPP_SIMDPP_DETAIL_INSN_DETAIL_CAST_INL

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <simdpp/detail/cast.h>
#include <simdpp/detail/cast_bitwise.h>
#include <simdpp/core/to_mask.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

template<class T> struct base_mask_vector_type { using type = T; };
template<unsigned N> struct base_mask_vector_type<mask_int8<N>> { using type = uint8<N>; };
template<unsigned N> struct base_mask_vector_type<mask_int16<N>> { using type = uint16<N>; };
template<unsigned N> struct base_mask_vector_type<mask_int32<N>> { using type = uint32<N>; };
template<unsigned N> struct base_mask_vector_type<mask_int64<N>> { using type = uint64<N>; };
template<unsigned N> struct base_mask_vector_type<mask_float32<N>> { using type = float32<N>; };
template<unsigned N> struct base_mask_vector_type<mask_float64<N>> { using type = float64<N>; };

template<class T, class R> SIMDPP_INL
void cast_bitwise_unmask(const T& t, R& r)
{
    using TT = typename base_mask_vector_type<T>::type;
    TT tt = t.unmask();
    cast_bitwise_vector(tt, r);
}

template<class T, class R> SIMDPP_INL
void cast_bitwise_remask(const T& t, R& r)
{
    using BaseMaskVector = typename base_mask_vector_type<R>::type;
    BaseMaskVector rr;
    cast_bitwise_vector(t.unmask(), rr);
    r = to_mask(rr);
}

template<>
struct cast_wrapper<CAST_TYPE_OTHER> {
    template<class T, class R> SIMDPP_INL
    static void run(const T& t, R& r)
    {
        cast_bitwise(t, r);
    }
};

template<>
struct cast_wrapper<CAST_TYPE_MASK_TO_MASK_BITWISE> {
    template<class T, class R> SIMDPP_INL
    static void run(const T& t, R& r)
    {
        static_assert(R::size_tag == T::size_tag,
                      "Conversions between masks with different element size is"
                      " not allowed");
        cast_bitwise_vector(t.eval(), r);
    }
};

template<>
struct cast_wrapper<CAST_TYPE_MASK_TO_MASK_UNMASK> {
    template<class R, class T> SIMDPP_INL
    static void run(const T& t, R& r)
    {
        static_assert(R::size_tag == T::size_tag,
                      "Conversions between masks with different element size is"
                      " not allowed");
        cast_bitwise_unmask(t.eval(), r);
    }
};

template<>
struct cast_wrapper<CAST_TYPE_MASK_TO_MASK_REMASK> {
    template<class R, class T> SIMDPP_INL
    static void run(const T& t, R& r)
    {
        static_assert(R::size_tag == T::size_tag,
                      "Conversions between masks with different element size is"
                      " not allowed");
        cast_bitwise_remask(t.eval(), r);
    }
};

template<>
struct cast_wrapper<CAST_TYPE_VECTOR_TO_MASK> {
    template<class R, class T> SIMDPP_INL
    static void run(const T& t, R& r)
    {
        (void) t; (void) r;
        static_assert(!std::is_same<T,T>::value, // fake dependency
                      "Conversion from non-mask type to a mask type is not allowed");
    }
};

template<>
struct cast_wrapper<CAST_TYPE_MASK_TO_VECTOR> {
    template<class R, class T> SIMDPP_INL
    static void run(const T& t, R& r)
    {
        cast_bitwise_unmask(t.eval(), r);
    }
};

template<>
struct cast_wrapper<CAST_TYPE_VECTOR_TO_VECTOR> {
    template<class R, class T> SIMDPP_INL
    static void run(const T& t, R& r)
    {
        cast_bitwise_vector(t.eval(), r);
    }
};

} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif

