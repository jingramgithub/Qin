/*
std::atomic<int32_t>和int32_t的内存是完全一样的，CAS操作是在函数里面实现的，和内存无关。所以在共享内存里面可以使用这种atomic变量。
*/

#include <atomic>
#include "track_new_delete.h"

int main() {
    std::atomic<int32_t> ai = 98;
    int32_t i = 98;
    show_memory(reinterpret_cast<unsigned char*>(&ai), sizeof(ai));
    show_memory(reinterpret_cast<unsigned char*>(&i), sizeof(i));
    return 0;
}