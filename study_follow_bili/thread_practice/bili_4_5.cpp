// 第四节：创建和等待多个线程，第五节：mutex、死锁
namespace a01 
{
    // 创建和等待多个线程
    // 结果：（1）子线程0——9顺序是乱的，与进程调度器有关
    //      （2）一次创建多个线程的方法：将 thread 对象放于容器中进行管理
#include <iostream>
#include <thread>
#include <vector>
// 很多个线程都可以用同一个入口函数
void myprint(int inum) {
    std::cout << "child thread start, id = " << inum << std::endl;
    // ... 
    std::cout << "child thread end  , id = " << inum << std::endl;
    return;
}

int main() {
    // 创建和等待多个线程
    std::vector<std::thread> my_threads;
    // 创建10个线程，线程入口函数均用 myprint()
    for (int i = 0; i < 10; ++i) {
        my_threads.push_back(std::thread(myprint, i));
        // 创建完10个线程，并且10个线程已经开始执行
    }
    // for (int i = 0; i < 10; ++i) {
    //     my_threads[i].join(); // 等待10个线程都返回
    // }
    for (auto iter = my_threads.begin(); iter != my_threads.end(); ++iter) {
        iter->join();
    }
    std::cout << "main thread is end" << std::endl;
    return 0;
    // child thread start, id = 0
    // child thread end  , id = 0
    // child thread start, id = 1
    // child thread end  , id = child thread start, id = child thread start, id = 18
    // child thread start, id = child thread start, id = 5
    // child thread start, id = child thread start, id = 43
    // 7
    // 9child thread end  , id = 
    // child thread end  , id = 
    // child thread end  , id = 5child thread end  , id = 7

    // child thread end  , id = child thread start, id = 46

    // child thread end  , id = 6
    // child thread end  , id = 9
    // child thread start, id = 
    // 32
    // 8

    // child thread end  , id = 2
    // main thread is end
}
}  // namespace a01

// 第五节
// 5.1 mutex 互斥量的用法
//    5.1.1 lock()、unlock()
//    5.1.2 std::lock_guard 类模板
//    5.1.3 多个共享变量，可以每个共享变量对应一个mutex
// 5.2 死锁
//    5.2.1 死锁演示
//    5.2.2 死锁的3个解决方案
// 保护共享数据 = 维护住数据的一致性
// 原则上，只锁住需要锁住的代码，既不能多也不能少。
// 具体哪些代码需要被锁住才能不破坏共享数据的一致性，需要程序员自己判断（凭经验）
// TODO 同一段代码（意味着访问相同的共享数据），有多个线程要用，如何用mutex？
// TODO 如果不同线程代码不同，但是这些代码需要访问或修改同一片内存，如何用mutex？

// mutex 互斥量的使用原则：
// 某个互斥量仅同时允许一个线程访问
// ---> 好像是错的！！！互斥量在系统的任何进程里都是可见的，一个进程创建了一个互斥量，另一个进程试图去获取该锁是合法的。
// 互斥量要求哪个线程获取了互斥量，就要负责释放这个锁，其他线程越俎代庖去释放互斥量是无效的。
namespace a02 {
// 互斥量的定义应该放在什么地方合适？
// 一般来说互斥锁是一个成员变量或者全局变量。
// 如果把互斥锁声明为临时变量，是完全没有发挥出互斥锁的作用。因为你每个线程内部都有一个锁，那线程A线程B之间的锁肯定不会相互影响。
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
// std::mutex my_mutex; //<---------------编译通过，运行通过

void myprint(std::vector<int> &vec) {
    // std::mutex my_mutex; //<----------可以写在这吗？编译通过，但是运行时发生资源抢占
    for (int i = 0; i < 1000; ++i) {  
        my_mutex.lock();
        vec.push_back(i);
        std::cout << i << std::endl;
        my_mutex.unlock();
    }
}
int main() {
    //std::mutex my_mutex; // <-------编译不通过use of undeclared identifier 'my_mutex'my_mutex.lock();
    std::vector<int> vec;
    std::thread t1(myprint, std::ref(vec));
    std::thread t2(myprint, std::ref(vec));
    t1.join();
    t2.join();
    std::cout << "main thread is end" << std::endl;
    return 0;
}
}  // namespace a02

