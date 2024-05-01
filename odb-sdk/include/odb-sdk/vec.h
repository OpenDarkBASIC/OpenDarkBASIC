/*!
 * @file vector.h
 * @brief Dynamic contiguous sequence container with guaranteed element order.
 * @page vector Ordered Vector
 *
 * Ordered vectors arrange all inserted elements next to each other in memory.
 * Because of this, vector access is just as efficient as a normal array, but
 * they are able to grow and shrink in size automatically.
 */
#pragma once

#include "odb-sdk/config.h"
#include "odb-sdk/log.h"
#include "odb-sdk/mem.h"
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <inttypes.h>

#if defined(ODBSDK_VEC_64BIT)
typedef uint64_t vec_size;
typedef int64_t vec_idx;
#else
typedef uint32_t vec_size;
typedef int32_t vec_idx;
#endif

#define VEC(T, B)                    \
    struct                           \
    {                                \
        int##B##_t count, capacity;  \
        T data[1];                   \
    }

static inline int _vec_realloc(void** v, size_t bytes)
{
    void* mem = mem_realloc(*v, bytes);
    if (mem)
    {
        *v = mem;
        return 0;
    }
    log_sdk_err("Failed to allocate %" PRIu64 " bytes in vec_reserve()\n", bytes);
    return -1;
}

/*!
 * @brief Destroys an existing vector object and frees all memory allocated by
 * inserted elements.
 * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.
 */
#define vec_free(v) do {        \
        if (v)                  \
            mem_free(v);        \
    } while (0)

/*!
 * @brief Allocates memory to fit exactly "num" amount of elements. 
 * Any existing elements are cleared.
 * Use this function if you know ahead of time how many elements will be pushed
 * back to save on unnecessary reallocations.
 * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.
 * @param[in] num Number of elements to reserve memory for.
 * @return Returns 0 on success, negative on error.
 */
#define vec_reserve(v, num) (           \
    _vec_realloc((void**)&(v),          \
            sizeof((v)->count)          \
          + sizeof((v)->capacity)       \
          + sizeof(*(v)->data) * (num)) \
      ? -1                              \
      : (((v)->count = 0), ((v)->capacity = (num)), 0))

/*!
 * @brief Reallocates the underlying memory to fit exactly "num" amount of
 * elements. Existing elements are preserved. If you specify a value smaller
 * than the number of elements currently in the vector, then superfluous
 * elements are removed.
 * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.
 * @param[in] num Number of elements to reserve memory for.
 * @return Returns 0 on success, negative on error.
 */
#define vec_resize(v, num) (            \
    vec_reserve((v), (num))             \
      ? -1                              \
      : (((v)->count = (num)), 0))
    

/*!
 * @brief Resets the vector's size to 0.
 * @note This does not actually free the underlying memory, it simply resets
 * the element counter. If you wish to free the underlying memory, see
 * vec_clear_compact().
 * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.
 */
#define vec_clear(v) do {   \
        if (v)              \
            (v)->count = 0; \
    } while (0)

/*!
 * @brief Frees any excess memory. The memory buffer is resized to fit the
 * current number of elements.
 * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.
 */
#define vec_compact(v) do {                             \
        if (v) {                                        \
            if ((v)->count == 0) {                      \
                mem_free(v);                            \
                (v) = NULL;                             \
            } else {                                    \
                _vec_realloc((void**)&(v),              \
                    sizeof((v)->count)                  \
                  + sizeof((v)->capacity)               \
                  + sizeof(*(v)->data * (v)->count));   \
                (v)->capacity = (v)->count;             \
            }                                           \
        }                                               \
    } while (0)

/*!
 * @brief A combination of @see vec_clear() and @see vec_compact(). Resets the
 * vector's count to 0 and frees all underlying memory.
 * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.
 */
#define vec_clear_compact(v) do {   \
        if (v)                      \
            mem_free(v);            \
        (v) = NULL;                 \
    } while (0)

/*!
 * @brief Inserts (copies) a new element into the end of the vector.
 * @note This can cause a re-allocation of the underlying memory. This
 * implementation expands the allocated memory by a factor of 2 every time the
 * capacity is reached to cut down on the frequency of re-allocations.
 * @note If you do not wish to copy data into the vector, but merely make
 * space, see vec_push_emplace().
 * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.
 * @param[in] element The value to copy into the vector.
 * @return Returns 0 on success, negative on error.
 */
#define vec_push(v, element) (                                      \
    (v) == NULL                                                     \
      ? vec_reserve(v, ODBSDK_VEC_MIN_CAPACITY)                     \
        ? -1                                                        \
        : (((v)->data[(v)->count++] = element), 0)                  \
      : (v)->count == (v)->capacity                                 \
        ? _vec_realloc((void**)&(v),                                \
              sizeof((v)->count)                                    \
            + sizeof((v)->capacity)                                 \
            + sizeof(*(v)->data) *                                  \
                ((v)->capacity * ODBSDK_VEC_EXPAND_FACTOR))         \
          ? -1                                                      \
          : (                                                       \
              ((v)->capacity *= ODBSDK_VEC_EXPAND_FACTOR),          \
              ((v)->data[(v)->count++] = element),                  \
              0                                                     \
            )                                                       \
        : (((v)->data[(v)->count++] = element), 0))

