#include <iostream>
#include <cassert>
#include <thread>
#include "crawler_common.hpp"

void TestParse() {
    std::string mock_html = R"(
        <table>
        <tr data-contestId="1077">
            <td>
                Codeforces Round 1077 (Div. 1)
            </td>
            <td>Writer</td>
            <td>
                Jan/29/2026 17:35UTC+8
            </td>
            <td>Length</td>
            <td>Registered</td>
        </tr>
        </table>
    )";
    
    std::vector<Contest> contests = ParseContests(mock_html);
    assert(contests.size() == 1);
    assert(contests[0].name == "Codeforces Round 1077 (Div. 1)");
    assert(contests[0].start_time == "Jan/29/2026 17:35UTC+8");
    assert(contests[0].link == "https://codeforces.com/contest/1077");
    
    std::cout << "TestParse Passed!" << std::endl;
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
    TestParse();
    TestRobotsParser();
    TestBackoff();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
