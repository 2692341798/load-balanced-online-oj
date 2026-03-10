#include "../../crawler/redis_queue.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <cassert>

int main() {
    ns_crawler::RedisQueue rq;

    std::string queue = "test_queue";
    std::string hash = "test_hash";

    // Clear previous data if any (optional, but good for testing)
    // In a real scenario, we might want to FLUSHDB or del keys, but here we just append.

    // Test Push
    std::cout << "[TEST] Pushing 'hello'..." << std::endl;
    if (rq.Push(queue, "hello")) {
        std::cout << "[PASS] Push 'hello' success" << std::endl;
    } else {
        std::cerr << "[FAIL] Push 'hello' failed" << std::endl;
    }

    std::cout << "[TEST] Pushing 'world'..." << std::endl;
    if (rq.Push(queue, "world")) {
        std::cout << "[PASS] Push 'world' success" << std::endl;
    } else {
        std::cerr << "[FAIL] Push 'world' failed" << std::endl;
    }

    // Test Pop
    std::string val1 = rq.Pop(queue);
    std::cout << "[TEST] Popped: " << val1 << std::endl;
    // Note: Since we don't clear the queue, we might get old values if the queue wasn't empty.
    // But we expect at least 'hello' or whatever was first.
    
    std::string val2 = rq.Pop(queue);
    std::cout << "[TEST] Popped: " << val2 << std::endl;

    // Test Visited TTL
    std::cout << "[TEST] Checking if hash is visited..." << std::endl;
    bool visited = rq.IsVisitedWithTTL(hash);
    std::cout << "Is visited? " << (visited ? "Yes" : "No") << std::endl;

    std::cout << "[TEST] Adding visited with 3s TTL..." << std::endl;
    if (rq.AddVisitedWithTTL(hash, 3)) {
        std::cout << "[PASS] AddVisitedWithTTL success" << std::endl;
    } else {
        std::cerr << "[FAIL] AddVisitedWithTTL failed" << std::endl;
    }

    visited = rq.IsVisitedWithTTL(hash);
    std::cout << "[TEST] Is visited immediately after add? " << (visited ? "Yes" : "No") << std::endl;
    if (!visited) std::cerr << "[FAIL] Should be visited!" << std::endl;

    std::cout << "[TEST] Waiting 4s for TTL expiration..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(4));
    
    visited = rq.IsVisitedWithTTL(hash);
    std::cout << "[TEST] Is visited after timeout? " << (visited ? "Yes" : "No") << std::endl;
    if (visited) std::cerr << "[FAIL] Should not be visited!" << std::endl;

    return 0;
}
