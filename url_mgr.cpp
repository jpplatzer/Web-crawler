/***
 # Released under the MIT License
 
 Copyright (C) 2024 Jeff Platzer <jeff@platzers.us>

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 ***/

#include <url_mgr.h>

#include <iostream>
void print_matches(const char* label, const std::smatch& m) {
    int i = 0; 
    std::cout << label << std::endl;
    for (auto mr: m) { 
        std::cout << i++ << ": " << mr.str() << std::endl;
    }
};

Url_mgr::Url_mgr(const Deconstructed_url& decon_url) : decon_url_(decon_url) {
    Page_paths_t page_paths{Page_path_t{decon_url.path, decon_url.page, 1}};
    update_page_paths(page_paths);
}

Deconstructed_url Url_mgr::deconstruct_url(const Url_t& url, bool allow_page_path_only) {
    Deconstructed_url durl{};
    static const std::regex domain_re{
        R"(^[Hh][Tt][Tt][Pp][Ss]?\://[a-zA-Z0-9\-]+(?:\.[a-zA-Z0-9\-]+)+)"
    };
    static const std::regex page_re{
        //  1. /                                  3. if extension then page, else end of path                                              4. extension  
        //       2. begin path / means abs path, else relative        4. extension               
        R"(^(/)?((?:[a-zA-Z0-9%_:\-]+/)+)?(?:\./)?([a-zA-Z0-9%_:\-]+)?(\.[Hh][Tt][Mm][Ll]?)?$)"
    };
    std::smatch domain_m; 
    if (std::regex_search(url, domain_m, domain_re)) {
        durl.domain = domain_m[0];
        // print_matches("Url matches: ", domain_m);
    }
    if (!durl.domain.empty() or allow_page_path_only) {
        std::smatch page_m;
        auto beg_iter = url.begin() + durl.domain.size();
        if (std::regex_search(beg_iter, url.end(), page_m, page_re) and page_m.size() == 5) {
            durl.path = page_m[1];
            durl.path.append(page_m[2]);
            if (page_m[4].length() == 0) {
                durl.path.append(page_m[3]);
            }
            else {
                durl.page = page_m[3]; 
                durl.page.append(page_m[4]);
            }
        }
        // print_matches("Page matches: ", page_m);
    }
    return durl;
}

std::string Url_mgr::make_page_path(const std::string& url_path, const std::string& url_page) {
    return url_path + 
        (!url_page.empty() and ((url_path.empty() or url_path.back() != '/')) ? "/" : "") +
        url_page;
}

Url_t Url_mgr::make_full_url_path(const Url_t& parents_path, const Url_t& links_path) const {
     Url_t url_path = links_path.empty() ? parents_path
        : links_path.front() == '/' ? links_path
            : parents_path + 
              ((!parents_path.empty() and parents_path.back() != '/') ? "/" : "") + 
              links_path;
    return url_path;
}

Url_t Url_mgr::make_full_url(const Url_t& site_domain, 
    const Url_t& url_path, const Url_t& url_page) {
    return site_domain + make_page_path(url_path, url_page);
}

Url_t Url_mgr::make_full_url(const Page_path_t& page_path) const {
    return make_full_url(decon_url_.domain, page_path.path, page_path.page);
}

Opt_page_path_t Url_mgr::make_child_page_path(const Url_t& url, 
    const Page_path_t& parents_page) const {    
    Opt_page_path_t opt_page_path;           
    Deconstructed_url decon_url = Url_mgr::deconstruct_url(url, true);
    if (!decon_url.path.empty() || !decon_url.page.empty()) {
        Url_t full_url_path = make_full_url_path(parents_page.path, decon_url.path);
        if (is_child_page(decon_url.domain, full_url_path)) {
            opt_page_path = Page_path_t{full_url_path, decon_url.page, parents_page.depth + 1};
        }
    }
    return opt_page_path;
}

bool Url_mgr::is_child_page(const Url_t& links_domain, const Url_t& links_url_path) const {
    bool is_child = ((links_domain.empty() || links_domain == decon_url_.domain) and
        (decon_url_.path.empty() || 
            (links_url_path.find(decon_url_.path) == 0 and
                (links_url_path.size() == decon_url_.path.size() || 
                decon_url_.path.back() == '/' ||
                links_url_path[decon_url_.path.size()] == '/'))));
    return is_child;
}

Page_paths_t Url_mgr::extract_page_paths(const Page_content_t& content, 
    const Page_path_t& parent_path) const {
    Page_paths_t paths;
    // Use string search to find hrefs instead of regex because regex is painfully slow for large docs
    size_t begin_pos{0}, end_pos{0};
    for (;;) {
        begin_pos = content.find("<a", begin_pos);
        if (begin_pos == std::string::npos) break;
        end_pos = content.find(">", begin_pos);
        if (end_pos == std::string::npos) break;  // Malformed
        begin_pos = content.find("href", begin_pos);
        if (begin_pos != std::string::npos and begin_pos < end_pos) {
            begin_pos = content.find("\"", begin_pos);
            if (begin_pos == std::string::npos or begin_pos > end_pos) break; // Malformed
            ++begin_pos;
            end_pos = content.find_first_of("\"#", begin_pos);
            if (end_pos == std::string::npos) break; // Malformed
            if (end_pos > begin_pos) {
                Url_t url{content, begin_pos, end_pos-begin_pos};
                // std::cout << "Found links url: " << url << std::endl;
                Opt_page_path_t opt_path = make_child_page_path(url, parent_path);
                if (opt_path) {
                    paths.push_back(*opt_path);
                    // std::cout << ">>>>>>>>>>>>>>>>>> Adding links path: " << opt_path->path <<
                    //     ", page: " << opt_path->page << std::endl;
                }
                else {
                    // std::cout << "!!!!!!!!!!!!!!!!!! Page links NOT added " << std::endl;
                }
                // std::cout << "---------------------------------------------------" << std::endl;
            }
        }
        begin_pos = end_pos + 1;
    }
    return paths;
}

void Url_mgr::update_page_paths(const Page_paths_t& page_paths) {
    std::lock_guard lock(mgr_mutex_);
    for (const Page_path_t& page_path: page_paths) {
        Url_t page_path_str = make_page_path(page_path.path, page_path.page);
        auto existing_result = existing_paths_.emplace(page_path_str);
        if (existing_result.second) { // The path is new
            new_paths_.push_back(page_path);
        }
    }
}

Opt_page_path_t Url_mgr::pop_new_path() {
    Opt_page_path_t opt_path;
    std::lock_guard lock(mgr_mutex_);
    if (!new_paths_.empty()) {
        opt_path = std::move(new_paths_.front());
        new_paths_.pop_front();
    }
    return opt_path;
}

int Url_mgr::num_new_paths() {
    std::lock_guard lock(mgr_mutex_);
    return new_paths_.size();
}
