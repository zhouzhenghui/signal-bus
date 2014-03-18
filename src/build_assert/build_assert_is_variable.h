/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __BUILD_ASSERT_IS_VARIABLE_H
#define __BUILD_ASSERT_IS_VARIABLE_H

#include <build_assert/build_assert.h>
#include <preprocessor/remove_parens.h>
#include <boost/preprocessor/stringize.hpp>

/**
 * BUILD_ASSERT_VARIABLE - assert the token is a variable name.
 * @token - the token to assert
 *
 * on occasion, the user code should pass a variable which is a lvalue
 * instead of address-of operator, or to ensure that the user code
 * had taken an volatile value of shared address with appropriate operation.
 *
 * Example:
 *  BUILD_ASSERT_IS_VARIABLE(i); // right
 * or
 *  BUILD_ASSERT_IS_VARIABLE(&i); // wrong
 */
#define BUILD_ASSERT_IS_VARIABLE(token) \
  BUILD_ASSERT(BOOST_PP_STRINGIZE(__PP_REMOVE_PARENS(token))" is not a variable name"[0]  != '&')

#define BUILD_ASSERT_IS_VARIABLE_OR_ZERO(token) \
  BUILD_ASSERT_OR_ZERO(BOOST_PP_STRINGIZE(__PP_REMOVE_PARENS(token))" is not a variable name"[0]  != '&')

#endif
