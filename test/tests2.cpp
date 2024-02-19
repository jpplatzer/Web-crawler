#include <iostream>
#include <algorithm>
#include <regex>
#include <web_page_reader.h>
#include <web_crawler.h>

class Dummy_content_processor : public Page_content_processor {
public:
    void process_page_content(const Url_t& page_url, 
        const Page_links_t& page_links, const Page_content_t& page_content) override {
        std::cout << "process_page_content for " << page_url <<
            " has " << page_content.size() << " bytes" <<
            " and has " << page_links.size() << " links " << std::endl;
    }
    void final() override {
        std::cout << "done processing pages" << std::endl;
    }
};

void test_web_crawler(const Url_t& base_url) {
    Dummy_content_processor dcp;
    Web_crawler web_crawler(4);
    web_crawler.crawl(base_url, &dcp);
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

using Http_success_or_error = Success_or_error<Http_code>;

Http_success_or_error error_test_fcn(bool is_error, Http_code code) {
    return is_error ? Http_success_or_error{code} : Http_success_or_error{};
}

void test_error(bool is_error, Http_code code) {
    Http_success_or_error result = error_test_fcn(is_error, code);
    std::cout << "Test error " << std::boolalpha << is_error <<
        " for " << code << std::endl;
    if (result) {
        std::cout << "Is not error" << std::endl;
    }
    else {
        std::cout << "Is error for " << result.error() << std::endl;
    }
}

void test_errors() {
   test_error(false, http_ok);
   test_error(true, http_internal_error);
   test_error(false, http_not_found);
   test_error(true, http_request_timeout);
}


/*** Avoid slow compilation
 
void test_http_regex(const std::string& test_str) {
std::regex re{
    // (            --- domain group ---               )( --opt path group-- ) (        -- opt page group --       ) 
    R"(([a-zA-Z]+://[a-zA-Z0-9\-]+(?:\.[a-zA-Z0-9\-]+)+)((?:/[a-zA-Z0-9\-]+)+)?(?:/([a-zA-Z0-9\-]+\.[a-zA-Z0-9\-]+))?/?)"
};
std::smatch sm;
bool matched = std::regex_match (test_str, sm, re);
std::cout << test_str <<
    ", matched: " << std::boolalpha << matched <<
    ", match count: " << sm.size() <<
    ", matches: " << std::endl;
for (auto match: sm) {
    std::cout << match << std::endl;
}
}

void test_regexes() {
    test_http_regex("https://gcc.gnu.org");
    test_http_regex("https://gcc.gnu.org/install");
    test_http_regex("https://gcc.gnu.org/install/foo/bar");
    test_http_regex("https://gcc.gnu.org/index.html");
    test_http_regex("https://gcc-foo.com");
    test_http_regex("https://gcc.gnu.org/foo/index.html");
    test_http_regex("https://gcc.gnu.org/");
    test_http_regex("https://gcc-foo");
    test_http_regex("https://gcc");
}

***/

int main() {
    // const std::string base_url{"https://gcc.gnu.org/onlinedocs/gcc/"};
    const std::string base_url{"https://gcc.gnu.org/install"};
    test_web_crawler(base_url);
    // test_page_reader(base_url);
    return 0;
}

