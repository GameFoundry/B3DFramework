/*  Copyright (C) 2011-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_SIMDPP_DETAIL_NOT_IMPLEMENTED_H
#define LIBSIMDPP_SIMDPP_DETAIL_NOT_IMPLEMENTED_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <simdpp/types.h>
#include <type_traits>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

namespace detail {

/** A data type that can be implicitly converted to all types used in the
    library. Used to silence 'no return value' warnings
*/
class dummy_value {
public:
    template<unsigned N> operator int8<N>() { return int8<N>(); }
    template<unsigned N> operator uint8<N>() { return uint8<N>(); }
    template<unsigned N> operator mask_int8<N>() { return mask_int8<N>(); }
    template<unsigned N> operator int16<N>() { return int16<N>(); }
    template<unsigned N> operator uint16<N>() { return uint16<N>(); }
    template<unsigned N> operator mask_int16<N>() { return mask_int16<N>(); }
    template<unsigned N> operator int32<N>() { return int32<N>(); }
    template<unsigned N> operator uint32<N>() { return uint32<N>(); }
    template<unsigned N> operator mask_int32<N>() { return mask_int32<N>(); }
    template<unsigned N> operator int64<N>() { return int64<N>(); }
    template<unsigned N> operator uint64<N>() { return uint64<N>(); }
    template<unsigned N> operator mask_int64<N>() { return mask_int64<N>(); }
    template<unsigned N> operator float32<N>() { return float32<N>(); }
    template<unsigned N> operator mask_float32<N>() { return mask_float32<N>(); }
    template<unsigned N> operator float64<N>() { return float64<N>(); }
    template<unsigned N> operator mask_float64<N>() { return mask_float64<N>(); }

    template<unsigned N> operator int8<N,expr_empty>() { return int8<N>(); }
    template<unsigned N> operator uint8<N,expr_empty>() { return uint8<N>(); }
    template<unsigned N> operator mask_int8<N,expr_empty>() { return mask_int8<N>(); }
    template<unsigned N> operator int16<N,expr_empty>() { return int16<N>(); }
    template<unsigned N> operator uint16<N,expr_empty>() { return uint16<N>(); }
    template<unsigned N> operator mask_int16<N,expr_empty>() { return mask_int16<N>(); }
    template<unsigned N> operator int32<N,expr_empty>() { return int32<N>(); }
    template<unsigned N> operator uint32<N,expr_empty>() { return uint32<N>(); }
    template<unsigned N> operator mask_int32<N,expr_empty>() { return mask_int32<N>(); }
    template<unsigned N> operator int64<N,expr_empty>() { return int64<N>(); }
    template<unsigned N> operator uint64<N,expr_empty>() { return uint64<N>(); }
    template<unsigned N> operator mask_int64<N,expr_empty>() { return mask_int64<N>(); }
    template<unsigned N> operator float32<N,expr_empty>() { return float32<N>(); }
    template<unsigned N> operator mask_float32<N,expr_empty>() { return mask_float32<N>(); }
    template<unsigned N> operator float64<N,expr_empty>() { return float64<N>(); }
    template<unsigned N> operator mask_float64<N,expr_empty>() { return mask_float64<N>(); }

    operator int32_t() { return 0; }
    operator uint32_t() { return 0; }
    operator int64_t() { return 0; }
    operator uint64_t() { return 0; }
    operator float() { return 0; }
    operator double() { return 0; }
};

} // namespace detail

/** Causes compile-time error whenever unimplemented functionality is used.
    The function may only be used in templates which are not instantiated by
    default.
*/
template<class T>
void libsimdpp_instruction_not_available_template()
{
    static_assert(!std::is_same<T, T>::value, "The instruction is not available");
}

/** Causes linker error whenever unimplemented functionality is used. Compared
    to libsimdpp_instruction_not_available, this function is not limited to
    template contexts, but the errors are much harder to track down.
*/
void libsimdpp_instruction_not_available0();
void libsimdpp_instruction_not_available1();
void libsimdpp_instruction_not_available2();
void libsimdpp_instruction_not_available3();
void libsimdpp_instruction_not_available4();

#define SIMDPP_NOT_IMPLEMENTED_TEMPLATE0(T)  (                          \
    libsimdpp_instruction_not_available_template<T>(),                  \
    ::simdpp::SIMDPP_ARCH_NAMESPACE::detail::dummy_value()              \
    )

#define SIMDPP_NOT_IMPLEMENTED_TEMPLATE1(T,A)  (                        \
    (void) A,                                                           \
    libsimdpp_instruction_not_available_template<T>(),                  \
    ::simdpp::SIMDPP_ARCH_NAMESPACE::detail::dummy_value()              \
    )

#define SIMDPP_NOT_IMPLEMENTED_TEMPLATE2(T,A,B)  (                      \
    (void) A, (void) B,                                                 \
    libsimdpp_instruction_not_available_template<T>(),                  \
    ::simdpp::SIMDPP_ARCH_NAMESPACE::detail::dummy_value()              \
    )

#define SIMDPP_NOT_IMPLEMENTED_TEMPLATE3(T,A,B,C)  (                    \
    (void) A, (void) B, (void) C,                                       \
    libsimdpp_instruction_not_available_template<T>(),                  \
    ::simdpp::SIMDPP_ARCH_NAMESPACE::detail::dummy_value()              \
    )

#define SIMDPP_NOT_IMPLEMENTED_TEMPLATE4(T,A,B,C,D)  (                  \
    (void) A, (void) B, (void) C, (void) D,                             \
    libsimdpp_instruction_not_available_template<T>(),                  \
    ::simdpp::SIMDPP_ARCH_NAMESPACE::detail::dummy_value()              \
    )

#define SIMDPP_NOT_IMPLEMENTED0()  (                                    \
    libsimdpp_instruction_not_available0(),                             \
    ::simdpp::SIMDPP_ARCH_NAMESPACE::detail::dummy_value()              \
    )

#define SIMDPP_NOT_IMPLEMENTED1(A)  (                                   \
    (void) A,                                                           \
    libsimdpp_instruction_not_available1(),                             \
    ::simdpp::SIMDPP_ARCH_NAMESPACE::detail::dummy_value()              \
    )

#define SIMDPP_NOT_IMPLEMENTED2(A,B)  (                                 \
    (void) A, (void) B,                                                 \
    libsimdpp_instruction_not_available2(),                             \
    ::simdpp::SIMDPP_ARCH_NAMESPACE::detail::dummy_value()              \
    )

#define SIMDPP_NOT_IMPLEMENTED3(A,B,C)  (                               \
    (void) A, (void) B, (void) C,                                       \
    libsimdpp_instruction_not_available3(),                             \
    ::simdpp::SIMDPP_ARCH_NAMESPACE::detail::dummy_value()              \
    )

#define SIMDPP_NOT_IMPLEMENTED4(A,B,C,D)  (                             \
    (void) A, (void) B, (void) C, (void) D,                             \
    libsimdpp_instruction_not_available4(),                             \
    ::simdpp::SIMDPP_ARCH_NAMESPACE::detail::dummy_value()              \
    )

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
