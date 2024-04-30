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
 * @param[in] vector The vector to free.
 */
#define vec_free(v) do {        \
        if (v)                  \
            mem_free(v);        \
    } while (0)

/*!
 * @brief Reserves memory for "num" amount of elements. Existing elements are
 * preserved. If you specify a value smaller than the number of elements
 * currently in the vector, then superfluous elements are removed.
 * @param[in] v Pointer to a vector of type VEC(T,S)*.
 * @param[in] num Number of elements to reserve memory for.
 * @return Returns 0 on success, negative on error.
 */
#define vec_reserve(v, num) (           \
    _vec_realloc((void**)&(v),          \
            sizeof((v)->count)          \
          + sizeof((v)->capacity)       \
          + sizeof(*(v)->data) * num)   \
      ? -1                              \
      : (((v)->count = 0), ((v)->capacity = num), 0))

/*!
 * @brief Resets the vector's size to 0.
 * @note This does not actually free the underlying memory, it simply resets
 * the element counter. If you wish to free the underlying memory, see
 * vec_clear_compact().
 * @param[in] v Pointer to a vector of type VEC(T,S)*.
 */
#define vec_clear(v) \
    (((v)->count = 0), (void)(v))

/*!
 * @brief Frees any excess memory using realloc(). The allocated memory should
 * be exactly enough to contain the current number of elements.
 * @param[in] v Pointer to a vector of type VEC(T,S)*.
 */
#define vec_compact(v) (                            \
    (v)->count == 0                                 \
      ? (mem_free(v), ((v) = NULL), (void)(v))      \
      : ((_vec_realloc((void**)&(v),                \
            sizeof((v)->count)                      \
          + sizeof((v)->capacity)                   \
          + sizeof(*(v)->data) * (v)->count)),      \
        ((v)->capacity = (v)->count),               \
        (void)(v)))

/*!
 * @brief A combination of @see vec_clear() and @see vec_compact(). Resets the
 * vector's count to 0 and frees all underlying memory.
 * @param[in] v Pointer to a vector of type VEC(T,S)*.
 */
#define vec_clear_compact(v) \
    (((v) ? (mem_free((void*)v), ((v) = NULL)) : NULL), (void)(v))

/*!
 * @brief Inserts (copies) a new element at the head of the vector.
 * @note This can cause a re-allocation of the underlying memory. This
 * implementation expands the allocated memory by a factor of 2 every time a
 * re-allocation occurs to cut down on the frequency of re-allocations.
 * @note If you do not wish to copy data into the vector, but merely make
 * space, see vec_push_emplace().
 * @param[in] vector The vector to push into.
 * @param[in] data The data to copy into the vector. It is assumed that
 * sizeof(data) is equal to what was specified when the vector was first
 * created. If this is not the case then it could cause undefined behaviour.
 * @return Returns ODBSDK_OK if the data was successfully pushed, ODBSDK_VEC_OOM
 * if otherwise.
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
 * @brief Allocates space for a new element at the head of the vector, but does
 * not initialize it.
 * @warning The returned pointer could be invalidated if any other
 * vector related function is called, as the underlying memory of the vector
 * could be re-allocated. Use the pointer immediately after calling this
 * function.
 * @param[in] vector The vector to emplace an element into.
 * @return A pointer to the allocated memory for the requested element. See
 * warning and use with caution.
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
 * @brief Removes an element from the end of the vector.
 * @warning The returned pointer could be invalidated if any other
 * vector related function is called, as the underlying memory of the vector
 * could be re-allocated. Use the pointer immediately after calling this
 * function.
 * @param[in] vector The vector to pop an element from.
 * @return A pointer to the popped element. See warning and use with caution.
 * If there are no elements to pop, NULL is returned.
 */
#define vec_pop(v) \
    ((v) && (v)->count ? &(v)->data[--(v)->count] : NULL)

#if 0

/*!
 * @brief Returns the very last element of the vector.
 * @warning The returned pointer could be invalidated if any other vector
 * related function is called, as the underlying memory of the vector could be
 * re-allocated. Use the pointer immediately after calling this function.
 *
 * @param[in] vector The vector to return the last element from.
 * @return A pointer to the last element. See warning and use with caution.
 * Vector must not be empty.
 */
