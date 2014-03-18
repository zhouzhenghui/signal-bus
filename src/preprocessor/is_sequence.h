/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __PP_IS_SEQUENCE_H
#define __PP_IS_SEQUENCE_H

#include <boost/preprocessor/variadic.hpp>
#include <boost/preprocessor/control.hpp>
#include <boost/preprocessor/comparison.hpp>
#include <preprocessor/is_begin_parens.h>
#include <preprocessor/is_empty.h>

#if BOOST_PP_VARIADICS
# define __PP_IS_SEQUENCE_OP_D(...) BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), 1), BOOST_PP_EMPTY(), BOOST_PP_VARIADIC_SIZE(__VA_ARGS__))
#else
# define __PP_IS_SEQUENCE_OP_D(a)
#endif

#define __PP_IS_SEQUENCE_PRED(d, a) __PP_IS_BEGIN_PARENS(a)
#define __PP_IS_SEQUENCE_OP(d, a) __PP_IS_SEQUENCE_OP_D a

#define __PP_IS_SEQUENCE(a) __PP_IS_EMPTY(BOOST_PP_WHILE(__PP_IS_SEQUENCE_PRED, __PP_IS_SEQUENCE_OP, a))

#endif /* __PP_IS_SEQUENCE_H */
