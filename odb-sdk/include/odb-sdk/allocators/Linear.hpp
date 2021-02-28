#pragma once

#include "odb-sdk/allocators/Block.hpp"
#include "odb-sdk/Reference.hpp"
#include <cassert>
#include <vector>

namespace odb::alloc {

template <typename Allocator, std::size_t blockSize>
class Linear : private Allocator
{
public:
    ~Linear()
    {
        // Free all blocks at once
        for (const auto& block : blocks_)
            Allocator::deallocate(Block(block, blockSize));
    }

    Block allocate(std::size_t bytes)
    {
        if (blocks_.size() == 0 || !blockHasCapacityFor(blocks_.back(), bytes))
        {
            Block block = Allocator::allocate(blockSize);
            if (!block)
                return block;
            ptr_ = static_cast<char*>(block.ptr);
            blocks_.push_back(ptr_);
        }

        Block block(ptr_, bytes);
        ptr_ += bytes;
        return block;
    }

    void deallocate(Block block)
    {
        // We defer deallocation to when the instancer is destroyed
    }

    bool blockHasCapacityFor(char* blockStart, std::size_t bytes)
    {
        return bytes <= blockSize - (ptr_ - blockStart);
    }

private:
    std::vector<char*> blocks_;
    char* ptr_ = nullptr;
};

}
