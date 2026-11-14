
 // bili_1.cpp
 // thread_practice

 // Created by 孙晓宁 on 2022/4/28.

// 第二节 线程的启动、结束，函数join()、detach()、joinable()；创建线程的方法（1）用类（2）用lambda表达式
//namespace a01{
//    // 程序代码运行起来，就生成了一个进程，该进程所属的主线程开始自动运行。当主线程从 main() 函数返回（也就是main()函数结束），则整个进程执行完毕。
//
//    // 主线程从 main() 函数开始执行（也叫起始函数），那么自己创建的线程也需要从一个函数（初始函数）开始执行。
//    // main() 函数执行完毕 意味着 主线程结束，那么自己创建的线程结束的标志是 本线程的初始函数执行完。
//    // 整个进程是否执行完毕？看主线程是否执行完，如果主线程执行完毕了，就代表整个进程执行完毕了，此时，一般情况下，如果子线程还没有执行完，也会被操作系统强行终止。
//    // 所以，一般情况下，如果大家想保持子线程（自己创建的线程）处于运行状态，那么大家要让主线程一直处于运行，不要让主线程结束。【此条有例外，以后会解释】
//    // 创建一个线程，（1）包含头文件thread（2）自己创建的线程需要起始函数（3）在main函数中开始写代码
//    // std::thread 类，接收的参数是可调用对象。
//    // 必须明确一点，进程中有两个线程在跑，相当于有两条线在同时走，即使一条线堵住了，另外一条线还是可以通行的
//#include <stdio.h>
//#include <thread>
//#include <iostream>
//void myprint() { //子线程的初始函数
//   std::cout << "my thread is start" << std::endl;
//   //...
//   std::cout << "my thread is end" << std::endl;
//}
//int main()
//{
//   std::thread mytobj(myprint); // 本句做了两件事：一是创建了线程，线程执行起点 myprint()，二是myprint线程开始执行
//   mytobj.join();               // 加入、汇合，=阻塞主线程，让主线程等待子线程 myprint()执行完毕，当子线程执行完毕后，主线程再往下走
//
//   std::cout << "main thread is end" << std::endl;
//
//   return 0;
//   // my thread is start
//   // my thread is end
//   // main thread is end
//}
//};
//
//namespace a02{
//    // .detach()：主线程不等待子线程结束，而是可以先执行结束，并不影响子线程的运行
//    // 一旦 detach() 之后，mytobj 就会失去与主线程的关联，此时这个子线程就会驻留在后台运行（无法交互，也不具有 mytobj 这个名字），相当于被c++运行时库接管，
//    // 当子线程执行完毕后，由运行时库负责清理该线程相关的资源（守护线程）。
//    // .detach()使子线程失去我们的控制，不能再用join()
//#include <iostream>
//#include <thread>
//
//void myprint() {
//    std::cout << "子线程开始执行" << std::endl;
//    //...
//    for(int i = 0; i < 20; ++i) {
//        std::cout << "子线程执行完毕 " << i << std::endl;
//    }
//}
//int main()
//{
//    std::thread mytobj(myprint);
//    mytobj.detach();
//    
//    for(int i = 0; i < 20; ++i) {
//        std::cout << "main thread is end " << i << std::endl;
//    }
//    
//    return 0;
//    // main thread is end 0
//    // main thread is end 1
//    // main thread is end 2
//    // main thread is end 3
//    // main thread is end 4
//    // main thread is end 5
//    // main thread is end 6
//    // main thread is end 7
//    // 子线程开始执行
//    // 子线程执行完毕 0
//    // 子线程执行完毕 1  
//    // main thread is end 8
//    // main thread is end 9
//    // main thread is end 10
//    // main thread is end 11
//    // main thread is end 12
//    // main thread is end 13
//    // 子线程执行完毕 2
//    // 子线程执行完毕 3
//    // 子线程执行完毕 4
//    // 子线程执行完毕 %  
//    // main thread is end 14
//    // main thread is end 15
//    // main thread is end 16
//    // main thread is end 17
//    // main thread is end 18
//    // main thread is end 19
//    // 每一次都不一样
//}
//}//namespace a01
//
//namespace a02{
//    // joinable() 判断能否对线程使用join()或detach().若返回true，表示该线程 可以 join()或detach()).
//#include <thread>
//#include <iostream>
//void myprint() {
//    //...
//}
//int main()
//{
//    std::thread mytobj(myprint);
//    if(mytobj.joinable())
//    {
//        std::cout << "1 : joinable() == true" << std::endl;
//    }
//    else
//    {
//        std::cout << "1 : joinable() == false" << std::endl;
//    }
//
//    mytobj.detach();
//
//    if(mytobj.joinable())
//    {
//        std::cout << "2 : joinable() == true" << std::endl;
//    }
//    else
//    {
//        std::cout << "2 : joinable() == false" << std::endl;
//    }
//    return 0;
//    // 1 : joinable() == true
//    // 2 : joinable() == false
//}
//}//namespace a02
//
//namespace a03{
//    // 创建线程的方法02：类的operator()作为可调用对象
//    // 错误示范，由于在分离后调用了已结束的主线程中的局部变量，产生了无法控制的错误
//#include <iostream>
//#include <thread>
//
//class TA
//{
//public:
//    void operator() ()
//    {
//        std::cout << "子线程开始执行" << std::endl;
//        //...
//        std::cout << "子线程执行结束" << std::endl;
//    }
//};
//
//int main()
//{
//    TA ta; // TA 类的对象 ta
//    std::thread mytobj(ta);
//    mytobj.join();
//
//    std::cout << "main thread is end " << std::endl;
//    return 0;
//    // 子线程operator()开始执行
//    // 子线程operator()执行结束
//    // main thread is end 
//}
//}//namespace a03
//
//namespace a04{
//    //
//#include <iostream>
//#include <thread>
//
//class TA
//{
//public:
//    int &m_i; //关键是用了引用&
//    TA(int &i): m_i(i) 
//    {
//        std::cout << "构造函数被执行" << std::endl;
//    }
//    TA(const TA& ta): m_i(ta.m_i)
//    {
//        std::cout << "拷贝构造函数被执行" << std::endl; 
//    }
//    ~TA()
//    {
//        std::cout << "析构函数被执行" << std::endl; 
//    }
//    void operator() ()
//    {
//        for(int i = 0; i < 40; ++i) {
//            std::cout << "m_i_" << i << " 的值为： " << m_i << std::endl;
//        }
//    }
//};
//
//int main()
//{
//    int myi = 6; // 主线程的局部变量，当主线程结束后，myi 的内存被回收，如果子线程没执行完，打印的 myi 的值是不可预料的
//    TA ta(myi); // TA 类的对象 ta
//    std::thread mytobj(ta); // 对象ta 是局部的，那么主线程结束后 ta 还存在吗？ta 对象已经不在。
//    // 既然不存在，还能调用ta的成员函数吗？原因是 ta 是被复制(调用TA的拷贝构造函数)到线程 mytobj 中去,a05证明，
//    // 所以执行完主线程后，复制的那个对象仍然存在。总结，只要你这个TA类的对象中没有引用或指针，那么就不会产生问题。
//    mytobj.detach();   // <-------------
//
//    std::cout << "main thread is end " << std::endl;
//    return 0;
//    // m_i_1 的值为：6
//    // main thread is end 
//    // m_i_2 的值为：
//    // m_i_3 的值为：
//    // 每一次都不一样
//}
//}//namespace a04
//
//namespace a05
//{
//    // 创建thread时调用了类的拷贝构造函数
//#include <iostream>
//#include <thread>
//
//class TA
//{
//public:
//    TA()
//    {
//        std::cout << "构造函数被执行" << std::endl;
//    }
//    TA(const TA& ta)
//    {
//        std::cout << "拷贝构造函数被执行" << std::endl; 
//    }
//    ~TA()
//    {
//        std::cout << "析构函数被执行" << std::endl; 
//    }
//    void operator() ()
//    {
//        for(int i = 0; i < 4; ++i) {
//            std::cout << "m_i_" << i << std::endl;
//        }
//    }
//};
//
//int main()
//{
//    TA ta; 
//    std::cout << "创建类对象" << std::endl;
//    
//    std::thread mytobj(ta); 
//    mytobj.join();   // <-------------
//
//    std::cout << "main thread is end " << std::endl;
//    return 0;
//    // 构造函数被执行
//    // 创建类对象
//    // 拷贝构造函数被执行
//    // m_i_0
//    // m_i_1
//    // m_i_2
//    // m_i_3
//    // 析构函数被执行  <----thread 中 ta 的复制品的析构）
//    // main thread is end 
//    // 析构函数被执行  <----ta 被析构）
//}
//   
//} // namespace a05
//
//namespace a06
//{
//    // 创建线程的方法03:用lambda 表达式
//#include <iostream>
//#include <thread>
//int main()
//{
//    auto mylamthread = [] {
//        std::cout << "子线程开始执行" << std::endl;
//        //...
//        std::cout << "子线程执行结束" << std::endl;
//    };
//    std::thread mytobj(mylamthread);
//    mytobj.join();
//    std::cout << "main thread is end " << std::endl;
//
//    return 0;
//    // 子线程开始执行
//    // 子线程执行结束
//    // main thread is end 
//}
//}// namespace a06
