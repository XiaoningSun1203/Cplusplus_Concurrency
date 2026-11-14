// 第六节 unique_lock详解
// unique_lock 类模板的默认格式：std::unique_lock<std::mutex> grd(my_mutex);
// 可以看到其用法与 lock_guard 相似std::lock_guard<std::mutex> grd(my_mutex);
// unique_lock 类模板的默认格式的作用：在 grd 构造时调用 mutex::lock();
//                                 在 grd 析构时调用 mutex::unlock();
// 可以看到其作用与 lock_guard 相似
// 但是，unique_lock 类模板比 lock_guard 类模板更灵活
// (2) unique_lock 类模板的第二个参数--> 起到标记作用
//       (2.1) std::adopt_lock   --->表示互斥量之前已被lock()
//       (2.2) std::try_to_lock  --->
//       (2.3) std::defer_lock   --->
// (3) unique_lock 类模板的成员函数
//       (3.1) lock()
//       (3.2) unlock()
//       (3.3) try_lock()
//       (3.4) release()
// (4) unique_lock 所有权的传递