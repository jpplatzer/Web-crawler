/***
 # Released under the MIT License
 
 Copyright (C) 2024 Jeff Platzer <jeff@platzers.us>

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 ***/

#include <iostream>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <url_mgr.h>
#include <web_crawler.h>


class Example_content_processor : public Page_content_processor {
public:
    void process_page_content(const Url_t& page_url, 
        const Url_t& site_domain, int http_code, int depth,
        const Page_paths_t& page_links, const Page_content_t& page_content) override;
    void final() override {
        is_done_ = true;
        std::cout << "Done processing the site's pages" << std::endl;
    }
    void print_site_info();

private:
    struct Page_info {
        int http_code;
        size_t size;
        int depth;
        int num_links;
        int num_backlinks;
    };
    bool is_done_{false};
    std::mutex proc_mutex_;
    using Page_info_map = std::unordered_map<Url_t, Page_info>;
    Page_info_map page_info_map_;
};

void Example_content_processor::process_page_content(const Url_t& page_url, 
    const Url_t& site_domain, int http_code, int depth,
    const Page_paths_t& page_links, const Page_content_t& page_content) {
    std::lock_guard lock(proc_mutex_);
    std::cout << "Process page_content for: " << page_url <<
        " HTTP code " << http_code <<
        " has " << page_content.size() << " bytes" <<
        " and has " << page_links.size() << " links " << std::endl;
    page_info_map_.emplace(page_url, 
        Page_info{http_code, page_content.size(), depth, 
            static_cast<int>(page_links.size()), 1});
    for (auto page_link: page_links) {
        Url_t full_url = Url_mgr::make_full_url(site_domain, 
            page_link.path, page_link.page);
        auto iter = page_info_map_.find(full_url);
        if (iter != page_info_map_.end()) {
            ++iter->second.num_backlinks;
        }
    }
}

void Example_content_processor::print_site_info() {
    if (!is_done_) {
        std::cout << "Site crawling is still in progress..." << std::endl;
        return;
    }
    for (auto info: page_info_map_) {
        std::cout << "Page: " << info.first <<
            ", code: " << info.second.http_code <<
            ", size: " << info.second.size <<
            ", depth: " << info.second.depth <<
            ", links: " << info.second.num_links <<
            ", backlinks: " << info.second.num_backlinks << std::endl;
    }
}

bool perform_crawler_test(const Url_t& site_url, int num_threads, int max_depth) {
    std::cout << "Peform web crawler test for: " << site_url << std::endl;
    Example_content_processor cp;
    Web_crawler web_crawler(num_threads, max_depth);
    Crawl_result_t result = web_crawler.crawl(site_url, &cp);
    if (!result) {
        std::cout << "Error crawling website: " << result.error().err_text << std::endl;
        return false;
    }
    else {
        cp.print_site_info();
    }
    return true;
}

void usage() {
    std::cout << "Multi-threaded crawler that crawls and processes the pages on the " << 
        "specified site and its children.\n"  << std::endl;
    std::cout << "Usage: web-crawler SITE_URL NUM_THREADS [MAX_DEPTH]"  << std::endl;
    std::cout << "E.g.:  web-crawler \"https://gcc.gnu.org/install/\" 5 3\n"  << std::endl;
    std::cout << "This platform supports " << 
        std::thread::hardware_concurrency() << " concurrent threads" << std::endl;
    std::cout << "See: include/thread_pool.h for the thread management\n"  << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        usage();
        return 1;
    }
    // In production code, do more error checking on the command line args. 
    // But, this is a demo project.
    const char* site_url = argv[1];
    const int num_threads = std::stoi(argv[2]);
    const int max_depth = argc >= 4 ? std::stoi(argv[3])
        : Web_crawler::unlimited_depth;
    return perform_crawler_test(site_url, num_threads, max_depth) ? 0 : 1;
}

