#include <gtest/gtest.h>
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <atomic>
#include <thread_pool.h>


constexpr const int thread_fcn_count_limit{100};
constexpr const int thread_fcn_max_ms_sleep{100};

std::atomic_int simple_fcn_guard_count{0};
std::atomic_int simple_fcn_exec_count{0};

bool common_thread_fcn(std::atomic_int& guard_count, 
    std::atomic_int& exec_count) {
    int count = ++guard_count;
    if (count <= thread_fcn_count_limit) {
        int sleep_ms = std::rand() % thread_fcn_max_ms_sleep;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
        ++exec_count;
    }
    return guard_count < thread_fcn_count_limit;
}

bool simple_thread_fcn() {
    return common_thread_fcn(simple_fcn_guard_count, simple_fcn_exec_count);
}

TEST(Thread_pool, Simple_Thread_Fcn) {
    constexpr const int num_threads{15};
    Thread_pool tp(&simple_thread_fcn, num_threads);
    tp.run();
    EXPECT_EQ(simple_fcn_exec_count, thread_fcn_count_limit);
}

class TP_test {
public:
    bool thread_fcn() {
        return common_thread_fcn(guard_count_, exec_count_);
    }
    int exec_count() {
        return exec_count_;
    }
private:
    std::atomic_int guard_count_{0};
    std::atomic_int exec_count_{0};
};

TEST(Thread_pool, Functor_Test) {
    constexpr const int num_threads{10};
    TP_test tp_test;
    Thread_pool_ftor tp_ftor(&TP_test::thread_fcn, &tp_test);
    Thread_pool tp(tp_ftor, num_threads);
    tp.run();
    EXPECT_EQ(tp_test.exec_count(), thread_fcn_count_limit);
}
