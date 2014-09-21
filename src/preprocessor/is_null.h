/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __PP_IS_NULL_H
#define __PP_IS_NULL_H

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/tuple.hpp>
#include <boost/preprocessor/control.hpp>
#include <boost/preprocessor/logical.hpp>
#include <boost/preprocessor/debug/assert.hpp>
#include <preprocessor/is_begin_parens.h>
#include <preprocessor/remove_parens.h>
#include <preprocessor/is_void_cast.h>

/* g++ */
#define assert_is_NULL_expansion_support_failed___null 1
#define __null_PP_IS_NULL_SUFFIX )(1)(

/* c++ 1x */
#define assert_is_NULL_expansion_support_failed_nullptr 1
#define nullptr_PP_IS_NULL_SUFFIX )(1)(

#define assert_is_NULL_expansion_support_failed_0 0
#define assert_is_NULL_expansion_support_failed_0L 0
#define assert_is_NULL_expansion_support_failed_0LL 0

#define __PP_IS_NULL_VALID_TOKEN_CAT \
  BOOST_PP_CAT(unknown_compiler_NULL_expand_to_invalid_token, __PP_REMOVE_PARENS(NULL)))

#define __PP_IS_NULL_VALID_TOKEN \
  1 BOOST_PP_TUPLE_EAT(1)(BOOST_PP_EXPR_IF(BOOST_PP_NOT(__PP_IS_BEGIN_PARENS(__PP_REMOVE_PARENS(NULL))), __PP_IS_NULL_VALID_TOKEN_CAT))

#define __PP_IS_NULL_SUPPORTED_CAT(ref) \
  BOOST_PP_CAT(assert_is_NULL_expansion_support_failed_, __PP_REMOVE_PARENS(NULL))

#define __PP_IS_NULL_SUPPORTED \
  BOOST_PP_IF(__PP_IS_BEGIN_PARENS(__PP_REMOVE_PARENS(NULL)), __PP_IS_VOID_CAST, __PP_IS_NULL_SUPPORTED_CAT)(__PP_REMOVE_PARENS(NULL))

#ifdef __cplusplus
# define __PP_IS_NULL_ERROR_MSG \
    ((struct { int unspported_NULL_expansion_detected_please_define_it_to_((void *)0) : 0; } *)0);
#else
# define __PP_IS_NULL_ERROR_MSG \
    "unspported NULL expansion detected, please define it to ((void *)0)"();
#endif

#define __PP_IS_NULL(ref) \
  BOOST_PP_ASSERT_MSG(BOOST_PP_AND(__PP_IS_NULL_VALID_TOKEN, __PP_IS_NULL_SUPPORTED), __PP_IS_NULL_ERROR_MSG) \
  BOOST_PP_IF(__PP_IS_BEGIN_PARENS(__PP_REMOVE_PARENS(ref)), __PP_IS_VOID_CAST, __PP_IS_NULL_D)(ref)

#define __PP_IS_NULL_D(ref) __PP_IS_NULL_HELPER(BOOST_PP_CAT(__PP_REMOVE_PARENS(ref), _PP_IS_NULL_SUFFIX))

#define __PP_IS_NULL_HELPER(ref) BOOST_PP_SEQ_ELEM(1, (ref)(0))

#endif /* __PP_IS_NULL_H */
