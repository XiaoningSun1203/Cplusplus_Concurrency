// async、future、packaged_task、promise
// (1) std::async、std::future 创建后台任务并返回值
// (2) std::packaged_task
// (3) std::promise
// (4) 小结 
namespace a01 {
// 假设有这么一个需求：希望线程返回一个东西（当然可以用一个全局变量，但是有没有别的方法？）
// std::async 是个函数模板，用来启动一个异步任务，它返回一个std::future 对象。std::future 是类模板。
// 什么叫“启动一个异步任务”？就是自动创建一个线程并开始执行对应的线程入口函数，返回std::future 对象,
// 这个 future 对象 result 里面就含有线程入口函数所返回的东西 = 线程返回的东西，
// 要得到这个返回的东西，可以通过调用 future 对象的成员函数 get() 得到。
// 有人称呼 std::future 提供了一种访问异步操作结果的机制，就是说这个结果你可能没办法立刻拿到，
// 但是在线程执行完毕时，你就能拿到结果了，理解成：future里会保存一个东西，这个东西是在将来的某个时刻会得到。
#include <iostream>
#include <thread>
#include <mutex>
#include <future>

int my_thread() {  // 线程入口函数
    std::cout << "my_thread() start," << "threadid = " ;
    std::cout << std::this_thread::get_id() << std::endl;

    std::chrono::milliseconds dura(5000); // 定义一个5秒的时间
    std::this_thread::sleep_for(dura);

    std::cout << "my_thread() end," << "threadid = " ;
    std::cout << std::this_thread::get_id() << std::endl;
    return 5;
}

int main() {
    std::cout << "main threadid = " << std::this_thread::get_id() << std::endl;
    std::future<int> my_future = std::async(my_thread);  // 创建一个线程并开始执行，绑定关系；流程不会在这里卡住
    std::cout << "continue............." << std::endl;
    int def = 0;
    // 通过 std::future 对象的 get() 成员函数等待线程执行结束并返回结果，不拿到返回结果誓不罢休。
    std::cout << my_future.get() << std::endl;  // 主线程卡在这里等着子线程结束 my_future 拿结果
    // 通过实践得出：不能 get 两次，会出现异常
    // my_future.wait();  // 等待线程返回，本身并不返回结果。
    std::cout << "main thread is end" << std::endl;
    return 0;
    // main threadid = 0x1027ba600
    // continue.............
    // my_thread() start,threadid = 0x7000063ae000
    // my_thread() end,threadid = 0x7000063ae000 <--- 一定在.get()之前
    // 5
    // main thread is end
}
}  // namespace a01 25'

namespace a02 {
    // 用类的成员函数作为线程的入口函数
#include <iostream>
#include <thread>
#include <mutex>
#include <future>
class A {
public:
    int my_thread(int param) {
        std::cout << param << std::endl;
        // 睡眠5秒
        std::chrono::milliseconds dura(5000);
        std::this_thread::sleep_for(dura);

        return 5;
    }
};
int main() {
    A a;
    int a_param = 12;
    std::future<int> my_future = std::async(&A::my_thread, &a, a_param);  // <---

    std::cout << "continue........" << std::endl;
    std::cout << my_future.get() << std::endl;
    std::cout << "main thread is end" << std::endl;
    return 0;
    // continue........
    // 12
    // 5
    // main thread is end
    // 如果没有my_future.get()这一句，会是如下结果：可见如果没有.get(),还是需要.wait()的
    // continue........
    // main thread is end
    // 12
}
}  // namespace a02

