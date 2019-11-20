/*
 * Copyright 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CONTINUATION_VECTOR_H
#define __CONTINUATION_VECTOR_H

#include <stdlib.h>
#include <string.h>
#include <static_assert/static_assert.h>

/**
 * @file
 * @ingroup continuation
 * @brief Dynamic array type of user specified objects as C++ vector.
 */

/**
 * @internal
 * @brief Internal type for generic vector container.
 */
struct __Vector {
  void *item; /**< Void pointer to the dynamical allocated memory. */
  size_t size; /**< Size of the vector. */
  size_t alloc; /**< Size of the allocated buffer for the vector. */
}
#if defined(__GNUC__) && (__GNUC__ >= 4 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3))
__attribute__((__may_alias__))
#endif
;

/**
 * @brief The vector type of user specified objects.
 * @param type: The type of user objects.
 * 
 * @see VECTOR_STATIC_INITIALIZER()
 * @see VECTOR_INIT()
 * @see VECTOR_APPEND()
 * @see VECTOR_FREE()
 *
 * @par Example:
 * @code
 *   VECTOR(int) int_array;
 * @endcode
 */
#define VECTOR(type) struct {type *item; size_t size; size_t alloc;}

/**
 * @brief Static initializer for a vector at declaration.
 *
 * @see VECTOR()
 * @see VECTOR_INIT()
 *
 * @par Example:
 * @code
 *   VECTOR(int) int_array = VECTOR_STATIC_INITIALIZER();
 * @endcode
 */
#define VECTOR_STATIC_INITIALIZER() {NULL, 0, 0}

/**
 * @brief Initialize a vector at runtime.
 * @param vector: pointer to the vector.
 *
 * @see VECTOR()
 * @see VECTOR_STATIC_INITIALIZER()
 *
 * @par Example:
 * @code
 *   VECTOR(int) int_array;
 *   VECTOR_INIT(&int_array);
 * @endcode
 */
#define VECTOR_INIT(vector) do {(vector)->item=NULL; (vector)->size=0; (vector)->alloc=0;} while(0)

/**
 * @brief Free a vector.
 * @param vector: pointer to the vector.
 *
 * @see VECTOR()
 *
 * @par Example:
 * @code
 *   VECTOR(int) int_array;
 *   VECTOR_INIT(&int_array);
 * @endcode
 */
#define VECTOR_FREE(vector) do {free((vector)->item);} while(0)

/**
 * @brief Access specified item.
 * @details It is evaluated as the reference to the requested element.
 * @param vector: pointer to the vector.
 * @param i: index in the vector.
 * 
 * @see VECTOR()
 * 
 * @par Example:
 * @code
 *   VECTOR(int) int_array;
 *   int i = VECTOR_ITEM(&int_array, 0);
 * @endcode
 */
#define VECTOR_ITEM(vector, i) ((vector)->item[i])

/**
 * @brief Size of a vector.
 * @param vector: pointer to the vector.
 * 
 * @see VECTOR()
 * 
 * @par Example:
 * @code
 *   VECTOR(int) int_array;
 *   VECTOR_SIZE(&int_array);
 * @endcode
 */
#define VECTOR_SIZE(vector) ((vector)->size)

/**
 * @brief Get the array buffer of a vector.
 * @param vector: pointer to the vector.
 * @return pointer to the array buffer.
 *
 * @see VECTOR()
 * 
 * @par Example:
 * @code
 *   VECTOR(int) int_array;
 *   int *p = VECTOR_ADDR(&int_array);
 * @endcode
 */
#define VECTOR_ADDR(vector) ((vector)->item)

/**
 * @brief Append an item to a vector.
 * @param vector: pointer to the vector.
 * @param elem: the element to be appended.
 *
 * @see VECTOR()
 * @see VECTOR_APPEND()
 * 
 * @par Example:
 * @code
 *   VECTOR(int) int_array;
 *   VECTOR_APPEND
 * @endcode
 */
