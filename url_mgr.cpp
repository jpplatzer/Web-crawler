/***
 # Released under the MIT License
 
 Copyright (C) 2024 Jeff Platzer <jeff@platzers.us>

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 ***/

#include <iostream>
#include <url_mgr.h>

Deconstructed_url Url_mgr::deconstruct_url(const Url_t& url, bool allow_page_link_only) {
    Deconstructed_url durl{};
    static const std::regex domain_re{
        // (                 --- opt domain group ---                   ) (                           --- opt page group ---                   ) (  -- opt page group --  ) 
        R"(([Hh][Tt][Tt][Pp][Ss]?\://[a-zA-Z0-9\-]+(?:\.[a-zA-Z0-9\-]+)+)?(?:(((?:/[a-zA-Z0-9_:\-]+)*)/(?:\./)?([a-zA-Z0-9_:\-]+\.[a-zA-Z0-9]+))|((?:/[a-zA-Z0-9_:\-]+)+/?))?)"
    };
    static const std::regex page_re{
        R"((?:\./)?([a-zA-Z0-9_:\-]+\.[a-zA-Z0-9]+)|([a-zA-Z0-9_:\-]+/?))"
    };
    std::smatch domain_m; 
    if (regex_search(url, domain_m, domain_re)) {
        // int i = 0; for (auto mr: domain_m) std::cout << i++ << ": " << mr.str() << std::endl;
        durl.domain = domain_m[1];
        durl.path = domain_m[5].length() ? domain_m[5] : domain_m[3];
        durl.page = domain_m[4];
    }
    if (allow_page_link_only and durl.domain.empty() and
        durl.path.empty() and durl.page.empty()) {
        std::smatch page_m; 
        if (regex_search(url, page_m, page_re) and page_m.size() == 3) {
            durl.page = page_m[1].length() ? page_m[1] : page_m[2];
        }
    }
    return durl;
}

std::string Url_mgr::make_page_path(const std::string& url_path, const std::string& url_page) {
    return url_path + 
        (!url_page.empty() and ((url_path.empty() or url_path.back() != '/')) ? "/" : "") +
        url_page;
}

bool Url_mgr::is_child_page(const std::string& domain, const std::string& path) const {
    return ((domain.empty() || decon_url_.domain == domain) and
        (path.empty() || 
            (decon_url_.path.compare(0, decon_url_.path.size(), path) == 0 and
            (decon_url_.path.size() == path.size() || path[decon_url_.path.size()] == '/'))));
}

Url_mgr::Url_mgr(const Deconstructed_url& decon_url) : decon_url_(decon_url) {
    Page_links_t page_links{
        Link_t{Url_mgr::make_page_path(decon_url_.path, decon_url_.page), 1}};
    update_page_links(page_links);
}

void Url_mgr::update_page_links(const Page_links_t& page_links) {
    std::lock_guard lock(mgr_mutex_);
    for (const Link_t& link: page_links) {
        auto existing_result = existing_links_.emplace(link.page_path, link.depth);
        if (existing_result.second) { // The link is new
            new_links_.push_back(link);
        }
    }
}

Opt_Link_t Url_mgr::pop_new_link() {
    Opt_Link_t opt_link;
    std::lock_guard lock(mgr_mutex_);
    if (!new_links_.empty()) {
        opt_link = std::move(new_links_.front());
        new_links_.pop_front();
    }
    return opt_link;
}

int Url_mgr::num_new_links() {
    std::lock_guard lock(mgr_mutex_);
    return new_links_.size();
}
