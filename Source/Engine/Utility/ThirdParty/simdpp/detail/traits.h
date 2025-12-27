/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_TRAITS_H
#define LIBSIMDPP_SIMDPP_DETAIL_TRAITS_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {
namespace detail {

/*  Normalizes vector type. Removes sign from a vector type.
 */
template<class V> struct remove_sign { using type = V; using expr = V; };

template<unsigned N, class E>
struct remove_sign<int8<N,E>> { using type = uint8<N>;   using expr = uint8<N,E>; };
template<unsigned N, class E>
struct remove_sign<int16<N,E>> { using type = uint16<N>; using expr = uint16<N,E>; };
template<unsigned N, class E>
struct remove_sign<int32<N,E>> { using type = uint32<N>; using expr = uint32<N,E>; };
template<unsigned N, class E>
struct remove_sign<int64<N,E>> { using type = uint64<N>; using expr = uint64<N,E>; };

/*  Normalizes vector type. Removes sign from a vector type. Result type is a
    value type, not an expression.
 */
template<class V> struct remove_mask { using type = V; using expr = V; };

template<unsigned N, class E>
struct remove_mask<mask_int8<N,E>> { using type = uint8<N>;   using expr = uint8<N,E>; };
template<unsigned N, class E>
struct remove_mask<mask_int16<N,E>> { using type = uint16<N>; using expr = uint16<N,E>; };
template<unsigned N, class E>
struct remove_mask<mask_int32<N,E>> { using type = uint32<N>; using expr = uint32<N,E>; };
template<unsigned N, class E>
struct remove_mask<mask_int64<N,E>> { using type = uint64<N>; using expr = uint64<N,E>; };
template<unsigned N, class E>
struct remove_mask<mask_float32<N,E>> { using type = float32<N>; using expr = float32<N,E>; };
template<unsigned N, class E>
struct remove_mask<mask_float64<N,E>> { using type = float64<N>; using expr = float64<N,E>; };

} // namespace detail
} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
