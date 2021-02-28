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

#include "odb-sdk/RefCounted.hpp"

#include <cassert>
#include <cstddef>

namespace odb {

/// Shared pointer template class with intrusive reference counting.
template <class T>
class Reference
{
public:

    /// Required by googletest's GetRawPointer() when compiling with c++03
    typedef T element_type;

    /// Construct a null shared pointer.
    Reference() :
        ptr_(nullptr)
    {
    }

    /// Copy-construct from another shared pointer.
    Reference(const Reference<T>& rhs)
        : ptr_(rhs.ptr_)
    {
        addRef();
    }

    /// Copy-construct from another shared pointer allowing implicit upcasting.
    template <class U>
    Reference(const Reference<U>& rhs)
        : ptr_(rhs.ptr_)
    {
        addRef();
    }

    /// Construct from a raw pointer.
    Reference(T* ptr)
        : ptr_(ptr)
    {
        addRef();
    }

    /// Destruct. Release the object reference.
    ~Reference()
    {
        releaseRef();
    }

    /// Assign from another shared pointer.
    Reference<T>& operator=(const Reference<T>& rhs)
    {
        T* tmp = ptr_;
        if (rhs.ptr_) rhs.ptr_->addRef();
        ptr_ = rhs.ptr_;
        if (tmp) tmp->releaseRef();

        return *this;
    }

    /// Assign from another shared pointer allowing implicit upcasting.
    template <class U>
    Reference<T>& operator=(const Reference<U>& rhs)
    {
        T* tmp = ptr_;
        if (rhs.ptr_) rhs.ptr_->addRef();
        ptr_ = rhs.ptr_;
        if (tmp) tmp->releaseRef();

        return *this;
    }

    /// Assign from a raw pointer.
    Reference<T>& operator=(T* ptr)
    {
        T* tmp = ptr_;
        if (ptr) ptr->addRef();
        ptr_ = ptr;
        if (tmp) tmp->releaseRef();

        return *this;
    }

    /// Point to the object.
    T* operator->() const
    {
        assert(ptr_);
        return ptr_;
    }

    /// Dereference the object.
    T& operator*() const
    {
        assert(ptr_);
        return *ptr_;
    }

    /// Subscript the object if applicable.
    T& operator[](const int index)
    {
        assert(ptr_);
        return ptr_[index];
    }

    /// Test for less than with another shared pointer.
    template <class U>
    bool operator<(const Reference<U>& rhs) const { return ptr_ < rhs.ptr_; }

    /// Test for equality with another shared pointer.
    template <class U>
    bool operator==(const Reference<U>& rhs) const { return ptr_ == rhs.ptr_; }

    /// Test for inequality with another shared pointer.
    template <class U>
    bool operator!=(const Reference<U>& rhs) const { return ptr_ != rhs.ptr_; }

    /// Convert to a raw pointer.
    operator T*() const { return ptr_; }

    /// Reset to null and release the object reference.
    void reset() { releaseRef(); }

    /// Detach without destroying the object even if the refcount goes zero. To be used for scripting language interoperation.
    void detach()
    {
        if (ptr_)
            ptr_->detach();
        ptr_ = nullptr;
    }

    /// Check if the pointer is null.
    bool isNull() const { return ptr_ == 0; }

    /// Check if the pointer is not null.
    bool notNull() const { return ptr_ != 0; }

    /// Return the raw pointer.
    T* get() const { return ptr_; }

    /// Return the object's reference count, or 0 if the pointer is null.
    int refs() const { return ptr_ ? ptr_->refs() : nullptr; }

    /// Return the object's weak reference count, or 0 if the pointer is null.
    int weakRefs() const { return ptr_ ? ptr_->weakRefs() : nullptr; }

    /// Return hash value for HashSet & HashMap.
    unsigned toHash() const { return (unsigned)((size_t)ptr_ / sizeof(T)); }

private:
    template <class U> friend class Reference;

    /// Add a reference to the object pointed to.
    void addRef()
    {
        if (ptr_)
            ptr_->addRef();
    }

    /// Release the object reference and delete it if necessary.
    void releaseRef()
    {
        if (ptr_)
        {
            ptr_->releaseRef();
            ptr_ = nullptr;
        }
    }

    /// Pointer to the object.
    T* ptr_;
};

/// Perform a static cast from one shared pointer type to another.
template <class T, class U>
Reference<T> staticCast(const Reference<U>& ptr)
{
    Reference<T> ret;
    ret.staticCast(ptr);
    return ret;
}

/// Perform a dynamic cast from one weak pointer type to another.
template <class T, class U>
Reference<T> dynamicCast(const Reference<U>& ptr)
{
    Reference<T> ret;
    ret.dynamicCast(ptr);
    return ret;
}

/// An easy way to make sure objects that were just allocated (and have 0 refs) get deleted
template <class T>
void TouchRef(T* obj)
{
    Reference<T> ref(obj);
}

}
