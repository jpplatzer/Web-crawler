/***
 # Released under the MIT License
 
 Copyright (C) 2024 Jeff Platzer <jeff@platzers.us>

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 ***/

#pragma once

#include <unordered_set>
#include <regex>
#include <list>
#include <mutex>
#include <web_common.h>

struct Deconstructed_url {
    std::string domain;
    std::string path;
    std::string page;
};

class Url_mgr {
public:
    Url_mgr(const Deconstructed_url& decon_url);
    static Deconstructed_url deconstruct_url(const Url_t& url, 
        bool allow_page_path_only = false);
    static Url_t make_page_path(const std::string& url_path, 
        const std::string& url_page);
    static Url_t make_full_url(const Url_t& site_domain, 
        const Url_t& url_path, const Url_t& url_page);
    const Url_t& site_domain() const {
        return decon_url_.domain;
    }
    Url_t make_full_url(const Page_path_t& path) const;
    Page_paths_t extract_page_paths(const Page_content_t& content, 
        const Page_path_t& parent_path) const;
    void update_page_paths(const Page_paths_t& page_paths);
    Opt_page_path_t pop_new_path();
    int num_new_paths();
private:
    const Deconstructed_url decon_url_;
    std::mutex mgr_mutex_;
    using Url_set_t = std::unordered_set<Url_t>;
    Url_set_t existing_paths_;
    std::list<Page_path_t> new_paths_;

    Url_t make_full_url_path(const Url_t& parents_path, 
        const Url_t& links_path) const;
    Opt_page_path_t make_child_page_path(const Url_t& url, 
        const Page_path_t& parents_page) const;
    bool is_child_page(const Url_t& links_domain,
        const Url_t& links_url_path) const;
};

