/***
 # Released under the MIT License
 
 Copyright (C) 2024 Jeff Platzer <jeff@platzers.us>

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 ***/

#include <iostream>
#include <algorithm>
#include <regex>
#include <web_page_reader.h>
#include <web_crawler.h>


void test_url_deconstruction(const Url_t& url) {
    std::cout << "test_url_deconstruction for " << url << std::endl;
    Deconstructed_url decon_url = Url_mgr::deconstruct_url(url, true);
    std::cout << "domain: " << decon_url.domain <<    
        ", path: " << decon_url.path <<    
        ", page: " << decon_url.page << std::endl;
}

void url_deconstruction_tests() {
    test_url_deconstruction("https://gcc.gnu.org");
    test_url_deconstruction("https://gcc.gnu.org/index.html");
    test_url_deconstruction("https://gcc.gnu.org/onlinedocs/gcc/");
    test_url_deconstruction("https://gcc.gnu.org/install");
    test_url_deconstruction("https://gcc.gnu.org/install/");
    test_url_deconstruction("https://gcc.gnu.org/install/index.html");
    test_url_deconstruction("https://gcc.gnu.org/install/foo/bar/index.html");
    test_url_deconstruction("https://en.cppreference.com/w");
    test_url_deconstruction("https://en.cppreference.com/w/cpp");
    test_url_deconstruction("https://gcc.gnu.org/./index.html");
    test_url_deconstruction("index.html");
    test_url_deconstruction("./index.html");
    test_url_deconstruction("https://foo-bad");
    test_url_deconstruction("ftps://en.cppreference.com/foo.bar");
    test_url_deconstruction("https://foo-bad");
}


void test_page_reader(const Url_t& url) {
    Web_page_reader reader;
    Read_Results_t results = reader.read_page(url);
    std::cout << "Page reader for " << url <<
        ", status " << results.http_code << 
        ", size " << results.content.size() << 
        // ", content:\n" << results.content << 
        std::endl;
}

class Dummy_content_processor : public Page_content_processor {
public:
    void process_page_content(const Url_t& page_url, int http_code,
        const Page_links_t& page_links, const Page_content_t& page_content) override {
        std::cout << "process_page_content for " << page_url <<
            " HTTP code " << http_code <<
            " has " << page_content.size() << " bytes" <<
            " and has " << page_links.size() << " links " << std::endl;
    }
    void final() override {
        std::cout << "done processing pages" << std::endl;
    }
};

void perform_crawler_test(const Url_t& site_url) {
    std::cout << "Peform web crawler test for: " << site_url << std::endl;
    Dummy_content_processor dcp;
    Web_crawler web_crawler(4);
    Crawl_result_t result = web_crawler.crawl(site_url, &dcp);
    if (!result) {
        std::cout << "Error crawling website: " << result.error().err_text << std::endl;
    }
}

void test_web_crawler() {
    perform_crawler_test("https://gcc.gnu.org");
    perform_crawler_test("https://gcc.gnu.org/onlinedocs/gcc/");
    perform_crawler_test("https://gcc.gnu.org/install");
    perform_crawler_test("https://gcc.gnu.org/install/");
    perform_crawler_test("https://gcc.gnu.org/install/index.html");
    perform_crawler_test("https://en.cppreference.com/w");
    perform_crawler_test("https://en.cppreference.com/w/cpp");
    perform_crawler_test("https://en.cppreference.com/w/cpp/");
    perform_crawler_test("https://foo-bad");
}

int main() {
    // url_deconstruction_tests();
    // test_page_reader(site_url);
    test_web_crawler();
    return 0;
}

