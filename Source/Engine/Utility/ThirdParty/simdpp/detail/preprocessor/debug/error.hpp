# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Paul Mensonides 2002.
#  *     Distributed under the Boost Software License, Version 1.0. (See
#  *     accompanying file LICENSE_1_0.txt or copy at
#  *     http://www.boost.org/LICENSE_1_0.txt)
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef SIMDPP_PREPROCESSOR_DEBUG_ERROR_HPP
# define SIMDPP_PREPROCESSOR_DEBUG_ERROR_HPP
#
# include <simdpp/detail/preprocessor/cat.hpp>
# include <simdpp/detail/preprocessor/config/config.hpp>
#
# /* SIMDPP_PP_ERROR */
#
# if SIMDPP_PP_CONFIG_ERRORS
#    define SIMDPP_PP_ERROR(code) SIMDPP_PP_CAT(SIMDPP_PP_ERROR_, code)
# endif
#
# define SIMDPP_PP_ERROR_0x0000 SIMDPP_PP_ERROR(0x0000, SIMDPP_PP_INDEX_OUT_OF_BOUNDS)
# define SIMDPP_PP_ERROR_0x0001 SIMDPP_PP_ERROR(0x0001, SIMDPP_PP_WHILE_OVERFLOW)
# define SIMDPP_PP_ERROR_0x0002 SIMDPP_PP_ERROR(0x0002, SIMDPP_PP_FOR_OVERFLOW)
# define SIMDPP_PP_ERROR_0x0003 SIMDPP_PP_ERROR(0x0003, SIMDPP_PP_REPEAT_OVERFLOW)
# define SIMDPP_PP_ERROR_0x0004 SIMDPP_PP_ERROR(0x0004, SIMDPP_PP_LIST_FOLD_OVERFLOW)
# define SIMDPP_PP_ERROR_0x0005 SIMDPP_PP_ERROR(0x0005, SIMDPP_PP_SEQ_FOLD_OVERFLOW)
# define SIMDPP_PP_ERROR_0x0006 SIMDPP_PP_ERROR(0x0006, SIMDPP_PP_ARITHMETIC_OVERFLOW)
# define SIMDPP_PP_ERROR_0x0007 SIMDPP_PP_ERROR(0x0007, SIMDPP_PP_DIVISION_BY_ZERO)
#
# endif
