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
# ifndef SIMDPP_PREPROCESSOR_CONTROL_IIF_HPP
# define SIMDPP_PREPROCESSOR_CONTROL_IIF_HPP
#
# include <simdpp/detail/preprocessor/config/config.hpp>
#
# if ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MWCC()
#    define SIMDPP_PP_IIF(bit, t, f) SIMDPP_PP_IIF_I(bit, t, f)
# else
#    define SIMDPP_PP_IIF(bit, t, f) SIMDPP_PP_IIF_OO((bit, t, f))
#    define SIMDPP_PP_IIF_OO(par) SIMDPP_PP_IIF_I ## par
# endif
#
# if ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MSVC()
#    define SIMDPP_PP_IIF_I(bit, t, f) SIMDPP_PP_IIF_ ## bit(t, f)
# else
#    define SIMDPP_PP_IIF_I(bit, t, f) SIMDPP_PP_IIF_II(SIMDPP_PP_IIF_ ## bit(t, f))
#    define SIMDPP_PP_IIF_II(id) id
# endif
#
# define SIMDPP_PP_IIF_0(t, f) f
# define SIMDPP_PP_IIF_1(t, f) t
#
# endif
