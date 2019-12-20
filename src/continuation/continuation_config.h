/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CONTINUATION_CONFIG_H
#define __CONTINUATION_CONFIG_H

/**
 * @file
 * @ingroup continuation
 * @brief Platform related configuration of continuation implementation.
 * 
 * Continuation is implemented according to the platform/compilation.
 * The compiler/platform related configurations are described here.
 *
 * The CONTINUATION_COMPILER_CONFIG macro specifies a compiler dependent file
 * to be included, it will be automatic configured without user specification.
 * 
 * User can specified the definition of any configuration macros at the moment
 * the header file is last included.
 */

/**
 * @def CONTINUATION_CONSTRUCT(cont)
 * @brief Macro definition to construct a continuation.
 * @details it will be invoked before any continuation structures are created.
 * @param cont: \p cont in CONTINUATION_CONNECT().
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_DESTRUCT()
 */
#ifndef CONTINUATION_CONSTRUCT
# define CONTINUATION_CONSTRUCT(cont) /* Empty definition for Doxygen */
# undef CONTINUATION_CONSTRUCT
#endif

/**
 * @def CONTINUATION_DESTRUCT(cont)
 * @brief Macro definition to destruct a continuation.
 * @details it will be invoked at then end of life cycle of any continuation structures 
 * @param cont: \p cont in CONTINUATION_CONNECT().
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_CONSTRUCT()
 */
#ifndef CONTINUATION_DESTRUCT
# define CONTINUATION_DESTRUCT(cont) /* Empty definition for Doxygen */
# undef CONTINUATION_DESTRUCT
#endif

/**
 * @def CONTINUATION_STUB_ENTRY(cont_stub)
 * @brief The low level entry point of a continuation.
 * @details It is macro defintion to generate a entry point of a continuation and determines to run
 *  the initialization and continuation statements.
 * @param cont_stub: \p cont_stub in CONTINUATION_CONNECT().
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_STUB_INVOKE()
 */
#ifndef CONTINUATION_STUB_ENTRY
# define CONTINUATION_STUB_ENTRY(cont_stub) /* Empty definition for Doxygen */
# undef CONTINUATION_STUB_ENTRY
#endif

/**
 * @def CONTINUATION_STUB_INVOKE(cont_stub)
 * @brief The low level call stub of a continuation.
 * @details It is a macro definition to execute the statements of a continuation in place.
 * @param cont_stub: the \p cont_stub in CONTINUATION_CONNECT().
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_STUB_ENTRY()
 */
#ifndef CONTINUATION_STUB_INVOKE
# define CONTINUATION_STUB_INVOKE(cont_stub) /* Empty definition for Doxygen */
# undef CONTINUATION_STUB_INVOKE
#endif

/**
 * @def CONTINUATION_NO_FRAME_POINTER
 * @brief Whether addressing of stack variables is based on frame pointer?
 */
#ifndef CONTINUATION_NO_FRAME_POINTER
# define CONTINUATION_NO_FRAME_POINTER /* Empty definition for Doxygen */
# undef CONTINUATION_NO_FRAME_POINTER
#endif

/**
 * @def CONTINUATION_STACK_PARAMETERS_SIZE
 * @brief Default size of space reserved for parameters before the stack frame.
 */
#ifndef CONTINUATION_STACK_PARAMETERS_SIZE
# define CONTINUATION_STACK_PARAMETERS_SIZE /* Empty definition for Doxygen */
# undef CONTINUATION_STACK_PARAMETERS_SIZE
#endif

/**
 * @def CONTINUATION_STACK_FRAME_PADDING
 * @brief Default size of space to be appended at the end of stack frame.
 */
#ifndef CONTINUATION_STACK_FRAME_PADDING
# define CONTINUATION_STACK_FRAME_PADDING /* Empty definition for Doxygen */
# undef CONTINUATION_STACK_FRAME_PADDING
#endif

/**
 * @def CONTINUATION_EXTEND_STACK_FRAME
 * @brief User defined dynamic extend facility of stack frame.
 * @param size: size of the extended stack frame.
 */
#ifndef CONTINUATION_EXTEND_STACK_FRAME
# define CONTINUATION_EXTEND_STACK_FRAME(size) /* Empty definition for Doxygen */
# undef CONTINUATION_EXTEND_STACK_FRAME
#endif

/**
 * @def CONTINUATION_USE_C99_VLA
 * @brief Whether C99 VLA facility can be used for extending a stack frame?
 */
#ifndef CONTINUATION_USE_C99_VLA
# define CONTINUATION_USE_C99_VLA /* Empty definition for Doxygen */
# undef CONTINUATION_USE_C99_VLA
#endif

/**
 * @def CONTINUATION_USE_ALLOCA
 * @brief Whether alloca() facility can be used for extending a stack frame?
 */
#ifndef CONTINUATION_USE_ALLOCA
# define CONTINUATION_USE_ALLOCA /* Empty definition for Doxygen */
# undef CONTINUATION_USE_ALLOCA
#endif

