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
# ifndef SIMDPP_PREPROCESSOR_IS_BEGIN_PARENS_HPP
# define SIMDPP_PREPROCESSOR_IS_BEGIN_PARENS_HPP

# include <simdpp/detail/preprocessor/config/config.hpp>

#if SIMDPP_PP_VARIADICS

#include <simdpp/detail/preprocessor/punctuation/detail/is_begin_parens.hpp>

#if SIMDPP_PP_VARIADICS_MSVC && _MSC_VER <= 1400

#define SIMDPP_PP_IS_BEGIN_PARENS(param) \
    SIMDPP_PP_DETAIL_IBP_SPLIT \
      ( \
      0, \
      SIMDPP_PP_DETAIL_IBP_CAT \
        ( \
        SIMDPP_PP_DETAIL_IBP_IS_VARIADIC_R_, \
        SIMDPP_PP_DETAIL_IBP_IS_VARIADIC_C param \
        ) \
      ) \
/**/

#else

#define SIMDPP_PP_IS_BEGIN_PARENS(...) \
    SIMDPP_PP_DETAIL_IBP_SPLIT \
      ( \
      0, \
      SIMDPP_PP_DETAIL_IBP_CAT \
        ( \
        SIMDPP_PP_DETAIL_IBP_IS_VARIADIC_R_, \
        SIMDPP_PP_DETAIL_IBP_IS_VARIADIC_C __VA_ARGS__ \
        ) \
      ) \
/**/

#endif /* SIMDPP_PP_VARIADICS_MSVC && _MSC_VER <= 1400 */
#endif /* SIMDPP_PP_VARIADICS */
#endif /* SIMDPP_PREPROCESSOR_IS_BEGIN_PARENS_HPP */
