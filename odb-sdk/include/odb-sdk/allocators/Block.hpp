#pragma once

#include <new>

namespace odb {
    class RefCount;
}

namespace odb::alloc {

class Block
{
public:
    Block(void* ptr, std::size_t size) : ptr(ptr), size(size) {}

    bool operator!() const { return ptr == nullptr; }
    operator bool() const { return ptr != nullptr; }

    void* ptr;
    std::size_t size;
};

}
