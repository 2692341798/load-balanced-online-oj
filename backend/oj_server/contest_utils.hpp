#pragma once
#include <string>
#include <vector>
#include <ctime>
#include <algorithm>
#include <iostream>

struct ContestEntry {
    std::string name;
    std::string start_time_str;
    std::string link;
    time_t start_time;
    time_t end_time;
    std::string end_time_str;
    std::string status;
    std::string status_class;
};

inline void SortContests(std::vector<ContestEntry>& entries) {
    std::sort(entries.begin(), entries.end(), [](const ContestEntry &a, const ContestEntry &b) {
        if (a.start_time != b.start_time) {
            return a.start_time < b.start_time;
        }
        return a.end_time < b.end_time;
    });
}
