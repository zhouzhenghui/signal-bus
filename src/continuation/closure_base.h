/*
 * Copyright 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CLOSURE_BASE_H
#define __CLOSURE_BASE_H

#include "continuation_base.h"
#include <boost/preprocessor/inc.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/tuple.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <preprocessor/variadic_size_or_zero.h>

struct __ClosureVar {
  void *addr;
  size_t size;
  void *value;
};

struct __ClosureVarDebug {
  const char *name;
  void *addr;
  size_t size;
  void *value;
};

typedef VECTOR(struct __ClosureVar) __ClosureVarVector;
typedef VECTOR(struct __ClosureVarDebug) __ClosureVarDebugVector;

struct __Closure {
  struct __Continuation cont; /* first field may be faster */
  int connected;
#ifdef CLOSURE_DEBUG
  __ClosureVarDebugVector argv;
#else
  __ClosureVarVector argv;
#endif
  char *frame;
};

struct __ClosureStub {
  struct __ContinuationStub cont_stub;
  struct __Closure *closure;
};

STATIC_ASSERT(offsetof(struct __ClosureStub, cont_stub) == 0, self_constraint_of_internal_struct_ClosureStub);

#define __CLOSURE_FIELDS(z, n, seq) \
  BOOST_PP_SEQ_ELEM(n, seq) BOOST_PP_CAT(_, BOOST_PP_INC(n));

#define CLOSURE_N(n, tuple) \
struct { \
  struct __Closure closure; \
  struct { \
      BOOST_PP_REPEAT(n, __CLOSURE_FIELDS, BOOST_PP_TUPLE_TO_SEQ(n, tuple)) \
      char end; /* for MSVC compatible */ \
  } arg; \
}

struct __ClosureEmpty { struct __Closure closure; struct { char end; } arg; };

#define CLOSURE_EMPTY_SIZE \
  sizeof(struct __ClosureEmpty)

#ifdef __GNUC__
# define CLOSURE_IS_EMPTY(closure_ptr) \
  (&((__typeof__((closure_ptr)->arg)*)0)->end == 0)
#else
# define CLOSURE_IS_EMPTY(closure_ptr) \
  ((void *)&(closure_ptr)->arg == (void *)&(closure_ptr)->arg.end)
#endif

#if BOOST_PP_VARIADICS
# define CLOSURE(...) CLOSURE_N(__PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CLOSURE CLOSURE_N
#endif

#define CLOSURE0() CLOSURE_N(0, ())
#define CLOSURE1(t1) CLOSURE_N(1, (t1))
#define CLOSURE2(t1, t2) CLOSURE_N(2, (t1, t2))
#define CLOSURE3(t1, t2, t3) CLOSURE_N(3, (t1, t2, t3))
#define CLOSURE4(t1, t2, t3, t4) CLOSURE_N(4, (t1, t2, t3, t4))
#define CLOSURE5(t1, t2, t3, t4, t5) CLOSURE_N(5, (t1, t2, t3, t4, t5))
#define CLOSURE6(t1, t2, t3, t4, t5, t6) CLOSURE_N(6, (t1, t2, t3, t4, t5, t6))
#define CLOSURE7(t1, t2, t3, t4, t5, t6, t7) CLOSURE_N(7, (t1, t2, t3, t4, t5, t6, t7))
#define CLOSURE8(t1, t2, t3, t4, t5, t6, t7, t8) CLOSURE_N(8, (t1, t2, t3, t4, t5, t6, t7, t8))
#define CLOSURE9(t1, t2, t3, t4, t5, t6, t7, t8, t9) CLOSURE_N(9, (t1, t2, t3, t4, t5, t6, t7, t8, t9))

inline static void closure_init(struct __Closure *closure)
{
  closure->connected = 0;
  VECTOR_INIT(&closure->argv);
  /* closure->frame = NULL; */
}

extern void(* closure_run)(void *closure);
#ifdef __cplusplus
extern "C" {
#endif
  extern void closure_free(struct __Closure *closure);
  extern void __closure_init_vars(struct __Closure *closure, __ClosureVarVector *argv);
  extern void __closure_init_vars_debug(struct __Closure *closure, __ClosureVarDebugVector *argv, const char *file, unsigned int line);
  extern void __closure_commit_vars(__ClosureVarVector *argv, size_t stack_frame_offset);
  extern void __closure_commit_vars_debug(__ClosureVarDebugVector *argv, size_t stack_frame_offset, const char *file, unsigned int line);
#ifdef __cplusplus
}
#endif

#endif /* __CLOSURE_BASE_H */
