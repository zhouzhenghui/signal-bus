/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CONINUATION_PTHREAD_H
#define __CONINUATION_PTHREAD_H

#if !defined(HAVE_PTHREAD)
# if defined(__CONTINUATION_H)
#  error "should just include the \"contiuation_pthread.h\" for asynchonous facility implemetation under pthread specification"
# endif
# define HAVE_PTHREAD 1
#endif

#define __ASYNC_TASK __async_task

#include "continuation.h"
#include <pthread.h>

/* async type and variable */
extern pthread_key_t __async_pthread_key;

struct __AsyncTask {
  struct __ContinuationStub cont_stub;
  struct __Continuation cont;
  int quitable;
  pthread_mutex_t mutex;
  pthread_cond_t running;
};

STATIC_ASSERT(offsetof(struct __AsyncTask, cont_stub) == 0, self_contraint_of_internal_struct_AsyncTask);

extern struct __AsyncTask *(*__async_copy_stack_frame)(struct __AsyncTask *);
#ifdef __cplusplus
extern "C" {
#endif
  extern pthread_t __async_pthread_create();
#ifdef __cplusplus
} /* extern "C" */
#endif

#if BOOST_PP_VARIADICS
# define ASYNC_RUN(...) __ASYNC_RUN((__VA_ARGS__))
#else
# define ASYNC_RUN __ASYNC_RUN
#endif

#define __ASYNC_RUN(continuation) \
    __async_pthread_create(); \
    { \
      struct __AsyncTask *__ASYNC_TASK = (struct __AsyncTask *)pthread_getspecific(__async_pthread_key); \
      assert(__ASYNC_TASK != NULL); \
      CONTINUATION_CONNECT(&__ASYNC_TASK->cont, __ASYNC_TASK \
        , () \
        , ( \
            __ASYNC_TASK = __async_copy_stack_frame(__ASYNC_TASK); \
            pthread_cond_signal(&__ASYNC_TASK->running); \
            { \
              __PP_REMOVE_PARENS(continuation); \
            } \
            CONTINUATION_DESTRUCT(&__ASYNC_TASK->cont); \
        ) \
      ); \
      pthread_cond_wait(&__ASYNC_TASK->running, &__ASYNC_TASK->mutex); \
      __ASYNC_TASK->quitable = 1; \
      pthread_cond_signal(&__ASYNC_TASK->running); \
      pthread_mutex_unlock(&__ASYNC_TASK->mutex); \
    }

#define ASYNC_HOST_VAR_ADDR(a) \
  CONTINUATION_HOST_VAR_ADDR(&__ASYNC_TASK->cont_stub, a)

#define ASYNC_HOST_VAR(a) \
  CONTINUATION_HOST_VAR(&__ASYNC_TASK->cont_stub, a)

#endif /* __CONINUATION_PTHREAD_H */
