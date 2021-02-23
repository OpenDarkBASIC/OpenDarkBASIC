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
        ptr_(0)
    {
    }

    /// Copy-construct from another shared pointer.
    Reference(const Reference<T>& rhs) :
        ptr_(rhs.ptr_)
    {
        addRef();
    }

    /// Copy-construct from another shared pointer allowing implicit upcasting.
    template <class U>
    Reference(const Reference<U>& rhs) :
        ptr_(rhs.ptr_)
    {
        addRef();
    }

    /// Construct from a raw pointer.
    Reference(T* ptr) :
        ptr_(ptr)
    {
        addRef();
    }

    /// Destruct. Release the object reference.
    ~Reference()
    {
        releaseRef();
    }

    /// Assign from another shared pointer.
    Reference<T>& operator =(const Reference<T>& rhs)
    {
        if (ptr_ == rhs.ptr_)
            return *this;

        releaseRef();
        ptr_ = rhs.ptr_;
        addRef();

        return *this;
    }

    /// Assign from another shared pointer allowing implicit upcasting.
    template <class U>
    Reference<T>& operator =(const Reference<U>& rhs)
    {
        if (ptr_ == rhs.ptr_)
            return *this;

        releaseRef();
        ptr_ = rhs.ptr_;
        addRef();

        return *this;
    }

    /// Assign from a raw pointer.
    Reference<T>& operator =(T* ptr)
    {
        if (ptr_ == ptr)
            return *this;

        releaseRef();
        ptr_ = ptr;
        addRef();

        return *this;
    }

    /// Point to the object.
    T* operator ->() const
    {
        assert(ptr_);
        return ptr_;
    }

    /// Dereference the object.
    T& operator *() const
    {
        assert(ptr_);
        return *ptr_;
    }

    /// Subscript the object if applicable.
    T& operator [](const int index)
    {
        assert(ptr_);
        return ptr_[index];
    }

    /// Test for less than with another shared pointer.
    template <class U>
    bool operator <(const Reference<U>& rhs) const { return ptr_ < rhs.ptr_; }

    /// Test for equality with another shared pointer.
    template <class U>
    bool operator ==(const Reference<U>& rhs) const { return ptr_ == rhs.ptr_; }

    /// Test for inequality with another shared pointer.
    template <class U>
    bool operator !=(const Reference<U>& rhs) const { return ptr_ != rhs.ptr_; }

    /// Convert to a raw pointer.
    operator T*() const { return ptr_; }

    /// Reset to null and release the object reference.
    void reset() { releaseRef(); }

    /// Detach without destroying the object even if the refcount goes zero. To be used for scripting language interoperation.
    void detach()
    {
        if (ptr_)
        {
            RefCount* refCount = refCountPtr();
            ++refCount->refs_; // 2 refs
            reset(); // 1 ref
            --refCount->refs_; // 0 refs
        }
    }

    /// Perform a static cast from a shared pointer of another type.
    template <class U>
    void staticCast(const Reference<U>& rhs)
    {
        releaseRef();
        ptr_ = static_cast<T*>(rhs.Get());
        addRef();
    }

    /// Perform a dynamic cast from a shared pointer of another type.
    template <class U>
    void dynamicCast(const Reference<U>& rhs)
    {
        releaseRef();
        ptr_ = dynamic_cast<T*>(rhs.Get());
        addRef();
    }

    /// Check if the pointer is null.
    bool isNull() const { return ptr_ == 0; }

    /// Check if the pointer is not null.
    bool notNull() const { return ptr_ != 0; }

    /// Return the raw pointer.
    T* get() const { return ptr_; }

    /// Return the object's reference count, or 0 if the pointer is null.
    int refs() const { return ptr_ ? refCountedPtr()->refs() : 0; }

    /// Return the object's weak reference count, or 0 if the pointer is null.
    int weakRefs() const { return ptr_ ? refCountedPtr()->weakRefs() : 0; }

    /// Return pointer to the RefCount structure.
    RefCount* refCountPtr() const { return ptr_ ? refCountedPtr()->refCountPtr() : 0; }

    /// Return hash value for HashSet & HashMap.
    unsigned toHash() const { return (unsigned)((size_t)ptr_ / sizeof(T)); }

