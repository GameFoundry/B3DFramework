# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Edward Diener 2014.                                    *
#  *     Distributed under the Boost Software License, Version 1.0. (See      *
#  *     accompanying file LICENSE_1_0.txt or copy at                         *
#  *     http://www.boost.org/LICENSE_1_0.txt)                                *
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef SIMDPP_PREPROCESSOR_TUPLE_DETAIL_IS_SINGLE_RETURN_HPP
# define SIMDPP_PREPROCESSOR_TUPLE_DETAIL_IS_SINGLE_RETURN_HPP
#
# include <simdpp/detail/preprocessor/config/config.hpp>
#
# /* SIMDPP_PP_TUPLE_IS_SINGLE_RETURN */
#
# if SIMDPP_PP_VARIADICS && SIMDPP_PP_VARIADICS_MSVC
# include <simdpp/detail/preprocessor/control/iif.hpp>
# include <simdpp/detail/preprocessor/facilities/is_1.hpp>
# include <simdpp/detail/preprocessor/tuple/size.hpp>
# define SIMDPP_PP_TUPLE_IS_SINGLE_RETURN(sr,nsr,tuple)	\
    SIMDPP_PP_IIF(SIMDPP_PP_IS_1(SIMDPP_PP_TUPLE_SIZE(tuple)),sr,nsr) \
	/**/
# endif /* SIMDPP_PP_VARIADICS && SIMDPP_PP_VARIADICS_MSVC */
#
# endif /* SIMDPP_PREPROCESSOR_TUPLE_DETAIL_IS_SINGLE_RETURN_HPP */
