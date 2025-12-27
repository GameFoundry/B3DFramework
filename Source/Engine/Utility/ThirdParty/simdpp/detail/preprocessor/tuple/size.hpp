# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Edward Diener 2011.                                    *
#  *     (C) Copyright Paul Mensonides 2011.                                  *
#  *     Distributed under the Boost Software License, Version 1.0. (See      *
#  *     accompanying file LICENSE_1_0.txt or copy at                         *
#  *     http://www.boost.org/LICENSE_1_0.txt)                                *
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef SIMDPP_PREPROCESSOR_TUPLE_SIZE_HPP
# define SIMDPP_PREPROCESSOR_TUPLE_SIZE_HPP
#
# include <simdpp/detail/preprocessor/cat.hpp>
# include <simdpp/detail/preprocessor/config/config.hpp>
# include <simdpp/detail/preprocessor/variadic/size.hpp>
#
# if SIMDPP_PP_VARIADICS
#    if SIMDPP_PP_VARIADICS_MSVC
#        define SIMDPP_PP_TUPLE_SIZE(tuple) SIMDPP_PP_CAT(SIMDPP_PP_VARIADIC_SIZE tuple,)
#    else
#        define SIMDPP_PP_TUPLE_SIZE(tuple) SIMDPP_PP_VARIADIC_SIZE tuple
#    endif
# endif
#
# endif
