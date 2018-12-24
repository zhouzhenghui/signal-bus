/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CONTINUATION_BASE_H
#define __CONTINUATION_BASE_H

#include "continuation_config.h"
#include "misc/continuation_inline.h"
#include "misc/no_omit_frame_pointer.h"
#include "misc/vector.h"

#include <setjmp.h>

struct __ContinuationStub;

struct __Continuation {
  int initialized;
  char *stack_frame_addr;
  const char *stack_frame_spot;
  char *stack_frame_tail;
  size_t stack_frame_size;
  size_t stack_parameters_size;
  size_t offset_to_frame_tail;
  void(*invoke)(struct __ContinuationStub *);
  void *func_addr;
  jmp_buf invoke_buf;
};

struct __ContinuationStub {
  struct __Continuation *cont;
  union {
    char *stack_frame_addr;
    char *stack_frame_tail;
  } addr;
  union {
    size_t stack_frame_size;
    ptrdiff_t stack_frame_offset;
  } size;
  jmp_buf return_buf;
};

inline static void continuation_init(struct __Continuation *cont, const void *stack_frame_spot)
{
  cont->initialized = 0;
  cont->stack_frame_addr = NULL;
  cont->stack_frame_tail = NULL;
  cont->stack_frame_size = 0;
  cont->stack_parameters_size = CONTINUATION_STACK_PARAMETERS_SIZE;
  cont->stack_frame_spot = (const char *)stack_frame_spot;
  cont->invoke = NULL;
}

extern void (*__continuation_enforce_var)(char * volatile);
extern void (*__continuation_invoke_helper)(struct __ContinuationStub *);
extern void (*__continuation_init_invoke_return)(struct __ContinuationStub *, const void *);
extern void *(*__continuation_init_frame_tail)(void *, void *);
extern struct __ContinuationStub *(*continuation_restore_stack_frame)(const struct __ContinuationStub *cont_stub, void *stack_frame);

inline static void continuation_stub_init(struct __ContinuationStub *cont_stub, struct __Continuation *cont)
{
  cont_stub->cont = cont;
}

inline static void continuation_invoke(struct __Continuation *cont)
{
  struct __ContinuationStub cont_stub;
  cont_stub.cont = cont;
  if (setjmp(cont_stub.return_buf) == 0) {
    __continuation_invoke_helper(&cont_stub);
  }
}

inline static void continuation_stub_invoke(struct __ContinuationStub *cont_stub)
{
  if (setjmp(cont_stub->return_buf) == 0) {
    __continuation_invoke_helper(cont_stub);
  }
}

inline static void continuation_backup_stack_frame(const struct __Continuation *cont, void *stack_frame)
{
  memcpy(stack_frame, cont->stack_frame_tail, cont->stack_frame_size);
}

#endif /* __CONTINUATION_BASE_H */
