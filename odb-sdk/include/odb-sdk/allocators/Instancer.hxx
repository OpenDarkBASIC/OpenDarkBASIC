#pragma once

#include "odb-sdk/RefCounted.hxx"

namespace odb::alloc {

template <typename Allocator, typename InstanceAllocator>
class Instancer : public RefCounted<InstanceAllocator>
{
public:
    template <typename T, typename... Args>
    T* create(Args&&... args);

    template <typename T>
    void destroy(T* o);

private:
    Allocator allocator_;
};

}
