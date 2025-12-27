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
# ifndef SIMDPP_PREPROCESSOR_FACILITIES_IS_EMPTY_VARIADIC_HPP
# define SIMDPP_PREPROCESSOR_FACILITIES_IS_EMPTY_VARIADIC_HPP
#
# include <simdpp/detail/preprocessor/config/config.hpp>
#
# if SIMDPP_PP_VARIADICS
#
# include <simdpp/detail/preprocessor/punctuation/is_begin_parens.hpp>
# include <simdpp/detail/preprocessor/facilities/detail/is_empty.hpp>
#
#if SIMDPP_PP_VARIADICS_MSVC && _MSC_VER <= 1400
#
#define SIMDPP_PP_IS_EMPTY(param) \
    SIMDPP_PP_DETAIL_IS_EMPTY_IIF \
      ( \
      SIMDPP_PP_IS_BEGIN_PARENS \
        ( \
        param \
        ) \
      ) \
      ( \
      SIMDPP_PP_IS_EMPTY_ZERO, \
      SIMDPP_PP_DETAIL_IS_EMPTY_PROCESS \
      ) \
    (param) \
/**/
#define SIMDPP_PP_IS_EMPTY_ZERO(param) 0
# else
#define SIMDPP_PP_IS_EMPTY(...) \
    SIMDPP_PP_DETAIL_IS_EMPTY_IIF \
      ( \
      SIMDPP_PP_IS_BEGIN_PARENS \
        ( \
        __VA_ARGS__ \
        ) \
      ) \
      ( \
      SIMDPP_PP_IS_EMPTY_ZERO, \
      SIMDPP_PP_DETAIL_IS_EMPTY_PROCESS \
      ) \
    (__VA_ARGS__) \
/**/
#define SIMDPP_PP_IS_EMPTY_ZERO(...) 0
# endif /* SIMDPP_PP_VARIADICS_MSVC && _MSC_VER <= 1400 */
# endif /* SIMDPP_PP_VARIADICS */
# endif /* SIMDPP_PREPROCESSOR_FACILITIES_IS_EMPTY_VARIADIC_HPP */
