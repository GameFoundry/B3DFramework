# /* Copyright (C) 2001
#  * Housemarque Oy
#  * http://www.housemarque.com
#  *
#  * Distributed under the Boost Software License, Version 1.0. (See
#  * accompanying file LICENSE_1_0.txt or copy at
#  * http://www.boost.org/LICENSE_1_0.txt)
#  */
#
# /* Revised by Paul Mensonides (2002) */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef SIMDPP_PREPROCESSOR_FACILITIES_EXPAND_HPP
# define SIMDPP_PREPROCESSOR_FACILITIES_EXPAND_HPP
#
# include <simdpp/detail/preprocessor/config/config.hpp>
#
# if ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MWCC() && ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_DMC()
#    define SIMDPP_PP_EXPAND(x) SIMDPP_PP_EXPAND_I(x)
# else
#    define SIMDPP_PP_EXPAND(x) SIMDPP_PP_EXPAND_OO((x))
#    define SIMDPP_PP_EXPAND_OO(par) SIMDPP_PP_EXPAND_I ## par
# endif
#
# define SIMDPP_PP_EXPAND_I(x) x
#
# endif
