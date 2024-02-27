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
#include <semaphore>

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
        run_threads_to_completion();
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
        run_threads_to_completion();
    }

private:
    enum { max_sem_count = 0xfff };
    using Semaphore_t = std::counting_semaphore<max_sem_count>;

    std::atomic_bool running_enabled_{true};
    std::atomic_int running_count_{0};
    Semaphore_t finished_sem_{0};

    template <class Fcn_t>
    void start_threads_from_fcn(Fcn_t& fcn, int num_threads) {
        for (int i = 0; i < num_threads; ++i) {
            start_thread<Fcn_t>(fcn);
        }
    }

    template <class In_iter_t>
    void start_threads_from_iter(In_iter_t beg, In_iter_t end) {
        using Fcn_t = typename std::iterator_traits<In_iter_t>::value_type;
        static_assert(std::is_invocable_v<Fcn_t>);
        for (; beg != end; ++beg) {
            start_thread<Fcn_t>(*beg);
        }
    }

    template <class Fcn_t>
    class Wrapped_fcn {
    public:
        Wrapped_fcn(Fcn_t& fcn,
            std::atomic_bool& running_enabled,
            std::atomic_int& running_count,
            Semaphore_t& finished_sem) 
            : fcn_(fcn), running_enabled_(running_enabled), 
              running_count_(running_count), finished_sem_(finished_sem) {}
        void operator() () {
            while (running_enabled_ && fcn_()) {}
            --running_count_;
            finished_sem_.release();
        }
    
    private:
        Fcn_t& fcn_;
        std::atomic_bool& running_enabled_;
        std::atomic_int& running_count_;
        Semaphore_t& finished_sem_;
    };

    template <class Fcn_t>
    void start_thread(Fcn_t& fcn) {
        Wrapped_fcn wrapped_fcn(fcn, running_enabled_, 
            running_count_, finished_sem_);
        try {
            ++running_count_;
            std::thread t(wrapped_fcn);
            t.detach();
        }
        catch (const std::system_error& e) {
            --running_count_;
            running_enabled_ = false;
            throw e;
        }
    }

    void run_threads_to_completion() {
        while (running_count_ > 0) {
            finished_sem_.acquire();
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

