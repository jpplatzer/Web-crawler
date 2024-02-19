/***
 # Released under the MIT License
 
 Copyright (C) 2024 Jeff Platzer <jeff@platzers.us>

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 ***/

#pragma once

#include <vector>
#include <atomic>
#include <semaphore>
#include <memory>
#include <thread_pool.h>
#include <web_common.h>
#include <url_mgr.h>

class Page_content_processor {
public:
    virtual void process_page_content(const Url_t& page_url, int http_code,
        const Page_links_t& page_links, const Page_content_t& page_content) = 0;

    virtual void final() = 0;
};

enum Crawl_error_code {
    crawl_error_invalid_url,
    crawl_error_thread_creation
};

struct Crawl_error {
    Crawl_error_code err_code;
    std::string err_text;
};

using Crawl_result_t = Success_or_error<Crawl_error>;

class Web_crawler
{
public:
    static const int unlimited_depth = INT_LEAST32_MAX;
    Web_crawler(int num_treads, int max_depth = unlimited_depth) :
        num_treads_(num_treads), max_depth_(max_depth), 
        thread_pool_(
            Thread_pool_ftor_t{&Web_crawler::process_next_page, this}, 
            num_treads) {}

    Crawl_result_t crawl(const Url_t& site_url, 
        Page_content_processor* page_processor_ptr);

private:
    enum { max_sem_count = 0xfff };
    using Url_mgr_ptr_t = std::shared_ptr<Url_mgr>;
    using Thread_pool_ftor_t = Thread_pool_ftor<Web_crawler>;
    std::atomic_int num_threads_waiting_to_proc_{0};
    std::counting_semaphore<max_sem_count> proc_wait_sem_{0};    
    Page_content_processor* page_proc_ptr_{nullptr};
    Url_mgr_ptr_t url_mgr_ptr_;
    int num_treads_;
    int max_depth_;
    Thread_pool<Thread_pool_ftor_t> thread_pool_;

    bool done_processing() {
        return num_threads_waiting_to_proc_ >= num_treads_;
    }
    bool process_next_page();
    void process_page(const Link_t& link);
    Page_links_t extract_page_links(const Page_content_t& content, int depth);

    // Simple test of the concurrent page processing logic
    void process_page_test(const Link_t& link);
};


