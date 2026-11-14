// 第三节 线程传参详解，detach()大坑，成员函数做线程函数
//（1）传递临时对象作为线程参数
//（2）传递类对象、智能指针作为线程参数
//（3）用成员函数指针做线程参数

namespace a01
{
    //
#include <iostream>
#include <thread>

void print(const int &var, char* arr)
{
    std::cout << "第一个参数：" << var << std::endl;
    std::cout << "第二个参数：" << arr << std::endl;
}
int main()
{
    //传递临时对象作为线程参数
    int num = 1;
    int& m_var = num;
    char m_arr[] = "this is a test";
    std::thread mytobj(print, m_var, m_arr);

    mytobj.join();
    std::cout << "主线程结束" << std::endl;
    return 0;
    // 第一个参数：1
    // 第二个参数：this is a test
    // 主线程结束
}
}//namespace a01

namespace a02
{
    // 使用 detach() 要避免的陷阱 01: 不推荐子线程参数用引用，绝对不推荐用指针
    // 假设并非是 join 而是 detach:子线程传入的是 num 的引用，如果主线程先于子线程执行完，num 可能被系统回收
    // 那么在子线程中使用 num 是否安全？第二个参数安全吗？
#include <iostream>
#include <thread>

void print(const int &var, char* arr)
{
    std::cout << "第一个参数 var 的值：" << var << ",地址：" << &var << std::endl;
    // 第一个参数 var 的值：1,地址：0x6000017f5130
    // 由地址不同得出，var 并非是以引用方式传入，而是以值的方式传入)
    // 结论：虽然主线程detach 了子线程，但是子线程的参数 var 以引用方式传入 num，仍然是安全的（实际是传值）
    std::cout << "第二个参数 arr 的值：" << arr << ",地址：" << &arr << std::endl;
    // 第二个参数 arr 的值：this is a test,地址：0x7ff7b1b17348
    // 结论：由地址相同得出，是同一块内存，如果主线程detach 了子线程，访问 arr 的内存是未知的，所以不安全。
}
int main()
{
    int num = 1; // num 的地址：0x7ff7b48cf340
    int& m_var = num; // m_var 所指的地址：0x7ff7b48cf340
    char m_arr[] = "this is a test"; // m_arr 地址：0x7ff7b1b17348

    std::thread mytobj(print, m_var, m_arr);

    mytobj.detach();
    std::cout << "主线程结束" << std::endl;
    return 0;
}
}//namespace a02

namespace a03
{
    // 由上可知，想要将int等普通类型传入线程，采用直接传值。那么，如何将字符串安全的传入线程中？
#include <iostream>
#include <thread>
#include <string>

void print(const int var, const std::string& arr) // <-----char* arr 改成 string 类
{
    std::cout << "第二个参数 arr 的值：" << arr.c_str() << ",地址：" << &arr << std::endl;
    // 第二个参数 arr 的值：this is a test,地址：0x0147f74C
    // 达到了地址不一样的效果，看起来是ok的。
}
int main()
{
    //传递临时对象作为线程参数
    int num = 1;
    char m_arr[] = "this is a test"; // m_arr 地址是0x012ffe30
    std::thread mytobj(print, num, m_arr); 
    // 我们希望系统将 m_arr 数组隐式转换为 string 类（string 类表现为类值）

    mytobj.detach();
    std::cout << "主线程结束" << std::endl;
    return 0;
}
}//namespace a03

namespace a04
{
    // 使用 detach() 要避免的陷阱 02: 
    // 问题：m_arr 数组是在什么时候被转换为 string 的？下面的程序可以测试出来
    // 本段代码说明了这种情况：主线程都执行结束了（m_arr 被系统回收了），系统才开始将 m_arr数组向 string 转换，是个未定义
    // 解决方法为:在这一句手动转换为 string 类临时对象 -> std::thread mytobj(print, m_var, std::string(m_arr));
    // 一切的问题根源在于 detach，如果是 join 就没有这两个陷阱了。
    // *总结：使用 detach 时（1）若传递 int 这种简单类型参数，建议都是值传递，不该使用引用
    //                    （2）若传递类对象，应该避免隐式类型转换，应该在创建线程时手动构造临时对象，然后在函数参数中用引用来接
    //                     （否则系统还会再构造一次，浪费）
    // 建议尽量不使用detach，只使用join，这样就不存在局部变量失效导致线程对内存的非法引用
#include <iostream>
#include <thread>
class Temp
{
public:
    int m_i; 
    // 类型转换函数：把 int 类型转换成 Temp 类
    Temp(int a) : m_i(a) { std::cout << "构造" << std::endl; } 
    Temp(const Temp& temp) : m_i(temp.m_i) { std::cout << "拷贝构造" << std::endl; }
    ~Temp() { std::cout << "析构" << std::endl; }
};

void print(const int var, const Temp& obj) // <-----假设 char* arr 需要自动转换为 Temp 类
{
    std::cout << "子线程运行结束" << std::endl;
}

int main()
{
    int first = 1;
    int second = 12;
    std::thread mytobj(print, first, second); // 我们希望 int 类型的 second 转换为 Temp 类型，传递给 print

    mytobj.detach();
    std::cout << "主线程结束" << std::endl;
    return 0;
    // ---->如果是 mytobj.join()将是如下输出结果：
    // 构造
    // 子线程 arr 的地址是：0x7000037daf20
    // 析构
    // 主线程结束

    // ---->如果是 mytobj.detach()将是如下输出结果：说明int 类转换为 Temp 类发生在主线程结束之后
    // 主线程结束
}
    // --->如果将代码中这一句改为 std::thread mytobj(print, first, Temp(second));  <---构造 Temp 类的临时对象
    // 输出结果为：(说明在主线程结束之前，用于子线程的 arr 一定已经被构造出来了，从而确保 detach 后子线程正确运行)
    // 构造
    // 拷贝构造 <---子线程接收的类对象 obj 在程序员的设计中不管是按引用接收还是值形式接收，主线程都会以安全为由，强制temp以值的传递，因此调用拷贝构造
    // 析构
    // 主线程结束

} // namespace a04

