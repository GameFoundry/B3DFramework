# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Paul Mensonides 2003.
#  *     Distributed under the Boost Software License, Version 1.0. (See
#  *     accompanying file LICENSE_1_0.txt or copy at
#  *     http://www.boost.org/LICENSE_1_0.txt)
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef SIMDPP_PREPROCESSOR_FACILITIES_IS_1_HPP
# define SIMDPP_PREPROCESSOR_FACILITIES_IS_1_HPP
#
# include <simdpp/detail/preprocessor/cat.hpp>
# include <simdpp/detail/preprocessor/facilities/is_empty.hpp>
#
# /* SIMDPP_PP_IS_1 */
#
# define SIMDPP_PP_IS_1(x) SIMDPP_PP_IS_EMPTY(SIMDPP_PP_CAT(SIMDPP_PP_IS_1_HELPER_, x))
# define SIMDPP_PP_IS_1_HELPER_1
#
# endif
