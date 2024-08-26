/*
条件变量不能用std::chrono::milliseconds(std::numeric_limits<uint64_t>::max())作为超时参数，用这个参数会导致没有超时。要阻塞式等待，使用wait。
*/
#include <iostream>
#include <typeinfo>
#include <memory>
#include <memory_resource>
#include <new>
#include <cstdio>
#include <cstdlib>
#include <condition_variable>
#include <chrono>
#include <numeric>

uint64_t get_timestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

int main() {
    bool var = false;
    std::condition_variable cv;
    std::mutex mtx;
    {
        auto tic = get_timestamp();
        std::unique_lock<std::mutex> lck(mtx);
        cv.wait_for(lck, std::chrono::milliseconds(std::numeric_limits<uint64_t>::max()), [&var]() {
            return var;
        });
        auto toc = get_timestamp();
        std::cout << "real timeout: " << toc - tic << std::endl;
    }
    {
        auto tic = get_timestamp();
        std::unique_lock<std::mutex> lck(mtx);
        cv.wait_for(lck, std::chrono::milliseconds(1000), [&var]() {
            return var;
        });
        auto toc = get_timestamp();
        std::cout << "real timeout: " << toc - tic << std::endl;
    }
    return 0;
}