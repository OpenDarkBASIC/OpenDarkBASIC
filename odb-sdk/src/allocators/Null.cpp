#include "odb-sdk/allocators/Null.hpp"
#include <cassert>

namespace odb::alloc {

// ----------------------------------------------------------------------------
Block Null::allocate(std::size_t bytes)
{
    return Block(nullptr, 0);
}

// ----------------------------------------------------------------------------
void Null::deallocate(Block block)
{
    assert(block.ptr == nullptr);
    assert(block.size == 0);
}

// ----------------------------------------------------------------------------
bool Null::owns(Block block) const
{
    return (block.ptr == nullptr && block.size == 0);
}

}
