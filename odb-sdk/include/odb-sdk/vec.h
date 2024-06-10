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
#include <inttypes.h>
#include <stddef.h>
#include <string.h>

#define VEC_DECLARE_API(prefix, T, bits, API)                                  \
    struct prefix                                                              \
    {                                                                          \
        int##bits##_t count, capacity;                                         \
        T             data[1];                                                 \
    };                                                                         \
    static struct prefix prefix##_null_vec;                                    \
                                                                               \
    /*!                                                                        \
     * @brief This must be called before operating on any vector. Initializes  \
     * the structure to a defined state.                                       \
     * @param[in] v Pointer to a vector of type VEC(T,B)*                      \
     */                                                                        \
    static inline void prefix##_init(struct prefix** v);                       \
                                                                               \
    /*!                                                                        \
     * @brief Destroys an existing vector object and frees all memory          \
     * allocated by inserted elements.                                         \
     * @param[in] v Vector of type VEC(T,B)                                    \
     */                                                                        \
    static inline void prefix##_deinit(struct prefix* v);                      \
                                                                               \
    /*!                                                                        \
     * @brief Allocates memory to fit exactly "num" amount of elements.        \
     * Any existing elements are cleared.                                      \
     * Use this function if you know ahead of time how many elements will be   \
     * pushed back to save on unnecessary reallocations.                       \
     * @param[in] v Pointer to a vector of type VEC(T,B)*                      \
     * @param[in] num Number of elements to reserve memory for.                \
     * @return Returns 0 on success, negative on error.                        \
     */                                                                        \
    API int prefix##_reserve(struct prefix** v, int##bits##_t count);          \
                                                                               \
    /*!                                                                        \
     * @brief Reallocates the underlying memory to fit exactly "num" amount of \
     * elements. Existing elements are preserved. If you specify a value       \
     * smaller than the number of elements currently in the vector, then       \
     * superfluous elements are removed.                                       \
     * @param[in] v Pointer to a vector of type VEC(T,B)*                      \
     * @param[in] num Number of elements to reserve memory for.                \
     * @return Returns 0 on success, negative on error.                        \
     */                                                                        \
    static inline int prefix##_resize(struct prefix** v, int##bits##_t count); \
                                                                               \
    /*!                                                                        \
     * @brief Resets the vector's size to 0.                                   \
     * @note This does not actually free the underlying memory, it simply      \
     * resets the element counter. If you wish to free the underlying memory,  \
     * see vec_clear_compact().                                                \
     * @param[in] v Pointer to a vector of type VEC(T,B)*                      \
     */                                                                        \
    static inline void prefix##_clear(struct prefix* v);                       \
                                                                               \
    /*!                                                                        \
     * @brief Frees any excess memory. The memory buffer is resized to fit the \
     * current number of elements.                                             \
     * @param[in] v Pointer to a vector of type VEC(T,B)*                      \
     */                                                                        \
    API void prefix##_compact(struct prefix** v);                              \
                                                                               \
    /*!                                                                        \
     * @brief A combination of @see vec_clear() and @see vec_compact(). Resets \
     * the vector's count to 0 and frees all underlying memory.                \
     * @param[in] v Pointer to a vector of type VEC(T,B)*                      \
     */                                                                        \
    static inline void prefix##_clear_compact(struct prefix** v);              \
                                                                               \
    /*!                                                                        \
     * @brief Inserts (copies) a new element into the end of the vector.       \
     * @note This can cause a re-allocation of the underlying memory. This     \
     * implementation expands the allocated memory by a factor of 2 every time \
     * the capacity is reached to cut down on the frequency of re-allocations. \
     * @note If you do not wish to copy data into the vector, but merely make  \
     * space, see vec_push_emplace().                                          \
     * @param[in] v Pointer to a vector of type VEC(T,B)*                      \
     * @param[in] elem The element to copy into the vector.                    \
     * @return Returns 0 on success, negative on error.                        \
     */                                                                        \
    static inline int prefix##_push(struct prefix** v, T elem);                \
                                                                               \
    /*!                                                                        \
     * @brief Allocates space for a new element at the end of the vector, but  \
     * does not initialize it.                                                 \
     * @warning The returned pointer could be invalidated if any other         \
     * vector related function is called, as the underlying memory of the      \
     * vector could be re-allocated. Use the pointer immediately after calling \
     * this function.                                                          \
     * @note This can cause a re-allocation of the underlying memory. This     \
     * implementation expands the allocated memory by a factor of 2 every time \
     * the capacity is reached to cut down on the frequency of re-allocations. \
     * @param[in] v Pointer to a vector of type VEC(T,B)*                      \
     * @param[in] element The element to copy into the vector.                 \
     * @return Returns a pointer to the inserted space.                        \
     */                                                                        \
    API T* prefix##_emplace(struct prefix** v);                                \
                                                                               \
    /*!                                                                        \
     * @brief Inserts (copies) a new element into the vector at the specified  \
     * index.                                                                  \
     * @note This can cause a re-allocation of the underlying memory. This     \
     * implementation expands the allocated memory by a factor of 2 every time \
     * the capacity is reached to cut down on the frequency of re-allocations. \
     * @note If you do not wish to copy data into the vector, but merely make  \
     * space, @see vec_insert_emplace().                                       \
     * @param[in] v Pointer to a vector of type VEC(T,B)*                      \
     * @param[in] i The index the new element should be inserted into.         \
     * Existing elements are shifted aside to make space. Should be between 0  \
     * to vec_count().                                                         \
     * @param[in] elem The element to copy into the vector.                    \
     * @return Returns 0 on success, negative on error.                        \
     */                                                                        \
    static inline int prefix##_insert(                                         \
        struct prefix** v, int##bits##_t i, T elem);                           \
                                                                               \
    /*!                                                                        \
     * @brief Allocates space for a new element at the specified index in the  \
     * vector, but does not initialize it.                                     \
     * @warning The returned pointer could be invalidated if any other         \
     * vector related function is called, as the underlying memory of the      \
     * vector could be re-allocated. Use the pointer immediately after calling \
     * this function.                                                          \
     * @note This can cause a re-allocation of the underlying memory. This     \
     * implementation expands the allocated memory by a factor of 2 every time \
     * the capacity is reached to cut down on the frequency of re-allocations. \
     * @param[in] v Pointer to a vector of type VEC(T,B)*                      \
     * @param[in] i The index the new element should be inserted into.         \
     * Existing elements are shifted aside to make space. Should be between 0  \
     * - vec_count().                                                          \
     * @return Returns a pointer to the inserted space.                        \
     */                                                                        \
    API T* prefix##_insert_emplace(struct prefix** v, int##bits##_t i);        \
                                                                               \
    /*!                                                                        \
     * @brief Removes an element from the end of the vector.                   \
     * @warning The vector must have at least 1 element. If you cannot         \
     * guarantee this, check @see vec_count() first.                           \
     * @warning The returned pointer could be invalidated if any other         \
     * vector related function is called, as the underlying memory of the      \
     * vector could be re-allocated. Use the pointer immediately after calling \
     * this function.                                                          \
     * @param[in] v Pointer to a vector of type VEC(T,B)*.                     \
     * @return A pointer to the popped element. See warning and use with       \
     * caution. Vector must not be empty.                                      \
     */                                                                        \
    static inline T* prefix##_pop(struct prefix* v);                           \
                                                                               \
    /*!                                                                        \
     * @brief Erases an element at the specified index from the vector.        \
     * @note This causes all elements with indices greater than **i** to be    \
     * shifted 1 element down so the vector remains contiguous.                \
     * @param[in] i The position of the element in the vector to erase. The    \
     * index ranges from 0 to vec_count()-1.                                   \
     */                                                                        \
    API void prefix##_erase(struct prefix* v, int##bits##_t i);                \
                                                                               \
    /*!                                                                        \
     * @brief Removes all elements for which the callback function returns     \
     * false (0).                                                              \
     * @param[in] v Pointer to a vector of type VEC(T,B)                       \
     * @param[in] on_element Callback function. Gets called for each element   \
     * once. Return 1 to keep the element in the vector. Return 0 to remove    \
     * the element from the vector. Return -1 to abort iterating and return an \
     * error.                                                                  \
     * @param[in] user A user defined pointer that gets passed in to the       \
     * callback function. Can be whatever you want.                            \
     * @return Returns 0 if iteration was successful. If the callback function \
     * returns a negative value, then this function will return the same       \
     * negative value. This allows propagating errors from within the callback \
     * function.                                                               \
     */                                                                        \
    API int prefix##_retain(                                                   \
        struct prefix* v,                                                      \
        int (*on_element)(T * elem, void* user),                               \
        void* user);                                                           \
                                                                               \
    static inline void prefix##_init(struct prefix** v)                        \
    {                                                                          \
        *v = &prefix##_null_vec;                                               \
    }                                                                          \
                                                                               \
    static inline void prefix##_deinit(struct prefix* v)                       \
    {                                                                          \
        if (v->capacity)                                                       \
            mem_free(v);                                                       \
    }                                                                          \
                                                                               \
    static inline int prefix##_resize(struct prefix** v, int##bits##_t elems)  \
    {                                                                          \
        if (prefix##_reserve(v, elems) == 0)                                   \
        {                                                                      \
            (*v)->count = elems;                                               \
            return 0;                                                          \
        }                                                                      \
        return -1;                                                             \
    }                                                                          \
                                                                               \
    static inline void prefix##_clear(struct prefix* v)                        \
    {                                                                          \
        v->count = 0;                                                          \
    }                                                                          \
    static inline void prefix##_clear_compact(struct prefix** v)               \
    {                                                                          \
        if ((*v)->capacity)                                                    \
            mem_free(*v);                                                      \
        *v = &prefix##_null_vec;                                               \
    }                                                                          \
    static inline int prefix##_push(struct prefix** v, T elem)                 \
    {                                                                          \
        T* ins = prefix##_emplace(v);                                          \
        if (ins == NULL)                                                       \
            return -1;                                                         \
        *ins = elem;                                                           \
        return 0;                                                              \
    }                                                                          \
    static inline int prefix##_insert(                                         \
        struct prefix** v, int##bits##_t i, T elem)                            \
    {                                                                          \
        T* ins = prefix##_insert_emplace(v, i);                                \
        if (ins == NULL)                                                       \
            return -1;                                                         \
        *ins = elem;                                                           \
        return 0;                                                              \
    }                                                                          \
    static inline T* prefix##_pop(struct prefix* v)                            \
    {                                                                          \
        return &v->data[--(v->count)];                                         \
    }

#define VEC_DEFINE_API(prefix, T, bits)                                        \
    VEC_DEFINE_API_FULL(prefix, T, bits, 32, 2)

#define VEC_DEFINE_API_FULL(prefix, T, bits, MIN_CAPACITY, EXPAND_FACTOR)      \
    static int prefix##_realloc(struct prefix** v, int##bits##_t elems)        \
    {                                                                          \
        mem_size header = sizeof(**v) - sizeof((*v)->data[0]);                 \
        mem_size data = sizeof((*v)->data[0]) * elems;                         \
        void*    new_mem                                                       \
            = mem_realloc(((*v)->capacity ? *v : NULL), header + data);        \
        if (new_mem == NULL)                                                   \
            return log_oom(header + data, "vec_reserve()");                    \
        *(void**)v = new_mem;                                                  \
        return 0;                                                              \
    }                                                                          \
                                                                               \
    int prefix##_reserve(struct prefix** v, int##bits##_t elems)               \
    {                                                                          \
        if (prefix##_realloc(v, elems) != 0)                                   \
            return -1;                                                         \
        (*v)->count = 0;                                                       \
        (*v)->capacity = elems;                                                \
        return 0;                                                              \
    }                                                                          \
    void prefix##_compact(struct prefix** v)                                   \
    {                                                                          \
        if ((*v)->capacity == 0)                                               \
            return;                                                            \
                                                                               \
        if ((*v)->count == 0)                                                  \
        {                                                                      \
            mem_free((*v));                                                    \
            *v = &prefix##_null_vec;                                           \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            mem_size header = sizeof(**v) - sizeof((*v)->data[0]);             \
            mem_size data = sizeof((*v)->data[0]) * (*v)->count;               \
            void*    new_mem = mem_realloc(*v, header + data);                 \
            *(void**)v = new_mem;                                              \
            (*v)->capacity = (*v)->count;                                      \
        }                                                                      \
    }                                                                          \
    T* prefix##_emplace(struct prefix** v)                                     \
    {                                                                          \
        if ((*v)->count == (*v)->capacity)                                     \
        {                                                                      \
            mem_size cap = (*v)->capacity;                                     \
            mem_size new_cap = cap ? cap * EXPAND_FACTOR : MIN_CAPACITY;       \
            if (prefix##_realloc(v, new_cap) != 0)                             \
                return NULL;                                                   \
            if (cap == 0)                                                      \
                (*v)->count = 0;                                               \
            (*v)->capacity = new_cap;                                          \
        }                                                                      \
                                                                               \
        return &(*v)->data[((*v)->count)++];                                   \
    }                                                                          \
    T* prefix##_insert_emplace(struct prefix** v, int##bits##_t i)             \
    {                                                                          \
        if (prefix##_emplace(v) == NULL)                                       \
            return NULL;                                                       \
                                                                               \
        memmove(                                                               \
            (*v)->data + i + 1,                                                \
            (*v)->data + i,                                                    \
            ((*v)->count - i - 1) * sizeof((*v)->data[0]));                    \
        return &(*v)->data[i];                                                 \
    }                                                                          \
    void prefix##_erase(struct prefix* v, int##bits##_t i)                     \
    {                                                                          \
        --(v->count);                                                          \
        memmove(                                                               \
            v->data + i,                                                       \
            v->data + i + 1,                                                   \
            (v->count - i) * sizeof(v->data[0]));                              \
    }                                                                          \
                                                                               \
    int prefix##_retain(                                                       \
        struct prefix* v, int (*on_element)(T * elem, void* user), void* user) \
    {                                                                          \
        int##bits##_t i;                                                       \
        for (i = 0; i != v->count; ++i)                                        \
        {                                                                      \
            int result = on_element(&v->data[i], user);                        \
            if (result < 0)                                                    \
                return result;                                                 \
            if (result == 0)                                                   \
            {                                                                  \
                prefix##_erase(v, i);                                          \
                --i;                                                           \
            }                                                                  \
        }                                                                      \
        return 0;                                                              \
    }

/*!
 * @brief Returns the first element of the vector.
 * @warning The returned pointer could be invalidated if any other vector
 * related function is called, as the underlying memory of the vector could be
 * re-allocated. Use the pointer immediately after calling this function.
 * @param[in] v Pointer to a vector of type VEC(T,B)*.
 * @return A pointer to the last element. See warning and use with caution.
 * Vector must not be empty.
 */
#define vec_first(v) (&(v)->data[0])

/*!
 * @brief Returns the first element of the vector.
 * @warning The returned pointer could be invalidated if any other vector
 * related function is called, as the underlying memory of the vector could be
 * re-allocated. Use the pointer immediately after calling this function.
 * @param[in] v Pointer to a vector of type VEC(T,B)*.
 * @return A pointer to the last element. See warning and use with caution.
 * Vector must not be empty.
 */
#define vec_last(v) (&(v)->data[(v)->count - 1])

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
#define vec_rget(v, i) (&(v)->data[(v)->count - (i) - 1])

/*!
 * @brief Iterates over the elements in a vector.
 *
 *   T* element;
 *   VEC(T,32)* v = some_vector();
 *   vec_for_each(v, element) {
 *       ... do stuff ...
 *   }
 */
#define vec_for_each(v, var)                                                   \
    for (var = &(v)->data[0]; var != &(v)->data[(v)->count]; var++)
