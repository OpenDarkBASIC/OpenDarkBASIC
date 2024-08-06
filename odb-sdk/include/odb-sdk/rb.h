/*!
 * @file rb.h
 * @author TheComet
 *
 * The ring buffer consists of a read and write index, and a chunk of memory:
 * ```c
 * struct rb_t {
 *     int read, write;
 *     T data[N];
 * }
 * ```
 *
 * The buffer is considered empty when rb->read == rb->write. It is considered
 * full when rb->write is one slot behind rb->read. This is necessary because
 * otherwise there would be no way to tell the difference between an empty and
 * a full ring buffer.
 */
#pragma once

#include "odb-sdk/config.h"
#include "odb-sdk/log.h"
#include <stdint.h>

#define RB_DECLARE_API(API, prefix, T, bits)                                   \
    RB_DECLARE_API_FULL(API, prefix, T, bits, 16)

#define RB_DECLARE_API_FULL(API, prefix, T, bits, MIN_CAPACITY)                \
    struct prefix                                                              \
    {                                                                          \
        int##bits##_t read, write, capacity;                                   \
        T             data[1];                                                 \
    };                                                                         \
    static struct prefix prefix##_null_rb;                                     \
                                                                               \
    /* NOTE:                                                                   \
     * For the following 4 functions it is very important that each member     \
     * is only accessed once. */                                               \
    static inline int##bits##_t prefix##_count(struct prefix* rb)              \
    {                                                                          \
        return (rb->write - rb->read) & (rb->capacity - 1);                    \
    }                                                                          \
                                                                               \
    static inline int##bits##_t prefix##_space(struct prefix* rb)              \
    {                                                                          \
        return (rb->read - rb->write - 1) & (rb->capacity - 1);                \
    }                                                                          \
    static inline int prefix##_is_full(struct prefix* rb)                      \
    {                                                                          \
        return ((rb->write + 1) & (rb->capacity - 1)) == rb->read;             \
    }                                                                          \
                                                                               \
    static inline int prefix##_is_empty(struct prefix* rb)                     \
    {                                                                          \
        return rb->read == rb->write;                                          \
    }                                                                          \
                                                                               \
    /*!                                                                        \
     * @brief This must be called before operating on any ring buffer.         \
     * Initializes the structure to a defined state.                           \
     * @param[in] rb Pointer to a ring buffer of type RB(T,B)*                 \
     */                                                                        \
    static inline void prefix##_init(struct prefix** rb)                       \
    {                                                                          \
        *rb = &prefix##_null_rb;                                               \
    }                                                                          \
                                                                               \
    /*!                                                                        \
     * @brief Destroys an existing ring buffer object and frees all memory     \
     * allocated by inserting elements.                                        \
     * @param[in] rb Pointer to a ring buffer of type RB(T,B)*                 \
     */                                                                        \
    static inline void prefix##_deinit(struct prefix* rb)                      \
    {                                                                          \
        if (rb->capacity)                                                      \
            mem_free(rb);                                                      \
    }                                                                          \
                                                                               \
    /*!                                                                        \
     * @brief Resizes the ring buffer to contain N number of slots.            \
     * @note Only N-1 elements can be inserted before the buffer is full.      \
     * @param[in] rb Pointer to a ring buffer of type RB(T,B)*                 \
     * @param[in] slots Number of slots to resize to. @note Value must be a    \
     * power of 2.                                                             \
     * @return Returns 0 on success, negative on error.                        \
     */                                                                        \
    API int prefix##_resize(struct prefix** rb, int##bits##_t elems);          \
                                                                               \
    /*!                                                                        \
     * @brief Adds (copies) an element into the writing-end of the ring        \
     * buffer. The write pointer is advanced to the next position.             \
     * @note If the buffer is full, this function will fail. If you want to    \
     * automatically resize the buffer if it is full, use @see                 \
     * rb_put_resize().                                                        \
     * @note If you do not wish to copy data into the ring buffer, but merely  \
     * make space, use @see rb_emplace() instead.                              \
     * @param[in] rb Pointer to a ring buffer of type RB(T,B)*                 \
     * @param[in] elem The element to copy into the ring buffer.               \
     * @return Returns 0 on success, or negative if the buffer was full.       \
     */                                                                        \
    static inline int prefix##_put(struct prefix* rb, T elem)                  \
    {                                                                          \
        int##bits##_t write = rb->write;                                       \
        if (prefix##_is_full(rb))                                              \
            return -1;                                                         \
        rb->write = (write + 1) & ((int##bits##_t)rb->capacity - 1);           \
        rb->data[write] = elem;                                                \
        return 0;                                                              \
    }                                                                          \
    static inline int prefix##_put_realloc(struct prefix** rb, T elem)         \
    {                                                                          \
        int##bits##_t write;                                                   \
        if (prefix##_is_full(*rb))                                             \
            if (prefix##_resize(                                               \
                    rb, (*rb)->capacity ? (*rb)->capacity * 2 : MIN_CAPACITY)  \
                != 0)                                                          \
            {                                                                  \
                return -1;                                                     \
            }                                                                  \
        write = (*rb)->write;                                                  \
        (*rb)->write = (write + 1) & ((int##bits##_t)(*rb)->capacity - 1);     \
        (*rb)->data[write] = elem;                                             \
        return 0;                                                              \
    }                                                                          \
                                                                               \
    /*!                                                                        \
     * @brief Allocates space for a new element at the writing-end of the ring \
     * buffer, but does not initialize it.                                     \
     * @warning The returned pointer could be invalidated if the buffer is     \
     * resized.                                                                \
     * @note If the buffer is full, this function will fail. If you want a     \
     * version of this function that automatically resizes the buffer in this  \
     * case, use @see rb_emplace_resize() instead.                             \
     * @param[in] rb Pointer to a ring buffer of type RB(T,B)*                 \
     * @return Returns a pointer to the uninitialized element if insertion is  \
     * successfull, or NULL if there is no space left.                         \
     */                                                                        \
    static inline T* prefix##_emplace(struct prefix* rb)                       \
    {                                                                          \
        int##bits##_t write = rb->write;                                       \
        T*            value = &rb->data[write];                                \
        if (prefix##_is_full(rb))                                              \
            return NULL;                                                       \
        rb->write = (write + 1) & ((int##bits##_t)rb->capacity - 1);           \
        return value;                                                          \
    }                                                                          \
    static inline T* prefix##_emplace_realloc(struct prefix** rb)              \
    {                                                                          \
        int##bits##_t write;                                                   \
        T*            value;                                                   \
        if (prefix##_is_full(*rb))                                             \
            if (prefix##_resize(                                               \
                    rb, (*rb)->capacity ? (*rb)->capacity * 2 : MIN_CAPACITY)  \
                != 0)                                                          \
            {                                                                  \
                return NULL;                                                   \
            }                                                                  \
        write = (*rb)->write;                                                  \
        value = &(*rb)->data[write];                                           \
        (*rb)->write = (write + 1) & ((int##bits##_t)(*rb)->capacity - 1);     \
        return value;                                                          \
    }                                                                          \
                                                                               \
    /*!                                                                        \
     * @brief Removes an element from the reading-end of the ring buffer and   \
     * returns it.                                                             \
     * @warning The ring buffer must contain at least 1 element.               \
     * Use @see rb_is_empty() first if you need to.                            \
     * @warning The returned pointer could be invalidated if the buffer is     \
     * resized.                                                                \
     * @param[in] rb Pointer to a ring buffer of type RB(T,B)*                 \
     */                                                                        \
    static inline T* prefix##_take(struct prefix* rb)                          \
    {                                                                          \
        ODBSDK_DEBUG_ASSERT(                                                   \
            !prefix##_is_empty(rb), log_sdk_err("rb is empty\n"));             \
        int##bits##_t read = rb->read;                                         \
        T*            data = &rb->data[read];                                  \
        rb->read = (read + 1) & ((int##bits##_t)rb->capacity - 1);             \
        return data;                                                           \
    }                                                                          \
                                                                               \
    static inline void prefix##_clear(struct prefix* rb)                       \
    {                                                                          \
        rb->read = rb->write;                                                  \
    }                                                                          \
                                                                               \
    static inline T* prefix##_peek_read(struct prefix* rb)                     \
    {                                                                          \
        return &rb->data[rb->read];                                            \
    }                                                                          \
                                                                               \
    static inline T* prefix##_peek_write(struct prefix* rb)                    \
    {                                                                          \
        return &rb->data[(rb->write - 1) & ((int##bits##_t)rb->capacity - 1)]; \
    }                                                                          \
                                                                               \
    static inline T* prefix##_peek(struct prefix* rb, int##bits##_t idx)       \
    {                                                                          \
        int##bits##_t offset = (rb->read + idx) & (rb->capacity - 1);          \
        return &rb->data[offset];                                              \
    }

#define IS_POWER_OF_2(x) (((x) & ((x) - 1)) == 0)
#define RB_DEFINE_API(prefix, T, bits)                                         \
    int prefix##_resize(struct prefix** rb, int##bits##_t elems)               \
    {                                                                          \
        struct prefix* new_rb;                                                 \
        mem_size       bytes                                                   \
            = offsetof(struct prefix, data) + sizeof((*rb)->data[0]) * elems;  \
                                                                               \
        ODBSDK_DEBUG_ASSERT(                                                   \
            IS_POWER_OF_2(elems), log_sdk_err("elems: %d\n", elems));          \
        new_rb = (struct prefix*)mem_realloc(                                  \
            (*rb)->capacity ? *rb : NULL, bytes);                              \
        if (new_rb == NULL)                                                    \
            return log_oom(bytes, "rb_resize()");                              \
        if ((*rb)->capacity == 0)                                              \
        {                                                                      \
            (*rb) = new_rb;                                                    \
            (*rb)->read = 0;                                                   \
            (*rb)->write = 0;                                                  \
        }                                                                      \
        else                                                                   \
            (*rb) = new_rb;                                                    \
                                                                               \
        /* Is the data wrapped? */                                             \
        if ((*rb)->read > (*rb)->write)                                        \
        {                                                                      \
            memmove(                                                           \
                (*rb)->data + (*rb)->capacity,                                 \
                (*rb)->data,                                                   \
                (*rb)->write * sizeof((*rb)->data[0]));                        \
            (*rb)->write += (*rb)->capacity;                                   \
        }                                                                      \
                                                                               \
        (*rb)->capacity = elems;                                               \
        return 0;                                                              \
    }

/*!
 * @brief Iterates over the elements in a ring buffer.
 *
 *   T* element;
 *   RB(T,32) rb = some_ring_buffer();
 *   rb_for_each(rb, element) {
 *       ... do stuff ...
 *   }
 */
#define rb_for_each(rb, elem)                                                  \
    for (intptr_t elem##_i = (rb)->read;                                       \
         elem##_i != (rb)->write && ((elem = &(rb)->data[elem##_i]) || 1);     \
         elem##_i = (elem##_i + 1) & ((rb)->capacity - 1))
