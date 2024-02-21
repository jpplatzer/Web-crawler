/***
 # Released under the MIT License
 
 Copyright (C) 2024 Jeff Platzer <jeff@platzers.us>

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 ***/

#pragma once

#include <string>
#include <vector>
#include <optional>
#include <utility>

using Url_t = std::string;

struct Page_path_t {
    Url_t path;
    Url_t page;
    int depth;
};

using Page_content_t = std::string;
using Page_paths_t = std::vector<Page_path_t>;
using Opt_page_path_t = std::optional<Page_path_t>;

// Supported HTTP codes
enum Http_code { 
    http_ok = 200,
    http_bad_request = 400,
    http_unauthorized = 401,
    http_forbidden = 403,
    http_not_found = 404,
    http_request_timeout = 408,
    http_internal_error = 500
};

// The Err_t error class/type must support an empty and a copy ctor
template <class Err_t>
class Success_or_error {
public:
    Success_or_error() {}
    Success_or_error(Err_t err_val) : is_error_(true), err_val_(err_val) {}
    operator bool() {
        return !is_error_;
    }
    const Err_t& error() {
        return err_val_;
    }
private:
    bool is_error_{false};
    Err_t err_val_;
};

