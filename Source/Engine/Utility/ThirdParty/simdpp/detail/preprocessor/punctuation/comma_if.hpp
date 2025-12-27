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
# ifndef SIMDPP_PREPROCESSOR_PUNCTUATION_COMMA_IF_HPP
# define SIMDPP_PREPROCESSOR_PUNCTUATION_COMMA_IF_HPP
#
# include <simdpp/detail/preprocessor/config/config.hpp>
# include <simdpp/detail/preprocessor/control/if.hpp>
# include <simdpp/detail/preprocessor/facilities/empty.hpp>
# include <simdpp/detail/preprocessor/punctuation/comma.hpp>
#
# /* SIMDPP_PP_COMMA_IF */
#
# if ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_EDG()
#    define SIMDPP_PP_COMMA_IF(cond) SIMDPP_PP_IF(cond, SIMDPP_PP_COMMA, SIMDPP_PP_EMPTY)()
# else
#    define SIMDPP_PP_COMMA_IF(cond) SIMDPP_PP_COMMA_IF_I(cond)
#    define SIMDPP_PP_COMMA_IF_I(cond) SIMDPP_PP_IF(cond, SIMDPP_PP_COMMA, SIMDPP_PP_EMPTY)()
# endif
#
# endif
