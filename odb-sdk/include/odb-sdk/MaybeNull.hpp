#pragma once

#include "odb-sdk/config.hpp"

namespace odb {

template <typename T>
class MaybeNull
{
public:
    MaybeNull() : ptr_(nullptr) {}
    MaybeNull(T* p) : ptr_(p) {}

    bool isNull() const { return ptr_ == nullptr; }
    bool notNull() const { return ptr_ != nullptr; }

    T* get() const {
        assert(ptr_ != nullptr);
        return ptr_;
    }

    T* operator->() const {
        assert(ptr_ != nullptr);
        return ptr_;
    }

    operator T*() const {
        assert(ptr_ != nullptr);
        return ptr_;
    }

private:
    T* ptr_;
};

}