#define VECTOR_APPEND_ITEM(vector, elem) \
do { \
  size_t __new_size = (vector)->size + 1; \
  (void)STATIC_ASSERT_OR_ZERO(sizeof(struct __Vector) == sizeof(*(vector)), incompatible_VECTOR_type); \
  __vector_resize((struct __Vector *)(vector), __new_size, sizeof((vector)->item[0])); \
  /* Some optimization (e.g. gcc) break the sequence between here, so use a __new_size to avoid it */ \
  (vector)->item[__new_size - 1] = elem; \
} while (0)

/**
 * @brief Append an item to a vector.
 * 
 * @details Same as VECTOR_APPEND_ITEM() but allowing the item to be passed as literal value
 * if BOOST_PP_VARIADICS is 1.
 *
 * @param vector: pointer to the vector.
 * @param ...: an element or a literal value encapsulated with "{ }" to be appended.
 *
 * @see VECTOR()
 * @see VECTOR_APPEND_ITEM()
 *
 * @par Example:
 * @code
 *   struct complex { int i; int ; };
 *   VECTOR(struct complex) complex_array;
 *   VECTOR_APPEND(&complex_array, { 1, 1 });
 * @endcode
 */
#define VECTOR_APPEND(vector) /* Empty definition for Doxygen */
#undef VECTOR_APPEND

/** @cond */
#if BOOST_PP_VARIADICS
# define VECTOR_APPEND(vector, ...) \
do { \
  size_t __new_size = (vector)->size + 1; \
  (void)STATIC_ASSERT_OR_ZERO(sizeof(struct __Vector) == sizeof(*(vector)), incompatible_VECTOR_type); \
  __vector_resize((struct __Vector *)(vector), __new_size, sizeof((vector)->item[0])); \
  /* Some optimization (e.g. gcc) break the sequence between here, so use a __new_size to avoid it */ \
  (vector)->item[__new_size - 1] = __VA_ARGS__; \
} while (0)
#else
# define VECTOR_APPEND(vector, elem) VECTOR_APPEND_ITEM(vector, elem)
#endif
/** @endcond */

/**
 * @brief Append multiple items to a vector.
 * @param vector: pointer to the vector.
 * @param count: number of items to be appended.
 * @param items: pointer to items to be appended.
 *
 * @see VECTOR()
 * @see VECTOR_APPEND_ITEM()
 * 
 * @par Example:
 * @code
 *   VECTOR(int) int_array;
 *   int i[10];
 *   VECTOR_APPEND_ITEMS(&int_array, 10, i);
 * @endcode
 */
#define VECTOR_APPEND_ITEMS(vector, count, items) \
do { \
  (void)STATIC_ASSERT_OR_ZERO(sizeof(struct __Vector) == sizeof(*(vector)), incompatible_VECTOR_type); \
  (void)STATIC_ASSERT_OR_ZERO(sizeof((vector)->item[0]) == sizeof((items)[0]), append_VECTOR_with_different_type_of_items); \
  if (0) { (vector)->item[0] = (items)[0]; } \
  __vector_append((struct __Vector *)(vector), count, sizeof((items)[0]), items); \
} while (0)

/**
 * @brief Reserve capacity of a vector.
 *
 * The capacity of the vector are increased to a value greater or equal to
 * a \p count.
 * 
 * @param vector: pointer to the vector.
 * @param count: minimal number of items that can be stored.
 * 
 * @see VECTOR()
 * @see VECTOR_RESIZE()
 * 
 * @par Example:
 * @code
 *   VECTOR(int) int_array;
 *   VECTOR_RESERVE(&int_array, 10);
 * @endcode
 */
#define VECTOR_RESERVE(vector, count) \
do { \
  (void)STATIC_ASSERT_OR_ZERO(sizeof(struct __Vector) == sizeof(*(vector)), incompatible_VECTOR_type); \
  __vector_reserve((struct __Vector *)(vector), (vector)->size + count, sizeof((vector)->item[0])); \
} while (0)

/**
 * @brief Resize the vector to contain a number of elements.
 *
 * The vector will contains \p count elements after resized.
 * If the current size if greater than the \p count, the vector is reduced.
 * If the current size if less than the \p count, additional elements are appended
 * without initialized.
 *
 * @param vector: pointer to the vector.
 * @param new_size: new size of the vector.
 *
 * @see VECTOR()
 * @see VECTOR_RESERVE()
 * 
 * @par Example:
 * @code
 *   VECTOR(int) int_array;
 *   VECTOR_RESIZE(&int_array, 10);
 *   VECTOR_ITEM(int_array, 0) = 0;
 * @endcode
 */
