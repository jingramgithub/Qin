#pragma once

#include <cstddef>
#include <cstdint> 
#include <iostream>
#include <iomanip>

int new_counter = 0;
int delete_counter = 0;
std::size_t allocated_mem = 0;

void reset_counter() {
    new_counter = 0;
    delete_counter = 0;
    allocated_mem = 0;
}
void new_delete_summary() {
    std::cout << std::dec << "#new: " << new_counter << " #delete: " << delete_counter << " #bytes: " << allocated_mem << std::endl;
    reset_counter();
}

void *operator new(std::size_t sz) {
    void* ptr = std::malloc(sz);
    if (ptr) {
        new_counter++;
        allocated_mem += sz;
        return ptr;
    } else
        throw std::bad_alloc{};
}
void operator delete(void *ptr) noexcept {
    delete_counter++;
    std::free(ptr);
}

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
        std::cout << " asc: ";
        for (int k = first; k < last; k++) {
            if ((buffer[k] >= 32) and (buffer[k] <= 127))
                std::cout << static_cast<char>(buffer[k]) << "\t";
            else
                std::cout << static_cast<int>(buffer[k]) << "\t";
        }
        i = i + 10;
        std::cout << std::endl;
    }
    std::cout << std::endl;
}