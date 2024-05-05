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

#define VEC_MEM_DEF(T, bits)                                                   \
    {                                                                          \
        int##bits##_t count, capacity;                                         \
        T             data[1];                                                 \
    }
#define VEC_DEF(T, bits)                                                       \
    {                                                                          \
        struct VEC_MEM_DEF(T, bits) * mem;                                     \
    }

#define VEC_DECLARE_API(prefix, T, bits)                                       \
    struct prefix VEC_DEF(T, bits);                                            \
                                                                               \
    static inline void prefix##_init(struct prefix* v);                        \
                                                                               \
    /*!                                                                        \
     * @brief Destroys an existing vector object and frees all memory          \
     * allocated by inserted elements.                                         \
     * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.        \
     */                                                                        \
    static inline void prefix##_deinit(struct prefix* v);                      \
                                                                               \
    /*!                                                                        \
     * @brief Allocates memory to fit exactly "num" amount of elements.        \
     * Any existing elements are cleared.                                      \
     * Use this function if you know ahead of time how many elements will be   \
     * pushed back to save on unnecessary reallocations.                       \
     * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.        \
     * @param[in] num Number of elements to reserve memory for.                \
     * @return Returns 0 on success, negative on error.                        \
     */                                                                        \
    int prefix##_reserve(struct prefix* v, int##bits##_t count);               \
                                                                               \
    /*!                                                                        \
     * @brief Reallocates the underlying memory to fit exactly "num" amount of \
     * elements. Existing elements are preserved. If you specify a value       \
     * smaller than the number of elements currently in the vector, then       \
     * superfluous elements are removed.                                       \
     * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.        \
     * @param[in] num Number of elements to reserve memory for.                \
     * @return Returns 0 on success, negative on error.                        \
     */                                                                        \
    static inline int prefix##_resize(struct prefix* v, int##bits##_t count);  \
                                                                               \
    /*!                                                                        \
     * @brief Resets the vector's size to 0.                                   \
     * @note This does not actually free the underlying memory, it simply      \
     * resets the element counter. If you wish to free the underlying memory,  \
     * see vec_clear_compact().                                                \
     * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.        \
     */                                                                        \
    static inline void prefix##_clear(struct prefix* v);                       \
                                                                               \
    /*!                                                                        \
     * @brief Frees any excess memory. The memory buffer is resized to fit the \
     * current number of elements.                                             \
     * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.        \
     */                                                                        \
    void prefix##_compact(struct prefix* v);                                   \
                                                                               \
    /*!                                                                        \
     * @brief A combination of @see vec_clear() and @see vec_compact(). Resets \
     * the vector's count to 0 and frees all underlying memory.                \
     * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.        \
     */                                                                        \
    static inline void prefix##_clear_compact(struct prefix* v);               \
                                                                               \
    /*!                                                                        \
     * @brief Inserts (copies) a new element into the end of the vector.       \
     * @note This can cause a re-allocation of the underlying memory. This     \
     * implementation expands the allocated memory by a factor of 2 every time \
     * the capacity is reached to cut down on the frequency of re-allocations. \
     * @note If you do not wish to copy data into the vector, but merely make  \
     * space, see vec_push_emplace().                                          \
     * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.        \
     * @param[in] element The value to copy into the vector.                   \
     * @return Returns 0 on success, negative on error.                        \
     */                                                                        \
    static inline int prefix##_push(struct prefix* v, T item);                 \
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
     * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.        \
     * @param[in] element The value to copy into the vector.                   \
     * @return Returns a pointer to the inserted space.                        \
     */                                                                        \
    T* prefix##_emplace(struct prefix* v);                                     \
                                                                               \
    /*!                                                                        \
     * @brief Inserts (copies) a new element into the vector at the specified  \
     * index.                                                                  \
     * @note This can cause a re-allocation of the underlying memory. This     \
     * implementation expands the allocated memory by a factor of 2 every time \
     * the capacity is reached to cut down on the frequency of re-allocations. \
     * @note If you do not wish to copy data into the vector, but merely make  \
     * space, see vec_insert_emplace().                                        \
     * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.        \
     * @param[in] i The index the new element should be inserted into.         \
     * Existing elements are shifted aside to make space. Should be between 0  \
     * - vec_count().                                                          \
     * @param[in] element The value to copy into the vector.                   \
     * @return Returns 0 on success, negative on error.                        \
     */                                                                        \
    static inline int prefix##_insert(                                         \
        struct prefix* v, int##bits##_t i, T item);                            \
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
     * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.        \
     * @param[in] i The index the new element should be inserted into.         \
     * Existing elements are shifted aside to make space. Should be between 0  \
     * - vec_count().                                                          \
     * @return Returns a pointer to the inserted space.                        \
     */                                                                        \
    T* prefix##_insert_emplace(struct prefix* v, int##bits##_t i);             \
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
    void prefix##_erase(struct prefix* v, int##bits##_t i);                    \
                                                                               \
    static inline void prefix##_init(struct prefix* v)                         \
    {                                                                          \
        v->mem = NULL;                                                         \
    }                                                                          \
                                                                               \
    static inline void prefix##_deinit(struct prefix* v)                       \
    {                                                                          \
        if (v->mem)                                                            \
            mem_free(v->mem);                                                  \
    }                                                                          \
                                                                               \
    static inline int prefix##_resize(struct prefix* v, int##bits##_t elems)   \
    {                                                                          \
        if (prefix##_reserve(v, elems) == 0)                                   \
        {                                                                      \
            v->mem->count = elems;                                             \
            return 0;                                                          \
        }                                                                      \
        return -1;                                                             \
    }                                                                          \
                                                                               \
    static inline void prefix##_clear(struct prefix* v)                        \
    {                                                                          \
        if (v->mem)                                                            \
            v->mem->count = 0;                                                 \
    }                                                                          \
    static inline void prefix##_clear_compact(struct prefix* v)                \
    {                                                                          \
        if (v->mem)                                                            \
            mem_free(v->mem);                                                  \
        v->mem = NULL;                                                         \
    }                                                                          \
    static inline int prefix##_push(struct prefix* v, T item)                  \
    {                                                                          \
        T* ins = prefix##_emplace(v);                                          \
        if (ins == NULL)                                                       \
            return -1;                                                         \
        *ins = item;                                                           \
        return 0;                                                              \
    }                                                                          \
    static inline int prefix##_insert(                                         \
        struct prefix* v, int##bits##_t i, T elem)                             \
    {                                                                          \
        T* ins = prefix##_insert_emplace(v, i);                                \
        if (ins == NULL)                                                       \
            return -1;                                                         \
        *ins = elem;                                                           \
        return 0;                                                              \
    }                                                                          \
    static inline T* prefix##_pop(struct prefix* v)                            \
    {                                                                          \
        return &v->mem->data[--(v->mem->count)];                               \
    }

#define VEC_DEFINE_API(prefix, T, bits)                                        \
    static int prefix##_realloc(struct prefix* v, int##bits##_t elems)         \
    {                                                                          \
        mem_size bytes = sizeof(*v->mem)                                       \
                       + sizeof(v->mem->data[0]) * (elems - 1);                \
        void* new_mem = mem_realloc(v->mem, bytes);                            \
        if (new_mem == NULL)                                                   \
            return mem_report_oom(bytes, #prefix "_reserve");                  \
        *(void**)&v->mem = new_mem;                                            \
        return 0;                                                              \
    }                                                                          \
                                                                               \
    int prefix##_reserve(struct prefix* v, int##bits##_t elems)                \
    {                                                                          \
        if (prefix##_realloc(v, elems) != 0)                                   \
            return -1;                                                         \
        v->mem->count = 0;                                                     \
        v->mem->capacity = elems;                                              \
        return 0;                                                              \
    }                                                                          \
    void prefix##_compact(struct prefix* v)                                    \
    {                                                                          \
        if (v->mem == NULL)                                                    \
            return;                                                            \
                                                                               \
        if (v->mem->count == 0)                                                \
        {                                                                      \
            mem_free(v->mem);                                                  \
            v->mem = NULL;                                                     \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            void* new_mem = mem_realloc(                                       \
                v->mem,                                                        \
                sizeof(*v->mem)                                                \
                    + sizeof(v->mem->data[0]) * (v->mem->count - 1));          \
            *(void**)&v->mem = new_mem;                                        \
            v->mem->capacity = v->mem->count;                                  \
        }                                                                      \
    }                                                                          \
    T* prefix##_emplace(struct prefix* v)                                      \
    {                                                                          \
        if (v->mem == NULL)                                                    \
            if (prefix##_reserve(v, ODBSDK_VEC_MIN_CAPACITY) != 0)             \
                return NULL;                                                   \
                                                                               \
        if (v->mem->count == v->mem->capacity)                                 \
        {                                                                      \
            if (prefix##_realloc(                                              \
                    v, v->mem->capacity * ODBSDK_VEC_EXPAND_FACTOR)            \
                != 0)                                                          \
                return NULL;                                                   \
            v->mem->capacity *= ODBSDK_VEC_EXPAND_FACTOR;                      \
        }                                                                      \
                                                                               \
        return &v->mem->data[(v->mem->count)++];                               \
    }                                                                          \
    T* prefix##_insert_emplace(struct prefix* v, int##bits##_t i)              \
    {                                                                          \
        if (prefix##_emplace(v) == NULL)                                       \
            return NULL;                                                       \
                                                                               \
        memmove(                                                               \
            v->mem->data + i + 1,                                              \
            v->mem->data + i,                                                  \
            (v->mem->count - 1) * sizeof(v->mem->data[0]));                    \
        return &v->mem->data[i];                                               \
    }                                                                          \
    void prefix##_erase(struct prefix* v, int##bits##_t i)                     \
    {                                                                          \
        memmove(                                                               \
            v->mem->data + i,                                                  \
            v->mem->data + i + 1,                                              \
            (v->mem->count - i) * sizeof(v->mem->data[0]));                    \
        --v->mem->count;                                                       \
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
#define vec_first(v) (&(v)->mem->data[0])

/*!
 * @brief Returns the first element of the vector.
 * @warning The returned pointer could be invalidated if any other vector
 * related function is called, as the underlying memory of the vector could be
 * re-allocated. Use the pointer immediately after calling this function.
 * @param[in] v Pointer to a vector of type VEC(T,B)*.
 * @return A pointer to the last element. See warning and use with caution.
 * Vector must not be empty.
 */
#define vec_last(v) (&(v)->mem->data[(v)->mem->count - 1])

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
#define vec_get(v, i) (&(v)->mem->data[i])

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
#define vec_rget(v, i) (&(v)->mem->data[(v)->mem->count - (i)-1])

/*!
 * @brief Returns the number of elements in the vector. Works on NULL vectors,
 * too.
 * @param[in] v Pointer to a vector of type VEC(T,B)*. Can be NULL.
 */
#define vec_count(v) ((v)->mem ? (v)->mem->count : 0)

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
    for (intptr_t var_##i = 0; (v)->mem && var_##i != (v)->mem->count          \
                               && ((var = &(v)->mem->data[var_##i]) || 1);     \
         ++var_##i)
