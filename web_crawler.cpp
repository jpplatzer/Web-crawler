/***
 # Released under the MIT License
 
 Copyright (C) 2024 Jeff Platzer <jeff@platzers.us>

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 ***/

#include <iostream>
#include <stdexcept>
#include <web_crawler.h>
#include <web_page_reader.h>

Crawl_result_t Web_crawler::crawl(const Url_t& site_url, 
    Page_content_processor* page_processor_ptr) {
    page_proc_ptr_ = page_processor_ptr;
    Deconstructed_url decon_url = Url_mgr::deconstruct_url(site_url);
    if (decon_url.domain.empty()) {
        return Crawl_result_t{Crawl_error{crawl_error_invalid_url, "invalid url"}};
    }
    url_mgr_ptr_ = std::make_shared<Url_mgr>(decon_url);
    try {
        thread_pool_.run();
    }
    catch (const std::system_error&) {
        return Crawl_result_t{Crawl_error{crawl_error_thread_creation, "thread creation system error"}};
    }
    page_proc_ptr_->final();
    return Crawl_result_t{};
}

// This is the top-level function that runs in each page processing thread.
// It is repeatedly called in its thread until it returns false.
// It performs multi-threaded coordination of page processing.
bool Web_crawler::process_next_page() {
    bool done = false;
    Opt_page_path_t opt_Link = url_mgr_ptr_->pop_new_path();
    if (opt_Link) {
        if (url_mgr_ptr_->num_new_paths() > 0 && 
            num_threads_waiting_to_proc_ > 0) { 
            // More work to do. Release waiting threads to do it.
            proc_wait_sem_.release();
        }
        process_page(*opt_Link);
    }
    else {
        ++num_threads_waiting_to_proc_;
        done = done_processing();
        if (!done) {
            // std::cout << "waiting to proc" << std::endl;
            proc_wait_sem_.acquire();
            done = done_processing();
        }
        if (done) {
            // All done processing. Make sure any waiting threads are released to exit
            proc_wait_sem_.release();
        }
        else {
            --num_threads_waiting_to_proc_;
        }
    }
    return !done;
}

void Web_crawler::process_page(const Page_path_t& path) {
    Page_paths_t paths;
    Web_page_reader reader;
    std::string url_path = url_mgr_ptr_->make_full_url(path);
    // std::cout << "-----------------> Reading page " << url_path << " depth " << path.depth << std::endl;
    Read_Results_t results = reader.read_page(url_path);
    if (results.http_code == http_ok) {
        paths = url_mgr_ptr_->extract_page_paths(results.content, path);
    }
    if (path.depth < max_depth_ and !paths.empty()) {
        url_mgr_ptr_->update_page_paths(paths);        
    }
    page_proc_ptr_->process_page_content(url_path, url_mgr_ptr_->site_domain(),
        results.http_code, path.depth, paths, results.content);
}



