#include <thread_pool.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <atomic>
#include <limits>

class Thread_test {
public:
    Thread_test (int times) : times_(times) {}
    bool thread_test_fcn() {
        auto tid = std::this_thread::get_id();
        int count = ++count_;
        // std::cout << "Entering concurrent thread " << tid << " count " << count << std::endl;
        if (count <= times_) {
            std::cout << "---------> Running concurrent thread " << tid << " count " << count << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        // std::cout << "Exiting concurrent thread " << tid << " count " << count << std::endl;
        return (count < times_);
    }
    void reset_count() {
        count_ = 0;
    }
private:
    int times_;
    std::atomic_int count_{0};
};

void test_thread_pool_runs(int num_threads, int num_calls, int num_runs) {
    std::cout << "Starting thread pool test for " << num_threads << " threads, " <<
        num_calls << " calls, and " << num_runs << " runs" << std::endl;
    Thread_test test(num_calls);
    Thread_pool_ftor t_ftor(&Thread_test::thread_test_fcn, &test);
    Thread_pool tp(t_ftor, num_threads);
    for (int i = 0; i < num_runs; ++i) {
        tp.run();
        test.reset_count();
    }
    std::cout << "Exiting thread pool" << std::endl;
}

bool dumb_thread_fcn() {
    std::cout << "Running dumb test" << std::endl;
    return false;
}

void dumb_test() {
    std::cout << "Starting dumb test" << std::endl;
    Thread_pool tp(&dumb_thread_fcn, 12);
    tp.run();
    std::cout << "Exiting dumb test" << std::endl;
}

int main() {
    test_thread_pool_runs(4, 33, 3);
    dumb_test();
    return 0;
}

