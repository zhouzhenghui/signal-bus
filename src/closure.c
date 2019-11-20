/*
 * Copyright 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#include <stdio.h>
#include "continuation/closure_base.h"

void __closure_invoke(struct __Closure *closure)
{
  struct __ClosureStub closure_stub;
  closure_stub.closure = closure;
  continuation_stub_init(&closure_stub.cont_stub, &closure->cont);
  continuation_stub_invoke(&closure_stub.cont_stub);
}

void __closure_init_vars(struct __Closure *closure, __ClosureVarVector *argv)
{
  struct __ClosureVar *arg;
  VECTOR_FOREACH(arg, argv) {
    size_t offset = (size_t)arg->addr - (size_t)closure->cont.stack_frame_tail;
    arg->value = (char *)closure->frame + offset;
  }
}

void __closure_init_vars_debug(struct __Closure *closure, __ClosureVarDebugVector *argv, const char *file, unsigned int line)
{
  struct __ClosureVarDebug *arg;
  VECTOR_FOREACH(arg, argv) {
    size_t offset = (size_t)arg->addr - (size_t)closure->cont.stack_frame_tail;
#if defined(__SIZEOF_SIZE_T__) && __SIZEOF_SIZE_T__ >= 8
# if defined(_WIN64) /* MSC or MINGW */
    fprintf(stderr, "[CLOSURE_DEBUG] The variable \"%s\" has an offset of %lld in %lld bytes stack frame. at: file \"%s\", line %d\n"
            , arg->name, offset, closure->cont.stack_frame_size, file, line);
# else
    fprintf(stderr, "[CLOSURE_DEBUG] The variable \"%s\" has an offset of %zd in %zd bytes stack frame. at: file \"%s\", line %d\n"
            , arg->name, offset, closure->cont.stack_frame_size, file, line);
# endif
#else
    fprintf(stderr, "[CLOSURE_DEBUG] The variable \"%s\" has an offset of %d in %d bytes stack frame. at: file \"%s\", line %d\n"
            , arg->name, offset, closure->cont.stack_frame_size, file, line);
#endif
    arg->value = (char *)closure->frame + offset;
  }
}

void __closure_commit_vars(__ClosureVarVector *argv, size_t stack_frame_offset)
{
  struct __ClosureVar *arg;
  VECTOR_FOREACH(arg, argv) {
    memcpy(arg->value, (char *)arg->addr + stack_frame_offset, arg->size);
  }
}

void __closure_commit_vars_debug(__ClosureVarDebugVector *argv, size_t stack_frame_offset, const char *file, unsigned int line)
{
  int updated = 0;
  struct __ClosureVarDebug *arg;
  VECTOR_FOREACH(arg, argv) {
    if (memcmp(arg->value, (char *)arg->addr + stack_frame_offset, arg->size)) {
      if (!updated) {
        fprintf(stderr, "[CLOSURE_DEBUG] Retain modification of variables: \"%s\"", arg->name);
        updated = 1;
      } else {
        fprintf(stderr, ", \"%s\"", arg->name);
      }
      memcpy(arg->value, (char *)arg->addr + stack_frame_offset, arg->size);
    }
  }
  if (updated) fprintf(stderr, ". at: file \"%s\", line %d\n", file, line);
}
