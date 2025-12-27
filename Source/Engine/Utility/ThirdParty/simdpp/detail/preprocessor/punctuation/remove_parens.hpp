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
#ifndef SIMDPP_PREPROCESSOR_REMOVE_PARENS_HPP
#define SIMDPP_PREPROCESSOR_REMOVE_PARENS_HPP

#include <simdpp/detail/preprocessor/config/config.hpp>

#if SIMDPP_PP_VARIADICS

#include <simdpp/detail/preprocessor/control/iif.hpp>
#include <simdpp/detail/preprocessor/facilities/identity.hpp>
#include <simdpp/detail/preprocessor/punctuation/is_begin_parens.hpp>
#include <simdpp/detail/preprocessor/tuple/enum.hpp>

#define SIMDPP_PP_REMOVE_PARENS(param) \
    SIMDPP_PP_IIF \
      ( \
      SIMDPP_PP_IS_BEGIN_PARENS(param), \
      SIMDPP_PP_REMOVE_PARENS_DO, \
      SIMDPP_PP_IDENTITY \
      ) \
    (param)() \
/**/

#define SIMDPP_PP_REMOVE_PARENS_DO(param) \
  SIMDPP_PP_IDENTITY(SIMDPP_PP_TUPLE_ENUM(param)) \
/**/

#endif /* SIMDPP_PP_VARIADICS */
#endif /* SIMDPP_PREPROCESSOR_REMOVE_PARENS_HPP */
