/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __PP_VARIADIC_SIZE_OR_ZERO_H
#define __PP_VARIADIC_SIZE_OR_ZERO_H

#include "is_empty.h"

#if BOOST_PP_VARIADICS
/* variadic arguments counting, 0 for empty mcaro arguments */
# define __PP_VARIADIC_SIZE_OR_ZERO(...) \
  BOOST_PP_IF(__PP_IS_EMPTY(__VA_ARGS__), 0, BOOST_PP_VARIADIC_SIZE(__VA_ARGS__))
#endif

#endif