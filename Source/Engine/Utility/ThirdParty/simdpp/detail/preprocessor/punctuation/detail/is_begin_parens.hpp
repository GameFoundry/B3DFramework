# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Edward Diener 2014.
#  *     Distributed under the Boost Software License, Version 1.0. (See
#  *     accompanying file LICENSE_1_0.txt or copy at
#  *     http://www.boost.org/LICENSE_1_0.txt)
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
#ifndef SIMDPP_PREPROCESSOR_DETAIL_IS_BEGIN_PARENS_HPP
#define SIMDPP_PREPROCESSOR_DETAIL_IS_BEGIN_PARENS_HPP

#if SIMDPP_PP_VARIADICS_MSVC

#include <simdpp/detail/preprocessor/facilities/empty.hpp>

#define SIMDPP_PP_DETAIL_VD_IBP_CAT(a, b) SIMDPP_PP_DETAIL_VD_IBP_CAT_I(a, b)
#define SIMDPP_PP_DETAIL_VD_IBP_CAT_I(a, b) SIMDPP_PP_DETAIL_VD_IBP_CAT_II(a ## b)
#define SIMDPP_PP_DETAIL_VD_IBP_CAT_II(res) res

#define SIMDPP_PP_DETAIL_IBP_SPLIT(i, ...) \
    SIMDPP_PP_DETAIL_VD_IBP_CAT(SIMDPP_PP_DETAIL_IBP_PRIMITIVE_CAT(SIMDPP_PP_DETAIL_IBP_SPLIT_,i)(__VA_ARGS__),SIMDPP_PP_EMPTY()) \
/**/

#define SIMDPP_PP_DETAIL_IBP_IS_VARIADIC_C(...) 1 1

#else

#define SIMDPP_PP_DETAIL_IBP_SPLIT(i, ...) \
    SIMDPP_PP_DETAIL_IBP_PRIMITIVE_CAT(SIMDPP_PP_DETAIL_IBP_SPLIT_,i)(__VA_ARGS__) \
/**/

#define SIMDPP_PP_DETAIL_IBP_IS_VARIADIC_C(...) 1

#endif /* SIMDPP_PP_VARIADICS_MSVC */

#define SIMDPP_PP_DETAIL_IBP_SPLIT_0(a, ...) a
#define SIMDPP_PP_DETAIL_IBP_SPLIT_1(a, ...) __VA_ARGS__

#define SIMDPP_PP_DETAIL_IBP_CAT(a, ...) SIMDPP_PP_DETAIL_IBP_PRIMITIVE_CAT(a,__VA_ARGS__)
#define SIMDPP_PP_DETAIL_IBP_PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

#define SIMDPP_PP_DETAIL_IBP_IS_VARIADIC_R_1 1,
#define SIMDPP_PP_DETAIL_IBP_IS_VARIADIC_R_SIMDPP_PP_DETAIL_IBP_IS_VARIADIC_C 0,

#endif /* SIMDPP_PREPROCESSOR_DETAIL_IS_BEGIN_PARENS_HPP */
