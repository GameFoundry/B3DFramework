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
# ifndef SIMDPP_PREPROCESSOR_LOGICAL_COMPL_HPP
# define SIMDPP_PREPROCESSOR_LOGICAL_COMPL_HPP
#
# include <simdpp/detail/preprocessor/config/config.hpp>
#
# /* SIMDPP_PP_COMPL */
#
# if ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MWCC()
#    define SIMDPP_PP_COMPL(x) SIMDPP_PP_COMPL_I(x)
# else
#    define SIMDPP_PP_COMPL(x) SIMDPP_PP_COMPL_OO((x))
#    define SIMDPP_PP_COMPL_OO(par) SIMDPP_PP_COMPL_I ## par
# endif
#
# if ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MSVC()
#    define SIMDPP_PP_COMPL_I(x) SIMDPP_PP_COMPL_ ## x
# else
#    define SIMDPP_PP_COMPL_I(x) SIMDPP_PP_COMPL_ID(SIMDPP_PP_COMPL_ ## x)
#    define SIMDPP_PP_COMPL_ID(id) id
# endif
#
# define SIMDPP_PP_COMPL_0 1
# define SIMDPP_PP_COMPL_1 0
#
# endif