/*!
 * @brief Allocates space for a new element at the end of the vector, but does
 * not initialize it.
 * @warning The returned pointer could be invalidated if any other
 * vector related function is called, as the underlying memory of the vector
 * could be re-allocated. Use the pointer immediately after calling this
 * function.
 * @note This can cause a re-allocation of the underlying memory. This
 * implementation expands the allocated memory by a factor of 2 every time the
 * capacity is reached to cut down on the frequency of re-allocations.
 * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.
 * @param[in] element The value to copy into the vector.
 * @return Returns a pointer to the inserted space.
 */
#define vec_emplace(v) (                                            \
    (v) == NULL                                                     \
      ? vec_reserve(v, ODBSDK_VEC_MIN_CAPACITY)                     \
        ? NULL                                                      \
        : &(v)->data[(v)->count++]                                  \
      : (v)->count == (v)->capacity                                 \
        ? _vec_realloc((void**)&(v),                                \
              sizeof((v)->count)                                    \
            + sizeof((v)->capacity)                                 \
            + sizeof(*(v)->data) *                                  \
                ((v)->capacity * ODBSDK_VEC_EXPAND_FACTOR))         \
          ? NULL                                                    \
          : (                                                       \
              ((v)->capacity *= ODBSDK_VEC_EXPAND_FACTOR),          \
              &(v)->data[(v)->count++]                              \
            )                                                       \
        : &(v)->data[(v)->count++])

/*!
 * @brief Inserts (copies) a new element into the vector at the specified index.
 * @note This can cause a re-allocation of the underlying memory. This
 * implementation expands the allocated memory by a factor of 2 every time the
 * capacity is reached to cut down on the frequency of re-allocations.
 * @note If you do not wish to copy data into the vector, but merely make
 * space, see vec_insert_emplace().
 * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.
 * @param[in] i The index the new element should be inserted into. Existing
 * elements are shifted aside to make space. Should be between 0 - vec_count().
 * @param[in] element The value to copy into the vector.
 * @return Returns 0 on success, negative on error.
 */
#define vec_insert(v, i, element) (                                 \
    (v) == NULL                                                     \
      ? vec_reserve(v, ODBSDK_VEC_MIN_CAPACITY)                     \
        ? -1                                                        \
        : (                                                         \
            ((v)->data[i] = element),                               \
            (v)->count++,                                           \
            0                                                       \
          )                                                         \
      : (v)->count == (v)->capacity                                 \
        ? _vec_realloc((void**)&(v),                                \
              sizeof((v)->count)                                    \
            + sizeof((v)->capacity)                                 \
            + sizeof(*(v)->data) *                                  \
                ((v)->capacity * ODBSDK_VEC_EXPAND_FACTOR))         \
          ? -1                                                      \
          : (                                                       \
              ((v)->capacity *= ODBSDK_VEC_EXPAND_FACTOR),          \
              memmove(                                              \
                (v)->data + i + 1,                                  \
                (v)->data + i,                                      \
                ((v)->count - i) * sizeof(*(v)->data)),             \
              (v)->count++,                                         \
              ((v)->data[i] = element),                             \
              0                                                     \
            )                                                       \
        : (                                                         \
            memmove(                                                \
              (v)->data + i + 1,                                    \
              (v)->data + i,                                        \
              ((v)->count - i) * sizeof(*(v)->data)),               \
            (v)->count++,                                           \
            ((v)->data[i] = element),                               \
            0                                                       \
          )                                                         \
        )

/*!
 * @brief Allocates space for a new element at the specified index in the vector,
 * but does not initialize it.
 * @warning The returned pointer could be invalidated if any other
 * vector related function is called, as the underlying memory of the vector
 * could be re-allocated. Use the pointer immediately after calling this
 * function.
 * @note This can cause a re-allocation of the underlying memory. This
 * implementation expands the allocated memory by a factor of 2 every time the
 * capacity is reached to cut down on the frequency of re-allocations.
 * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.
 * @param[in] i The index the new element should be inserted into. Existing
 * elements are shifted aside to make space. Should be between 0 - vec_count().
 * @return Returns a pointer to the inserted space.
 */
