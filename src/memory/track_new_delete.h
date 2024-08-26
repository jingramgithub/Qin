#pragma once

#include <cstddef>
#include <cstdint> 
#include <iostream>
#include <iomanip>

void show_memory(unsigned char* buffer, std::size_t buffer_size, const char* headline = "") {
    if (headline != "")
        std::cout << headline << std::endl;
    std::cout << "&buffer=0x" << std::hex << reinterpret_cast<uintptr_t>(buffer)
              << " " << std::dec << buffer_size << " bytes" << std::endl;
    int i = 0;
    while (i < buffer_size) {
        int first = i;
        int last = i + std::min(10, int(buffer_size - first));
        std::cout << "&=" << std::setw(2) << std::hex << std::size_t(first);
        std::cout << " : ";
        for (int k = first; k < last; k++) {
            if ((buffer[k] >= 32) and (buffer[k] <= 127))
                std::cout << static_cast<int>(buffer[k]) << "\t";
            else
                std::cout << static_cast<int>(buffer[k]) << "\t";
        }
        i = i + 10;
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

class TrackNew {
public:
    static void reset() {
        numMalloc = 0;
        sumSize = 0;
        numDealloc = 0;
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

    static void dealloc(void* p) {
        ++numDealloc;
        std::free(p);
    }

    static void status(const char* desc = "") {
        std::cout << desc << " ";
        std::cout << std::dec << "#new: " << numMalloc << " #delete: " << numDealloc << " #bytes: " << sumSize << std::endl;
    }

private:
    static inline int numMalloc = 0;
    static inline int numDealloc = 0;
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
    TrackNew::dealloc(p);
}
void operator delete (void* p, std::size_t) noexcept {
    TrackNew::dealloc(p);
}
void operator delete (void* p, std::align_val_t) noexcept {
    TrackNew::dealloc(p);
}
void operator delete (void* p, std::size_t, std::align_val_t align) noexcept {
    TrackNew::dealloc(p);
}