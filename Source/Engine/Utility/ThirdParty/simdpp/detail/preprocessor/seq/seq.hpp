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
# ifndef SIMDPP_PREPROCESSOR_SEQ_SEQ_HPP
# define SIMDPP_PREPROCESSOR_SEQ_SEQ_HPP
#
# include <simdpp/detail/preprocessor/config/config.hpp>
# include <simdpp/detail/preprocessor/seq/elem.hpp>
#
# /* SIMDPP_PP_SEQ_HEAD */
#
# define SIMDPP_PP_SEQ_HEAD(seq) SIMDPP_PP_SEQ_ELEM(0, seq)
#
# /* SIMDPP_PP_SEQ_TAIL */
#
# if SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MWCC()
#    define SIMDPP_PP_SEQ_TAIL(seq) SIMDPP_PP_SEQ_TAIL_1((seq))
#    define SIMDPP_PP_SEQ_TAIL_1(par) SIMDPP_PP_SEQ_TAIL_2 ## par
#    define SIMDPP_PP_SEQ_TAIL_2(seq) SIMDPP_PP_SEQ_TAIL_I ## seq
# elif SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MSVC()
#    define SIMDPP_PP_SEQ_TAIL(seq) SIMDPP_PP_SEQ_TAIL_ID(SIMDPP_PP_SEQ_TAIL_I seq)
#    define SIMDPP_PP_SEQ_TAIL_ID(id) id
# elif SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_EDG()
#    define SIMDPP_PP_SEQ_TAIL(seq) SIMDPP_PP_SEQ_TAIL_D(seq)
#    define SIMDPP_PP_SEQ_TAIL_D(seq) SIMDPP_PP_SEQ_TAIL_I seq
# else
#    define SIMDPP_PP_SEQ_TAIL(seq) SIMDPP_PP_SEQ_TAIL_I seq
# endif
#
# define SIMDPP_PP_SEQ_TAIL_I(x)
#
# /* SIMDPP_PP_SEQ_NIL */
#
# define SIMDPP_PP_SEQ_NIL(x) (x)
#
# endif
