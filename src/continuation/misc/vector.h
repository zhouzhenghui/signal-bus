/*
 * Copyright 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <string.h>
#include <static_assert/static_assert.h>

struct __Vector {
  char *item;
  size_t size;
  size_t alloc;
};

#define VECTOR(type) struct {type *item; size_t size; size_t alloc;}
#define VECTOR_STATIC_INITIALIZER() {NULL,0,0}
#define VECTOR_INIT(vector) do {(vector)->item=NULL; (vector)->size=0; (vector)->alloc=0;} while(0)
#define VECTOR_FREE(vector) do {free((vector)->item);} while(0)

#define VECTOR_ITEM(vector, i) ((vector)->item[i])
#define VECTOR_SIZE(vector) ((vector)->size)
#define VECTOR_ADDR(vector) ((vector)->item)

#define VECTOR_APPEND_ITEM(vector, elem) \
do { \
  size_t __new_size = (vector)->size + 1; \
  (void)STATIC_ASSERT_OR_ZERO(sizeof(struct __Vector) == sizeof(*(vector)), uncompatible_VECTOR_type); \
  __vector_resize((struct __Vector *)(vector), __new_size, sizeof(*(vector)->item)); \
  /* Some optimization (e.g. gcc) break the sequence between here, so use a __new_size to avoid it */ \
  (vector)->item[__new_size - 1] = elem; \
} while (0)

#if BOOST_PP_VARIADICS
# define VECTOR_APPEND(vector, ...) \
do { \
  size_t __new_size = (vector)->size + 1; \
  (void)STATIC_ASSERT_OR_ZERO(sizeof(struct __Vector) == sizeof(*(vector)), uncompatible_VECTOR_type); \
  __vector_resize((struct __Vector *)(vector), __new_size, sizeof(*(vector)->item)); \
  /* Some optimization (e.g. gcc) break the sequence between here, so use a __new_size to avoid it */ \
  (vector)->item[__new_size - 1] = __VA_ARGS__; \
} while (0)
#else
# define VECTOR_APPEND VECTOR_APPEND_ITEM
#endif

#define VECTOR_APPEND_ITEMS(vector, count, items) \
do { \
  (void)STATIC_ASSERT_OR_ZERO(sizeof(struct __Vector) == sizeof(*(vector)), uncompatible_VECTOR_type); \
  (void)STATIC_ASSERT_OR_ZERO(sizeof(*((vector)->item)) == sizeof(*(items)), append_VECTOR_with_different_type_of_items); \
  if (0) { (vector)->item[0] = (items)[0]; } \
  __vector_append((struct __Vector *)(vector), count, sizeof(*(items)), items); \
} while (0)

#define VECTOR_RESERVE(vector, count) \
do { \
  (void)STATIC_ASSERT_OR_ZERO(sizeof(struct __Vector) == sizeof(*(vector)), uncompatible_VECTOR_type); \
  __vector_reserve((struct __Vector *)(vector), (vector)->size + count, sizeof(*((vector)->item))); \
} while (0)

#define VECTOR_RESIZE(vector, new_size) \
do { \
  (void)STATIC_ASSERT_OR_ZERO(sizeof(struct __Vector) == sizeof(*(vector)), uncompatible_VECTOR_type); \
  __vector_resize((struct __Vector *)(vector), new_size, sizeof(*((vector)->item))); \
} while (0)

#define VECTOR_CLEAR(vector) \
do { \
  (void)STATIC_ASSERT_OR_ZERO(sizeof(struct __Vector) == sizeof(*(vector)), uncompatible_VECTOR_type); \
  (vector)->size = 0; \
} while (0)

inline static void __vector_reserve(struct __Vector *vector, size_t new_size, size_t item_size)
{
  if (vector->alloc > new_size) return;
  if (!vector->item) vector->alloc = 1;
  while (vector->alloc < new_size) vector->alloc <<= 1;
  vector->item = realloc(vector->item, vector->alloc * item_size);
}

inline static void __vector_resize(struct __Vector *vector, size_t new_size, size_t item_size)
{
  __vector_reserve(vector, new_size, item_size);
  vector->size = new_size;
}

inline static void __vector_append(struct __Vector *vector, size_t count, size_t item_size, void *items)
{
  size_t old_size = vector->size;
  size_t new_size = vector->size + count;
  __vector_resize(vector, new_size, item_size);
  memcpy(vector->item + old_size * item_size, items, count * item_size);
}

#define VECTOR_FOREACH(i, vector) \
  for ((i) = &(vector)->item[0]; (i) < &(vector)->item[(vector)->size]; (i)++)

#define VECTOR_FOREACH_REVERSE(i, vector) \
  for ((i) = &(vector)->item[size]; (i)-- < &(vector)->item[(vector)->size];)

#endif /* VECTOR_H */
