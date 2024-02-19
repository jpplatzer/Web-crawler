#include <string>
#include <regex>

// From: https://www.freecodecamp.org/news/how-to-write-a-regular-expression-for-a-url/
// R"/(https:\/\/www\.|http:\/\/www\.|https:\/\/|http:\/\/)?[a-zA-Z]{2,}(\.[a-zA-Z]{2,})(\.[a-zA-Z]{2,})?\/[a-zA-Z0-9]{2,}|((https:\/\/www\.|http:\/\/www\.|https:\/\/|http:\/\/)?[a-zA-Z]{2,}(\.[a-zA-Z]{2,})(\.[a-zA-Z]{2,})?)|(https:\/\/www\.|http:\/\/www\.|https:\/\/|http:\/\/)?[a-zA-Z0-9]{2,}\.[a-zA-Z0-9]{2,}\.[a-zA-Z0-9]{2,}(\.[a-zA-Z0-9]{2,})?/g"

std::string pattern{
R"(//[a-zA-Z](?:.[a-zA-Z])+(?:/[a-zA-Z0-9]+)*(/[a-zA-Z0-9]+.[a-zA-Z0-9]+)|/?$)"
};

void Web_crawler::process_page_test(const Link_t& link) {
    const int max_num = 1000;
    auto num = std::stoul(link.page_path);
    std::cout << "process_page for " << link.page_path << 
        ", depth " << link.depth << std::endl;
    if (num < max_num) {
        Page_links_t links;
        int num_links = (num % 2) + 2;
        for (int i = 1; i < num_links; ++i) {
            links.push_back(Link_t{std::to_string(num + i), link.depth+1});
        }
        if (link.depth < max_depth_) {
            url_mgr_ptr_->update_page_links(links);
        }
    }
}

