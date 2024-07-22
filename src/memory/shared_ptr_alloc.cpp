#include <iostream>
#include <typeinfo>
#include <memory>
#include <memory_resource>
#include <new>
#include <cstdio>
#include <cstdlib>

class TrackNew {
public:
    static void reset() {
        numMalloc = 0;
        sumSize = 0;
    }

    static size_t totalSize() { return sumSize; }
    static int totalNumMalloc() { return numMalloc; }

    static void* allocate(std::size_t size, std::size_t align, const char* call) {
        ++numMalloc;
        sumSize += size;
        void* p;
        if (align == 0) {
            p = std::malloc(size);
        }
        else {
            p = std::aligned_alloc(align, size);
        }
        return p;
    }

    static void status() {
        printf("%d allocations for %zu bytes\n", numMalloc, sumSize);
    }

private:
    static inline int numMalloc = 0;
    static inline size_t sumSize = 0;
};

[[nodiscard]]
void* operator new (std::size_t size) {
    return TrackNew::allocate(size, 0, "::new");
}
[[nodiscard]]
void* operator new (std::size_t size, std::align_val_t align) {
    return TrackNew::allocate(size, static_cast<std::size_t>(align), "::new aligned");
}
[[nodiscard]]
void* operator new[] (std::size_t size) {
    return TrackNew::allocate(size, 0, "::new[]");
}
[[nodiscard]]
void* operator new[] (std::size_t size, std::align_val_t align) {
    return TrackNew::allocate(size, static_cast<std::size_t>(align), "::new[] aligned");
}

void operator delete (void* p) noexcept {
    std::free(p);
}
void operator delete (void* p, std::size_t) noexcept {
    ::operator delete(p);
}
void operator delete (void* p, std::align_val_t) noexcept {
    std::free(p);       // C++17 API
}
void operator delete (void* p, std::size_t, std::align_val_t align) noexcept {
    ::operator delete(p, align);
}

struct A {
    A(int i) : val(i) {
        std::cout << "A constructed\n";
    }
    ~A() {
        std::cout << "A deconstructed\n";
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
};

int main() {
    std::cout << "sizeof std::string: " << sizeof(std::string) << std::endl;
    {
        std::cout << "----------" << std::endl;
        std::cout << "sizeof shared_ptr: " << sizeof(std::shared_ptr<B>) << std::endl;
        B* pi = new B();
        TrackNew::reset();
        std::shared_ptr<B> spa(pi);
        TrackNew::status();
        std::cout << "sizeof shared_ptr: " << sizeof(spa) << std::endl;
    }
    {
        std::cout << "----------" << std::endl;
        TrackNew::reset();
        A* pa = new A(1);
        std::shared_ptr<A> spa{pa, [](auto&& p) {delete (A*)p;}};
        TrackNew::status();
    }
    {
        std::cout << "----------" << std::endl;
        TrackNew::reset();
        A* pa = new A(1);
        TrackNew::status();
        delete pa;
    }
    {
        std::cout << "----------" << std::endl;
        TrackNew::reset();
        std::shared_ptr<B> pa = std::make_shared<B>();
        TrackNew::status();
    }
    {
        std::cout << "----------" << std::endl;
        TrackNew::reset();
        MockResource res;
        {
            std::pmr::polymorphic_allocator<A> alloc(&res);
            std::cout << "size of alloc: " << sizeof(alloc) << std::endl;
            std::shared_ptr<A> pa = std::allocate_shared<A>(alloc, 1);
        }
        std::cout << "allocated: " << std::boolalpha << res.allocate_called << std::endl;
        std::cout << "deallocated: " << std::boolalpha << res.deallocate_called << std::endl;
        std::cout << "total allocated size: " << res.total_allocated_size << std::endl;
        TrackNew::status();
    }
    return 0;
}