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
#include <mysql/mysql.h>
#include "crawler_common.hpp"

// Redis Config (Global or Static for now, could be in Config)
const std::string REDIS_HOST = "127.0.0.1";
const int REDIS_PORT = 6379;
const int REDIS_TTL = 86400; // 24 hours
const std::string LOG_FILE = "../logs/crawler.log";
const std::string DATA_FILE = "../data/contests.json";

#include <sys/file.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <cstdlib>

// ... (existing includes)

class ContestCrawler {
public:
    ContestCrawler(const CrawlerConfig& config) : config_(config) {
        // Ensure log directory exists (simple check)
    }

    void Run() {
        // Implement Singleton Lock using flock
        int lock_fd = open("/tmp/contest_crawler.lock", O_RDWR | O_CREAT, 0666);
        if (lock_fd < 0) {
            Log("Failed to open lock file. Exiting.");
            return;
        }
        
        if (flock(lock_fd, LOCK_EX | LOCK_NB) < 0) {
            Log("Another instance is running. Exiting.");
            close(lock_fd);
            return;
        }

        Log("Crawler service started with 30min interval.");
        
        // Initial robots.txt fetch
        FetchRobotsTxt("https://codeforces.com", cf_robots_parser_);
        FetchRobotsTxt("https://leetcode.cn", lc_robots_parser_);

        while (true) {
            auto start_time = std::chrono::steady_clock::now();
            
            PerformCodeforcesCrawl();
            PerformLeetCodeCrawl();

            auto end_time = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
            
            // Calculate sleep time
            int sleep_time = config_.min_interval_seconds;
            
            // Respect robots.txt Crawl-delay (use max of both)
            int cf_delay = cf_robots_parser_.GetCrawlDelay();
            int lc_delay = lc_robots_parser_.GetCrawlDelay();
            int robot_delay = std::max(cf_delay, lc_delay);
            
            if (robot_delay > sleep_time) {
                sleep_time = robot_delay;
                Log("Adjusting interval to " + std::to_string(sleep_time) + "s based on robots.txt Crawl-delay.");
            }

            // Add randomness to avoid fingerprinting
            int range = config_.max_interval_seconds - config_.min_interval_seconds;
            if (range > 0) {
                sleep_time += rand() % range;
            }
            
            // Deduct execution time
            if (duration < sleep_time) {
                sleep_time -= duration;
            } else {
                sleep_time = 0; // Run immediately if it took too long
            }

            Log("Sleeping for " + std::to_string(sleep_time) + " seconds before next crawl.");
            std::this_thread::sleep_for(std::chrono::seconds(sleep_time));
            
            // Refresh robots.txt
            FetchRobotsTxt("https://codeforces.com", cf_robots_parser_);
            FetchRobotsTxt("https://leetcode.cn", lc_robots_parser_);
        }
        
        // Release lock (though OS does it on exit)
        flock(lock_fd, LOCK_UN);
        close(lock_fd);
    }

private:
    CrawlerConfig config_;
    RobotsParser cf_robots_parser_;
    RobotsParser lc_robots_parser_;

