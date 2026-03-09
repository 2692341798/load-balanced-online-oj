#ifndef REDIS_QUEUE_HPP
#define REDIS_QUEUE_HPP

#include <string>
#include <vector>
#include <iostream>
#include <queue>
#include <set>
#include <map>
#include <mutex>
#include <chrono>

namespace ns_crawler {

class RedisQueue {
public:
    RedisQueue(const std::string& host = "127.0.0.1", int port = 6379) {
        // Mock implementation, ignore host/port
        (void)host;
        (void)port;
    }

    ~RedisQueue() {
    }

    // Push value to queue
    bool Push(const std::string& queue_name, const std::string& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queues_[queue_name].push(value);
        return true;
    }

    // Pop value from queue
    std::string Pop(const std::string& queue_name) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queues_.find(queue_name) == queues_.end() || queues_[queue_name].empty()) {
            return "";
        }
        std::string val = queues_[queue_name].front();
        queues_[queue_name].pop();
        return val;
    }

    // Check if hash is visited with TTL
    bool IsVisitedWithTTL(const std::string& hash) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = visited_.find(hash);
        if (it == visited_.end()) {
            return false;
        }
        if (std::chrono::steady_clock::now() > it->second) {
            visited_.erase(it);
            return false;
        }
        return true;
    }

    // Add visited hash with TTL
    bool AddVisitedWithTTL(const std::string& hash, int ttl) {
        std::lock_guard<std::mutex> lock(mutex_);
        visited_[hash] = std::chrono::steady_clock::now() + std::chrono::seconds(ttl);
        return true;
    }

private:
    std::map<std::string, std::queue<std::string>> queues_;
    std::map<std::string, std::chrono::steady_clock::time_point> visited_;
    std::mutex mutex_;
};

} // namespace ns_crawler

#endif
