/***
 # Released under the MIT License
 
 Copyright (C) 2024 Jeff Platzer <jeff@platzers.us>

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 ***/

#pragma once

#include <unordered_map>
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
    static Deconstructed_url deconstruct_url(const Url_t& url, 
        bool allow_page_link_only = false);
    static std::string make_page_path(const std::string& url_path, 
        const std::string& url_page);
    Url_mgr(const Deconstructed_url& decon_url);
    void update_page_links(const Page_links_t& page_links);
    Opt_Link_t pop_new_link();
    int num_new_links();
    std::string make_full_url(const std::string& page_path) const {
        return decon_url_.domain + page_path;
    }
    bool is_child_page(const std::string& domain, const std::string& path) const;

private:
    const Deconstructed_url decon_url_;
    std::mutex mgr_mutex_;
    using Url_map_t = std::unordered_map<Url_t, int>;
    Url_map_t existing_links_;
    std::list<Link_t> new_links_;
};

