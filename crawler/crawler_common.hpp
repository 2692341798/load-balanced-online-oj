#pragma once
#include <string>
#include <vector>
#include <regex>
#include <sstream>
#include <iostream>
#include <map>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cmath>

struct Contest {
    std::string contest_id;
    std::string name;
    std::string start_time;
    std::string end_time;
    std::string link;
    std::string source = "Codeforces";
    std::string status;
};

struct CrawlerConfig {
    std::string user_agent = "ContestBot/2.0 (+https://yourdomain.com/bot; contact@yourdomain.com)";
    int min_interval_seconds = 7200; // 2 hours
    int max_interval_seconds = 14400; // 4 hours
    int max_concurrent_requests = 3;
    int max_retries = 5;
    std::string host = "codeforces.com";
    std::string robots_txt_path = "/robots.txt";
};

// Helper to parse Codeforces duration "2:00" or "05:00"
inline int ParseDuration(const std::string& dur) {
    int h = 0, m = 0;
    if (sscanf(dur.c_str(), "%d:%d", &h, &m) == 2) {
        return h * 3600 + m * 60;
    }
    return 7200; // Default 2 hours
}

// Helper to parse time string to MySQL datetime
// Codeforces format: "Feb/07/2026 17:35" (needs adjustment based on actual CF output)
// Note: Codeforces times in HTML are often in UTC+3 (Moscow) or UTC.
// Better to store as is or convert. For simplicity, assume server runs in same timezone or convert to UTC.
// Let's assume we store what we get, but standardized to YYYY-MM-DD HH:MM:SS.
inline std::string StandardizeTime(const std::string& raw) {
    // Implement parsing if needed. For now just return raw if it's already close, 
    // but usually it needs conversion.
    // Example: "Feb/07/2026 17:35" -> "2026-02-07 17:35:00"
    struct tm tm = {0};
    if (strptime(raw.c_str(), "%b/%d/%Y %H:%M", &tm)) {
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
        return std::string(buf);
    }
    return raw; // Fallback
}

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
        c.contest_id = contest_id;
        c.link = "https://codeforces.com/contest/" + contest_id;
        c.source = "Codeforces";
        
        std::regex td_regex("<td[^>]*>([\\s\\S]*?)</td>");
        auto td_begin = std::sregex_iterator(row_content.begin(), row_content.end(), td_regex);
        auto td_end = std::sregex_iterator();
        
        int idx = 0;
        int duration_seconds = 7200;
        std::string raw_start;

        for (std::sregex_iterator j = td_begin; j != td_end; ++j) {
            std::string cell = (*j)[1];
            std::regex tag_remove("<[^>]*>");
            std::string clean_text = std::regex_replace(cell, tag_remove, "");
            clean_text = std::regex_replace(clean_text, std::regex("^\\s+|\\s+$"), ""); // Trim
            
            if (idx == 0) { 
                c.name = clean_text;
            } else if (idx == 2) { 
                raw_start = clean_text;
                // Clean up "Enter »" or similar artifacts if present in column 2?
                // Usually column 2 is start time.
                // Remove extra spaces/newlines
                // Codeforces format: "Feb/07/2026 17:35"
                // Sometimes it has a link.
                // Let's assume clean_text is the time string.
            } else if (idx == 3) {
                // Duration
                duration_seconds = ParseDuration(clean_text);
            }
            idx++;
        }
        
        if (!c.name.empty() && !raw_start.empty()) {
             // Standardize start time
             c.start_time = StandardizeTime(raw_start);
             
             // Calculate End Time
             struct tm tm = {0};
             if (strptime(c.start_time.c_str(), "%Y-%m-%d %H:%M:%S", &tm)) {
                 time_t start_t = mktime(&tm);
                 time_t end_t = start_t + duration_seconds;
                 char buf[64];
                 struct tm *end_tm = localtime(&end_t);
                 strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", end_tm);
                 c.end_time = std::string(buf);
                 
                 // Determine Status
                 time_t now = time(nullptr);
                 if (now < start_t) c.status = "upcoming";
                 else if (now >= start_t && now <= end_t) c.status = "running";
                 else c.status = "ended";
             } else {
                 c.end_time = c.start_time; // Fallback
                 c.status = "upcoming"; // Safe default
             }

             if (c.name.find("Enter") == std::string::npos && raw_start.find("Before start") == std::string::npos) {
                 contests.push_back(c);
             }
        }
    }
    return contests;
}