namespace a03 {
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
int main() {
    std::vector<int> arr;
    std::mutex mtx;  //<---一定要放在这里吗
    std::thread t1([&] {
        for (int i = 0; i < 1000; ++i) {
            mtx.lock();
            arr.push_back(1);
            std::cout << 1 << std::endl;
            mtx.unlock();
        }
    });
    std::thread t2([&] {
        for (int i = 0; i < 1000; ++i) {
            mtx.lock();
            arr.push_back(2);
            std::cout << 2 << std::endl;
            mtx.unlock();
        }
    });
    t1.join();
    t2.join();
    std::cout << "main thread is end" << std::endl;
    return 0;
}
}  // namespace a03


namespace a04 {
    // 用于与 a05 对比
#include <iostream>
#include <thread>
#include <list>
#include <map>
#include <string>
#include <mutex>

class Manage {
public:
    // “把收到的玩家命令放入队列”的 thread
    void in_queue() {
        for (int i = 0; i < 1000; ++i) {
            std::cout << "in_queue() 被执行，插入一个元素" << i << std::endl;
            // 假设这个数字就是我收到的命令，我直接放进消息队列
             msg_queue_.push_back(i);  
        }
    }
    // “把收到的玩家命令从队列取出”的 thread
    void out_queue() {
        for (int i = 0; i < 1000; ++i) {
            if (!msg_queue_.empty()) {
                // 消息不为空
                int command = msg_queue_.front();  // 返回第一个元素，但不检查元素是否存在
                msg_queue_.pop_front();  // 移除第一个元素，但不返回
            }
            else {
                std::cout << "out_queue()被执行，但是队列为空" << std::endl;
            }
        }
    }
private:
    std::list<int> msg_queue_;
};
int main() {
    Manage my_manage;
    // 若要将某个类的成员函数设定为线程函数，应传入指向该成员函数的函数指针，还要给出合适的对象指针（由my_manage的地址充当）
    std::thread my_out(&Manage::out_queue, &my_manage);  
    std::thread my_in(&Manage::in_queue, &my_manage);
    my_out.join();
    my_in.join();
    
    return 0;
}
}  // namespace a04

// 主线程、线程1 、线程2 都在运行，互斥量 mutex 需要维持共享数据（链表 msg_queue_ ）的一致性。
namespace a05 {
// 原则上，只锁住需要锁住的代码，既不能多也不能少。
// 具体哪些代码需要被锁住才能不破坏共享数据的一致性，需要程序员自己判断（凭经验）
#include <iostream>
#include <thread>
#include <list>
#include <map>
#include <string>
#include <mutex>

class Manage {
public:
    void in_queue() {
        for (int i = 0; i < 100000; ++i) {
            my_mutex_.lock();
            std::cout << "in_queue() 被执行，插入一个元素" << i << std::endl;// cout 放在锁内，可以避免打印混乱
            msg_queue_.push_back(i);  // <-------------------------------需要被保护
            my_mutex_.unlock();
        }
    }
    void out_queue() {
        for (int i = 0; i < 100000; ++i) {
            my_mutex_.lock();
            if (!msg_queue_.empty()) {  // <------------------------------需要被保护
                int command = msg_queue_.front();  // <-------------------需要被保护
                msg_queue_.pop_front();  // <-----------------------------需要被保护
                std::cout << "out_queue()被执行，取出一个数据" << std::endl;
                my_mutex_.unlock();
                return;
            }
            std::cout << "out_queue()被执行，但是队列为空" << std::endl;
            my_mutex_.unlock();
            return;
        }
    }
private:
    std::list<int> msg_queue_;
    std::mutex my_mutex_; // 创建了一个互斥量
    // mutex类的对象有成员函数 lock() unlock()。
    // 同一个 mutex 呢？ 有什么使用规则？某个 mutex 仅同时允许一个线程访问
    // 多个线程尝试对 同一个互斥量 lock，但是只有一个线程能成功 lock
    // 成功的标志是lock（） 函数返回，没成功的 thread 会卡在这里，不断尝试去锁，直到成功。
    // lock()->unlock()的范围由开发者决定，范围小了没达到保护效果，范围大了会影响效率
};
int main() {
    Manage my_manage;
    std::thread my_out(&Manage::out_queue, &my_manage);
    std::thread my_in(&Manage::in_queue, &my_manage);
    my_out.join();
    my_in.join();
    
    return 0;
}
}  // namespace a05

