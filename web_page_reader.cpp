/***
 # Released under the MIT License
 
 Copyright (C) 2024 Jeff Platzer <jeff@platzers.us>

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 ***/

#include <web_page_reader.h>
#include <common_macros.h>
#include <atomic>
#include <iostream>

extern "C" {
#include <curl/curl.h>
}

std::atomic_bool is_curl_initialized{false}; 

static void onetime_curl_init() {
    bool is_initialized = is_curl_initialized.exchange(true);
    if (!is_initialized) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }
}

class Curl_reader {
public:
    Curl_reader();
    ~Curl_reader();
    Read_Results_t read_page(const Url_t& url);

private: 
    CURL* handle_{nullptr};

    static size_t copy_curl_read_cb(void *contents, size_t sz, 
        size_t nmemb, void *ctx);
    void log_error(const char* err_text);
    bool setup_handle();
    Read_Results_t perform_read(const Url_t& url);
    int perform_curl_read();
};

Curl_reader::Curl_reader() {
    onetime_curl_init();
    handle_ = curl_easy_init();
}

Curl_reader::~Curl_reader() {
    if (handle_) {
        curl_easy_cleanup(handle_);
    }
}

size_t Curl_reader::copy_curl_read_cb(void *contents, size_t sz, 
    size_t nmemb, void *ctx) {
    size_t total_size = sz * nmemb;
    std::string* content_ptr = reinterpret_cast<std::string*>(ctx);
    content_ptr->append(reinterpret_cast<char*>(contents), total_size);
    return total_size;
}

void Curl_reader::log_error(const char* err_text) {
    std::cout << "curl error:" << err_text << std::endl;
}

Read_Results_t Curl_reader::read_page(const Url_t& url) {
    if (setup_handle()) {
        return perform_read(url);
    }
    else {
        return Read_Results_t{http_internal_error, ""};
    }
}

bool Curl_reader::setup_handle() {
    bool error = false;
;
    BEGIN_COND_LOOP
        IF_COND_ASSIGN_PROC_EXIT_LOOP(handle_ == nullptr, 
            error, log_error("curl init handle is null"))
        IF_COND_ASSIGN_PROC_EXIT_LOOP(curl_easy_setopt(handle_,
            CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS) != CURLE_OK, 
            error, log_error("curl setting HTTP version"))

        // For completeness
        IF_COND_ASSIGN_PROC_EXIT_LOOP(curl_easy_setopt(handle_,
            CURLOPT_ACCEPT_ENCODING, "text/html") != CURLE_OK, 
            error, log_error("curl setting CURLOPT_ACCEPT_ENCODING"))
        IF_COND_ASSIGN_PROC_EXIT_LOOP(curl_easy_setopt(handle_,
            CURLOPT_TIMEOUT, 5L) != CURLE_OK, 
            error, log_error("curl setting CURLOPT_TIMEOUT"))
        IF_COND_ASSIGN_PROC_EXIT_LOOP(curl_easy_setopt(handle_,
            CURLOPT_FOLLOWLOCATION, 1L) != CURLE_OK, 
            error, log_error("curl setting CURLOPT_FOLLOWLOCATION"))
        
        /*** Not supported in the curl api
        // only allow redirects to HTTP and HTTPS URLs 
        IF_COND_ASSIGN_PROC_EXIT_LOOP(curl_easy_setopt(handle_,
            CURLOPT_REDIR_PROTOCOLS_STR, "http,https") != CURLE_OK, 
            error, log_error("curl setting CURLOPT_REDIR_PROTOCOLS_STR"))
        ***/

        // Handle redirects
        IF_COND_ASSIGN_PROC_EXIT_LOOP(curl_easy_setopt(handle_,
            CURLOPT_AUTOREFERER, 1L) != CURLE_OK, 
            error, log_error("curl setting CURLOPT_AUTOREFERER"))
        IF_COND_ASSIGN_PROC_EXIT_LOOP(curl_easy_setopt(handle_,
            CURLOPT_MAXREDIRS, 10L) != CURLE_OK, 
            error, log_error("curl setting CURLOPT_MAXREDIRS"))

        // Limit page load time to 20s
        IF_COND_ASSIGN_PROC_EXIT_LOOP(curl_easy_setopt(handle_,
            CURLOPT_TIMEOUT_MS, 20000L) != CURLE_OK, 
            error, log_error("curl setting CURLOPT_TIMEOUT_MS"))

        // Connect fast or fail
        IF_COND_ASSIGN_PROC_EXIT_LOOP(curl_easy_setopt(handle_,
            CURLOPT_CONNECTTIMEOUT_MS, 4000L) != CURLE_OK, 
            error, log_error("curl setting CURLOPT_CONNECTTIMEOUT_MS"))

        // Limit loads to < 10 MB
        IF_COND_ASSIGN_PROC_EXIT_LOOP(curl_easy_setopt(handle_,
            CURLOPT_MAXFILESIZE_LARGE, (curl_off_t)10*1024*1024) != CURLE_OK, 
            error, log_error("curl setting CURLOPT_MAXFILESIZE_LARGE"))

        IF_COND_ASSIGN_PROC_EXIT_LOOP(curl_easy_setopt(handle_,
            CURLOPT_COOKIEFILE, "") != CURLE_OK, 
            error, log_error("curl setting CURLOPT_COOKIEFILE"))
        IF_COND_ASSIGN_PROC_EXIT_LOOP(curl_easy_setopt(handle_,
            CURLOPT_FILETIME, 1L) != CURLE_OK, 
            error, log_error("curl setting CURLOPT_FILETIME"))
        IF_COND_ASSIGN_PROC_EXIT_LOOP(curl_easy_setopt(handle_,
            CURLOPT_USERAGENT, "Mozilla/5.0") != CURLE_OK, 
            error, log_error("curl setting CURLOPT_USERAGENT"))
        IF_COND_ASSIGN_PROC_EXIT_LOOP(curl_easy_setopt(handle_,
            CURLOPT_EXPECT_100_TIMEOUT_MS, 0L) != CURLE_OK, 
            error, log_error("curl setting CURLOPT_EXPECT_100_TIMEOUT_MS"))
    END_COND_LOOP

    return !error;
}

