#pragma once
#include <string>
#include <vector>
#include <regex>

struct Contest {
    std::string name;
    std::string start_time;
    std::string link;
};

// Simple HTML parsing using Regex
inline std::vector<Contest> ParseContests(const std::string& html) {
    std::vector<Contest> contests;
    std::regex row_regex("<tr[^>]*data-contestId=\"(\\d+)\"[^>]*>([\\s\\S]*?)</tr>");
    auto words_begin = std::sregex_iterator(html.begin(), html.end(), row_regex);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string row_content = match[2];
        std::string contest_id = match[1];
        
        Contest c;
        c.link = "https://codeforces.com/contest/" + contest_id;
        
        std::regex td_regex("<td[^>]*>([\\s\\S]*?)</td>");
        auto td_begin = std::sregex_iterator(row_content.begin(), row_content.end(), td_regex);
        auto td_end = std::sregex_iterator();
        
        int idx = 0;
        for (std::sregex_iterator j = td_begin; j != td_end; ++j) {
            std::string cell = (*j)[1];
            std::regex tag_remove("<[^>]*>");
            std::string clean_text = std::regex_replace(cell, tag_remove, "");
            clean_text = std::regex_replace(clean_text, std::regex("^\\s+|\\s+$"), "");
            
            if (idx == 0) { 
                c.name = clean_text;
            } else if (idx == 2) { 
                 c.start_time = clean_text;
            }
            idx++;
        }
        
        if (!c.name.empty() && !c.start_time.empty()) {
             if (c.name.find("Enter") == std::string::npos && c.start_time.find("Before start") == std::string::npos) {
                 contests.push_back(c);
             }
        }
    }
    return contests;
}
