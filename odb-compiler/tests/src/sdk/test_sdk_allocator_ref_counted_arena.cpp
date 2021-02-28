#include "gmock/gmock.h"
#include "odb-sdk/allocators/Mallocator.hpp"
#include "odb-sdk/allocators/Instancer.hpp"
#include "odb-sdk/allocators/Page.hpp"
#include "odb-sdk/allocators/Linear.hpp"
#include "odb-sdk/Reference.hpp"

#define NAME sdk_allocator_ref_counted_arena

using namespace odb;
using namespace alloc;

TEST(NAME, bla)
{
    typedef Linear<Page, 128> FooAlloc;

    struct Foo : public RefCounted<FooAlloc>
    {
        int x, y, z;
    };

    auto i = Foo::newInstancer();

    Foo* foo1 = i->create<Foo>();
    foo1->addRef();

    Foo* foo2 = i->create<Foo>();
    foo2->addRef();

    Foo* foo3 = i->create<Foo>();
    foo3->addRef();

    Foo* foo4 = i->create<Foo>();
    foo4->addRef();

    foo1->releaseRef();
    foo2->releaseRef();
    foo3->releaseRef();
    foo4->releaseRef();
    i.reset();
}