namespace a03 {
// 额外向 std::async() 传递一个参数，该参数类型是 std::launch （枚举类型），来达到特殊目的。
// (a)如果传递 std::launch::deferred,表示线程入口函数的调用被延迟到 std::future 的 .wait() 或 .get() 函数调用时才执行；
// 如果传递 std::launch::deferred，并且没有调用.wait() 或 .get()，会发生什么？答案：不会创建 my_thread 线程，在主线程中调用my_thread函数
// (b)如果传递 std::launch::async,表示调用 async 函数时就开始创建 my_thread 线程；
#include <iostream>
#include <thread>
#include <mutex>
#include <future>
class A {
public:
    int my_thread(int param) {
        std::cout << param << std::endl;
        std::cout << "main thread = " << std::this_thread::get_id() << std::endl;
        // 睡眠5秒
        std::chrono::milliseconds dura(5000); 
        std::this_thread::sleep_for(dura);

        return 5;
    }
};
int main() {
    A a;
    int a_param = 12;
    std::cout << "main thread = " << std::this_thread::get_id() << std::endl;
    std::future<int> my_future = std::async(std::launch::deferred, &A::my_thread, &a, a_param);  // <---
    //std::future<int> result = std::async(std::launch::async, &A::my_thread, &a, a_param);  // <---

    std::cout << "continue........" << std::endl;
    //std::cout << my_future.get() << std::endl;
    std::cout << "main thread is end" << std::endl;
    return 0;
    // ::deferred 和 ::async 结果都是：
    // continue........
    // 12
    // 5
    // main thread is end
}
}  // namespace a03 50'

namespace a04 {
// std::packaged_task:打包任务，把任务包装起来
// 是个类模板，它的模板参数是 各种可调用对象；通过 std::packaged_task 来把各种可调用对象包装起来，方便将来作为线程入口。
// 怎么用？假如 my_thread 是个独立的线程入口函数,不是某类的成员函数
std::packaged_task<int(int)> my_packaged_task(my_thread); // <返回值类型（参数类型）>
std::thread t1(std::ref(my_packaged_task), 1);  // 1是线程入口函数的参数
t1.join();
std::future<int> my_future = my_packaged_task.get_future();
std::cout << my_future.get() << std::endl;
}  // namespace a04

namespace a05 {
// std::promise 类模板
// 我们能够在某个线程中给它赋值，然后在其他线程中把这个值取出来用。
// =通过 promise 保存一个值，在将来某个时刻，通过把一个 future 对象绑定到 promise 上来得到这个绑定的值；
#include <iostream>
#include <thread>
#include <mutex>
#include <future>
void my_thread(std::promise<int> &prom, int calc) {
    // 做一系列复杂运算，花了5秒；
    std::chrono::milliseconds dura(5000);
    std::this_thread::sleep_for(dura);
    // 假设计算的结果 calc，将其保存在 result 中
    int result = calc;
    // 把 result 保存在对象 prom中
    prom.set_value(result);
    return;
}
int main() {
    std::promise<int> my_promise;  // 声明一个 std::promise 对象 my_promise,保存的值类型为 int；
    std::thread t1(my_thread, std::ref(my_promise), 180);
    t1.join();  // 必须有，否则会报异常。为什么？？？？
    // 获取结果
    std::future<int> my_future = my_promise.get_future();  // 把 promise 和future 绑定，用于获取线程返回值
    std::cout << my_future.get() << std::endl;
    std::cout << "main thread is end" << std::endl;
}
}  //namespace a05

namespace a06 {
#include <iostream>
#include <thread>
#include <mutex>
#include <future>

void my_thread2(std::future<int> &my_future) {
    auto result = my_future.get();
    std::cout << "my_thread2 result = " << result << std::endl;
}
void my_thread(std::promise<int> &prom, int calc) {
    // 做一系列复杂运算，花了5秒；
    std::chrono::milliseconds dura(5000);
    std::this_thread::sleep_for(dura);
    // 假设计算的结果是 calc，将其保存在 result 中
    int result = calc;
    // 把 result 保存在对象 prom中
    prom.set_value(result);
    return;
}
int main() {
    std::promise<int> my_promise;  
    std::thread t1(my_thread, std::ref(my_promise), 180);
    t1.join(); 
    // 获取结果
    std::future<int> my_future = my_promise.get_future(); 
    // 第一个线程计算出来的结果 my_future，第二个线程拿来用
    std::thread t2(my_thread, std::ref(my_future));
    t2.join(); 

    std::cout << "main thread is end" << std::endl;
}
}  // namespace a06
