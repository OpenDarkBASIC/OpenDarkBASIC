#pragma once

#include "odb-sdk/allocators/Block.hpp"

namespace odb::alloc {

template <typename P, typename FB>
class Fallback : private P, private FB
{
public:
    Block allocate(std::size_t bytes);
    void deallocate(Block);
    bool owns(Block) const;
};

// ----------------------------------------------------------------------------
template <typename P, typename FB>
Block Fallback<P, FB>::allocate(std::size_t bytes)
{
    Block block = P::allocate(bytes);
    if (block.ptr == nullptr)
        block = FB::allocate(bytes);
    return block;
}

// ----------------------------------------------------------------------------
template <typename P, typename FB>
void Fallback<P, FB>::deallocate(Block block)
{
    if (P::owns(block))
        P::deallocate(block);
    else
        FB::deallocate(block);
}

// ----------------------------------------------------------------------------
template <typename P, typename FB>
bool Fallback<P, FB>::owns(Block block) const
{
    return P::owns(block) || FB::owns(block);
}

}
