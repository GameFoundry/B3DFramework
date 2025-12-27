/*  Copyright (C) 2012  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_TYPES_TRAITS_H
#define LIBSIMDPP_SIMDPP_TYPES_TRAITS_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <type_traits>
namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/// Allows detection whether specific type is a simdpp vector
template<class T>
struct is_vector : std::false_type {};

template<unsigned N, class E> struct is_vector<float32<N,E>>   : std::true_type {};
template<unsigned N, class E> struct is_vector<float64<N,E>>   : std::true_type {};
template<unsigned N, class E> struct is_vector<int8<N,E>>      : std::true_type {};
template<unsigned N, class E> struct is_vector<int16<N,E>>     : std::true_type {};
template<unsigned N, class E> struct is_vector<int32<N,E>>     : std::true_type {};
template<unsigned N, class E> struct is_vector<int64<N,E>>     : std::true_type {};
template<unsigned N, class E> struct is_vector<uint8<N,E>>     : std::true_type {};
template<unsigned N, class E> struct is_vector<uint16<N,E>>    : std::true_type {};
template<unsigned N, class E> struct is_vector<uint32<N,E>>    : std::true_type {};
template<unsigned N, class E> struct is_vector<uint64<N,E>>    : std::true_type {};
template<unsigned N, class E> struct is_vector<mask_int8<N,E>> : std::true_type {};
template<unsigned N, class E> struct is_vector<mask_int16<N,E>>    : std::true_type {};
template<unsigned N, class E> struct is_vector<mask_int32<N,E>>    : std::true_type {};
template<unsigned N, class E> struct is_vector<mask_int64<N,E>>    : std::true_type {};
template<unsigned N, class E> struct is_vector<mask_float32<N,E>>  : std::true_type {};
template<unsigned N, class E> struct is_vector<mask_float64<N,E>>  : std::true_type {};

/// Allows detection whether specific type is a simdpp value (i.e. not expression) vector
template<class V>
struct is_value_vector : std::false_type {};

template<unsigned N> struct is_value_vector<float32<N>>         : std::true_type {};
template<unsigned N> struct is_value_vector<float64<N>>         : std::true_type {};
template<unsigned N> struct is_value_vector<int8<N>>            : std::true_type {};
template<unsigned N> struct is_value_vector<int16<N>>           : std::true_type {};
template<unsigned N> struct is_value_vector<int32<N>>           : std::true_type {};
template<unsigned N> struct is_value_vector<int64<N>>           : std::true_type {};
template<unsigned N> struct is_value_vector<uint8<N>>           : std::true_type {};
template<unsigned N> struct is_value_vector<uint16<N>>          : std::true_type {};
template<unsigned N> struct is_value_vector<uint32<N>>          : std::true_type {};
template<unsigned N> struct is_value_vector<uint64<N>>          : std::true_type {};
template<unsigned N> struct is_value_vector<mask_int8<N>>       : std::true_type {};
template<unsigned N> struct is_value_vector<mask_int16<N>>      : std::true_type {};
template<unsigned N> struct is_value_vector<mask_int32<N>>      : std::true_type {};
template<unsigned N> struct is_value_vector<mask_int64<N>>      : std::true_type {};
template<unsigned N> struct is_value_vector<mask_float32<N>>    : std::true_type {};
template<unsigned N> struct is_value_vector<mask_float64<N>>    : std::true_type {};

/// Allows detection whether specific type is a simdpp mask
template<class T>
struct is_mask : std::false_type {};

template<unsigned N, class E> struct is_mask<mask_int8<N,E>>    : std::true_type {};
template<unsigned N, class E> struct is_mask<mask_int16<N,E>>   : std::true_type {};
template<unsigned N, class E> struct is_mask<mask_int32<N,E>>   : std::true_type {};
template<unsigned N, class E> struct is_mask<mask_int64<N,E>>   : std::true_type {};
template<unsigned N, class E> struct is_mask<mask_float32<N,E>> : std::true_type {};
template<unsigned N, class E> struct is_mask<mask_float64<N,E>> : std::true_type {};

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
