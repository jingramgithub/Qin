/*
区分左值和右值的一个简单办法是：看能不能对表达式取地址，如果能，则为左值，否则为右值。
通过右值引用的声明，右值又“重获新生”，其生命周期与右值引用类型变量的生命周期一样长，只要该变量还活着，该右值临时量将会一直存活下去。
常量左值引用是一个“万能”的引用类型，可以接受左值、右值、常量左值和常量右值。
关于右值引用一个有意思的问题是：T&&是什么，一定是右值吗？让我们来看看下面的例子：
    template<typename T>
    void f(T&& t){}
    f(10); //t是右值
    int x = 10;
    f(x); //t是左值
*/

#include <iostream>
#include <typeinfo>
#include <memory>
#include <memory_resource>
#include <new>
#include <cstdio>
#include <cstdlib>

struct A {
    A() {
        std::cout << "in A()\n";
    };
    void do_sth() {
        std::cout << "in A::do()\n";
    };
    ~A() {
        std::cout << "in ~A()\n";
    };
};

void f1(A&& a) {
    std::cout << "in f1\n";
}

void f2(std::unique_ptr<A> a) {
    std::cout << "in f2\n";
    a->do_sth();
}

int main() {
    {
        A a;
        std::cout << "before f\n";
        f1(std::move(a));
        std::cout << "after f\n";
        // a在这里析构
    }
    {
        std::unique_ptr<A> a = std::make_unique<A>();
        std::cout << "before f\n";
        f2(std::move(a)); // a在f2里面析构了
        std::cout << "after f\n";
    }
    return 0;
}