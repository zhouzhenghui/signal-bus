/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __PP_REMOVE_BEGIN_PARENS_H
#define __PP_REMOVE_BEGIN_PARENS_H

#include <boost/preprocessor/control.hpp>
#include <preprocessor/is_begin_parens.h>

#define __PP_REMOVE_BEGIN_PARENS_PRED(d, a) __PP_IS_BEGIN_PARENS(a)
#define __PP_REMOVE_BEGIN_PARENS_OP(d, a) __PP_REMOVE_BEGIN_PARENS_D a
#define __PP_REMOVE_BEGIN_PARENS_D(a) a
#define __PP_REMOVE_BEGIN_PARENS(a) BOOST_PP_WHILE(__PP_REMOVE_BEGIN_PARENS_PRED, __PP_REMOVE_BEGIN_PARENS_OP, a)

#endif /* __PP_REMOVE_BEGIN_PARENS_H */
