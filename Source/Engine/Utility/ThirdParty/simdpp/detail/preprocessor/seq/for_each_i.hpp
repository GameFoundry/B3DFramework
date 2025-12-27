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
# ifndef SIMDPP_PREPROCESSOR_SEQ_FOR_EACH_I_HPP
# define SIMDPP_PREPROCESSOR_SEQ_FOR_EACH_I_HPP
#
# include <simdpp/detail/preprocessor/arithmetic/dec.hpp>
# include <simdpp/detail/preprocessor/arithmetic/inc.hpp>
# include <simdpp/detail/preprocessor/config/config.hpp>
# include <simdpp/detail/preprocessor/control/if.hpp>
# include <simdpp/detail/preprocessor/control/iif.hpp>
# include <simdpp/detail/preprocessor/repetition/for.hpp>
# include <simdpp/detail/preprocessor/seq/seq.hpp>
# include <simdpp/detail/preprocessor/seq/size.hpp>
# include <simdpp/detail/preprocessor/seq/detail/is_empty.hpp>
# include <simdpp/detail/preprocessor/tuple/elem.hpp>
# include <simdpp/detail/preprocessor/tuple/rem.hpp>
#
# /* SIMDPP_PP_SEQ_FOR_EACH_I */
#
# if ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_EDG()
#    define SIMDPP_PP_SEQ_FOR_EACH_I(macro, data, seq) SIMDPP_PP_SEQ_FOR_EACH_I_DETAIL_CHECK(macro, data, seq)
# else
#    define SIMDPP_PP_SEQ_FOR_EACH_I(macro, data, seq) SIMDPP_PP_SEQ_FOR_EACH_I_I(macro, data, seq)
#    define SIMDPP_PP_SEQ_FOR_EACH_I_I(macro, data, seq) SIMDPP_PP_SEQ_FOR_EACH_I_DETAIL_CHECK(macro, data, seq)
# endif
#
#    define SIMDPP_PP_SEQ_FOR_EACH_I_DETAIL_CHECK_EXEC(macro, data, seq) SIMDPP_PP_FOR((macro, data, seq, 0, SIMDPP_PP_SEQ_SIZE(seq)), SIMDPP_PP_SEQ_FOR_EACH_I_P, SIMDPP_PP_SEQ_FOR_EACH_I_O, SIMDPP_PP_SEQ_FOR_EACH_I_M)
#    define SIMDPP_PP_SEQ_FOR_EACH_I_DETAIL_CHECK_EMPTY(macro, data, seq)
#
#    define SIMDPP_PP_SEQ_FOR_EACH_I_DETAIL_CHECK(macro, data, seq) \
        SIMDPP_PP_IIF \
			( \
            SIMDPP_PP_SEQ_DETAIL_IS_NOT_EMPTY(seq), \
            SIMDPP_PP_SEQ_FOR_EACH_I_DETAIL_CHECK_EXEC, \
            SIMDPP_PP_SEQ_FOR_EACH_I_DETAIL_CHECK_EMPTY \
			) \
		(macro, data, seq) \
/**/
#
# define SIMDPP_PP_SEQ_FOR_EACH_I_P(r, x) SIMDPP_PP_TUPLE_ELEM(5, 4, x)
#
# if SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_STRICT()
#    define SIMDPP_PP_SEQ_FOR_EACH_I_O(r, x) SIMDPP_PP_SEQ_FOR_EACH_I_O_I x
# else
#    define SIMDPP_PP_SEQ_FOR_EACH_I_O(r, x) SIMDPP_PP_SEQ_FOR_EACH_I_O_I(SIMDPP_PP_TUPLE_ELEM(5, 0, x), SIMDPP_PP_TUPLE_ELEM(5, 1, x), SIMDPP_PP_TUPLE_ELEM(5, 2, x), SIMDPP_PP_TUPLE_ELEM(5, 3, x), SIMDPP_PP_TUPLE_ELEM(5, 4, x))
# endif
#
# define SIMDPP_PP_SEQ_FOR_EACH_I_O_I(macro, data, seq, i, sz) \
    SIMDPP_PP_SEQ_FOR_EACH_I_O_I_DEC(macro, data, seq, i, SIMDPP_PP_DEC(sz)) \
