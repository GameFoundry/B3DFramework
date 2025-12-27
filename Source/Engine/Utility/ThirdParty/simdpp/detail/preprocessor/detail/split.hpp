# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Paul Mensonides 2002.
#  *     Distributed under the Boost Software License, Version 1.0. (See
#  *     accompanying file LICENSE_1_0.txt or copy at
#  *     http://www.boost.org/LICENSE_1_0.txt)
#  *                                                                          *
#  ************************************************************************** */
#
# ifndef SIMDPP_PREPROCESSOR_DETAIL_SPLIT_HPP
# define SIMDPP_PREPROCESSOR_DETAIL_SPLIT_HPP
#
# include <simdpp/detail/preprocessor/config/config.hpp>
#
# /* SIMDPP_PP_SPLIT */
#
# if SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MWCC()
#    define SIMDPP_PP_SPLIT(n, im) SIMDPP_PP_SPLIT_I((n, im))
#    define SIMDPP_PP_SPLIT_I(par) SIMDPP_PP_SPLIT_II ## par
#    define SIMDPP_PP_SPLIT_II(n, a, b) SIMDPP_PP_SPLIT_ ## n(a, b)
# elif SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MSVC()
#    define SIMDPP_PP_SPLIT(n, im) SIMDPP_PP_SPLIT_I(n((im)))
#    define SIMDPP_PP_SPLIT_I(n) SIMDPP_PP_SPLIT_ID(SIMDPP_PP_SPLIT_II_ ## n)
#    define SIMDPP_PP_SPLIT_II_0(s) SIMDPP_PP_SPLIT_ID(SIMDPP_PP_SPLIT_0 s)
#    define SIMDPP_PP_SPLIT_II_1(s) SIMDPP_PP_SPLIT_ID(SIMDPP_PP_SPLIT_1 s)
#    define SIMDPP_PP_SPLIT_ID(id) id
# else
#    define SIMDPP_PP_SPLIT(n, im) SIMDPP_PP_SPLIT_I(n)(im)
#    define SIMDPP_PP_SPLIT_I(n) SIMDPP_PP_SPLIT_ ## n
# endif
#
# define SIMDPP_PP_SPLIT_0(a, b) a
# define SIMDPP_PP_SPLIT_1(a, b) b
#
# endif
