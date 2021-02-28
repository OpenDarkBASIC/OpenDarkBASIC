#pragma once

#include "odb-sdk/allocators/Block.hpp"

namespace odb::alloc {

class Null
{
public:
    Block allocate(std::size_t bytes);
    void deallocate(Block block);
    bool owns(Block block) const;
};

}