/**/
# define SIMDPP_PP_SEQ_FOR_EACH_I_O_I_DEC(macro, data, seq, i, sz) \
	( \
	macro, \
	data, \
    SIMDPP_PP_IF \
		( \
		sz, \
        SIMDPP_PP_SEQ_FOR_EACH_I_O_I_TAIL, \
        SIMDPP_PP_SEQ_FOR_EACH_I_O_I_NIL \
		) \
	(seq), \
    SIMDPP_PP_INC(i), \
	sz \
	) \
/**/
# define SIMDPP_PP_SEQ_FOR_EACH_I_O_I_TAIL(seq) SIMDPP_PP_SEQ_TAIL(seq)
# define SIMDPP_PP_SEQ_FOR_EACH_I_O_I_NIL(seq) SIMDPP_PP_NIL
#
# if SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_STRICT()
#    define SIMDPP_PP_SEQ_FOR_EACH_I_M(r, x) SIMDPP_PP_SEQ_FOR_EACH_I_M_IM(r, SIMDPP_PP_TUPLE_REM_5 x)
#    define SIMDPP_PP_SEQ_FOR_EACH_I_M_IM(r, im) SIMDPP_PP_SEQ_FOR_EACH_I_M_I(r, im)
# else
#    define SIMDPP_PP_SEQ_FOR_EACH_I_M(r, x) SIMDPP_PP_SEQ_FOR_EACH_I_M_I(r, SIMDPP_PP_TUPLE_ELEM(5, 0, x), SIMDPP_PP_TUPLE_ELEM(5, 1, x), SIMDPP_PP_TUPLE_ELEM(5, 2, x), SIMDPP_PP_TUPLE_ELEM(5, 3, x), SIMDPP_PP_TUPLE_ELEM(5, 4, x))
# endif
#
# define SIMDPP_PP_SEQ_FOR_EACH_I_M_I(r, macro, data, seq, i, sz) macro(r, data, i, SIMDPP_PP_SEQ_HEAD(seq))
#
# /* SIMDPP_PP_SEQ_FOR_EACH_I_R */
#
# if ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_EDG()
#    define SIMDPP_PP_SEQ_FOR_EACH_I_R(r, macro, data, seq) SIMDPP_PP_SEQ_FOR_EACH_I_R_DETAIL_CHECK(r, macro, data, seq)
# else
#    define SIMDPP_PP_SEQ_FOR_EACH_I_R(r, macro, data, seq) SIMDPP_PP_SEQ_FOR_EACH_I_R_I(r, macro, data, seq)
#    define SIMDPP_PP_SEQ_FOR_EACH_I_R_I(r, macro, data, seq) SIMDPP_PP_SEQ_FOR_EACH_I_R_DETAIL_CHECK(r, macro, data, seq)
# endif
#
#    define SIMDPP_PP_SEQ_FOR_EACH_I_R_DETAIL_CHECK_EXEC(r, macro, data, seq) SIMDPP_PP_FOR_ ## r((macro, data, seq, 0, SIMDPP_PP_SEQ_SIZE(seq)), SIMDPP_PP_SEQ_FOR_EACH_I_P, SIMDPP_PP_SEQ_FOR_EACH_I_O, SIMDPP_PP_SEQ_FOR_EACH_I_M)
#    define SIMDPP_PP_SEQ_FOR_EACH_I_R_DETAIL_CHECK_EMPTY(r, macro, data, seq)
#
#    define SIMDPP_PP_SEQ_FOR_EACH_I_R_DETAIL_CHECK(r, macro, data, seq) \
        SIMDPP_PP_IIF \
			( \
            SIMDPP_PP_SEQ_DETAIL_IS_NOT_EMPTY(seq), \
            SIMDPP_PP_SEQ_FOR_EACH_I_R_DETAIL_CHECK_EXEC, \
            SIMDPP_PP_SEQ_FOR_EACH_I_R_DETAIL_CHECK_EMPTY \
			) \
		(r, macro, data, seq) \
/**/
#
# endif
