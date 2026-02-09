#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "../comm/httplib.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <thread>
#include <chrono>
#include <random>
#include <json/json.h>

// Configuration
const int START_PAGE = 1;
const int END_PAGE = 2;
const std::string OUTPUT_FILE = "insert_luogu_questions.sql";
const std::string USER_AGENT = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.114 Safari/537.36";

// Difficulty mapping
std::string get_difficulty(int diff_int) {
    if (diff_int <= 1) return "简单";
    if (diff_int <= 3) return "中等";
    return "困难";
}

// SQL Escape
std::string escape_sql(const std::string& text) {
    std::string result;
    result.reserve(text.length() * 2);
    for (char c : text) {
        if (c == '\'') result += "''";
        else if (c == '\\') result += "\\\\";
        else result += c;
    }
    return result;
}

// URL Decode
std::string url_decode(const std::string& encoded) {
    std::string decoded;
    decoded.reserve(encoded.length());
    for (size_t i = 0; i < encoded.length(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.length()) {
            std::string hex = encoded.substr(i + 1, 2);
            char c = static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
            decoded += c;
            i += 2;
        } else if (encoded[i] == '+') {
            decoded += ' ';
        } else {
            decoded += encoded[i];
        }
    }
    return decoded;
}

// Extract JSON data from HTML
Json::Value extract_data(const std::string& html) {
    // Try new format: <script id="lentille-context" type="application/json">{...}</script>
    // Note: Use [\s\S] to match any character including newlines
    std::regex pattern(R"(<script id="lentille-context" type="application/json">([\s\S]*?)</script>)");
    std::smatch match;
    if (std::regex_search(html, match, pattern)) {
        std::string json_str = match[1];
        Json::Value root;
        Json::Reader reader;
        if (reader.parse(json_str, root)) {
            return root;
        }
    }

    // Fallback to old format
    std::regex pattern_old(R"reg(decodeURIComponent\("([^"]+)"\))reg");
    if (std::regex_search(html, match, pattern_old)) {
        std::string encoded_json = match[1];
        std::string decoded_json = url_decode(encoded_json);
        Json::Value root;
        Json::Reader reader;
        if (reader.parse(decoded_json, root)) {
            return root;
        }
    }

    return Json::Value(); // Null value
}

// Helper to fetch URL via curl (fallback)
std::string fetch_url_via_curl(const std::string& url) {
    std::string cookie_file = "/tmp/luogu_crawler_cookies.txt";
    std::string command = "curl -s -L -c " + cookie_file + " -b " + cookie_file + " -A '" + USER_AGENT + "' '" + url + "'";
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return "";
    char buffer[128];
    std::string result = "";
    while (fgets(buffer, 128, pipe) != NULL) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

// Helper to fetch URL with retries
std::string fetch_url(const std::string& path) {
    std::string full_url = "https://www.luogu.com.cn" + path;
    // Try curl directly since httplib is having SSL issues
    return fetch_url_via_curl(full_url);
}

int main() {
    std::vector<std::string> sqls;
    std::cout << "Starting crawl from page " << START_PAGE << " to " << END_PAGE << "..." << std::endl;

    for (int page = START_PAGE; page <= END_PAGE; ++page) {
        std::string list_url = "/problem/list?page=" + std::to_string(page);
        std::cout << "Fetching list page " << page << ": " << list_url << std::endl;

        std::string html = fetch_url(list_url);
        if (html.empty()) {
            std::cerr << "Failed to fetch page " << page << std::endl;
            continue;
        }

        Json::Value data = extract_data(html);
        if (data.isNull()) {
            std::cerr << "Failed to extract data from page " << page << std::endl;
            continue;
        }

        Json::Value problems;
        if (data.isMember("data") && data["data"].isMember("problems")) {
            problems = data["data"]["problems"]["result"];
        } else if (data.isMember("currentData")) {
            problems = data["currentData"]["problems"]["result"];
        } else {
            std::cerr << "Unexpected JSON structure in page " << page << std::endl;
            continue;
        }

        for (const auto& p : problems) {
            std::string pid = p["pid"].asString();
            std::string title = p["title"].asString();
            int difficulty_int = p["difficulty"].asInt();

            std::cout << "  Processing " << pid << ": " << title << "..." << std::endl;

            std::string detail_url = "/problem/" + pid;
            std::string d_html = fetch_url(detail_url);
            if (d_html.empty()) {
                std::cerr << "    Failed to fetch details for " << pid << std::endl;
                continue;
            }

            Json::Value d_data = extract_data(d_html);
            if (d_data.isNull()) {
                std::cerr << "    Failed to extract data for " << pid << std::endl;
                continue;
            }

            Json::Value p_data;
            if (d_data.isMember("data") && d_data["data"].isMember("problem")) {
                p_data = d_data["data"]["problem"];
            } else if (d_data.isMember("currentData")) {
                p_data = d_data["currentData"]["problem"];
            } else {
                std::cerr << "    Unexpected JSON structure for " << pid << std::endl;
                continue;
            }

            Json::Value content_dict;
            if (p_data.isMember("content") && p_data["content"].isObject()) {
                content_dict = p_data["content"];
            }

            std::string desc = content_dict.get("description", "").asString();
            std::string background = content_dict.get("background", "").asString();
            std::string input_fmt = content_dict.get("formatI", "").asString();
            std::string output_fmt = content_dict.get("formatO", "").asString();
            std::string hint = content_dict.get("hint", "").asString();

            std::string full_desc = "";
            if (!background.empty()) full_desc += "## 题目背景\n" + background + "\n\n";
            full_desc += "## 题目描述\n" + desc + "\n\n";
            full_desc += "## 输入格式\n" + input_fmt + "\n\n";
            full_desc += "## 输出格式\n" + output_fmt + "\n\n";

            Json::Value samples = p_data["samples"];
            Json::Value formatted_samples(Json::arrayValue);
            full_desc += "## 样例\n\n";
            
            for (int i = 0; i < static_cast<int>(samples.size()); ++i) {
                std::string input_val = samples[i][0].asString();
                std::string output_val = samples[i][1].asString();
                
                Json::Value sample_obj;
                sample_obj["input"] = input_val;
                sample_obj["expect"] = output_val;
                formatted_samples.append(sample_obj);

                full_desc += "输入 #" + std::to_string(i + 1) + ":\n```\n" + input_val + "\n```\n";
                full_desc += "输出 #" + std::to_string(i + 1) + ":\n```\n" + output_val + "\n```\n\n";
            }

            if (!hint.empty()) full_desc += "## 说明/提示\n" + hint + "\n";

            Json::Value limits = p_data["limits"];
            int time_limit = 1;
            if (limits.isMember("time")) {
                Json::Value time_arr = limits["time"];
                int max_time = 0;
                for (const auto& t : time_arr) {
                    if (t.asInt() > max_time) max_time = t.asInt();
                }
                time_limit = std::max(1, max_time / 1000);
            }

            int mem_limit = 128000;
            if (limits.isMember("memory")) {
                Json::Value mem_arr = limits["memory"];
                int max_mem = 0;
                for (const auto& m : mem_arr) {
                    if (m.asInt() > max_mem) max_mem = m.asInt();
                }
                mem_limit = max_mem;
            }

            std::string star = get_difficulty(difficulty_int);
            Json::FastWriter writer;
            std::string tail_json = writer.write(formatted_samples);
            // Remove trailing newline added by FastWriter
            if (!tail_json.empty() && tail_json.back() == '\n') tail_json.pop_back();

            std::string sql = "INSERT INTO oj_questions (title, star, cpu_limit, mem_limit, description, header, tail_code, status) VALUES ('" + 
                pid + " " + escape_sql(title) + "', '" + 
                star + "', " + 
                std::to_string(time_limit) + ", " + 
                std::to_string(mem_limit) + ", '" + 
                escape_sql(full_desc) + "', '', '" + 
                escape_sql(tail_json) + "', 1);";
            
            sqls.push_back(sql);

            // Rate limiting: sleep 1.0 to 2.5 seconds
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(1000, 2500);
            std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
        }
    }

    std::ofstream out_file(OUTPUT_FILE);
    if (out_file.is_open()) {
        out_file << "-- Generated by luogu_crawler.cc" << std::endl;
        for (const auto& s : sqls) {
            out_file << s << std::endl;
        }
        std::cout << "Successfully generated " << sqls.size() << " questions in " << OUTPUT_FILE << std::endl;
    } else {
        std::cerr << "Failed to open output file: " << OUTPUT_FILE << std::endl;
    }

    return 0;
}
