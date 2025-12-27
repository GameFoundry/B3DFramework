# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Paul Mensonides 2011.                                  *
#  *     (C) Copyright Edward Diener 2011.                                    *
#  *     Distributed under the Boost Software License, Version 1.0. (See      *
#  *     accompanying file LICENSE_1_0.txt or copy at                         *
#  *     http://www.boost.org/LICENSE_1_0.txt)                                *
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef SIMDPP_PREPROCESSOR_FACILITIES_OVERLOAD_HPP
# define SIMDPP_PREPROCESSOR_FACILITIES_OVERLOAD_HPP
#
# include <simdpp/detail/preprocessor/cat.hpp>
# include <simdpp/detail/preprocessor/variadic/size.hpp>
#
# /* SIMDPP_PP_OVERLOAD */
#
# if SIMDPP_PP_VARIADICS
#    define SIMDPP_PP_OVERLOAD(prefix, ...) SIMDPP_PP_CAT(prefix, SIMDPP_PP_VARIADIC_SIZE(__VA_ARGS__))
# endif
#
# endif
