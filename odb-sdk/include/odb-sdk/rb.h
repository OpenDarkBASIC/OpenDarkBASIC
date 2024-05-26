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

#define RB_DECLARE_TYPE(name, T, bits)                                         \
    struct name                                                                \
    {                                                                          \
        struct                                                                 \
        {                                                                      \
            int##bits##_t read, write, capacity;                               \
            T             data[1];                                             \
        }* mem;                                                                \
    }

#define RB_DECLARE_API(prefix, T, bits)                                        \
    RB_DECLARE_TYPE(prefix, T, bits);                                          \
                                                                               \
    /* NOTE:                                                                   \
     * For the following 4 functions it is very important that each argument   \
     * is only accessed once. */                                               \
                                                                               \
    static inline int##bits##_t prefix##_count(struct prefix rb)               \
    {                                                                          \
        return (rb.mem->write - rb.mem->read) & (rb.mem->capacity - 1);        \
    }                                                                          \
                                                                               \
    static inline int##bits##_t prefix##_space(struct prefix rb)               \
    {                                                                          \
        return (rb.mem->read - rb.mem->write - 1) & (rb.mem->capacity - 1);    \
    }                                                                          \
    static inline int prefix##_is_full(struct prefix rb)                       \
    {                                                                          \
        return ((rb.mem->write + 1) & (rb.mem->capacity - 1)) == rb.mem->read; \
    }                                                                          \
                                                                               \
    static inline int prefix##_is_empty(struct prefix rb)                      \
    {                                                                          \
        return rb.mem->read == rb.mem->write;                                  \
    }                                                                          \
                                                                               \
    /*!                                                                        \
     * @brief This must be called before operating on any ring buffer.         \
     * Initializes the structure to a defined state.                           \
     * @param[in] rb Pointer to a ring buffer of type RB(T,B)*                 \
     */                                                                        \
    static inline void prefix##_init(struct prefix* rb)                        \
    {                                                                          \
        rb->mem = NULL;                                                        \
    }                                                                          \
                                                                               \
    /*!                                                                        \
     * @brief Destroys an existing ring buffer object and frees all memory     \
     * allocated by inserting elements.                                        \
     * @param[in] rb Pointer to a ring buffer of type RB(T,B)*                 \
     */                                                                        \
    void prefix##_deinit(struct prefix* rb);                                   \
                                                                               \
    /*!                                                                        \
     * @brief Resizes the ring buffer to contain N number of slots.            \
     * @note Only N-1 elements can be inserted before the buffer is full.      \
     * @param[in] rb Pointer to a ring buffer of type RB(T,B)*                 \
     * @param[in] slots Number of slots to resize to. @note Value must be a    \
     * power of 2.                                                             \
     * @return Returns 0 on success, negative on error.                        \
     */                                                                        \
    int prefix##_resize(struct prefix* rb, int##bits##_t elems);               \
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
    static inline int prefix##_put(struct prefix rb, T elem)                   \
    {                                                                          \
        int##bits##_t write = rb.mem->write;                                   \
        if (prefix##_is_full(rb))                                              \
            return -1;                                                         \
        rb.mem->write = (write + 1) & ((int##bits##_t)rb.mem->capacity - 1);   \
        rb.mem->data[write] = elem;                                            \
        return 0;                                                              \
    }                                                                          \
    static inline int prefix##_put_realloc(struct prefix* rb, T elem)          \
    {                                                                          \
        int##bits##_t write;                                                   \
        if (prefix##_is_full(*rb))                                             \
            if (prefix##_resize(rb, rb->mem->capacity * 2) != 0)               \
                return -1;                                                     \
        write = rb->mem->write;                                                \
        rb->mem->write = (write + 1) & ((int##bits##_t)rb->mem->capacity - 1); \
        rb->mem->data[write] = elem;                                           \
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
    static inline T* prefix##_emplace(struct prefix rb)                        \
    {                                                                          \
        int##bits##_t write = rb.mem->write;                                   \
        T*            value = &rb.mem->data[write];                            \
        if (prefix##_is_full(rb))                                              \
            return NULL;                                                       \
        rb.mem->write = (write + 1) & ((int##bits##_t)rb.mem->capacity - 1);   \
        return value;                                                          \
    }                                                                          \
    static inline T* prefix##_emplace_realloc(struct prefix* rb)               \
    {                                                                          \
        int##bits##_t write;                                                   \
        T*            value;                                                   \
        if (prefix##_is_full(*rb))                                             \
            if (prefix##_resize(rb, rb->mem->capacity * 2) != 0)               \
                return NULL;                                                   \
        write = rb->mem->write;                                                \
        value = &rb->mem->data[write];                                         \
        rb->mem->write = (write + 1) & ((int##bits##_t)rb->mem->capacity - 1); \
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
    static inline T* prefix##_take(struct prefix rb)                           \
    {                                                                          \
        int##bits##_t read = rb.mem->read;                                     \
        T*            data = &rb.mem->data[read];                              \
        rb.mem->read = (read + 1) & ((int##bits##_t)rb.mem->capacity - 1);     \
        return data;                                                           \
    }                                                                          \
                                                                               \
    static inline void prefix##_clear(struct prefix rb)                        \
    {                                                                          \
        rb.mem->read = rb.mem->write;                                          \
    }                                                                          \
                                                                               \
    static inline T* prefix##_peek_read(struct prefix rb)                      \
    {                                                                          \
        return &rb.mem->data[rb.mem->read];                                    \
    }                                                                          \
                                                                               \
    static inline T* prefix##_peek_write(struct prefix rb)                     \
    {                                                                          \
        return &rb.mem->data                                                   \
                    [(rb.mem->write - 1)                                       \
                     & ((int##bits##_t)rb.mem->capacity - 1)];                 \
    }                                                                          \
                                                                               \
    static inline T* prefix##_peek(struct prefix rb, int##bits##_t idx)        \
    {                                                                          \
        int##bits##_t offset = (rb.mem->read + idx) & (rb.mem->capacity - 1);  \
        return &rb.mem->data[offset];                                          \
    }

#define IS_POWER_OF_2(x) (((x) & ((x)-1)) == 0)
#define RB_DEFINE_API(prefix, T, bits)                                         \
    void prefix##_deinit(struct prefix* rb)                                    \
    {                                                                          \
        if (rb->mem)                                                           \
            mem_free(rb->mem);                                                 \
    }                                                                          \
                                                                               \
    int prefix##_resize(struct prefix* rb, int##bits##_t elems)                \
    {                                                                          \
        void*    new_mem;                                                      \
        mem_size bytes                                                         \
            = sizeof(*rb->mem) + sizeof(rb->mem->data[0]) * (elems - 1);       \
                                                                               \
        ODBSDK_DEBUG_ASSERT(IS_POWER_OF_2(elems));                             \
        new_mem = mem_realloc(rb->mem, bytes);                                 \
        if (new_mem == NULL)                                                   \
            return log_oom(bytes, "rb_resize()");                              \
        if (rb->mem == NULL)                                                   \
        {                                                                      \
            *(void**)&rb->mem = new_mem;                                       \
            rb->mem->read = 0;                                                 \
            rb->mem->write = 0;                                                \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            *(void**)&rb->mem = new_mem;                                       \
        }                                                                      \
                                                                               \
        /* Is the data wrapped? */                                             \
        if (rb->mem->read > rb->mem->write)                                    \
        {                                                                      \
            memmove(                                                           \
                rb->mem->data + rb->mem->capacity,                             \
                rb->mem->data,                                                 \
                rb->mem->write * sizeof(rb->mem->data[0]));                    \
            rb->mem->write += rb->mem->capacity;                               \
        }                                                                      \
                                                                               \
        rb->mem->capacity = elems;                                             \
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
    for (intptr_t elem##_i = (rb).mem->read;                                   \
         elem##_i != (rb).mem->write                                           \
         && ((elem = &(rb).mem->data[elem##_i]) || 1);                         \
         elem##_i = (elem##_i + 1) & ((rb).mem->capacity - 1))