static inline void* vec_back(const struct vec* vec)
{
    assert(vec->count > 0);
    return vec->data + (vec->element_size * (vec->count - 1));
}

static inline void* vec_front(const struct vec* vec)
{
    assert(vec->count > 0);
    return vec->data;
}

/*!
 * @brief Allocates space for a new element at the specified index, but does
 * not initialize it.
 * @note This can cause a re-allocation of the underlying memory. This
 * implementation expands the allocated memory by a factor of 2 every time a
 * re-allocation occurs to cut down on the frequency of re-allocations.
 * @warning The returned pointer could be invalidated if any other
 * vector related function is called, as the underlying memory of the vector
 * could be re-allocated. Use the pointer immediately after calling this
 * function.
 * @param[in] vector The vector to emplace an element into.
 * @param[in] index Where to insert.
 * @return A pointer to the emplaced element. See warning and use with caution.
 */
ODBSDK_PUBLIC_API void*
vec_insert_emplace(struct vec* vec, vec_idx index);

/*!
 * @brief Inserts (copies) a new element at the specified index.
 * @note This can cause a re-allocation of the underlying memory. This
 * implementation expands the allocated memory by a factor of 2 every time a
 * re-allocation occurs to cut down on the frequency of re-allocations.
 * @note If you do not wish to copy data into the vector, but merely make
 * space, see vec_insert_emplace().
 * @param[in] vector The vector to insert into.
 * @param[in] data The data to copy into the vector. It is assumed that
 * sizeof(data) is equal to what was specified when the vector was first
 * created. If this is not the case then it could cause undefined behaviour.
 */
ODBSDK_PUBLIC_API int
vec_insert(struct vec* vec, vec_idx index, const void* data);

/*!
 * @brief Erases the specified element from the vector.
 * @note This causes all elements with indices greater than **index** to be
 * re-allocated (shifted 1 element down) so the vector remains contiguous.
 * @param[in] index The position of the element in the vector to erase. The index
 * ranges from **0** to **vec_count()-1**.
 */
ODBSDK_PUBLIC_API void
vec_erase_index(struct vec* vec, vec_idx index);

/*!
 * @brief Removes the element in the vector pointed to by **element**.
 * @param[in] vector The vector from which to erase the data.
 * @param[in] element A pointer to an element within the vector.
 * @note The pointer must point into the vector's data.
 */
ODBSDK_PUBLIC_API void
vec_erase_element(struct vec* vec, void* element);

/*!
 * @brief Gets a pointer to the specified element in the vector.
 * @warning The returned pointer could be invalidated if any other
 * vector related function is called, as the underlying memory of the vector
 * could be re-allocated. Use the pointer immediately after calling this
 * function.
 * @param[in] vector The vector to get the element from.
 * @param[in] index The index of the element to get. The index ranges from
 * **0** to **vec_count()-1**.
 * @return [in] A pointer to the element. See warning and use with caution.
 * If the specified element doesn't exist (index out of bounds), NULL is
 * returned.
 */
static inline void*
vec_get(const struct vec* vector, vec_idx index)
{
    assert(vector);
    assert(index < (vec_idx)vector->count);
    return vector->data + index * (vec_idx)vector->element_size;
}

static inline void*
vec_get_back(const struct vec* vector, vec_idx offset)
{
    assert(vector);
    assert((vec_idx)vector->count - offset >= 0);
    return vector->data + ((vec_idx)vector->count - offset) * (vec_idx)vector->element_size;
}

ODBSDK_PUBLIC_API vec_idx
vec_find(const struct vec* vector, const void* element);

ODBSDK_PUBLIC_API void
vec_reverse(struct vec* vector);

/*!
 * @brief Convenient macro for iterating a vector's elements.
 *
 * Example:
 * ```
 * vec* some_vector = (a vector containing elements of type "bar")
 * ODBSDK_VEC_FOR_EACH(some_vector, bar, element)
 * {
 *     do_something_with(element);  ("element" is now of type "bar*")
 * }
 * ```
 * @param[in] vector A pointer to the vector to iterate.
 * @param[in] var_type Should be the type of data stored in the vector.
 * @param[in] var The name of a temporary variable you'd lrfe to use within the
 * for-loop to reference the current element.
 */
