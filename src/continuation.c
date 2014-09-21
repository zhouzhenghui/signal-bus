/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#include "continuation/continuation_base.h"

int __continuation_jmpbuf_initialized = 0;
int __continuation_jmpcode[sizeof(jmp_buf) / sizeof(void *)] = { 0 };
void(*__continuation_enforce_var)(void * volatile) = 0;

/* this function should not be compiled with any optimization */
#if defined(__GNUC__) && ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || __GNUC__ > 4)
  static void continuation_diff_jmpbuf_help(jmp_buf *) __attribute__((optimize("no-omit-frame-pointer")));
  static void continuation_diff_jmpbuf(void) __attribute__((optimize("no-omit-frame-pointer")));
#else
  static void continuation_diff_jmpbuf_help(jmp_buf *);
  static void continuation_diff_jmpbuf(void);
#endif

static void continuation_invoke_helper(struct __ContinuationStub *cont_stub);
static void continuation_init_invoke_helper(struct __ContinuationStub *cont_stub, const void *stack_frame_spot);
static void*continuation_init_frame_tail(void *);
static struct __ContinuationStub *__continuation_restore_stack_frame(const struct __ContinuationStub *cont_stub, void *stack_frame);

/* these function pointers prevent link-time optimization */
void (*__continuation_diff_jmpbuf)(void) = continuation_diff_jmpbuf;
void (*__continuation_invoke_helper)(struct __ContinuationStub *) = continuation_invoke_helper;
void*(*__continuation_init_frame_tail)(void *) = continuation_init_frame_tail;
void (*__continuation_init_invoke_helper)(struct __ContinuationStub *, const void *) = continuation_init_invoke_helper;
struct __ContinuationStub *(*continuation_restore_stack_frame)(const struct __ContinuationStub *cont_stub, void *stack_frame) = __continuation_restore_stack_frame;

static void continuation_diff_jmpbuf_help(jmp_buf *environment)
{
  setjmp(*environment);
  FORCE_NO_OMIT_FRAME_POINTER();
}

static void continuation_diff_jmpbuf(void)
{
  static void(* volatile diff_jmpbuf_help)(jmp_buf *) = &continuation_diff_jmpbuf_help;
  volatile int jmpcode[sizeof(jmp_buf) / sizeof(void *)];
  jmp_buf jmpbuf1, jmpbuf2;
  if (setjmp(jmpbuf1) == 0) {
    memcpy((void *)&jmpbuf2, (void *)&jmpbuf1, sizeof(jmp_buf));
    if (setjmp(jmpbuf1) == 0) {
      int i, j = 0;
      for (i = 0; i < sizeof(jmp_buf) / sizeof(void *); i++) {
        if (((void **)&jmpbuf1)[i] != ((void **)&jmpbuf2)[i]) {
          jmpcode[j++] = i;
        }
      }
      jmpcode[j] = -1;
      memcpy((void *)&jmpbuf1, (void *)&jmpbuf2, sizeof(jmp_buf));
    }
    longjmp(jmpbuf1, 1);
  } else {
    diff_jmpbuf_help(&jmpbuf1);
    {
      volatile int i, j = 0, k = 0;
      for (i = 0; i < sizeof(jmp_buf) / sizeof(void *); i++) {
        if (((void **)&jmpbuf1)[i] != ((void **)&jmpbuf2)[i]) {
          while(jmpcode[j] >= 0 && jmpcode[j] < i) j++;
          if (jmpcode[j] != i) {
            __continuation_jmpcode[k++] = i;
          }
        }
      }
      __continuation_jmpcode[k] = -1;
    }
  }
  __continuation_jmpbuf_initialized = 1;
  FORCE_NO_OMIT_FRAME_POINTER();
}

void __continuation_patch_jmpbuf(jmp_buf *dst, jmp_buf *src)
{
  int i;
  for (i = 0; __continuation_jmpcode[i] >= 0; i++) {
    ((void **)dst)[__continuation_jmpcode[i]] = ((void **)src)[__continuation_jmpcode[i]];
  }
}

/* a proxy for stack frame pre-expand and guard bypass */
static void continuation_invoke_helper(struct __ContinuationStub *cont_stub)
{
  volatile char anti_optimize[1];
  cont_stub->addr.stack_frame_addr = (char *)&anti_optimize[0];
  cont_stub->cont->invoke(cont_stub);
  anti_optimize[0] = 0; /* prevent tail-call optimization */
}

/* to ensure the longjmp can be finished when stack frame size is unknown */
static void continuation_init_invoke_helper(struct __ContinuationStub *cont_stub, const void *stack_frame_spot)
{
  assert(cont_stub->cont->stack_frame_tail > (char *)stack_frame_spot + sizeof(void *) && "TODO: incompatible compilation/optimization");
  cont_stub->size.stack_frame_offset = (size_t)stack_frame_spot - (size_t)cont_stub->cont->stack_frame_spot;
  cont_stub->cont->stack_frame_addr -= cont_stub->size.stack_frame_offset;
  longjmp(cont_stub->return_buf, 1);
}

/* I'd like to make the function prototype match with continuation_restore_stack_frame
 * to ensure the calling of later will reserve the stack space of parameters
 * and return address, especially required for reversed stack frame.
 * if any stack gap need to fill, in case of data losing, try increasing the
 * value of macro CONTINUATION_STACK_FRAME_PADDING.
 */
static void *continuation_init_frame_tail(void *null_ptr)
{
  static size_t continuation_frame_tail_forward = 0;
  void *tail;
  if (continuation_frame_tail_forward == 0) {
    if (null_ptr) {
      continuation_frame_tail_forward = (size_t)null_ptr - (size_t)&tail;
      assert(continuation_frame_tail_forward > 0 && "TODO: hasn't yet support a stack which grows upward or strange");
      return NULL;
    } else {
      __continuation_init_frame_tail(&tail);
    }
  }
  tail = (void *)((size_t)&tail + continuation_frame_tail_forward);
  return tail;
}

static struct __ContinuationStub *__continuation_restore_stack_frame(const struct __ContinuationStub *cont_stub, void *stack_frame)
{
  memcpy(cont_stub->addr.stack_frame_tail, stack_frame, cont_stub->cont->stack_frame_size);
  return (struct __ContinuationStub *)cont_stub;
}