class RobotsParser {
public:
    struct Rule {
        std::vector<std::string> disallowed_paths;
        int crawl_delay = -1; // -1 means undefined
    };

    void Parse(const std::string& robots_content, const std::string& my_user_agent) {
        std::stringstream ss(robots_content);
        std::string line;
        
        bool current_agent_matches = false;
        bool wildcard_match = false;
        
        // Temporary storage for wildcard rules
        Rule wildcard_rules;
        Rule specific_rules;
        bool has_specific_rules = false;

        // Simple state machine
        // We are looking for "User-agent: <my_agent>" or "User-agent: *"
        // If we find my_agent, we prioritize its rules.
        
        // This is a simplified parser. It assumes standard grouping.
        // Group: User-agent line(s) followed by Disallow/Allow/Crawl-delay lines.
        
        std::string current_group_agent = "";
        bool in_relevant_group = false;

        while (std::getline(ss, line)) {
            // Trim whitespace
            line = std::regex_replace(line, std::regex("^\\s+|\\s+$"), "");
            // Remove comments
            size_t comment_pos = line.find('#');
            if (comment_pos != std::string::npos) {
                line = line.substr(0, comment_pos);
            }
            if (line.empty()) continue;

            std::regex ua_regex("User-agent:\\s*(.*)", std::regex::icase);
            std::smatch match;
            if (std::regex_match(line, match, ua_regex)) {
                std::string agent = match[1];
                // Check if this new group applies to us
                if (agent == "*" ) {
                    in_relevant_group = true;
                    current_group_agent = "*";
                } else if (my_user_agent.find(agent) != std::string::npos) {
                     in_relevant_group = true;
                     current_group_agent = agent;
                     has_specific_rules = true;
                } else {
                    in_relevant_group = false;
                    current_group_agent = "";
                }
                continue;
            }

            if (in_relevant_group) {
                std::regex disallow_regex("Disallow:\\s*(.*)", std::regex::icase);
                std::regex delay_regex("Crawl-delay:\\s*(\\d+)", std::regex::icase);
                
                if (std::regex_match(line, match, disallow_regex)) {
                    std::string path = match[1];
                    if (!path.empty()) {
                        if (current_group_agent == "*") {
                            wildcard_rules.disallowed_paths.push_back(path);
                        } else {
                            specific_rules.disallowed_paths.push_back(path);
                        }
                    }
                } else if (std::regex_match(line, match, delay_regex)) {
                    int delay = std::stoi(match[1]);
                    if (current_group_agent == "*") {
                        wildcard_rules.crawl_delay = delay;
                    } else {
                        specific_rules.crawl_delay = delay;
                    }
                }
            }
        }

        if (has_specific_rules) {
            rules_ = specific_rules;
        } else {
            rules_ = wildcard_rules;
        }
    }

    bool IsAllowed(const std::string& path) const {
        for (const auto& disallow : rules_.disallowed_paths) {
            // Simple prefix matching
            if (path.find(disallow) == 0) return false;
        }
        return true;
    }

    int GetCrawlDelay() const {
        return rules_.crawl_delay;
    }

private:
    Rule rules_;
};

class BackoffStrategy {
public:
    BackoffStrategy(int max_retries = 5, int base_delay_ms = 3000) 
        : max_retries_(max_retries), base_delay_ms_(base_delay_ms) {}

    int GetWaitTime(int retry_count) {
        if (retry_count > max_retries_) return -1; // Give up
        // Exponential backoff: base * 2^retry
        return base_delay_ms_ * std::pow(2, retry_count);
    }
    
    int GetMaxRetries() const { return max_retries_; }

private:
    int max_retries_;
    int base_delay_ms_;
};
