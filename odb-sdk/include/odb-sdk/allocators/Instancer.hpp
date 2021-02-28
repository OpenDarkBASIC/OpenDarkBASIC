#pragma once

#include "odb-sdk/allocators/Instancer.hxx"

namespace odb::alloc {

template <typename Allocator, typename InstanceAllocator>
template <typename T, typename... Args>
T* Instancer<Allocator, InstanceAllocator>::create(Args&&... args)
{
    Block block = allocator_.allocate(sizeof(T));
    if (!block)
        throw std::bad_alloc();

    T* o = new (block.ptr) T(std::forward<Args>(args)...);
    o->setInstancer(this);
    return o;
}

template <typename Allocator, typename InstanceAllocator>
template <typename T>
void Instancer<Allocator, InstanceAllocator>::destroy(T* o)
{
    void* mem = dynamic_cast<void*>(o);
    o->~T();
    allocator_.deallocate(Block(mem, sizeof(T)));
}

}