#define vec_insert_emplace(v, i) (                                  \
    (v) == NULL                                                     \
      ? vec_reserve(v, ODBSDK_VEC_MIN_CAPACITY)                     \
        ? NULL                                                      \
        : ((v)->count++, &(v)->data[i])                             \
      : (v)->count == (v)->capacity                                 \
        ? _vec_realloc((void**)&(v),                                \
              sizeof((v)->count)                                    \
            + sizeof((v)->capacity)                                 \
            + sizeof(*(v)->data) *                                  \
                ((v)->capacity * ODBSDK_VEC_EXPAND_FACTOR))         \
          ? NULL                                                    \
          : (                                                       \
              ((v)->capacity *= ODBSDK_VEC_EXPAND_FACTOR),          \
              (v)->count++,                                         \
              memmove(                                              \
                (v)->data + i + 1,                                  \
                (v)->data + i,                                      \
                ((v)->count - i) * sizeof(*(v)->data)),             \
              &(v)->data[i]                                         \
            )                                                       \
        : (                                                         \
            (v)->count++,                                           \
            memmove(                                                \
              (v)->data + i + 1,                                    \
              (v)->data + i,                                        \
              ((v)->count - i) * sizeof(*(v)->data)),               \
            &(v)->data[i]                                           \
          )                                                         \
        )

/*!
 * @brief Removes an element from the end of the vector.
 * @warning The vector must have at least 1 element. If you cannot guarantee
 * this, check @see vec_count() first.
 * @warning The returned pointer could be invalidated if any other
 * vector related function is called, as the underlying memory of the vector
 * could be re-allocated. Use the pointer immediately after calling this
 * function.
 * @param[in] v Pointer to a vector of type VEC(T,B)*.
 * @return A pointer to the popped element. See warning and use with caution.
 * Vector must not be empty.
 */
#define vec_pop(v) \
    (&(v)->data[--(v)->count])

/*!
 * @brief Returns the first element of the vector.
 * @warning The returned pointer could be invalidated if any other vector
 * related function is called, as the underlying memory of the vector could be
 * re-allocated. Use the pointer immediately after calling this function.
 * @param[in] v Pointer to a vector of type VEC(T,B)*.
 * @return A pointer to the last element. See warning and use with caution.
 * Vector must not be empty.
 */
#define vec_first(v) \
    (&(v)->data[0])

/*!
 * @brief Returns the very last element of the vector.
 * @warning The returned pointer could be invalidated if any other vector
 * related function is called, as the underlying memory of the vector could be
 * re-allocated. Use the pointer immediately after calling this function.
 * @param[in] v Pointer to a vector of type VEC(T,B)*.
 * @return A pointer to the last element. See warning and use with caution.
 * Vector must not be empty.
 */
#define vec_last(v) \
    (vec_rget((v), 0))

/*!
 * @brief Returns the nth element of the vector, starting at 0.
 * @warning The returned pointer could be invalidated if any other vector
 * related function is called, as the underlying memory of the vector could be
 * re-allocated. Use the pointer immediately after calling this function.
 * @param[in] v Pointer to a vector of type VEC(T,B)*.
 * @param[in] i Index, ranging from 0 to vec_count()-1
 * @return A pointer to the element. See warning and use with caution.
 * Vector must not be empty.
 */
#define vec_get(v, i) \
    (&(v)->data[i])

/*!
 * @brief Returns the nth last element of the vector, starting at 0.
 * @warning The returned pointer could be invalidated if any other vector
 * related function is called, as the underlying memory of the vector could be
 * re-allocated. Use the pointer immediately after calling this function.
 * @param[in] v Pointer to a vector of type VEC(T,B)*.
 * @param[in] i Index, ranging from 0 to vec_count()-1
 * @return A pointer to the element. See warning and use with caution.
 * Vector must not be empty.
 */
#define vec_rget(v, i) \
    (&(v)->data[(v)->count - (i) - 1])

/*!
 * @brief Returns the number of elements in the vector. Works on NULL vectors, too.
 * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.
 */
#define vec_count(v) \
    ((v) ? (v)->count : 0)

/*!
 * @brief Erases an element at the specified index from the vector.
 * @note This causes all elements with indices greater than **i** to be
 * shifted 1 element down so the vector remains contiguous.
 * @param[in] i The position of the element in the vector to erase. The index
 * ranges from 0 to vec_count()-1.
 */
#define vec_erase(v, i) (                           \
    memmove(                                        \
        (v)->data + i,                              \
        (v)->data + i + 1,                          \
        ((v)->count - i) * sizeof(*(v)->data)),     \
    --(v)->count,                                   \
    (void)(v))

#if 0
/*!
 * @brief Removes the element in the vector pointed to by **element**.
 * @param[in] vector The vector from which to erase the data.
 * @param[in] element A pointer to an element within the vector.
 * @note The pointer must point into the vector's data.
 */
ODBSDK_PUBLIC_API void
vec_erase_element(struct vec* vec, void* element);

ODBSDK_PUBLIC_API vec_idx
vec_find(const struct vec* vector, const void* element);

ODBSDK_PUBLIC_API void
vec_reverse(struct vec* vector);

#endif

#define vec_for_each(v, var)    \
    for (intptr_t var_##i = 0;  \
        (v) && (var_##i != (v)->count) && ((var = &(v)->data[var_##i]) || 1); \
        ++var_##i)

