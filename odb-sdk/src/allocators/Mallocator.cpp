#include "odb-sdk/allocators/Mallocator.hpp"
#include <memory>
#include <cstdio>

namespace odb::alloc {

// ----------------------------------------------------------------------------
Block Mallocator::allocate(std::size_t bytes)
{
    void* p = std::malloc(bytes);
    if (p == nullptr)
        return Block(nullptr, 0);
    fprintf(stderr, "Mallocator::allocate(%p, %ld)\n", p, bytes);
    return Block(p, bytes);
}

// ----------------------------------------------------------------------------
void Mallocator::deallocate(Block block)
{
    fprintf(stderr, "Mallocator::deallocate(%p, %ld)\n", block.ptr, block.size);
    std::free(block.ptr);
}

}

