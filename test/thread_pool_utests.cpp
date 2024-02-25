#include <gtest/gtest.h>
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <atomic>
#include <deque>
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
    Thread_pool tp;
    tp.run(&simple_thread_fcn, num_threads);
    EXPECT_EQ(simple_fcn_exec_count, thread_fcn_count_limit);
}

class Shared_fcn_test {
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
    Shared_fcn_test tp_test;
    Thread_pool_ftor tp_ftor(&Shared_fcn_test::thread_fcn, &tp_test);
    Thread_pool tp;
    tp.run(tp_ftor, num_threads);
    EXPECT_EQ(tp_test.exec_count(), thread_fcn_count_limit);
}

class Separate_fcn_test {
public:
    Separate_fcn_test(int instance_num) : instance_num_(instance_num),
        exec_limit_(50 + (instance_num * 2)), 
        sleep_ms_((instance_num % 2) + 1) {}
    bool thread_fcn() {
        bool not_done = ++exec_count_ < exec_limit_;
        if (not_done) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms_));
        }
        return not_done;
    }
    int exec_count() const {
        return exec_count_;
    }
    int exec_limit() const {
        return exec_limit_;
    }
private:
    int instance_num_;
    int exec_limit_;
    int sleep_ms_;
    std::atomic_int exec_count_{0};
};

TEST(Thread_pool, Iterator_Test) {
    constexpr const int num_threads{9};
    using Separate_fcn_tests_t = std::deque<Separate_fcn_test>;
    using Ftor_t = Thread_pool_ftor<Separate_fcn_test>;
    using Ftors_t = std::deque<Ftor_t>;
    Separate_fcn_tests_t separate_fcn_tests;
    Ftors_t ftors;
    for (int i = 0; i < num_threads; ++i) {
        separate_fcn_tests.emplace_back(i);
        ftors.emplace_back(&Separate_fcn_test::thread_fcn, 
            &separate_fcn_tests.back());
    }
    Thread_pool tp;
    tp.run(ftors.begin(), ftors.end());
    // int i = 0;
    for (const Separate_fcn_test& test: separate_fcn_tests) {
        /***
        std::cout << "Test " << i++ << " executed " << 
            test.exec_count() << " expected " <<
            test.exec_limit() << std::endl;
        ***/
        EXPECT_TRUE(test.exec_count() > 0);
        EXPECT_EQ(test.exec_count(), test.exec_limit());
    }
}

