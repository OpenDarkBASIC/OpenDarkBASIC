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

#include "odb-sdk/RefCounted.hxx"
#include <cassert>
#include <new>

namespace odb {

namespace detail {

template <typename RefCountType>
RefCountedBase<RefCountType>::RefCountedBase()
    : refs_(0)
{
}

template <typename RefCountType>
RefCountedBase<RefCountType>::~RefCountedBase() {}


template <typename RefCountType>
void RefCountedBase<RefCountType>::addRef()
{
    assert(refs_ >= 0);
    ++refs_;
}

template <typename RefCountType>
void RefCountedBase<RefCountType>::releaseRef()
{
    assert(refs_ > 0);
    if (!--refs_)
        seppuku();
}

template <typename RefCountType>
void RefCountedBase<RefCountType>::detach()
{
    ++refs_; // 2 refs
    releaseRef(); // 1 ref
    --refs_; // 0 refs
}

template <typename RefCountType>
RefCountType RefCountedBase<RefCountType>::refs() const
{
    return refs_;
}

}

template <typename Allocator, typename RefCountType>
Reference<typename RefCounted<Allocator, RefCountType>::Instancer> RefCounted<Allocator, RefCountType>::newInstancer()
{
    return new Instancer();
}

template <typename Allocator, typename RefCountType>
RefCounted<Allocator, RefCountType>::~RefCounted()
{
    instancer_->releaseRef();
}

template <typename Allocator, typename RefCountType>
void RefCounted<Allocator, RefCountType>::setInstancer(Instancer* instancer)
{
    instancer_ = instancer;
    instancer_->addRef();
}

template <typename Allocator, typename RefCountType>
void RefCounted<Allocator, RefCountType>::seppuku()
{
    instancer_->destroy(this);
}

template <typename RefCountType>
void RefCounted<alloc::DefaultAllocator, RefCountType>::seppuku()
{
    delete this;
}

}
