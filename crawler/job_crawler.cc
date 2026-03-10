#include "crawler_common.hpp"
#include "redis_queue.hpp"
#include "../oj_server/oj_model.hpp"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "../comm/httplib.h"
#include <regex>
#include <thread>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <set>

// Use the same JobInfo struct as in oj_model.hpp
using ns_model::JobInfo;

class JobCrawler {
public:
    JobCrawler() : redis_queue_("127.0.0.1", 6379), model_() {
        // Initialize Redis keys
        job_queue_key_ = "job_queue";
        visited_set_key_ = "job_visited";
    }

    void Run() {
        std::cout << "JobCrawler started..." << std::endl;
        while (true) {
            // 1. Pop URL from job_queue
            std::string url = redis_queue_.Pop(job_queue_key_);
            if (url.empty()) {
                std::cout << "Queue empty, waiting..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(5));
                continue;
            }

            // 2. Check deduplication
            // Generate a hash for the URL to check if visited
            std::string url_hash = ns_model::Model().SHA256Hash(url); // Use Model's helper or local one
            if (redis_queue_.IsVisitedWithTTL(visited_set_key_ + ":" + url_hash)) {
                std::cout << "Skipping visited URL: " << url << std::endl;
                continue;
            }

            // Mark as visited (expire after 24 hours)
            redis_queue_.AddVisitedWithTTL(visited_set_key_ + ":" + url_hash, 86400);

            std::cout << "Processing: " << url << std::endl;

            // 3. Dispatch based on domain
            Dispatch(url);

            // 4. Rate limiting
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }

    void PushSeedUrl(const std::string& url) {
        // Check if queue is empty (simple check: pop returns empty? No, that consumes it.
        // For now, just push it. Redis list allows duplicates, but our visited check handles processing.
        // Better: Check if visited before pushing? 
        // Here we just push unconditionally on startup if we want to ensure seed is there.
        // User requirement: "Pushes seed URL ... to queue if empty."
        // Since RedisQueue doesn't have IsEmpty(), we might just push it.
        // Or we can try to LLEN in RedisQueue, but let's just push it.
        redis_queue_.Push(job_queue_key_, url);
    }

private:
    ns_crawler::RedisQueue redis_queue_;
    ns_model::Model model_;
    std::string job_queue_key_;
    std::string visited_set_key_;

    void Dispatch(const std::string& url) {
        if (url.find("xingjian.ncss.cn") != std::string::npos) {
            CrawlNCSS(url);
        } else {
            std::cerr << "Unknown domain for URL: " << url << std::endl;
        }
    }

    void CrawlNCSS(const std::string& url) {
        // Parse host and path
        std::string proto_delimiter = "://";
        size_t pos = url.find(proto_delimiter);
        if (pos == std::string::npos) return;
        
        std::string proto = url.substr(0, pos);
        std::string remaining = url.substr(pos + proto_delimiter.length());
        
        size_t path_pos = remaining.find('/');
        std::string host = (path_pos == std::string::npos) ? remaining : remaining.substr(0, path_pos);
        std::string path = (path_pos == std::string::npos) ? "/" : remaining.substr(path_pos);

        // Fetch content
        // Use unified Client interface which handles scheme (http/https) automatically
        // if CPPHTTPLIB_OPENSSL_SUPPORT is defined.
        httplib::Client cli((proto + "://" + host).c_str());
        cli.set_follow_location(true);

        auto res = cli.Get(path.c_str());
        if (res && res->status == 200) {
            ParseNCSS(res->body, url);
        } else {
            std::cerr << "Failed to fetch " << url << " Status: " << (res ? std::to_string(res->status) : "Error") << std::endl;
        }
    }

    void ParseNCSS(const std::string& html, const std::string& current_url) {
        // This function handles both list pages and detail pages.
        
        // 1. Extract Links (if it's a list page or contains related links)
        // Regex to find job detail links.
        // Assumption: Detail links match pattern like "/job/detail?id=..." or "/job/..."
        // For NCSS, let's assume links look like href="/student/jobs/..."
        
        std::regex link_regex("href=\"(/student/jobs/[^\"]+)\"");
        std::smatch link_match;
        std::string::const_iterator search_start(html.cbegin());
        while (std::regex_search(search_start, html.cend(), link_match, link_regex)) {
            std::string relative_path = link_match[1];
            std::string full_url = "https://xingjian.ncss.cn" + relative_path;
            
            // Push discovered links to queue
            // Filter out obviously non-job links if possible
            redis_queue_.Push(job_queue_key_, full_url);
            
            search_start = link_match.suffix().first;
        }

        // 2. Extract Job Info (if it's a detail page)
        // Heuristic: If we can find a Title and Company, it's likely a job page.
        
        // Regex patterns (Hypothetical - adapted to common structures)
        // Title: <h1 class="title">...</h1> or similar
        std::regex title_regex("<h1[^>]*>(.*?)</h1>"); 
        // Company: <div class="company-name">...</div> or <a class="company">...</a>
        std::regex company_regex("company[^>]*>(.*?)<");
        // Location
        std::regex location_regex("location[^>]*>([^<]+)<");
        // Salary
        std::regex salary_regex("salary[^>]*>([^<]+)<");
        // Description (usually in a div with id/class content/desc)
        std::regex desc_regex("class=\"job-desc\"[^>]*>([\\s\\S]*?)</div>");

        JobInfo job;
        std::smatch match;

        if (std::regex_search(html, match, title_regex)) {
            job.title = match[1];
        } else {
            // If no title found, probably not a detail page
            return; 
        }

        if (std::regex_search(html, match, company_regex)) {
            job.company = match[1];
        } else {
            // Default or skip
            job.company = "Unknown Company";
        }

        if (std::regex_search(html, match, location_regex)) job.location = match[1];
        else job.location = "Unknown";

        if (std::regex_search(html, match, salary_regex)) job.salary = match[1];
        else job.salary = "Negotiable";

        if (std::regex_search(html, match, desc_regex)) {
            job.description = match[1];
            // Clean HTML tags from description if needed, or keep them
        } else {
            job.description = "No description available.";
        }
        
        // Metadata
        job.url = current_url;
        job.source = "NCSS";
        job.publish_date = "2026-01-01"; // Placeholder, or extract date
        
        // Save
        SaveToDB(job);
    }

    void SaveToDB(const JobInfo& job) {
        if (model_.AddJob(job)) {
            std::cout << "Saved job: " << job.title << " at " << job.company << std::endl;
        } else {
            std::cerr << "Failed to save job: " << job.title << std::endl;
        }
    }
};

int main() {
    JobCrawler crawler;
    
    // Seed URL
    // Ideally we check if Redis queue is empty before pushing.
    // For simplicity, we just push it. The crawler handles visited URLs.
    crawler.PushSeedUrl("https://xingjian.ncss.cn/student/jobs/index.html");
    
    crawler.Run();
    
    return 0;
}
