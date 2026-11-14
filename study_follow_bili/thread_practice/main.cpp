//
//  main.cpp
//  thread_practice
//
//  Created by 孙晓宁 on 2021/12/17.
//

//第一章
//打印信息的代码被移动到了一个独立的函数中②。
//因为每个线程都必须具有一个初始函数(initial function)，新线程的执行从这里开始。
//对于应用程序来说，初始线程是main()，
//但是对于其他线程，可以在std::thread对象的构造函数中指定.
//本例中，被命名为t③的std::thread对象拥有新函数hello()作为其初始函数。

//#include <iostream>
//#include <thread> //①
//
//void hello()  //②
//{
//    std::cout << "Hello Concurrent World\n";
//}
//int main()
//{
//    std::thread t(hello);  //③ 创建thread类的对象t，用hello函数初始化线程t
//    t.join();  //④ 在④这里调用join()的原因：等待与std::thread对象相关联的线程
//}
//第2章 线程管理
//本章将从基本开始：启动一个线程，等待这个线程结束，或放在后台运行。
//再看看怎么给已经启动的线程函数传递参数，以及怎么将一个线程的所有权从当前std::thread对象移交给另一个。
//最后，再来确定线程数，以及识别特殊线程。
//
//2.1.1 启动线程
//3种启动方式->核心：构造std::thread对象
//1、
//void func();
//std::thread my_thread(func);
//2、用“可调用类型”构造.将带有函数调用符类型的实例f传入std::thread类中，替换默认的构造函数。
//class background_task
//{
//public:
//    void operator()()const    //operator()说明是仿函数，下面的（）代表函数无参数
//    {
//        do_something();
//        do_something_else();
//    }
//};
//background_task func;
//std::thread my_thread(func);
//注意：如果你传递了一个临时变量，而不是一个命名的变量；C++编译器会将其解析为函数声明，而不是类型对象的定义。
//std::thread my_thread(func());//这里相当与声明了一个名为my_thread的函数,而非启动了一个线程.
//使用在前面命名函数对象的方式，或使用多组括号①，或使用新统一的初始化语法②，可以避免这个问题。

//3、用lambda表达式

//new thread启动之后，你需要决定的是main thread是否要等待new thread结束，等待(my_thread.join())，不等待(my_thread.detach())。
//如果std::thread对象销毁之前还没有做出决定，程序就会终止(std::thread的析构函数会调std::terminate())。因此，即便是有异常存在，也需要确保线程能够正确的加入(joined)或分离(detached)。2.1.3节中，会介绍对应的方法来处理这两种情况。需要注意的是，必须在std::thread对象销毁之前做出决定，否则你的程序将会终止(std::thread的析构函数会调用std::terminate()，这时再去决定会触发相应异常)。
//使用join()，主线程不仅会等待new thread结束，还会在new thread结束后清理了它相关的存储部分。这样对象my_thread将不再与已经完成的线程new thread有任何关联。这意味着，只能对一个线程使用一次join();当对my_thread.joinable()时，将返回false。
//使用std::thread成员函数detach()，会让new thread在后台运行，主线程不能与new thread产生直接交互。也就是说，主线程不会等待new thread结束；如果线程分离，那任何std::thread对象都不能引用它，分离线程的确在后台运行，所以分离线程不能被加入。不过C++运行库保证，当线程退出时，相关资源的能够正确回收，后台线程的归属和控制C++运行库都会处理。
//#include <iostream>
//#include <thread>
//#include <string>
//
//void func(int i, std::string const &s);
//
//void oops(int some_param){
//    char buffer[1024];
//    sprintf(buffer, "%i", some_param);
//}
//int main(){
//    std::thread t(func, 3, "hello");
//    
//}
#include <iostream>
#include <thread>
#include <vector>
std::vector<int> arr;
int main() {
    std::thread t1([] {
        for (int i = 0; i < 10000; ++i) {
            arr.push_back(1);
        }
    });
    std::thread t2([] {
        for (int i = 0; i < 10000; ++i) {
            arr.push_back(2);
        }
    });
    t1.join();
    t2.join();
    std::cout << "main thread is end" << std::endl;
    return 0;
}
