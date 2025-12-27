# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Edward Diener 2015.
#  *     Distributed under the Boost Software License, Version 1.0. (See
#  *     accompanying file LICENSE_1_0.txt or copy at
#  *     http://www.boost.org/LICENSE_1_0.txt)
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef SIMDPP_PREPROCESSOR_SEQ_DETAIL_IS_EMPTY_HPP
# define SIMDPP_PREPROCESSOR_SEQ_DETAIL_IS_EMPTY_HPP
#
# include <simdpp/detail/preprocessor/config/config.hpp>
# include <simdpp/detail/preprocessor/arithmetic/dec.hpp>
# include <simdpp/detail/preprocessor/logical/bool.hpp>
# include <simdpp/detail/preprocessor/logical/compl.hpp>
# include <simdpp/detail/preprocessor/seq/size.hpp>
#
/* An empty seq is one that is just SIMDPP_PP_SEQ_NIL */
#
# define SIMDPP_PP_SEQ_DETAIL_IS_EMPTY(seq) \
    SIMDPP_PP_COMPL \
		( \
        SIMDPP_PP_SEQ_DETAIL_IS_NOT_EMPTY(seq) \
		) \
/**/
#
# define SIMDPP_PP_SEQ_DETAIL_IS_EMPTY_SIZE(size) \
    SIMDPP_PP_COMPL \
		( \
        SIMDPP_PP_SEQ_DETAIL_IS_NOT_EMPTY_SIZE(size) \
		) \
/**/
#
# define SIMDPP_PP_SEQ_DETAIL_IS_NOT_EMPTY(seq) \
    SIMDPP_PP_SEQ_DETAIL_IS_NOT_EMPTY_SIZE(SIMDPP_PP_SEQ_DETAIL_EMPTY_SIZE(seq)) \
/**/
#
# define SIMDPP_PP_SEQ_DETAIL_IS_NOT_EMPTY_SIZE(size) \
    SIMDPP_PP_BOOL(size) \
/**/
#
# define SIMDPP_PP_SEQ_DETAIL_EMPTY_SIZE(seq) \
    SIMDPP_PP_DEC(SIMDPP_PP_SEQ_SIZE(seq (nil))) \
/**/
#
# endif
