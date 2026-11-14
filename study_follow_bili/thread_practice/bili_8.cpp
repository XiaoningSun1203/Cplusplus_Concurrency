// std::condition_variable类、wait()、notify_one()、notify_all()
// 条件变量需要 mutex 配合使用
// 条件变量是同步的一种方法，线程有两种操作条件变量的方式：等待、通知
// 等待：一个条件变量可以被多个线程等待
// 通知：通知某个或全部在等待此条件变量的线程
// 有两个线程：A、B，线程A：等待某个条件成立
// 线程B：专门往队列中添加
namespace a01 {
// 基于第五节代码 namespace 03
#include <iostream>
#include <thread>
#include <list>
#include <map>
#include <string>
#include <mutex>

class Manage {
public:
    void in_queue() {  // “把收到的玩家命令放入队列”的 thread
        for (int i = 0; i < 100000; ++i) {
            std::cout << "in_queue() 被执行，插入一个元素" << i << std::endl;
            my_mutex_.lock();
            msg_queue_.push_back(i);  // <-------------------需要被保护
            my_mutex_.unlock();
        }
    }
    void out_queue() {  // “把收到的玩家命令从队列取出”的 thread
        for (int i = 0; i < 100000; ++i) {
            if (msg_queue_.empty() == false){       // 本行避免了“即使 msg_queue_ 为空还是要先锁一下”的效率损耗
                my_mutex_.lock();
                if (msg_queue_.empty() == false) {  // <------------------------------读共享区域
                    int command = msg_queue_.front();  // <-------------------需要被保护
                    msg_queue_.pop_front();  // <-----------------------------需要被保护
                    my_mutex_.unlock();
                    std::cout << "out_queue()被执行，取出一个数据" << std::endl;
                    return;
                }
                my_mutex_.unlock();
                std::cout << "out_queue()被执行，但是队列为空" << std::endl;
                return;
            }
            
        }
    }
private:
    std::list<int> msg_queue_;
    std::mutex my_mutex_; // 创建了一个互斥量
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
}  // namespace a01

namespace a02 {
// 如何避免不断的判断消息队列是否为空？改成当消息队列不为空时通知我们（引入 condition_variable 的目的：减少不断去看empty（）的开销）
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
            std::cout << "in_queue() 被执行，插入一个元素" << i << std::endl;
            my_mutex_.lock();
            msg_queue_.push_back(i); 
            // 假如 out_queue() 在抽卡，需要100毫秒时间，并没有卡在 wait 那里等待，此时 notify_one() 无效 ！！！
            my_condition_.notify_one();  // 执行完这行，out_queue()中的 wait 就会被通知（但 wait 不一定能抢到my_mutex_ ）
            my_mutex_.unlock();
        }
    }
    void out_queue() {
        int command = 0;
        while (true) {
            // .wait()是 condition_variable 的成员函数
            // 如果 lambda式返回 false，wait()将直接返回，流程继续向下走
            // 如果 lambda式返回 false，wait()将解锁 my_mutex_ （有利于把锁尽快让给其他线程），并堵塞（等待）到本行
                // 等待到什么时候为止呢？等待到其他某个线程调用 notify_one() 成员函数为止
            // 如果 wait 第二个参数不写，这种情况相当于 lambda 式返回的是 false。

            // 当其他线程用 notify_one() 对本线程 wait() 通知时（原本是等待状态），wait 就开始恢复干活了，wait会做什么呢？
            // （1）wait 不断地尝试获取 互斥量my_mutex_ ，如果获取不到，流程就卡在 wait 这里不断尝试
            // （2）如果获取了 my_mutex_，就上锁
            // （2.1）如果 wait 有第二个参数 && 第二个参数返回 false，那 wait 又对 my_mutex_ 解锁，然后又陷入等待，等着被通知；
            // （2.2）如果 wait 有第二个参数 && 第二个参数返回 true，则 wait 返回，流程继续走（此时 my_mutex_ 已上锁）；
            // （2.3）如果 wait 没有第二个参数，则 wait 返回，流程继续走。
            my_condition_.wait(my_mutex_, [this]{ // 第二个参数可以是任何一个可调用对象
                if (msg_queue_.empty() == false){
                    return true;
                }
                return false;
            });
            // 只要流程能走到这里， my_mutex_ 一定是锁着的，队列里至少有一条数据
            command = msg_queue_.front();  
            msg_queue_.pop_front(); 
            my_mutex_.unlock();
            std::cout << "out_queue() 执行，取出" << command << std::endl;

            // ...执行一些具体动作，比如帮助玩家抽卡，需要100毫秒时间
        } 
    }
private:
    std::list<int> msg_queue_;
    std::mutex my_mutex_; 
    std::condition_variable my_condition_; // 创建一个 condition_variable 对象
};
int main() {
    Manage my_manage;
    std::thread my_in(&Manage::in_queue, &my_manage);
    my_out.join();
    my_in.join();
    return 0;
}
}  // namespace a02
// 演讲者深入思考 50‘左右
namespace a02 {
//（3）notify_all()
// 如果有多个线程里都有 my_condition.wait，my_condition.notify_one() 只会通知某一个线程，具体通知哪个不一定 
// 如果多个线程的任务相同，notify 哪个都无所谓
// 如果多个线程执行的代码不同，但是都卡在 my_condition.wait ，可以用 my_condition.notify_all() 通知所有线程
}
