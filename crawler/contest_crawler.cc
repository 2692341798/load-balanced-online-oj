#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "../comm/httplib.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <regex>
#include <thread>
#include <chrono>
#include <json/json.h>

// #define ENABLE_REDIS
#ifdef ENABLE_REDIS
#include <hiredis/hiredis.h>
#endif

#include <sys/stat.h>
#include <ctime>
#include "crawler_common.hpp"

// Configuration
const std::string CF_HOST = "codeforces.com";
const std::string CF_PATH = "/contests";
const std::string DATA_FILE = "../data/contests.json";
const std::string LOG_FILE = "../logs/crawler.log";
const std::string REDIS_HOST = "127.0.0.1";
const int REDIS_PORT = 6379;
const int REDIS_TTL = 86400; // 24 hours

void Log(const std::string& message) {
    std::ofstream log_file(LOG_FILE, std::ios::app);
    std::time_t now = std::time(nullptr);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    log_file << "[" << buf << "] " << message << std::endl;
    std::cout << "[" << buf << "] " << message << std::endl;
}

void WriteToRedis(const std::string& json_data) {
#ifdef ENABLE_REDIS
    redisContext *c = redisConnect(REDIS_HOST.c_str(), REDIS_PORT);
    if (c == NULL || c->err) {
        if (c) {
            Log("Redis Connection Error: " + std::string(c->errstr));
            redisFree(c);
        } else {
            Log("Redis Connection Error: Can't allocate redis context");
        }
        return;
    }

    redisReply *reply = (redisReply*)redisCommand(c, "SET contests %s EX %d", json_data.c_str(), REDIS_TTL);
    if (reply == NULL) {
        Log("Redis Command Error");
        redisFree(c);
        return;
    }
    
    freeReplyObject(reply);
    redisFree(c);
    Log("Synced to Redis successfully.");
#else
    // Log("Redis sync skipped (ENABLE_REDIS not defined).");
#endif
}

std::string FetchHTML() {
    httplib::Client cli(CF_HOST.c_str());
    cli.set_connection_timeout(10);
    cli.set_read_timeout(10);
    cli.set_follow_location(true);
    
    httplib::Headers headers = {
        {"User-Agent", "ContestBot/1.0 (+https://yourdomain.com/bot)"}
    };

    int retries = 2;
    while (retries >= 0) {
        auto res = cli.Get(CF_PATH.c_str(), headers);
        if (res && res->status == 200) {
            return res->body;
        }
        
        Log("Request failed. Retrying...");
        retries--;
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    return "";
}

int main() {
    Log("Crawler started.");
    
    std::string html = FetchHTML();
    if (html.empty()) {
        Log("Failed to fetch contests.");
        return 1;
    }

    Log("Fetch successful. Parsing...");

    std::vector<Contest> contests = ParseContests(html);
    Log("Parsed " + std::to_string(contests.size()) + " contests.");

    Json::Value root;
    Json::Value arr(Json::arrayValue);
    
    for (const auto& c : contests) {
        Json::Value item;
        item["name"] = c.name;
        item["start_time"] = c.start_time;
        item["link"] = c.link;
        arr.append(item);
    }
    root["contests"] = arr;
    root["updated_at"] = (Json::UInt64)time(nullptr);

    Json::FastWriter writer;
    std::string json_output = writer.write(root);

    std::ofstream file(DATA_FILE);
    if (file.is_open()) {
        file << json_output;
        file.close();
        Log("Saved to " + DATA_FILE);
    } else {
        Log("Failed to open " + DATA_FILE + " for writing.");
    }

    WriteToRedis(json_output);

    Log("Crawler finished.");
    return 0;
}