namespace a06 {
// 5.1.2 std::lock_guard 类模板
// lock_guard类模版的构造函数里执行了mutex::lock()
// 出作用域时，lock_guard类模版的析构函数里执行了mutex::unlock()
// 可以用{}提前结束lock_guard
#include <iostream>
#include <thread>
#include <vector>
int main() {
    std::vector<int> arr;
    std::mutex my_mutex;
    std::thread t1([&] {
        for (int i = 0; i < 1000; ++i) {
            std::lock_guard<std::mutex> my_guard(my_mutex);
            arr.push_back(1);
        }
    });
    std::thread t2([&] {
        for (int i = 0; i < 1000; ++i) {
            std::lock_guard my_guard(my_mutex);  // 模板参数推导
            arr.push_back(2);
        }
    });
    t1.join();
    t2.join();
    std::cout << "main thread is end" << std::endl;
    return 0;
}
}  // namespace a06

namespace a07 {
// 5.1.3 多个共享变量，每个共享变量都对应一个mutex
// 如果有两个共享变量1、共享变量2 需要被分别保护的，可以设置两个互斥量：mtx1 用来锁定 arr1，mtx2 用来锁定 arr2
#include <iostream>
#include <thread>
#include <vector>
int main() {
    std::vector<int> arr1;
    std::mutex my_mutex1;

    std::vector<int> arr2;
    std::mutex my_mutex2;

    std::thread t1([&] {
        for (int i = 0; i < 1000; ++i) {
            my_mutex1.lock();
            arr1.push_back(1); 
            my_mutex1.unlock();         
        }
    });
    std::thread t2([&] {
        for (int i = 0; i < 1000; ++i) {
            my_mutex2.lock();
            arr2.push_back(2);
            my_mutex2.unlock(); 
        }
    });
    t1.join();
    t2.join();
    std::cout << "main thread is end" << std::endl;
    return 0;
}
}  // namespace a07

namespace a08 {
// 死锁这个问题由至少两把锁（两个互斥量）才会产生的问题
// 假设有两把锁🔒，金锁和银锁.step1 : 线程 A 执行，先锁住Jin锁，然后准备锁Yin锁；
// step2 ： 出现了上下文切换
// step3 ： 线程 B 执行，先锁住Yin锁（因为 Yin锁之前没被锁上，所以会锁成功），然后准备锁Jin锁；
// 此时此刻，死锁发生

#include <iostream>
#include <thread>
#include <vector>
int main() {
    std::vector<int> arr1;
    std::mutex my_mutex1;

    std::vector<int> arr2;
    std::mutex my_mutex2;

    std::thread t1([&] {
        for (int i = 0; i < 1000; ++i) {
            my_mutex1.lock();
            my_mutex2.lock();
            arr1.push_back(1);  // ...
            my_mutex2.unlock(); 
            my_mutex1.unlock();         
        }
    });
    std::thread t2([&] {
        for (int i = 0; i < 1000; ++i) {
            my_mutex2.lock();
            my_mutex1.lock();
            arr2.push_back(2); // ...
            my_mutex1.unlock();
            my_mutex2.unlock(); 
        }
    });
    t1.join();
    t2.join();
    std::cout << "main thread is end" << std::endl;
    return 0;
}
}  // namespace a08

namespace a09 {
// 死锁的解决方案一：一个线程永远不要同时持有两个锁
// 死锁的解决方案二：只要保证两个互斥量上锁的顺序一致，就不会死锁
// 死锁的解决方案三：std::lock()函数模板：它能一次锁住两个或两个以上互斥量，解决了因为锁顺序导致的死锁情况。
//                要么两个互斥量都锁住，要么都没锁住。
//                只要其中有任何一个互斥量没锁住时，它就在那里等着（立即把已经锁了的那个释放掉），继续尝试直到所有互斥量都锁住才会往下走。
}  //namespace a09

namespace a10 {
// 只用 std::lock() 函数模板时，仍然需要自己 .unlock()，如何能不自己解锁？
// 答：同时用 std::lock() 函数模板和 std::lock_guard 类模板，需要做一点改变：std::lock_guard 中的 std::adopt_lock 作为第二个参数
// std::lock_guard 中的 std::adopt_lock 作为第二个参数，意味着不负责构造，只负责析构（本来lock_guard 既负责构造又负责析构）
std::thread t2([&] {
    for (int i = 0; i < 1000; ++i) {
        std::lock(mtx1, mtx2);

        std::lock_guard<std::mutex> grd(mtx1, std::adopt_lock);
        std::lock_guard<std::mutex> grd(mtx2, std::adopt_lock);
        // ...
    }
});
}  // namespace a10