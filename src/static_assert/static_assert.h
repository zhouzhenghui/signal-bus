/*
 * static assert - compile time assertion.
 * Copyright 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __STATIC_ASSERT_H
#define __STATIC_ASSERT_H

/**
 * STATIC_ASSERT - assert a compile time dependency inside static definition.
 * @cond: the compile-time condition which must be true.
 * @msg: a valid identifier which may be displayed as the error message.
 *
 * Your compile will fail if the condition isn't true, or can't be evaluated
 * by the compiler. This can only be used without a function.
 *
 * Example:
 *	#include <stddef.h>
 *	...
 *	STATIC_ASSERT(offsetof(struct foo, string) == 0 \
 *	        , string_should_be_first_member_of_struct_foo);
 */
#define STATIC_ASSERT_CONCAT_1(x, y) x##y
#define STATIC_ASSERT_CONCAT(x, y) STATIC_ASSERT_CONCAT_1(x, y)
#if defined(__COUNTER__) && __COUNTER__ != __COUNTER__
# define STATIC_ASSERT(expr, msg) \
    typedef struct { int STATIC_ASSERT_CONCAT(static_assertion_failed_, msg) : !!(expr); } \
      STATIC_ASSERT_CONCAT(static_assertion_failed_, __COUNTER__)
#else
# if defined(__GNUC__) && !defined(__cplusplus)
#   define STATIC_ASSERT_HELPER(expr, msg) \
      (!!sizeof(struct { unsigned int static_assertion_failed_##msg: (expr) ? 1 : -1; }))
#   define STATIC_ASSERT(expr, msg) \
      extern int (*assert_function__(void)) [STATIC_ASSERT_HELPER(expr, msg)]
# else
#   define STATIC_ASSERT(expr, msg) \
      extern char static_assertion_failed_##msg[1]; \
      extern char static_assertion_failed_##msg[(expr)?1:2]
# endif /* __GNUC__ */
#endif /* __COUNTER__ */

/**
 * STATIC_ASSERT_OR_TYPE - assert a compile time dependency, as a struct type.
 * @cond: the compile-time condition which must be true.
 * @msg: a valid identifier which may be displayed as the error message.
 *
 * Your compile will fail if the condition isn't true, or can't be evaluated
 * by the compiler.  Use it to define anything by yourself.
 *
 * Example:
 * 
 *	typedef ASSERT_ASSERT_OR_TYPE(offsetof(struct foo, string) == 0 \
 *	                , string_should_be_first_member_of_struct_foo) foo;
 * or
 *	See the next STATIC_ASSERT_OR_ZERO expression.
 */
#define STATIC_ASSERT_OR_TYPE(cond, msg) \
  struct { int STATIC_ASSERT_CONCAT(static_assertion_failed_, msg) : !!(cond); }

/**
 * STATIC_ASSERT_OR_ZERO - assert a compile time dependency, as an expression.
 * @cond: the compile-time condition which must be true. 
 * @msg: a valid identifier which may be displayed as the error message.
 *
 * Your compile will fail if the condition isn't true, or can't be evaluated
 * by the compiler.  This can be used in an expression: its value is "0".
 *
 * Example:
 *
 *	(void)STATIC_ASSERT_OR_ZERO(offsetof(struct foo, string) == 0 \
 *	                , string_should_be_first_member_of_struct_foo);
 * or
 *	#define foo_to_char(foo) \
 *		 ((char *)(foo) \
 *		  + STATIC_ASSERT_OR_ZERO(offsetof(struct foo, string) == 0) \
 *		    , string_should_be_first_member_of_struct_foo)
 */
#define STATIC_ASSERT_OR_ZERO(cond, msg) \
  offsetof(struct { int _1; STATIC_ASSERT_OR_TYPE(cond, msg) _2; }, _1)

#endif /* __STATIC_ASSERT_H */