namespace a05 
{
    // 测试大法：证明：隐式类型转换为啥不安全，而构造临时对象就是安全的？
    //（2.1）线程 id 概念：id 是个数字，每个线程（不管是主线程还是子线程）都对应着各自的数字。用C++标准库里的std::this_tread::get_id()获取。
    // 问题：Temp类临时对象是在哪个线程被构造的？由输出结果2可以看出：是在主线程中被构造的,因此，即使使用 detach 也是安全的
#include <iostream>
#include <thread>
class Temp
{
public:
    int m_i; 
    Temp(int a) : m_i(a)
    { 
        std::cout << "构造," << "线程号：" << std::this_thread::get_id() << std::endl; 
    }
    Temp(const Temp& temp) : m_i(temp.m_i) 
    {
        std::cout << "拷贝构造," << "线程号：" << std::this_thread::get_id() << std::endl; 
    }
    ~Temp() 
    { 
        std::cout << "析构," << "线程号：" << std::this_thread::get_id() << std::endl; 
    }
};

void print( const Temp& obj) 
{
    std::cout << "子线程的第2个参数的地址是：" << &obj << ",线程号：" << std::this_thread::get_id() << std::endl;
}

int main()
{
    std::cout << "主线程线程号：" << std::this_thread::get_id() << std::endl; 
    int second = 12;
    //std::thread mytobj(print, second); // 输出结果1
    std::thread mytobj(print, Temp(second)); // 输出结果1

    mytobj.join();
    std::cout << "主线程结束" << std::endl;
    return 0;
    //---->输出结果1：说明 arr 在子线程中被构造！！！！！
    // 主线程线程号：0x112a48600
    // 构造,线程号：0x70000758f000
    // 子线程的第2个参数的地址是：0x70000758ef28线程号：0x70000758f000
    // 析构,线程号：0x70000758f000
    // 主线程结束
    //---->输出结果2：说明 arr 在主线程中被构造！！！！！
    // 主线程线程号：0x10ebf0600
    // 构造,线程号：0x10ebf0600
    // 拷贝构造,线程号：0x10ebf0600
    // 析构,线程号：0x10ebf0600 
    // 子线程的第2个参数的地址是：0x60000271d130,线程号：0x700003e04000
    // 析构,线程号：0x700003e04000
    // 主线程结束
} 
}//namespace a05

namespace a06
{
    // 如果 print 的参数不使用引用,
    // std::thread mytobj(print, Temp(second));
    // void print( const Temp arr) {...}
    // 输出结果是：(注意打箭头的两行的id)
        // 主线程线程号：0x11a25e600
        // 构造,0x7ff7b5539338线程号：0x11a25e600
        // 拷贝构造,0x7ff7b5539288线程号：0x11a25e600
        // 析构,0x7ff7b5539288线程号：0x11a25e600
        // 拷贝构造,0x7000078f0f28线程号：0x7000078f1000 <-------
        // 子线程的第2个参数的地址是：0x7000078f0f28线程号：0x7000078f1000
        // 析构,0x7000078f0f28线程号：0x7000078f1000 <------
        // 析构,0x6000030c9130线程号：0x7000078f1000
        // 主线程结束
}
namespace a07
{
    // 例子：传递类对象作为创建线程的参数
    // 修改了子线程中的成员变量，不会反馈到主线程中
#include <iostream>
#include <thread>

class Temp
{
public:
    mutable int m_i; // <-------因为子线程要改变 var.m_i 却本身是 const，所以在此处加 mutable
    Temp(int a) : m_i(a){ std::cout << "构造," << "线程号：" << std::this_thread::get_id() << std::endl; }
    Temp(const Temp& temp) : m_i(temp.m_i) { std::cout << "拷贝构造," << "线程号：" << std::this_thread::get_id() << std::endl; }
    ~Temp() {std::cout << "析构," << "线程号：" << std::this_thread::get_id() << std::endl;}
};
void print(const Temp& obj)
{
    obj.m_i = 199; // <-------------
    std::cout << "obj的地址是：" << &obj << ",子线程号：" << std::this_thread::get_id() << std::endl;
}
int main()
{
    std::cout << "主线程线程号：" << std::this_thread::get_id() << std::endl; 
    int num = 1;
    Temp temp(99);
    std::thread mytobj(print, temp); // <---类对象作为线程参数

    mytobj.join();
    std::cout << "主线程结束,temp = " << temp.m_i << " ,地址是" << &temp << std::endl;
    return 0;  
    // 主线程线程号：0x11300a600
    // 构造,线程号：0x11300a600 <---类对象temp
    // 拷贝构造,线程号：0x11300a600 
    // 拷贝构造,线程号：0x11300a600 
    // 析构,线程号：0x11300a600
    // obj的地址是：0x600003abd130,子线程号：0x70000f15b000
    // 析构,线程号：0x70000f15b000
    // 主线程结束,temp = 99 ,地址是0x7ff7bdb65348 <----说明子线程中的改变，无法影响到主线程，因为 var 与 temp 的内存地址根本不一样
    // 析构,线程号：0x11300a600
}
}//namespace a07

