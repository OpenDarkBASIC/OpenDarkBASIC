#pragma once

#include "odb-sdk/allocators/Block.hpp"

namespace odb::alloc {

template <std::size_t capacity>
class Stack
{
public:
    Stack();

    Block allocate(std::size_t bytes);
    void deallocate(Block);
    bool owns(Block) const;

protected:
    char buffer_[capacity];
    char* ptr_;
};

// ----------------------------------------------------------------------------
template <std::size_t capacity>
Stack<capacity>::Stack()
    : ptr_(buffer_)
{
}

// ----------------------------------------------------------------------------
template <std::size_t capacity>
Block Stack<capacity>::allocate(std::size_t bytes)
{
    if (bytes > buffer_ + capacity - ptr_)
        return Block(nullptr, 0);

    Block block(ptr_, bytes);
    ptr_ += bytes;
    return block;
}

// ----------------------------------------------------------------------------
template <std::size_t capacity>
void Stack<capacity>::deallocate(Block block)
{
    if (ptr_ == static_cast<char*>(block.ptr) + block.size)
        ptr_ -= block.size;
}

// ----------------------------------------------------------------------------
template <std::size_t capacity>
bool Stack<capacity>::owns(Block block)
{
    return block.ptr >= buffer_ && block.ptr < buffer_ + capacity;
}

}
