//
// Copyright (c) 2008-2016 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include <cassert>
#include <new>

namespace odb {

template <typename T> class Reference;

namespace alloc {
    class DefaultAllocator {};
    template <typename Allocator, typename InstanceAllocator=DefaultAllocator> class Instancer;
    template <std::size_t capacity> class Stack;
}

namespace detail {
template <typename RefCountType>
class RefCountedBase
{
public:
    RefCountedBase();

    virtual ~RefCountedBase();

    RefCountedBase(const RefCountedBase& rhs) = delete;
    RefCountedBase& operator=(const RefCountedBase& rhs) = delete;

    /// Increment reference count. Can also be called outside of a Reference for traditional reference counting.
    void addRef();

    /// Decrement reference count and delete self if no more references. Can also be called outside of a Reference for traditional reference counting.
    void releaseRef();

    /// Detach without destroying the object even if the refcount goes zero. To be used for scripting language interoperation.
    void detach();

    /// Return reference count.
    RefCountType refs() const;

protected:
    virtual void seppuku() = 0;

private:
    RefCountType refs_;
};
}

template <typename Allocator=alloc::DefaultAllocator, typename RefCountType=int>
class RefCounted;

template <typename Allocator, typename RefCountType>
class RefCounted : public detail::RefCountedBase<RefCountType>
{
public:
    typedef typename alloc::Instancer<Allocator> Instancer;

    ~RefCounted();
    static Reference<Instancer> newInstancer();

private:
    friend alloc::Instancer<Allocator>;

    void setInstancer(Instancer* instancer);
    void seppuku() override final;

private:
    Instancer* instancer_;
};

template <typename RefCountType>
class RefCounted<alloc::DefaultAllocator, RefCountType> : public detail::RefCountedBase<RefCountType>
{
private:
    void seppuku() override final;
};

}