#define VECTOR_RESIZE(vector, new_size) \
do { \
  (void)STATIC_ASSERT_OR_ZERO(sizeof(struct __Vector) == sizeof(*(vector)), incompatible_VECTOR_type); \
  __vector_resize((struct __Vector *)(vector), new_size, sizeof((vector)->item[0])); \
} while (0)

/**
 * @brief Clear the vector.
 * 
 * The vector will be empty after cleared.
 * 
 * @param vector: pointer to the vector.
 * 
 * @see VECTOR()
 *
 * @par Example:
 * @code
 *   VECTOR_CLEAR(&int_array);
 * @endcode
 */
#define VECTOR_CLEAR(vector) \
do { \
  (void)STATIC_ASSERT_OR_ZERO(sizeof(struct __Vector) == sizeof(*(vector)), incompatible_VECTOR_type); \
  (vector)->size = 0; \
} while (0)

/**
 * @internal
 * @brief Internal function for VECTOR_RESERVE().
 * @see VECTOR_RESERVE()
 */
inline static void __vector_reserve(struct __Vector *vector, size_t new_size, size_t item_size)
{
  if (vector->alloc > new_size) return;
  if (!vector->item) vector->alloc = 1;
  while (vector->alloc < new_size) vector->alloc <<= 1;
  vector->item = realloc(vector->item, vector->alloc * item_size);
}

/**
 * @internal
 * @brief Internal function for VECTOR_RESIZE() and so on.
 * @see VECTOR_RESIZE()
 */
inline static void __vector_resize(struct __Vector *vector, size_t new_size, size_t item_size)
{
  __vector_reserve(vector, new_size, item_size);
  vector->size = new_size;
}

/**
 * @internal
 * @brief Internal function for VECTOR_APPEND() and so on.
 * @see VECTOR_APPEND_ITEM()
 * @see VECTOR_APPEND()
 */
inline static void __vector_append(struct __Vector *vector, size_t count, size_t item_size, void *items)
{
  size_t old_size = vector->size;
  size_t new_size = vector->size + count;
  __vector_resize(vector, new_size, item_size);
  memcpy((char *)vector->item + (old_size * item_size), items, count * item_size);
}

/**
 * @brief Traverse a vector.
 * 
 * The macro expansion is a for-loop, user append
 * statements block will apply to each traversed item.
 * 
 * @param i: pointer to an item of the vector.
 * @param vector: pointer to the vector.
 * 
 * @note \p i must be declared in advance.
 * 
 * @see VECTOR()
 * @see VECTOR_FOREACH_REVERSE()
 * 
 * @par Example:
 * @code
 *   VECTOR(int) int_array;
 *   int *i;
 *   VECTOR_FOREACH(i, &int_array) {
 *     if (*i != 0) ...
 *   }
 * @endcode
 */
#define VECTOR_FOREACH(i, vector) \
  for ((i) = &(vector)->item[0]; (i) < &(vector)->item[(vector)->size]; (i)++)

/**
 * @brief Traverse a vector in reverse order.
 * 
 * The macro expansion is a for-loop, user append
 * statements block will apply to each traversed item.
 * 
 * @param i: pointer to an item of the vector.
 * @param vector: pointer to the vector.
 * 
 * @note \p i must be declared in advance.
 * 
 * @see VECTOR()
 * @see VECTOR_FOREACH()
 * 
 * @par Example:
 * @code
 *   VECTOR(int) int_array;
 *   int *i;
 *   VECTOR_FOREACH_REVERSE(i, &int_array) {
 *     if (*i != 0) ...
 *   }
 * @endcode
 */
#define VECTOR_FOREACH_REVERSE(i, vector) \
  for ((i) = &(vector)->item[size]; (i)-- >= (&(vector)->item[0]);)

#endif /* __CONTINUATION_VECTOR_H */
