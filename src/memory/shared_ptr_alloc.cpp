/*
make_shared<T> 如果T的大小<8B，那么总共需要8+16=24B；如果T的大小>8B，那么总共需要sizeof<T>+16B
控制块里面可能是有两个atomic<unsigned long>，需要16字节。
如果使用alloc_shared，控制块的大小会更大一些，似乎控制块的大小不是固定的。
*/

#include <iostream>
#include <typeinfo>
#include <memory>
#include <memory_resource>
#include <new>
#include <istream>
#include <cstdlib>

#include "track_new_delete.h"

struct A {
    A(int i) : val(i) {
    }
    ~A() {
    }
    uint32_t val;
};

struct MockResource : public ::std::pmr::memory_resource {
    MockResource() : buffer(), res(buffer.data(), buffer.size()) {}

    virtual void* do_allocate(size_t bytes, size_t alignment) noexcept override {
        total_allocated_size += bytes;
        allocate_called = true;
        return res.allocate(bytes, alignment);
    }

    virtual void do_deallocate(void* ptr, size_t bytes,
                                size_t alignment) noexcept override {
        deallocate_called = true;
        res.deallocate(ptr, bytes, alignment);
    }

    virtual bool do_is_equal(
        const memory_resource& __other) const noexcept override {
        return this == &__other;
    }

    bool mem_inside(void* addr) {
        return (addr >= buffer.data()) && (addr < buffer.data() + buffer.size());
    }

    size_t total_allocated_size{0};
    bool allocate_called {false};
    bool deallocate_called {false};
    std::array<char, 64> buffer;
    std::pmr::monotonic_buffer_resource res;
};

struct B {
    uint64_t a;
    uint64_t b;
    uint64_t c;
};

int main() {
    std::cout << "sizeof A: " << sizeof(A) << std::endl;
    std::cout << "sizeof B: " << sizeof(B) << std::endl;
    std::cout << "sizeof shared_ptr<A>: " << sizeof(std::shared_ptr<A>) << std::endl; // 成员包括两个指针，所以是16B
    std::cout << "sizeof shared_ptr<B>: " << sizeof(std::shared_ptr<B>) << std::endl; // 同上
    {
        B* pi = new B();
        TrackNew::reset();
        std::shared_ptr<B> spa(pi, [](auto&& p) { delete (B*)p; });
        TrackNew::status("construct shared B with raw ptr");
    }
    {
        A* pa = new A(1);
        TrackNew::reset();
        std::shared_ptr<A> spa{pa};
        TrackNew::status("construct shared A with raw ptr");
    }
    {
        TrackNew::reset();
        A* pa = new A(1);
        TrackNew::status("new raw A ptr");
        delete pa;
    }
    {
        TrackNew::reset();
        std::shared_ptr<A> pa = std::make_shared<A>(1);
        TrackNew::status("make shared A");
    }
    {
        TrackNew::reset();
        B* pb = new B();
        TrackNew::status("new raw B ptr");
        delete pb;
    }
    {
        TrackNew::reset();
        std::shared_ptr<B> pa = std::make_shared<B>();
        // show_memory(reinterpret_cast<unsigned char*>(&pa), 40);
        TrackNew::status("make shared B");
    }
    {
        TrackNew::reset();
        std::pmr::polymorphic_allocator<A> alloc(std::pmr::new_delete_resource());
        std::shared_ptr<A> pa = std::allocate_shared<A>(alloc, 1);
        TrackNew::status("allocate_shared A");
    }
    {
        TrackNew::reset();
        std::pmr::polymorphic_allocator<B> alloc(std::pmr::new_delete_resource());
        std::shared_ptr<B> pa = std::allocate_shared<B>(alloc);
        TrackNew::status("allocate_shared B");
    }
    {
        TrackNew::reset();
        MockResource res;
        {
            std::pmr::polymorphic_allocator<B> alloc(&res);
            std::cout << "total allocated size: " << res.total_allocated_size << std::endl;
            std::cout << "size of alloc: " << sizeof(alloc) << std::endl;
            std::shared_ptr<B> pa = std::allocate_shared<B>(alloc);
            // show_memory(reinterpret_cast<unsigned char*>(&pa), res.total_allocated_size);
        }
        std::cout << "allocated: " << std::boolalpha << res.allocate_called << std::endl;
        std::cout << "deallocated: " << std::boolalpha << res.deallocate_called << std::endl;
        std::cout << "total allocated size: " << res.total_allocated_size << std::endl;
        TrackNew::status();
    }
    return 0;
}