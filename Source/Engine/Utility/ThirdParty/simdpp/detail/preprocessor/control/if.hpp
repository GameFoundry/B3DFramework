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
# ifndef SIMDPP_PREPROCESSOR_CONTROL_IF_HPP
# define SIMDPP_PREPROCESSOR_CONTROL_IF_HPP
#
# include <simdpp/detail/preprocessor/config/config.hpp>
# include <simdpp/detail/preprocessor/control/iif.hpp>
# include <simdpp/detail/preprocessor/logical/bool.hpp>
#
# /* SIMDPP_PP_IF */
#
# if ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_EDG()
#    define SIMDPP_PP_IF(cond, t, f) SIMDPP_PP_IIF(SIMDPP_PP_BOOL(cond), t, f)
# else
#    define SIMDPP_PP_IF(cond, t, f) SIMDPP_PP_IF_I(cond, t, f)
#    define SIMDPP_PP_IF_I(cond, t, f) SIMDPP_PP_IIF(SIMDPP_PP_BOOL(cond), t, f)
# endif
#
# endif
