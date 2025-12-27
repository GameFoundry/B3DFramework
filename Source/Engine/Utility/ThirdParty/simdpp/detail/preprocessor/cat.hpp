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
# ifndef SIMDPP_PREPROCESSOR_CAT_HPP
# define SIMDPP_PREPROCESSOR_CAT_HPP
#
# include <simdpp/detail/preprocessor/config/config.hpp>
#
# /* SIMDPP_PP_CAT */
#
# if ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MWCC()
#    define SIMDPP_PP_CAT(a, b) SIMDPP_PP_CAT_I(a, b)
# else
#    define SIMDPP_PP_CAT(a, b) SIMDPP_PP_CAT_OO((a, b))
#    define SIMDPP_PP_CAT_OO(par) SIMDPP_PP_CAT_I ## par
# endif
#
# if ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MSVC()
#    define SIMDPP_PP_CAT_I(a, b) a ## b
# else
#    define SIMDPP_PP_CAT_I(a, b) SIMDPP_PP_CAT_II(~, a ## b)
#    define SIMDPP_PP_CAT_II(p, res) res
# endif
#
# endif
