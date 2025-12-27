# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Paul Mensonides 2003.
#  *     (C) Copyright Edward Diener 2014.
#  *     Distributed under the Boost Software License, Version 1.0. (See
#  *     accompanying file LICENSE_1_0.txt or copy at
#  *     http://www.boost.org/LICENSE_1_0.txt)
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef SIMDPP_PREPROCESSOR_FACILITIES_IS_EMPTY_HPP
# define SIMDPP_PREPROCESSOR_FACILITIES_IS_EMPTY_HPP
#
# include <simdpp/detail/preprocessor/config/config.hpp>
#
# if SIMDPP_PP_VARIADICS
#
# include <simdpp/detail/preprocessor/facilities/is_empty_variadic.hpp>
#
# else
#
# if ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MSVC() && ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MWCC()
# include <simdpp/detail/preprocessor/tuple/elem.hpp>
# include <simdpp/detail/preprocessor/facilities/identity.hpp>
# else
# include <simdpp/detail/preprocessor/cat.hpp>
# include <simdpp/detail/preprocessor/detail/split.hpp>
# endif
#
# /* SIMDPP_PP_IS_EMPTY */
#
# if ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MSVC() && ~SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MWCC()
#    define SIMDPP_PP_IS_EMPTY(x) SIMDPP_PP_IS_EMPTY_I(x SIMDPP_PP_IS_EMPTY_HELPER)
#    define SIMDPP_PP_IS_EMPTY_I(contents) SIMDPP_PP_TUPLE_ELEM(2, 1, (SIMDPP_PP_IS_EMPTY_DEF_ ## contents()))
#    define SIMDPP_PP_IS_EMPTY_DEF_SIMDPP_PP_IS_EMPTY_HELPER 1, SIMDPP_PP_IDENTITY(1)
#    define SIMDPP_PP_IS_EMPTY_HELPER() , 0
# else
#    if SIMDPP_PP_CONFIG_FLAGS() & SIMDPP_PP_CONFIG_MSVC()
#        define SIMDPP_PP_IS_EMPTY(x) SIMDPP_PP_IS_EMPTY_I(SIMDPP_PP_IS_EMPTY_HELPER x ())
#        define SIMDPP_PP_IS_EMPTY_I(test) SIMDPP_PP_IS_EMPTY_II(SIMDPP_PP_SPLIT(0, SIMDPP_PP_CAT(SIMDPP_PP_IS_EMPTY_DEF_, test)))
#        define SIMDPP_PP_IS_EMPTY_II(id) id
#    else
#        define SIMDPP_PP_IS_EMPTY(x) SIMDPP_PP_IS_EMPTY_I((SIMDPP_PP_IS_EMPTY_HELPER x ()))
#        define SIMDPP_PP_IS_EMPTY_I(par) SIMDPP_PP_IS_EMPTY_II ## par
#        define SIMDPP_PP_IS_EMPTY_II(test) SIMDPP_PP_SPLIT(0, SIMDPP_PP_CAT(SIMDPP_PP_IS_EMPTY_DEF_, test))
#    endif
#    define SIMDPP_PP_IS_EMPTY_HELPER() 1
#    define SIMDPP_PP_IS_EMPTY_DEF_1 1, SIMDPP_PP_NIL
#    define SIMDPP_PP_IS_EMPTY_DEF_SIMDPP_PP_IS_EMPTY_HELPER 0, SIMDPP_PP_NIL
# endif
#
# endif /* SIMDPP_PP_VARIADICS */
#
# endif /* SIMDPP_PREPROCESSOR_FACILITIES_IS_EMPTY_HPP */