/**
 * @def CONTINUATION_STACK_FRAME_SIZE
 * @brief Specify the size of stack frame at compile time.
 * @details With C99 variable length array or alloca() function
 * as if it extends the stack frame in a proper way
 * the size of stack frame can be adjusted dynamically,
 * otherwise the size of stack frames, defined by
 * CONTINUATION_STACK_FRAME_SIZE, should be specified
 * before the header files are included.
 * 
 * @note The value of macro CONTINUATION_STACK_FRAME_SIZE must be
 * greater than the length of any stack frames
 * in a compilation unit (a .c source file maybe).
 * 
 * @see CONTINUATION_USE_C99_VLA
 * @see CONTINUATION_USE_ALLOCA
 * @see CONTINUATION_EXTEND_STACK_FRAME()
 */
#ifndef CONTINUATION_STACK_FRAME_SIZE
# define CONTINUATION_STACK_FRAME_SIZE /* Empty definition for Doxygen */
# undef CONTINUATION_STACK_FRAME_SIZE
#endif

/**
 * @def CONTINUATION_STACK_BLOCK_SIZE
 * @brief Size of memory block to extend stack frame by recursive call stubs.
 * 
 * The size will apply to a byte array on the stack in the recursive
 * continuation invoke stub function to simulate the dynamic stack allocation.
 */
#ifndef CONTINUATION_STACK_BLOCK_SIZE
# define CONTINUATION_STACK_BLOCK_SIZE /* Empty definition for Doxygen */
# undef CONTINUATION_STACK_BLOCK_SIZE
#endif

/**
 * @internal
 * @def continuation_stub_setjmp
 * @brief Defined to the actual setjmp() to set the return buf of a continuation stub.
 * @see continuation_stub_longjmp
 */
#ifndef continuation_stub_setjmp
# define continuation_stub_setjmp /* Empty definition for Doxygen */
# undef continuation_stub_setjmp
#endif

/**
 * @internal
 * @def continuation_stub_longjmp
 * @brief Defined to the actual longjmp() to return from continuation through a continuation stub.
 * @see continuation_stub_setjmp
 */
#ifndef continuation_stub_longjmp
# define continuation_stub_longjmp /* Empty definition for Doxygen */
# undef continuation_stub_longjmp
#endif

/** @cond */
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

/* unknown platform or force pure c code using longjmp */
#include "compiler_unknown.h"

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

/* some platforms/compilations omit the stack frame pointer
 * and addressing of local variables is based on stack pointer,
 * so that the library can not deduce the size of stack
 * frame by the continuation_init_frame_tail() call.
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

#if defined(CONTINUATION_STACK_FRAME_REVERSE) \
      && !CONTINUATION_STACK_FRAME_REVERSE \
      && !CONTINUATION_USE_C99_VLA \
      && !CONTINUATION_USE_ALLOCA \
      && !defined(CONTINUATION_EXTEND_STACK_FRAME)
# if !defined(CONTINUATION_STACK_FRAME_SIZE)
#   error "must define CONTINUATION_STACK_FRAME_SIZE without dynamic stack frame stretching support"
# endif
#endif

/*
 * length of bytes to be appended to stack frame evaluated by the library
 */
#if !defined(CONTINUATION_STACK_FRAME_PADDING)
# if defined(__x86_64__) || defined(_WIN64)
#   define CONTINUATION_STACK_FRAME_PADDING 128
# else
#   define CONTINUATION_STACK_FRAME_PADDING 0
# endif
#endif

/*
 * length of bytes to reserved in stack for the parameters of host function.
 */
#if !defined(CONTINUATION_STACK_PARAMETERS_SIZE)
# define CONTINUATION_STACK_PARAMETERS_SIZE 128
#endif

#ifndef CONTINUATION_USE_C99_VLA
# define CONTINUATION_USE_C99_VLA 0
#endif

#ifndef CONTINUATION_USE_ALLOCA
# define CONTINUATION_USE_ALLOCA 0
#endif

/* 
 * size of memory block for stack allocation and other purposes.
 */
#ifndef CONTINUATION_STACK_BLOCK_SIZE
# define CONTINUATION_STACK_BLOCK_SIZE 1024
#endif

#if defined(__GNUC__) && (__GNUC__ >= 4 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3))
# define CONTINUATION_ATTRIBUTE_MAY_ALIAS __attribute__((__may_alias__))
#else
# define CONTINUATION_ATTRIBUTE_MAY_ALIAS
#endif

#if defined (__LP64__) || defined (_LP64) || defined (__64BIT__) /* IBM XL */ || defined(_WIN64) /* MSVC */
# ifndef __SIZEOF_SIZE_T__
#   define __SIZEOF_SIZE_T__ 8
# endif
#endif

#ifndef continuation_stub_setjmp
# define continuation_stub_setjmp setjmp
#endif

#ifndef continuation_stub_longjmp
# define continuation_stub_longjmp longjmp
#endif

/** @endcond */

#endif /* __CONTINUATION_CONFIG_H */
