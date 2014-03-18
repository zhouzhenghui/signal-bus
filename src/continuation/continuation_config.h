/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CONTINUATION_CONFIG_H
#define __CONTINUATION_CONFIG_H

#include <stddef.h>

/* if we don't have a compiler config set, try and find one: */
#if !defined(CONTINUATION_COMPILER_CONFIG) && !defined(CONTINUATION_NO_COMPILER_CONFIG) && !defined(CONTINUATION_NO_CONFIG)
# include "select_compiler_config.h"
#endif

/* if we have a compiler config, include it now: */
#ifdef CONTINUATION_COMPILER_CONFIG
# include CONTINUATION_COMPILER_CONFIG
#endif

/* user define assert */
#ifndef assert
# include <assert.h>
#endif

/* if we have no existing compiler implementation of CONTINUATION_STUB_ENTRY, fallback to longjmp */
#if !defined(CONTINUATION_USE_LONGJMP) && !defined(CONTINUATION_STUB_ENTRY)
# define CONTINUATION_USE_LONGJMP 1
#endif

/* unknown platform or force pure c code */
#include "compiler/unknown.h"

/* The implementation of
 *   CONTINUATION_STUB_ENTRY and CONTINUATION_STUB_INVOKE 
 * must be macro or forced inline.
 */
#ifndef CONTINUATION_STUB_ENTRY
# error "should define CONTINUATION_STUB_ENTRY by yourself"
#endif

#ifndef CONTINUATION_STUB_INVOKE
/* default continuation invoke method */
# define CONTINUATION_STUB_INVOKE(cont_stub) \
  ((void(*)(void *))((cont_stub)->cont->func_addr))(cont_stub)
#endif

#ifndef CONTINUATION_CONSTRUCT
# define CONTINUATION_CONSTRUCT(cont)
#endif

#ifndef CONTINUATION_DESTRUCT
# define CONTINUATION_DESTRUCT(cont)
#endif

/* some platform use stack pointer as stack frame pointer,
 * the local variables is allocated from the top of stack,
 * so we can not deduce the size of host stack frame with
 * the default function-call method.
 * to notify such situation:
 *
 * #define CONTINUATION_STACK_FRAME_REVERSE 1
 *
 * or if not as you know:
 *
 * #define CONTINUATION_STACK_FRAME_REVERSE 0
 */
#if defined(CONTINUATION_NO_FRAME_POINTER)
# undef CONTINUATION_STACK_FRAME_REVERSE
# define CONTINUATION_STACK_FRAME_REVERSE 1
#endif

/* with C99 variable lenght array of alloca() function
 * as if it extends the stack frame in a proper way,
 * we need not specify the size of stack frame.
 * otherwise you should pre-define it before include directive,
 * the value of macro CONTINUATION_STACK_FRAME_SIZE must be
 * greater than the max length of any host stack frame
 * in a compliation unit(a .c source file maybe).
 */
#if defined(CONTINUATION_STACK_FRAME_REVERSE) \
      && !CONTINUATION_STACK_FRAME_REVERSE \
      && !CONTINUATION_USE_C99_VLA \
      && !CONTINUATION_USE_ALLOCA \
      && !defined(CONTINUATION_EXTEND_STACK_FRAME)
# if !defined(CONTINUATION_STACK_FRAME_SIZE)
#   error "must define CONTINUATION_STACK_FRAME_SIZE without dynamic stack frame stretching support"
# endif
#endif

/* how many bytes to extend the deduced length of stack frame */
#if !defined(CONTINUATION_STACK_FRAME_PADDING)
# define CONTINUATION_STACK_FRAME_PADDING 0
#endif

/* the stack block size will apply to an array in
 * recursive invoking to fake the dynamic stack allocation.
 * also maybe for other purpose.
 */
#ifndef CONTINUATION_STACK_BLOCK_SIZE
# define CONTINUATION_STACK_BLOCK_SIZE 1024
#endif

#endif /* __CONTINUATION_CONFIG_H */