private:
    template <class U> friend class Reference;

    /// Add a reference to the object pointed to.
    void addRef()
    {
        if (ptr_)
            refCountedPtr()->addRef();
    }

    /// Release the object reference and delete it if necessary.
    void releaseRef()
    {
        if (ptr_)
        {
            refCountedPtr()->releaseRef();
            ptr_ = 0;
        }
    }

    /// Returns ptr_ casted to RefCounted*. Needed in case T is forward declared.
    RefCounted* refCountedPtr() const
    {
        // As T may be forward declared, the compiler will not be able to figure out whether this is a valid cast at
        // compile time, so we need to use reinterpret_cast instead.
        return reinterpret_cast<RefCounted*>(ptr_);
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

/// Weak pointer template class with intrusive reference counting. Does not keep the object pointed to alive.
template <class T>
class WeakReference
{
public:
    /// Construct a null weak pointer.
    WeakReference() :
        ptr_(0),
        refCount_(0)
    {
    }

    /// Copy-construct from another weak pointer.
    WeakReference(const WeakReference<T>& rhs) :
        ptr_(rhs.ptr_),
        refCount_(rhs.refCount_)
    {
        addRef();
    }

    /// Copy-construct from another weak pointer allowing implicit upcasting.
    template <class U>
    WeakReference(const WeakReference<U>& rhs) :
        ptr_(rhs.ptr_),
        refCount_(rhs.refCount_)
    {
        addRef();
    }

    /// Construct from a shared pointer.
    WeakReference(const Reference<T>& rhs) :
        ptr_(rhs.get()),
        refCount_(rhs.refCountPtr())
    {
        addRef();
    }

    /// Construct from a raw pointer.
    explicit WeakReference(T* ptr) :
        ptr_(ptr),
        refCount_(ptr ? ptr->refCountPtr() : 0)
    {
        addRef();
    }

    /// Destruct. Release the weak reference to the object.
    ~WeakReference()
    {
        releaseRef();
    }

    /// Assign from a shared pointer.
    WeakReference<T>& operator =(const Reference<T>& rhs)
    {
        if (ptr_ == rhs.Get() && refCount_ == rhs.refCountPtr())
            return *this;

        releaseRef();
        ptr_ = rhs.Get();
        refCount_ = rhs.refCountPtr();
        addRef();

        return *this;
    }

    /// Assign from a weak pointer.
    WeakReference<T>& operator =(const WeakReference<T>& rhs)
    {
        if (ptr_ == rhs.ptr_ && refCount_ == rhs.refCount_)
            return *this;

        releaseRef();
        ptr_ = rhs.ptr_;
        refCount_ = rhs.refCount_;
        addRef();

        return *this;
    }

    /// Assign from another weak pointer allowing implicit upcasting.
    template <class U>
    WeakReference<T>& operator =(const WeakReference<U>& rhs)
    {
        if (ptr_ == rhs.ptr_ && refCount_ == rhs.refCount_)
            return *this;

        releaseRef();
        ptr_ = rhs.ptr_;
        refCount_ = rhs.refCount_;
        addRef();

        return *this;
    }

    /// Assign from a raw pointer.
    WeakReference<T>& operator =(T* ptr)
    {
        RefCount* refCount = ptr ? ptr->refCountPtr() : 0;

        if (ptr_ == ptr && refCount_ == refCount)
            return *this;

        releaseRef();
        ptr_ = ptr;
        refCount_ = refCount;
        addRef();

        return *this;
    }

    /// Convert to a shared pointer. If expired, return a null shared pointer.
    Reference<T> lock() const
    {
        if (expired())
            return Reference<T>();
        else
            return Reference<T>(ptr_);
    }

    /// Return raw pointer. If expired, return null.
    T* get() const
    {
        if (expired())
            return 0;
        else
            return ptr_;
    }

    /// Point to the object.
    T* operator ->() const
    {
        T* rawPtr = get();
        assert(rawPtr);
        return rawPtr;
    }

    /// Dereference the object.
    T& operator *() const
    {
        T* rawPtr = get();
        assert(rawPtr);
        return *rawPtr;
    }

    /// Subscript the object if applicable.
    T& operator [](const int index)
    {
        T* rawPtr = get();
        assert(rawPtr);
        return (*rawPtr)[index];
    }

    /// Test for equality with another weak pointer.
    template <class U>
    bool operator ==(const WeakReference<U>& rhs) const { return ptr_ == rhs.ptr_ && refCount_ == rhs.refCount_; }

    /// Test for inequality with another weak pointer.
    template <class U>
    bool operator !=(const WeakReference<U>& rhs) const { return ptr_ != rhs.ptr_ || refCount_ != rhs.refCount_; }

    /// Test for less than with another weak pointer.
    template <class U>
    bool operator <(const WeakReference<U>& rhs) const { return ptr_ < rhs.ptr_; }

    /// Convert to a raw pointer, null if the object is expired.
    operator const T*() const { return get(); }

    /// Reset to null and release the weak reference.
    void reset() { releaseRef(); }

    /// Perform a static cast from a weak pointer of another type.
    template <class U>
    void staticCast(const WeakReference<U>& rhs)
    {
        releaseRef();
        ptr_ = static_cast<T*>(rhs.Get());
        refCount_ = rhs.refCount_;
        addRef();
    }

    /// Perform a dynamic cast from a weak pointer of another type.
    template <class U>
    void dynamicCast(const WeakReference<U>& rhs)
    {
        releaseRef();
        ptr_ = dynamic_cast<T*>(rhs.Get());

        if (ptr_)
        {
            refCount_ = rhs.refCount_;
            addRef();
        }
        else
            refCount_ = 0;
    }

    /// Check if the pointer is null.
    bool null() const { return refCount_ == 0; }

    /// Check if the pointer is not null.
    bool notNull() const { return refCount_ != 0; }

    /// Return the object's reference count, or 0 if null pointer or if object has expired.
    int refs() const { return (refCount_ && refCount_->refs_ >= 0) ? refCount_->refs_ : 0; }

    /// Return the object's weak reference count.
    int weakRefs() const
    {
        if (!expired())
            return refCountedPtr()->weakRefs();
        else
            return refCount_ ? refCount_->weakRefs_ : 0;
    }

    /// Return whether the object has expired. If null pointer, always return true.
    bool expired() const { return refCount_ ? refCount_->refs_ < 0 : true; }

    /// Return pointer to the RefCount structure.
    RefCount* refCountPtr() const { return refCount_; }

    /// Return hash value for HashSet & HashMap.
    unsigned toHash() const { return (unsigned)((size_t)ptr_ / sizeof(T)); }

private:
    template <class U> friend class WeakReference;

    /// Add a weak reference to the object pointed to.
    void addRef()
    {
        if (refCount_)
        {
            assert(refCount_->weakRefs_ >= 0);
            ++(refCount_->weakRefs_);
        }
    }

    /// Release the weak reference. Delete the Refcount structure if necessary.
    void releaseRef()
    {
        if (refCount_)
        {
            assert(refCount_->weakRefs_ > 0);
            --(refCount_->weakRefs_);

            if (expired() && !refCount_->weakRefs_)
                delete refCount_;
        }

        ptr_ = 0;
        refCount_ = 0;
    }

    /// Returns ptr_ casted to RefCounted*. Needed in case T is forward declared.
    RefCounted* refCountedPtr() const
    {
        // As T may be forward declared, the compiler will not be able to figure out whether this is a valid cast at
        // compile time, so we need to use reinterpret_cast instead.
        return reinterpret_cast<RefCounted*>(ptr_);
    }

    /// Pointer to the object.
    T* ptr_;
    /// Pointer to the RefCount structure.
    RefCount* refCount_;
};

/// Perform a static cast from one weak pointer type to another.
template <class T, class U>
WeakReference<T> staticCast(const WeakReference<U>& ptr)
{
    WeakReference<T> ret;
    ret.staticCast(ptr);
    return ret;
}

/// Perform a dynamic cast from one weak pointer type to another.
template <class T, class U>
WeakReference<T> dynamicCast(const WeakReference<U>& ptr)
{
    WeakReference<T> ret;
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
