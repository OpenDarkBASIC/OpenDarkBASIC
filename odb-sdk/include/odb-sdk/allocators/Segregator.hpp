#pragma once

#include "odb-sdk/allocators/Block.hpp"

namespace odb::alloc {

template <std::size_t threshold, typename SmallAllocator, typename LargeAllocator>
class Segregator : private SmallAllocator, private LargeAllocator
{
public:
    Block allocate(std::size_t bytes);
    void deallocate(Block block);
    bool owns(Block block) const;
};

// ----------------------------------------------------------------------------
template <std::size_t threshold, typename SmallAllocator, typename LargeAllocator>
Block Segregator<threshold, SmallAllocator, LargeAllocator>::allocate(std::size_t bytes)
{
    if (bytes <= threshold)
        return SmallAllocator::allocate(bytes);
    return LargeAllocator::allocate(bytes);
}

// ----------------------------------------------------------------------------
template <std::size_t threshold, typename SmallAllocator, typename LargeAllocator>
void Segregator<threshold, SmallAllocator, LargeAllocator>::deallocate(Block block)
{
    if (block.size <= threshold)
        return SmallAllocator::deallocate(block);
    return LargeAllocator::deallocate(block);
}

// ----------------------------------------------------------------------------
template <std::size_t threshold, typename SmallAllocator, typename LargeAllocator>
bool Segregator<threshold, SmallAllocator, LargeAllocator>::owns(Block block) const
{
    if (block.size <= threshold)
        return SmallAllocator::owns(block);
    return LargeAllocator::owns(block);
}

}