Read_Results_t Curl_reader::perform_read(const Url_t& url) {
    Read_Results_t result{http_internal_error, ""};
    bool error = false;
    BEGIN_COND_LOOP
        IF_COND_ASSIGN_PROC_EXIT_LOOP(curl_easy_setopt(handle_,
            CURLOPT_URL, url.c_str()) != CURLE_OK, 
            error, log_error("curl setting CURLOPT_URL"))
        IF_COND_ASSIGN_PROC_EXIT_LOOP(curl_easy_setopt(handle_,
            CURLOPT_WRITEFUNCTION, copy_curl_read_cb) != CURLE_OK,
            error, log_error("curl setting CURLOPT_WRITEFUNCTION"))
        IF_COND_ASSIGN_PROC_EXIT_LOOP(curl_easy_setopt(handle_,
            CURLOPT_WRITEDATA, reinterpret_cast<void*>(&result.content)) != CURLE_OK, 
            error, log_error("curl setting CURLOPT_WRITEDATA"))
    END_COND_LOOP
    
    if (!error) {
        result.http_code = perform_curl_read();
    }

    return result;
}

int Curl_reader::perform_curl_read() {
    int http_code = http_internal_error;
    CURLcode curl_code = curl_easy_perform(handle_);
    switch (curl_code) {
        case CURLE_OK: {
            long curl_http_status;
            CURLcode res = curl_easy_getinfo(handle_, 
                CURLINFO_RESPONSE_CODE, &curl_http_status);
            http_code = (res == CURLE_OK) ? static_cast<int>(curl_http_status)
                : http_internal_error;
            break;
        }
        case CURLE_OPERATION_TIMEDOUT: {
            http_code = http_request_timeout;
            break;
        }
        default: {
            http_code = http_internal_error;
            break;
        }
    }
    return http_code;
}


Read_Results_t Web_page_reader::read_page(const std::string& url) {
    Curl_reader curl_reader;
    return curl_reader.read_page(url);
}