#define VEC_FOR_EACH(vector, var_type, var) {                                \
    var_type* var;                                                           \
    uint8_t* internal_##var##_end_of_vector = (vector)->data + (vector)->count * (vector)->element_size; \
    for(var = (var_type*)(vector)->data;                                     \
        (uint8_t*)var != internal_##var##_end_of_vector;                     \
        var = (var_type*)(((uint8_t*)var) + (vector)->element_size)) {


#define VEC_FOR_EACH_R(vector, var_type, var) {                              \
    var_type* var;                                                           \
    uint8_t* internal_##var##_start_of_vector = (vector)->data - (vector)->element_size; \
    for(var = (var_type*)((vector)->data + (vector)->count * (vector)->element_size - (vector)->element_size); \
        (uint8_t*)var != internal_##var##_start_of_vector;                   \
        var = (var_type*)(((uint8_t*)var) - (vector)->element_size)) {

/*!
 * @brief Convenient macro for iterating a range of a vector's elements.
 * @param[in] vector A pointer to the vector to iterate.
 * @param[in] var_type Should be the type of data stored in the vector. For
 * example, if your vector is storing ```type_t*``` objects then
 * var_type should equal ```type_t``` (without the pointer).
 * @param[in] var The name of a temporary variable you'd lrfe to use within the
 * for loop to reference the current element.
 * @param[in] begin_index The index (starting at 0) of the first element to
 * start with (inclusive).
 * @param[in] end_index The index of the last element to iterate (exclusive).
 */
#define VEC_FOR_EACH_RANGE(vector, var_type, var, begin_index, end_index) {    \
    var_type* var; \
    uint8_t* internal_##var##_end_of_vector = (vector)->data + (end_index) * (vector)->element_size; \
    for(var = (var_type*)((vector)->data + (begin_index) * (vector)->element_size); \
        (uint8_t*)var < internal_##var##_end_of_vector;                        \
        var = (var_type*)(((uint8_t*)var) + (vector)->element_size)) {

/*!
 * @brief Convenient macro for iterating a range of a vector's elements in reverse.
 * @param[in] vector A pointer to the vector to iterate.
 * @param[in] var_type Should be the type of data stored in the vector. For
 * example, if your vector is storing ```type_t*``` objects then
 * var_type should equal ```type_t``` (without the pointer).
 * @param[in] var The name of a temporary variable you'd lrfe to use within the
 * for loop to reference the current element.
 * @param[in] begin_index The "lower" index (starting at 0) of the last element (inclusive).
 * @param[in] end_index The "upper" index of the first element (exclusive).
 */
#define VEC_FOR_EACH_RANGE_R(vector, var_type, var, begin_index, end_index) {    \
    var_type* var;                                                               \
    uint8_t* internal_##var##_start_of_vector = (vector)->data + (begin_index) * (vector)->element_size - (vector)->element_size; \
    for(var = (var_type*)((vector)->data + (end_index) * (vector)->element_size - (vector)->element_size); \
        (uint8_t*)var > internal_##var##_start_of_vector;                        \
        var = (var_type*)(((uint8_t*)var) - (vector)->element_size)) {


#define VEC_ERASE_IN_FOR_LOOP(vector, var_type, var) do {            \
        vec_erase_element(vector, var);                              \
        var = (var_type*)(((uint8_t*)var) - (vector)->element_size); \
        internal_##var##_end_of_vector = (vector)->data + (vector)->count * (vector)->element_size; \
    } while (0)

/*!
 * @brief Closes a for each scope previously opened by ODBSDK_VEC_FOR_EACH.
 */
#define VEC_END_EACH }}

#endif

#define vec_for_each(v, var) \
    for (intptr_t var_##i = 0; (v) && (var_##i != (v)->count) && ((var = &(v)->data[var_##i]) || 1); ++var_##i)