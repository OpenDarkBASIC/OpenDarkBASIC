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
#include <stdint.h>
#include <assert.h>

#if defined(ODBSDK_VEC_64BIT)
typedef uint64_t vec_size;
typedef int64_t vec_idx;
#else
typedef uint32_t vec_size;
typedef int32_t vec_idx;
#endif

struct vec
{
    uint8_t* data;          /* pointer to the contiguous section of memory */
    vec_size capacity;      /* how many elements actually fit into the allocated space */
    vec_size count;         /* number of elements inserted */
    vec_size element_size;  /* how large one element is in bytes */
};

/*!
 * @brief Creates a new vector object. See @ref vector for details.
 * @param[in] element_size Specifies the size in bytes of the type of data you want
 * the vector to store. Typically one would pass sizeof(my_data_type).
 * @return Returns the newly created vector object.
 */
ODBSDK_PUBLIC_API struct vec*
vec_alloc(const vec_size element_size);

/*!
 * @brief Initializes an existing vector object.
 * @note This does **not** free existing memory. If you've pushed elements
 * into your vector and call this, you will have created a memory leak.
 * @param[in] vector The vector to initialize.
 * @param[in] element_size Specifies the size in bytes of the type of data you
 * want the vector to store. Typically one would pass sizeof(my_data_type).
 */
ODBSDK_PUBLIC_API void
vec_init(struct vec* vec, const vec_size element_size);

ODBSDK_PUBLIC_API int
vec_init_copy(struct vec* vec, const struct vec* src);

ODBSDK_PUBLIC_API void
vec_deinit(struct vec* vec);

/*!
 * @brief Destroys an existing vector object and frees all memory allocated by
 * inserted elements.
 * @param[in] vector The vector to free.
 */
ODBSDK_PUBLIC_API void
vec_free(struct vec* vec);

/*!
 * @brief Erases all elements in a vector.
 * @note This does not actually erase the underlying memory, it simply resets
 * the element counter. If you wish to free the underlying memory, see
 * vec_clear_compact().
 * @param[in] vector The vector to clear.
 */
ODBSDK_PUBLIC_API void
vec_clear(struct vec* vec);

/*!
 * @brief Erases all elements in a vector and frees their memory.
 * @param[in] vector The vector to clear.
 */
ODBSDK_PUBLIC_API void
vec_compact(struct vec* vec);

ODBSDK_PUBLIC_API void
vec_clear_compact(struct vec* vec);

ODBSDK_PUBLIC_API int
vec_reserve(struct vec* vec, vec_size size);

/*!
 * @brief Sets the size of the vector to exactly the size specified. If the
 * vector was smaller then memory will be reallocated. If the vector was larger
 * then the capacity will remain the same and the size will adjusted.
 * @param[in] vector The vector to resize.
 * @param[in] size The new size of the vector.
 * @return Returns ODBSDK_VEC_OOM on failure, ODBSDK_OK on success.
 */
ODBSDK_PUBLIC_API int
vec_resize(struct vec* vec, vec_size size);

/*!
 * @brief Gets the number of elements that have been inserted into the vector.
 */
#define vec_count(x) ((x)->count)

#define vec_capacity(x) ((x)->capacity)

#define vec_data(x) ((void*)(x)->data)

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
ODBSDK_PUBLIC_API int
vec_push(struct vec* vec, const void* data);

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
ODBSDK_PUBLIC_API void*
vec_emplace(struct vec* vec);

/*!
 * @brief Copies the contents of another vector and pushes it into the vector.
 * @return Returns 0 if successful, -1 if a memory allocation failed, -2 if
 * the two vectors are incompatible.
 */
ODBSDK_PUBLIC_API int
vec_push_vec(struct vec* vec, const struct vec* src_vec);

static void
vec_steal_vector(struct vec* vec, struct vec* src_vec)
{
    vec_deinit(vec);
    *vec = *src_vec;
    src_vec->data = (uint8_t*)(void*)0;
    src_vec->count = 0;
    src_vec->capacity = 0;
}

static void*
vec_steal_data(struct vec* vec)
{
    void* data = vec->data;
    vec->data = (uint8_t*)(void*)0;
    vec->count = 0;
    vec->capacity = 0;
    return data;
}

/*!
 * @brief Removes an element from the back (end) of the vector.
 * @warning The returned pointer could be invalidated if any other
 * vector related function is called, as the underlying memory of the vector
 * could be re-allocated. Use the pointer immediately after calling this
 * function.
 * @param[in] vector The vector to pop an element from.
 * @return A pointer to the popped element. See warning and use with caution.
 * If there are no elements to pop, NULL is returned.
 */
ODBSDK_PUBLIC_API void*
vec_pop(struct vec* vec);

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
