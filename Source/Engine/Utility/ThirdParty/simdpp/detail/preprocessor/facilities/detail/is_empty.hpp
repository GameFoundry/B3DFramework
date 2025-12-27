# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Edward Diener 2014.
#  *     Distributed under the Boost Software License, Version 1.0. (See
#  *     accompanying file LICENSE_1_0.txt or copy at
#  *     http://www.boost.org/LICENSE_1_0.txt)
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
#ifndef SIMDPP_PREPROCESSOR_DETAIL_IS_EMPTY_HPP
#define SIMDPP_PREPROCESSOR_DETAIL_IS_EMPTY_HPP

#include <simdpp/detail/preprocessor/punctuation/is_begin_parens.hpp>

#if SIMDPP_PP_VARIADICS_MSVC

# pragma warning(once:4002)

#define SIMDPP_PP_DETAIL_IS_EMPTY_IIF_0(t, b) b
#define SIMDPP_PP_DETAIL_IS_EMPTY_IIF_1(t, b) t

#else

#define SIMDPP_PP_DETAIL_IS_EMPTY_IIF_0(t, ...) __VA_ARGS__
#define SIMDPP_PP_DETAIL_IS_EMPTY_IIF_1(t, ...) t

#endif

#if SIMDPP_PP_VARIADICS_MSVC && _MSC_VER <= 1400

#define SIMDPP_PP_DETAIL_IS_EMPTY_PROCESS(param) \
    SIMDPP_PP_IS_BEGIN_PARENS \
    	( \
        SIMDPP_PP_DETAIL_IS_EMPTY_NON_FUNCTION_C param () \
        ) \
/**/

#else

#define SIMDPP_PP_DETAIL_IS_EMPTY_PROCESS(...) \
    SIMDPP_PP_IS_BEGIN_PARENS \
        ( \
        SIMDPP_PP_DETAIL_IS_EMPTY_NON_FUNCTION_C __VA_ARGS__ () \
        ) \
/**/

#endif

#define SIMDPP_PP_DETAIL_IS_EMPTY_PRIMITIVE_CAT(a, b) a ## b
#define SIMDPP_PP_DETAIL_IS_EMPTY_IIF(bit) SIMDPP_PP_DETAIL_IS_EMPTY_PRIMITIVE_CAT(SIMDPP_PP_DETAIL_IS_EMPTY_IIF_,bit)
#define SIMDPP_PP_DETAIL_IS_EMPTY_NON_FUNCTION_C(...) ()

#endif /* SIMDPP_PREPROCESSOR_DETAIL_IS_EMPTY_HPP */
