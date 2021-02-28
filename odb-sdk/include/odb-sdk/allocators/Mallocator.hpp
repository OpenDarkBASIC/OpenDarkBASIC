#pragma once

#include "odb-sdk/config.hpp"
#include "odb-sdk/allocators/Block.hpp"

namespace odb::alloc {

class ODBSDK_PUBLIC_API Mallocator
{
public:
    Block allocate(std::size_t bytes);
    void deallocate(Block block);
};

}
