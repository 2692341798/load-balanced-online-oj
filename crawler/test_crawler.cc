#include <iostream>
#include <cassert>
#include <thread>
#include "crawler_common.hpp"

void TestCodeforcesParse() {
    std::string mock_json = R"({
        "status": "OK",
        "result": [
            {
                "id": 1077,
                "name": "Codeforces Round 1077 (Div. 1)",
                "type": "CF",
                "phase": "FINISHED",
                "frozen": false,
                "durationSeconds": 7200,
                "startTimeSeconds": 1769679300, 
                "relativeTimeSeconds": -100000
            }
        ]
    })";
    
    std::vector<Contest> contests = ParseCodeforcesAPI(mock_json);
    assert(contests.size() == 1);
    assert(contests[0].name == "Codeforces Round 1077 (Div. 1)");
    assert(contests[0].contest_id == "1077");
    assert(contests[0].link == "https://codeforces.com/contest/1077");
    
    std::cout << "TestCodeforcesParse Passed!" << std::endl;
}

void TestLeetCodeParse() {
    std::string mock_json = R"({
        "data": {
            "contestHistory": {
                "contests": [
                    {
                        "title": "Weekly Contest 300",
                        "titleSlug": "weekly-contest-300",
                        "startTime": 1656813600,
                        "duration": 5400,
                        "originStartTime": 1656813600
                    }
                ]
            }
        }
    })";

    std::vector<Contest> contests = ParseLeetCodeContests(mock_json);
    assert(contests.size() == 1);
    assert(contests[0].name == "Weekly Contest 300");
    assert(contests[0].contest_id == "weekly-contest-300");
    assert(contests[0].link == "https://leetcode.cn/contest/weekly-contest-300");
    assert(contests[0].source == "LeetCode");
    
    // 1656813600 is 2022-07-03 02:00:00 UTC
    // Our parser converts to local time string. 
    // Since we can't easily assert exact string due to timezone, check non-empty.
    assert(!contests[0].start_time.empty());
    assert(!contests[0].end_time.empty());
    
    std::cout << "TestLeetCodeParse Passed!" << std::endl;
}

void TestRobotsParser() {
    RobotsParser parser;
    std::string my_agent = "ContestBot/2.0";
    
    // Case 1: Simple wildcard
    std::string robots1 = R"(
        User-agent: *
        Disallow: /admin
        Disallow: /private
        Crawl-delay: 5
    )";
    parser.Parse(robots1, my_agent);
    assert(parser.IsAllowed("/public") == true);
    assert(parser.IsAllowed("/admin/login") == false);
    assert(parser.IsAllowed("/private/data") == false);
    assert(parser.GetCrawlDelay() == 5);
    
    // Case 2: Specific agent overrides wildcard
    std::string robots2 = R"(
        User-agent: *
        Disallow: /
        
        User-agent: ContestBot
        Disallow: /secret
        Crawl-delay: 2
    )";
    parser.Parse(robots2, my_agent); // "ContestBot/2.0" contains "ContestBot"
    assert(parser.IsAllowed("/public") == true);
    assert(parser.IsAllowed("/secret") == false);
    assert(parser.GetCrawlDelay() == 2);
    
    // Case 3: Specific agent not matching
    std::string robots3 = R"(
        User-agent: Googlebot
        Disallow: /
        
        User-agent: *
        Disallow: /admin
    )";
    parser.Parse(robots3, my_agent);
    assert(parser.IsAllowed("/public") == true);
    assert(parser.IsAllowed("/admin") == false);
    
    std::cout << "TestRobotsParser Passed!" << std::endl;
}

void TestBackoff() {
    BackoffStrategy backoff(3, 100); // Max 3 retries, base 100ms
    
    assert(backoff.GetWaitTime(0) == 100);
    assert(backoff.GetWaitTime(1) == 200);
    assert(backoff.GetWaitTime(2) == 400);
    assert(backoff.GetWaitTime(3) == 800);
    assert(backoff.GetWaitTime(4) == -1);
    
    std::cout << "TestBackoff Passed!" << std::endl;
}

int main() {
    TestCodeforcesParse();
    TestLeetCodeParse();
    TestRobotsParser();
    TestBackoff();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