    void Log(const std::string& message) {
        std::ofstream log_file(LOG_FILE, std::ios::app);
        std::time_t now = std::time(nullptr);
        char buf[100];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        
        std::string log_entry = "[" + std::string(buf) + "] " + message;
        
        if (log_file.is_open()) {
            log_file << log_entry << std::endl;
        }
        std::cout << log_entry << std::endl;
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

    void FetchRobotsTxt(const std::string& host, RobotsParser& parser) {
        Log("Fetching robots.txt from " + host + "...");
        httplib::Client cli(host.c_str());
        cli.set_connection_timeout(10);
        cli.set_read_timeout(10);
        cli.set_follow_location(true);

        auto res = cli.Get("/robots.txt");
        if (res && res->status == 200) {
            parser.Parse(res->body, config_.user_agent);
            Log("Robots.txt parsed successfully for " + host);
        } else {
            Log("Failed to fetch robots.txt for " + host + ". Assuming no restrictions.");
        }
    }

    std::string FetchHTML(const std::string& host, const std::string& path, RobotsParser& parser) {
        if (!parser.IsAllowed(path)) {
            Log("Access to " + path + " denied by robots.txt (" + host + ")");
            return "";
        }

        httplib::Client cli(host.c_str());
        cli.set_connection_timeout(10);
        cli.set_read_timeout(10);
        cli.set_follow_location(true);
        cli.enable_server_certificate_verification(false);
        
        httplib::Headers headers = {
            {"User-Agent", config_.user_agent}
        };

        BackoffStrategy backoff(config_.max_retries);
        int retries = 0;
        
        while (retries <= backoff.GetMaxRetries()) {
            auto start = std::chrono::steady_clock::now();
            auto res = cli.Get(path.c_str(), headers);
            auto end = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            if (res) {
                if (res->status == 200) {
                    Log("Request to " + host + path + " success. Status: 200. Time: " + std::to_string(elapsed) + "ms");
                    return res->body;
                } else if (res->status == 429 || res->status == 503) {
                    int wait_time = backoff.GetWaitTime(retries);
                    Log("Rate limited (" + std::to_string(res->status) + "). Retrying in " + std::to_string(wait_time) + "ms");
                    std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
                    retries++;
                } else {
                    Log("Request failed with status " + std::to_string(res->status));
                    if (res->status >= 500) {
                         int wait_time = backoff.GetWaitTime(retries);
                         std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
                         retries++;
                    } else {
                        // Client error (404, etc.), do not retry
                        break; 
                    }
                }
            } else {
                Log("Request failed (network error).");
                 int wait_time = backoff.GetWaitTime(retries);
                 std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
                 retries++;
            }
        }
        
        Log("Max retries reached for " + host + path);
        return "";
    }
    
    std::string FetchGraphQL(const std::string& host, const std::string& query, RobotsParser& parser) {
         if (!parser.IsAllowed("/graphql")) {
            // Check if explicitly disallowed
         }

        httplib::Client cli(host.c_str());
        cli.set_connection_timeout(10);
        cli.set_read_timeout(10);
        cli.set_follow_location(true);
        cli.enable_server_certificate_verification(false); // Disable cert verification for simplicity, though not ideal for prod
        
        httplib::Headers headers = {
            {"User-Agent", config_.user_agent},
            // Content-Type is handled by Post arguments
        };
        
        Json::Value req_body;
        req_body["query"] = query;
        Json::FastWriter writer;
        std::string body = writer.write(req_body);

        BackoffStrategy backoff(config_.max_retries);
        int retries = 0;
        
        while (retries <= backoff.GetMaxRetries()) {
            auto start = std::chrono::steady_clock::now();
            // Pass content-type explicitly
            auto res = cli.Post("/graphql", headers, body, "application/json");
            auto end = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            if (res) {
                if (res->status == 200) {
                    Log("GraphQL Request to " + host + " success. Status: 200. Time: " + std::to_string(elapsed) + "ms");
                    return res->body;
                } else if (res->status == 429 || res->status == 503) {
                    int wait_time = backoff.GetWaitTime(retries);
                    Log("Rate limited (" + std::to_string(res->status) + "). Retrying in " + std::to_string(wait_time) + "ms");
                    std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
                    retries++;
                } else {
                    Log("GraphQL Request failed with status " + std::to_string(res->status));
                    if (res->status >= 500) {
                         int wait_time = backoff.GetWaitTime(retries);
                         std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
                         retries++;
                    } else {
                        break; 
                    }
                }
            } else {
                Log("GraphQL Request failed (network error).");
                 int wait_time = backoff.GetWaitTime(retries);
                 std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
                 retries++;
            }
        }
        return "";
    }

    void SaveToDB(const std::vector<Contest>& contests) {
        MYSQL *conn = mysql_init(NULL);
        if (conn == NULL) {
            Log("mysql_init failed");
            return;
        }

        const char* db_host = std::getenv("MYSQL_HOST") ? std::getenv("MYSQL_HOST") : "127.0.0.1";
        const char* db_user = std::getenv("MYSQL_USER") ? std::getenv("MYSQL_USER") : "oj_client";
        const char* db_pass = std::getenv("MYSQL_PASSWORD") ? std::getenv("MYSQL_PASSWORD") : "123456";
        const char* db_name = std::getenv("MYSQL_DB") ? std::getenv("MYSQL_DB") : "oj";
        int db_port = std::getenv("MYSQL_PORT") ? std::stoi(std::getenv("MYSQL_PORT")) : 3306;

        if (mysql_real_connect(conn, db_host, db_user, db_pass, db_name, db_port, NULL, 0) == NULL) {
            Log("mysql_real_connect failed: " + std::string(mysql_error(conn)));
            mysql_close(conn);
            return;
        }
        
        mysql_set_character_set(conn, "utf8mb4");

        int count = 0;
        for(const auto& c : contests) {
            if (count >= 20) break; 

            char* name_esc = new char[c.name.length() * 2 + 1];
            mysql_real_escape_string(conn, name_esc, c.name.c_str(), c.name.length());
            
            char* link_esc = new char[c.link.length() * 2 + 1];
            mysql_real_escape_string(conn, link_esc, c.link.c_str(), c.link.length());

            char* cid_esc = new char[c.contest_id.length() * 2 + 1];
            mysql_real_escape_string(conn, cid_esc, c.contest_id.c_str(), c.contest_id.length());

            char* source_esc = new char[c.source.length() * 2 + 1];
            mysql_real_escape_string(conn, source_esc, c.source.c_str(), c.source.length());

            std::string sql = "INSERT INTO contests (contest_id, name, start_time, end_time, link, source, status, last_crawl_time) VALUES ('" 
                              + std::string(cid_esc) + "', '" 
                              + std::string(name_esc) + "', '" 
                              + c.start_time + "', '" 
                              + c.end_time + "', '" 
                              + std::string(link_esc) + "', '" 
                              + std::string(source_esc) + "', '" 
                              + c.status + "', NOW()) "
                              "ON DUPLICATE KEY UPDATE "
                              "name='" + std::string(name_esc) + "', "
                              "start_time='" + c.start_time + "', "
                              "end_time='" + c.end_time + "', "
                              "status='" + c.status + "', "
                              "last_crawl_time=NOW()";
            
            if (mysql_query(conn, sql.c_str())) {
                Log("Insert/Update failed for " + c.contest_id + ": " + mysql_error(conn));
            }

            delete[] name_esc;
            delete[] link_esc;
            delete[] cid_esc;
            delete[] source_esc;
            count++;
        }
        
        mysql_close(conn);
        Log("Database sync completed for " + std::to_string(count) + " items.");
    }

    void InvalidateRedisCache() {
#ifdef ENABLE_REDIS
        redisContext *c = redisConnect(REDIS_HOST.c_str(), REDIS_PORT);
        if (c == NULL || c->err) {
            if (c) redisFree(c);
            return;
        }
        
        redisReply *reply = (redisReply*)redisCommand(c, "KEYS contest:page:*");
        if (reply->type == REDIS_REPLY_ARRAY) {
            for (size_t i = 0; i < reply->elements; i++) {
                redisCommand(c, "DEL %s", reply->element[i]->str);
            }
        }
        freeReplyObject(reply);
        redisFree(c);
        Log("Redis cache invalidated.");
#endif
    }

    void PerformCodeforcesCrawl() {
        Log("Starting Codeforces crawl cycle...");
        // Use API
        std::string json_resp = FetchHTML("https://codeforces.com", "/api/contest.list?gym=false", cf_robots_parser_);
        
        if (json_resp.empty()) {
            Log("Failed to fetch Codeforces contests.");
            return;
        }

        Log("Fetch successful. Parsing Codeforces...");
        std::vector<Contest> contests = ParseCodeforcesAPI(json_resp);
        Log("Parsed " + std::to_string(contests.size()) + " Codeforces contests.");

        SaveToDB(contests);
        InvalidateRedisCache();
    }
    
    void PerformLeetCodeCrawl() {
        Log("Starting LeetCode crawl cycle...");
        std::string gql_query = "query { contestHistory(pageNum: 1, pageSize: 10) { contests { title titleSlug startTime duration } } }";
        
        std::string json_resp = FetchGraphQL("https://leetcode.cn", gql_query, lc_robots_parser_);
        
        if (json_resp.empty()) {
            Log("Failed to fetch LeetCode contests.");
            return;
        }
        
        Log("Fetch successful. Parsing LeetCode...");
        std::vector<Contest> contests = ParseLeetCodeContests(json_resp);
        Log("Parsed " + std::to_string(contests.size()) + " LeetCode contests.");
        
        SaveToDB(contests);
        InvalidateRedisCache();
    }
};

int main() {
    // Seed random number generator
    std::srand(std::time(nullptr));

    CrawlerConfig config;
    // You can load config from file here if needed, 
    // for now using defaults as requested: 2-4 hours interval.
    config.min_interval_seconds = 2 * 3600; // 2 hours
    config.max_interval_seconds = 4 * 3600; // 4 hours
    
    ContestCrawler crawler(config);
    crawler.Run();
    
    return 0;
}
