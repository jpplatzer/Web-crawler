/***
 # Released under the MIT License
 
 Copyright (C) 2024 Jeff Platzer <jeff@platzers.us>

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 ***/

#pragma once

#include <thread>
#include <atomic>
#include <vector>
#include <semaphore>
#include <numeric>

template <class Fcn_t>
class Thread_pool {
public:
    Thread_pool(Fcn_t fcn, int num_threads) 
        : fcn_(fcn), num_threads_(num_threads), running_statuses_(num_threads) {}
    void run();

private:
    enum { max_sem_count = 0xfff };
    using Semaphore_t = std::counting_semaphore<max_sem_count>;
    class Wrapped_fcn {
    public:
        Wrapped_fcn(Fcn_t& fcn,
            std::atomic_bool& running_status,
            Semaphore_t& finished_sem) 
            : fcn_(fcn), running_status_(running_status), finished_sem_(finished_sem) {}
        void operator() ();
    
    private:
        Fcn_t& fcn_;
        std::atomic_bool& running_status_;
        Semaphore_t& finished_sem_;
    };

    Fcn_t fcn_;
    int num_threads_;
    std::vector<std::atomic_bool> running_statuses_;
    Semaphore_t finished_sem_{0};

    void start_thread(int thread_idx);
    void stop_thread(int thread_idx) {
        running_statuses_[thread_idx] = false;
    }
};

template <class Fcn_t>
void Thread_pool<Fcn_t>::run() {
    try {
        for (int i = 0; i < num_threads_; ++i ) {
            start_thread(i);
        }
    }
    catch (const std::system_error& e) {
        // thread creation can throw a system_error
        // in this case, stop all threads and rethrow
        for (int i = 0; i < num_threads_; ++i ) {
            stop_thread(i);
        }
        throw e;
    }
    while (std::accumulate(running_statuses_.begin(), running_statuses_.end(), false)) {
        finished_sem_.acquire();
    }
}

template <class Fcn_t>
void Thread_pool<Fcn_t>::start_thread(int thread_idx) {
    running_statuses_[thread_idx] = true;
    Wrapped_fcn wrapped_fcn(fcn_, running_statuses_[thread_idx], finished_sem_);
    std::thread t(wrapped_fcn);
    t.detach();
}

template <class Fcn_t>
void Thread_pool<Fcn_t>::Wrapped_fcn::operator() () {
    while (running_status_ && fcn_()) {}
    running_status_ = false;
    finished_sem_.release();
};

template <class Obj_t>
class Thread_pool_ftor {
public:
    using Fcn_p = bool (Obj_t::*)();
    Thread_pool_ftor(Fcn_p fcn_p, Obj_t* obj_p) : fcn_p_(fcn_p), obj_p_(obj_p) {}
    bool operator() () {
        return (obj_p_->*fcn_p_)();
    }
private:
    Fcn_p fcn_p_;
    Obj_t* obj_p_;
};

