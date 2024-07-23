/*
在C++中，对象在内存中占据一定的存储位置，并且有其特定的生命周期。在对象的生命周期内，编译器可以对该对象的访问进行各种优化，例如缓存其成员变量值、重排序指令等。如果在同一个内存位置重新构造对象，这些优化可能会导致不一致的行为，因为编译器仍然认为原对象是有效的，并继续使用之前的优化结果。
*/

#include <iostream>
#include <memory> // std::aligned_storage

struct MyStruct {
    int a;
    int b;
    MyStruct(int x, int y) : a(x), b(y) {}
};

int main() {
    std::aligned_storage_t<sizeof(MyStruct), alignof(MyStruct)> buffer;

    // 构造对象
    MyStruct* ptr = new (&buffer) MyStruct(1, 2);
    std::cout << "Before: " << ptr->a << ", " << ptr->b << std::endl;

    // 在相同的内存位置重新构造对象
    ptr->~MyStruct();
    new (&buffer) MyStruct(3, 4);

    // 不使用std::launder
    ptr = reinterpret_cast<MyStruct*>(&buffer);
    // 使用std::launder，确保编译期不会保留针对之前的对象的优化
    ptr = std::launder(reinterpret_cast<MyStruct*>(&buffer));

    // 输出重新构造后的对象
    std::cout << "After: " << ptr->a << ", " << ptr->b << std::endl;

    // 手动调用析构函数
    ptr->~MyStruct();

    return 0;
}
