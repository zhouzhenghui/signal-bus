/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __PP_IS_VOID_CAST_H
#define __PP_IS_VOID_CAST_H

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/control.hpp>
#include <preprocessor/is_begin_parens.h>
#include <preprocessor/remove_begin_parens.h>
#include <preprocessor/remove_parens.h>

#define __PP_IS_VOID_CAST(ref) \
  BOOST_PP_IF(__PP_IS_BEGIN_PARENS(__PP_REMOVE_PARENS(ref)), __PP_IS_VOID_CAST_1, __PP_IS_VOID_CAST_0)(__PP_REMOVE_PARENS(ref))
#define __PP_IS_VOID_CAST_0(ref) 0
#define __PP_IS_VOID_CAST_1(ref) __PP_IS_VOID_CAST_2(__PP_IS_VOID_CAST_D(__PP_IS_VOID_CAST_R ref))
#define __PP_IS_VOID_CAST_D(ref) __PP_IS_VOID_CAST_HELP(ref)
#define __PP_IS_VOID_CAST_R(ref) ref,
#define __PP_IS_VOID_CAST_SPLIT(ref, _) ref
#define __PP_IS_VOID_CAST_HELP __PP_IS_VOID_CAST_SPLIT
#define __PP_IS_VOID_CAST_2(ref) \
  BOOST_PP_IF(__PP_IS_BEGIN_PARENS(ref), __PP_IS_VOID_CAST_0, __PP_IS_VOID_CAST_3)(ref)
#define __PP_IS_VOID_CAST_3(ref) \
  BOOST_PP_SEQ_ELEM(1, (BOOST_PP_CAT(__PP_IS_VOID_CAST_DEF_, ref))(0))
#define __PP_IS_VOID_CAST_DEF_void )(1)(

#endif  /* __PP_IS_VOID_CAST_H */
