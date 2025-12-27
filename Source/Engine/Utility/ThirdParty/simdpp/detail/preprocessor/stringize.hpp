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
# ifndef SIMDPP_PREPROCESSOR_STRINGIZE_HPP
# define SIMDPP_PREPROCESSOR_STRINGIZE_HPP
#
# include <simdpp/detail/preprocessor/config/config.hpp>
#
# /* SIMDPP_PP_STRINGIZE */
#
# if SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MSVC()
#    define SIMDPP_PP_STRINGIZE(text) SIMDPP_PP_STRINGIZE_A((text))
#    define SIMDPP_PP_STRINGIZE_A(arg) SIMDPP_PP_STRINGIZE_I arg
# elif SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MWCC()
#    define SIMDPP_PP_STRINGIZE(text) SIMDPP_PP_STRINGIZE_OO((text))
#    define SIMDPP_PP_STRINGIZE_OO(par) SIMDPP_PP_STRINGIZE_I ## par
# else
#    define SIMDPP_PP_STRINGIZE(text) SIMDPP_PP_STRINGIZE_I(text)
# endif
#
# define SIMDPP_PP_STRINGIZE_I(text) #text
#
# endif
