#include "odb-sdk/allocators/Page.hpp"
#include <sys/mman.h>
#include <unistd.h>

namespace odb::alloc {

// ----------------------------------------------------------------------------
Page::Page()
    : pageSize_(sysconf(_SC_PAGE_SIZE))
{
}

// ----------------------------------------------------------------------------
Block Page::allocate(std::size_t bytes)
{
    std::size_t pageSizeMultiple = ((bytes + pageSize_ - 1) / pageSize_) * pageSize_;
    return Block(mmap(NULL, pageSizeMultiple, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0), bytes);
}

// ----------------------------------------------------------------------------
void Page::deallocate(Block block)
{
    std::size_t pageSizeMultiple = ((block.size + pageSize_ - 1) / pageSize_) * pageSize_;
    munmap(block.ptr, pageSizeMultiple);
}

}