namespace a08
{
    // void print(const Temp& obj) <---这里面const是因为引用才必须有的
    // 但是有时候需要改变 obj 的值，但是也不能每一个类成员都用 mutable；而且对于引用变量的修改，如果没有多线程，应该是会有一致性的。
    // 子线程接收的类对象 obj 在程序员的设计中无论是按引用接收还是值形式接收，主线程都会以安全为由，强制temp以值传递，因此调用拷贝构造
    // 那么，如何真正地传引用呢？用std::ref 函数
#include <iostream>
#include <thread>

class Temp
{
public:
    int m_i; 
    Temp(int a) : m_i(a){ std::cout << "构造," << "线程号：" << std::this_thread::get_id() << std::endl; }
    Temp(const Temp& temp) : m_i(temp.m_i) { std::cout << "拷贝构造," << "线程号：" << std::this_thread::get_id() << std::endl; }
    ~Temp() {std::cout << "析构," << "线程号：" << std::this_thread::get_id() << std::endl;}
};

void print(Temp& obj) // <----因此去掉了const
{
    obj.m_i = 199; 
    std::cout << "obj的地址是：" << &obj << ",子线程号：" << std::this_thread::get_id() << std::endl;
}
int main()
{
    std::cout << "主线程线程号：" << std::this_thread::get_id() << std::endl; 
    int num = 1;
    Temp temp(99);
    std::thread mytobj(print, std::ref(temp)); // <-------

    mytobj.join();
    std::cout << "主线程结束,temp = " << temp.m_i << " ,地址是" << &temp << std::endl;
    return 0;  
    // 主线程线程号：0x116241600
    // 构造,线程号：0x116241600 <-------------------------没调用拷贝构造
    // obj的地址是：0x7ff7b22e6348,子线程号：0x70000b3f7000
    // 主线程结束,temp = 199 ,地址是0x7ff7b22e6348 <------obj 与 temp 地址相同，值都为199！！！
    // 析构,线程号：0x116241600    
}
}//namespace a08

namespace a09
{
    // 智能指针作为参数
    // 如果detach，智能指针有可能指向被系统回收的内存
#include <iostream>
#include <thread>
void print(std::unique_ptr<int> child_ptr) 
{
    std::cout << "child_ptr = " << child_ptr << std::endl;
}
int main()
{
    std::unique_ptr<int> main_ptr(new int(100));
    std::cout << "main_ptr = " << main_ptr << std::endl;
    std::thread mytobj(print, std::move(main_ptr)); 

    mytobj.join();
    std::cout << "move 后 main_ptr = " << main_ptr << std::endl;
    std::cout << "主线程结束" << std::endl;
    return 0;  
    // main_ptr = 0x60000393c030
    // child_ptr = 0x60000393c030
    // move 后 main_ptr = 0x0
    // 主线程结束
}
}//namespace a09

namespace a10
{
    // 用 任意一个成员函数指针 作为线程函数
#include <iostream>
#include <thread>

class Temp
{
public:
    void thread_work(int num){ 
        std::cout << "子线程的入口函数thread_work" << std::endl; 
    }
    void operator()(int num){
        std::cout << "子线程的入口函数operator()" << std::endl;
    }
};

int main()
{
    Temp temp;
    // 若要将某个类的成员函数设定为线程函数，应传入指向该成员函数的函数指针，还要给出合适的对象指针（由 temp 的地址充当）
    std::thread mytobj(&Temp::thread_work, &temp, 20); // 对应输出1
    // std::thread mytobj(temp, 20); // 对应输出2

    mytobj.join();
    std::cout << "主线程结束" << std::endl;
    return 0;
    //--->输出1
    // 子线程的入口函数thread_work
    // 主线程结束
    //--->输出2
    // 子线程的入口函数operator()
    // 主线程结束
}
}//namespace a10
