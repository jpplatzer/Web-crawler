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
#include <deque>
#include <semaphore>
#include <numeric>

/// @brief Thread pool runs specified function(s) in the pool's threads
class Thread_pool {
public:
    /// @brief Run the shared function specified in the ctor until the function returns false.
    /// @param fcn [in] Function or functor that runs concurrently in the pool's threads. 
    /// The function signature is bool fcn() or bool operator()(). 
    /// The threads continue running the function until it returns false. 
    /// The Thread_pool_ftor class below provide a working example functor.
    /// @param num_threads [in] The number of threads concurrently running in the pool
    /// @throws Can throw a std::system_error when a system error occurs while creating the pool's threads.
    template <class Fcn_t> requires std::invocable<Fcn_t>
    void run(Fcn_t fcn, int num_threads) {
        start_threads_from_fcn(fcn, num_threads);
        wait_for_threads();
        running_statuses_.clear();
    }

    /// @brief Run each function in the container of functions/functors in its own thread.
    /// @param beg [in] The beginning iterator to the container of functions
    /// The iterators value must be a function or functor that runs concurrently in the pool's threads. 
    /// The function signature is bool fcn() or bool operator()(). 
    /// The threads continue running the function until it returns false. 
    /// The Thread_pool_ftor class below provide a working example functor.
    /// @param end [in] The ending iterator to the container of functions
    /// @throws Can throw a std::system_error when a system error occurs while creating the pool's threads.
    template <class In_iter_t>
    void run(In_iter_t beg, In_iter_t end) {
        using category = typename std::iterator_traits<In_iter_t>::iterator_category;
        static_assert(std::is_base_of_v<std::input_iterator_tag, category>);        
        start_threads_from_iter(beg, end);
        wait_for_threads();
        running_statuses_.clear();
    }

private:
    enum { max_sem_count = 0xfff };
    using Running_status_t = std::atomic_bool;
    using Running_statuses_t = std::deque<Running_status_t>;
    using Semaphore_t = std::counting_semaphore<max_sem_count>;

    template <class Fcn_t>
    class Wrapped_fcn {
    public:
        Wrapped_fcn(Fcn_t& fcn,
            std::atomic_bool& running_status,
            Semaphore_t& finished_sem) 
            : fcn_(fcn), running_status_(running_status), finished_sem_(finished_sem) {}
        void operator() () {
            while (running_status_ && fcn_()) {}
            running_status_ = false;
            finished_sem_.release();
        }
    
    private:
        Fcn_t& fcn_;
        std::atomic_bool& running_status_;
        Semaphore_t& finished_sem_;
    };

    Running_statuses_t running_statuses_;
    Semaphore_t finished_sem_{0};

    template <class Fcn_t>
    void start_threads_from_fcn(Fcn_t& fcn, int num_threads) {
        for (int i = 0; i < num_threads; ++i) {
            running_statuses_.emplace_back(true);
            start_thread<Fcn_t>(fcn, running_statuses_.back());
        }
    }

    template <class In_iter_t>
    void start_threads_from_iter(In_iter_t beg, In_iter_t end) {
        using Fcn_t = typename std::iterator_traits<In_iter_t>::value_type;
        static_assert(std::is_invocable_v<Fcn_t>);
        for (int i = 0; beg != end; ++beg, ++i) {
            running_statuses_.emplace_back(true);
            start_thread<Fcn_t>(*beg, running_statuses_.back());
        }
    }

    template <class Fcn_t>
    void start_thread(Fcn_t& fcn, Running_status_t& running_status) {
        Wrapped_fcn wrapped_fcn(fcn, running_status, finished_sem_);
        try {
            std::thread t(wrapped_fcn);
            t.detach();
        }
        catch (const std::system_error& e) {
            stop_threads();
            throw e;
        }
    }

    void wait_for_threads() {
        while (std::accumulate(running_statuses_.begin(), running_statuses_.end(), false)) {
            finished_sem_.acquire();
        }
    }

    void stop_threads() {
        for (auto iter = running_statuses_.begin(); iter != running_statuses_.end(); ++iter) {
            *iter = false;
        }
    }
};

template <class Obj_t>
class Thread_pool_ftor {
public:
    using Fcn_p = bool (Obj_t::*)();
    /// @brief Functor for encapsulating calls to an object's method that is invoked by the threads in the thread pool.
    /// @param fcn_p The object's method to be repeatedly called by the pool's thread.
    /// The method must have the signature bool method()
    /// @param obj_p Pointer to the object whose method will be called
    Thread_pool_ftor(Fcn_p fcn_p, Obj_t* obj_p) : fcn_p_(fcn_p), obj_p_(obj_p) {}
    bool operator() () {
        return (obj_p_->*fcn_p_)();
    }
private:
    Fcn_p fcn_p_;
    Obj_t* obj_p_;
};

