#pragma once

#include "odb-sdk/allocators/Block.hpp"

namespace odb::alloc {

class Page
{
public:
    Page();
    Block allocate(std::size_t bytes);
    void deallocate(Block block);

private:
    long pageSize_;
};

}
