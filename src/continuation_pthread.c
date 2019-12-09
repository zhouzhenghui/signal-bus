/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#include "continuation/continuation_pthread.h"

pthread_once_t __continuation_pthread_once = PTHREAD_ONCE_INIT;
pthread_key_t __async_pthread_key;
int __continuation_pthread_jmpbuf_initialized = 0;
int __continuation_pthread_jmpcode[sizeof(jmp_buf) / sizeof(void *)] = { 0 };

static pthread_mutex_t jmpbuf_mutex;
/* these functions should be compiled without any optimization */
#if defined(__GNUC__) && ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || __GNUC__ > 4)
  static void *continuation_pthread_diff_jmpbuf_help(jmp_buf *) __attribute__((optimize("no-omit-frame-pointer")));
  static void continuation_pthread_diff_jmpbuf(void) __attribute__((optimize("no-omit-frame-pointer")));
#else
  static void *continuation_pthread_diff_jmpbuf_help(jmp_buf *);
  static void continuation_pthread_diff_jmpbuf(void);
#endif

static void * __async_pthread_run(struct __AsyncTask * async_task);
static struct __AsyncTask *async_copy_stack_frame(struct __AsyncTask *async_task);

/* these function pointers prevent link-time optimization */
void(*__continuation_pthread_diff_jmpbuf)(void) = &continuation_pthread_diff_jmpbuf;
struct __AsyncTask *(*__async_copy_stack_frame)(struct __AsyncTask *) = &async_copy_stack_frame;

static void *continuation_pthread_diff_jmpbuf_help(jmp_buf *environment)
{
  pthread_mutex_lock(&jmpbuf_mutex);
  continuation_setjmp(*environment);
  pthread_mutex_unlock(&jmpbuf_mutex);
  FORCE_NO_OMIT_FRAME_POINTER();
  return NULL;
}

static void continuation_pthread_diff_jmpbuf(void)
{
  static void *(* volatile diff_jmpbuf_help)(jmp_buf *) = &continuation_pthread_diff_jmpbuf_help;
  volatile int jmpcode[sizeof(jmp_buf) / sizeof(void *)];
  jmp_buf jmpbuf1, jmpbuf2;
  pthread_t pthread_id;
  pthread_mutex_init(&jmpbuf_mutex, NULL);
  pthread_mutex_lock(&jmpbuf_mutex);
  continuation_setjmp(jmpbuf1);
  pthread_create(&pthread_id, NULL, (void *(*)(void *))diff_jmpbuf_help, (void *)&jmpbuf1);;
  memcpy((void *)&jmpbuf2, (void *)&jmpbuf1, sizeof(jmp_buf));
  continuation_setjmp(jmpbuf1);
  {
    int i, j = 0;
    for (i = 0; i < sizeof(jmp_buf) / sizeof(void *); i++) {
      if (((void **)&jmpbuf1)[i] != ((void **)&jmpbuf2)[i]) {
        jmpcode[j++] = i;
      }
    }
    jmpcode[j] = -1;
  }

  pthread_mutex_unlock(&jmpbuf_mutex);
  pthread_join(pthread_id, NULL);
  {
    volatile int i, j = 0, k = 0;
    for (i = 0; i < sizeof(jmp_buf) / sizeof(void *); i++) {
      if (((void **)&jmpbuf1)[i] != ((void **)&jmpbuf2)[i]) {
        while(jmpcode[j] >= 0 && jmpcode[j] < i) j++;
        if (jmpcode[j] != i) {
          __continuation_pthread_jmpcode[k++] = i;
        }
      }
    }
    __continuation_pthread_jmpcode[k] = -1;
  }
  __continuation_pthread_jmpbuf_initialized = 1;
  pthread_mutex_destroy(&jmpbuf_mutex);
  FORCE_NO_OMIT_FRAME_POINTER();
}

void __continuation_pthread_patch_jmpbuf(jmp_buf *dest, jmp_buf *src)
{
  int i;
  for (i = 0; __continuation_pthread_jmpcode[i] >= 0; i++) {
    ((void **)dest)[__continuation_pthread_jmpcode[i]] = ((void **)src)[__continuation_pthread_jmpcode[i]];
  }
}

static void make_key()
{
  pthread_key_create(&__async_pthread_key, NULL);
}

pthread_t __async_pthread_create()
{
  static pthread_once_t __async_pthread_once = PTHREAD_ONCE_INIT;
  pthread_t pthread_id;
  int error;
  struct __AsyncTask *async_task = (struct __AsyncTask *)malloc(sizeof(struct __AsyncTask));
  pthread_once(&__async_pthread_once, make_key);
  async_task->quitable = 0;
  pthread_mutex_init(&async_task->mutex, NULL);
  pthread_cond_init(&async_task->running, NULL);
  /* run thread with async_task as it's argument */
  pthread_mutex_lock(&async_task->mutex);
  error = pthread_create(&pthread_id, NULL, (void *(*)(void *))&__async_pthread_run, (void *)async_task);
  if (error) {
      free(async_task);
      async_task = NULL;
  }
  pthread_setspecific(__async_pthread_key, async_task);
  return pthread_id;
}

static void * __async_pthread_run(struct __AsyncTask * async_task)
{
  /* ensure the parent thread had prepared the continuation */
  pthread_mutex_lock(&async_task->mutex);
  pthread_mutex_unlock(&async_task->mutex);
  assert(async_task->cont_stub.cont == &async_task->cont);
  continuation_stub_invoke(&async_task->cont_stub);
  /* ensure the parent thread had released the lock */
  pthread_mutex_lock(&async_task->mutex);
  if (!async_task->quitable) {
    pthread_cond_wait(&async_task->running, &async_task->mutex);
  }
  pthread_mutex_unlock(&async_task->mutex);
  pthread_cond_destroy(&async_task->running);
  pthread_mutex_destroy(&async_task->mutex);
  free(async_task);
  return NULL;
}

static struct __AsyncTask *async_copy_stack_frame(struct __AsyncTask *async_task)
{
  struct __ContinuationStub *cont_stub = &async_task->cont_stub;
  struct __Continuation *cont = &async_task->cont;
  memcpy(cont_stub->addr.stack_frame_tail
          , cont->stack_frame_tail
          , cont->stack_frame_size);
  return async_task;
}
