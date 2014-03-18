/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __PP_IS_BEGIN_PARENS_H
#define __PP_IS_BEGIN_PARENS_H

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/tuple.hpp>
#include <boost/preprocessor/variadic.hpp>

/**
 * tests whether a parameter begins with a set of parentheses
 * ported from variadic_macro_data in BOOST sandbox and chaospp project on sourceforge.net
 */

#if BOOST_PP_VARIADICS
#  define __PP_IS_BEGIN_PARENS_CAT(a, ...) __PP_IS_BEGIN_PARENS_CAT_D(a, __VA_ARGS__)
#  define __PP_IS_BEGIN_PARENS_CAT_D(a, ...) a ## __VA_ARGS__
#  define __PP_IS_BEGIN_PARENS_SPLIT_0(a, ...) __PP_IS_BEGIN_PARENS_SPLIT_0_D(a, __VA_ARGS__)
#  define __PP_IS_BEGIN_PARENS_SPLIT_0_D(a, ...) a

#  define __PP_IS_BEGIN_PARENS(...) \
  __PP_IS_BEGIN_PARENS_SPLIT_0(__PP_IS_BEGIN_PARENS_CAT(__PP_IS_BEGIN_PARENS_R_, __PP_IS_BEGIN_PARENS_I(__VA_ARGS__)))
#  define __PP_IS_BEGIN_PARENS_I(...) __PP_IS_BEGIN_PARENS_C __VA_ARGS__
#  define __PP_IS_BEGIN_PARENS_C(...) 1
#else
#  define __PP_IS_BEGIN_PARENS(a) BOOST_PP_TUPLE_ELEM(2, 0, (BOOST_PP_CAT(__PP_IS_BEGIN_PARENS_R_, __PP_IS_BEGIN_PARENS_C a)))
#  define __PP_IS_BEGIN_PARENS_C(_) 1
#endif /* BOOST_PP_VARIADICS */

#define __PP_IS_BEGIN_PARENS_R___PP_IS_BEGIN_PARENS_C 0,
#define __PP_IS_BEGIN_PARENS_R_1 1,

#endif /* __PP_IS_BEGIN_PARENS_H */
