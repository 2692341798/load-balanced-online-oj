#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include "../comm/httplib.h"

using namespace httplib;

std::atomic<int> success_count(0);
std::atomic<int> fail_count(0);
std::atomic<long long> total_latency(0);

// Config
std::string target_ip = "127.0.0.1";
int target_port = 8094; // OJ Server

void Worker(int id, int requests_per_thread) {
    Client cli(target_ip, target_port);
    cli.set_connection_timeout(5); // 5s timeout
    cli.set_read_timeout(10);
    
    for (int i = 0; i < requests_per_thread; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        
        // 1. Get Questions List (Read Heavy)
        auto res = cli.Get("/api/problems");
        
        auto end = std::chrono::high_resolution_clock::now();
        auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        if (res && res->status == 200) {
            success_count++;
            total_latency += latency;
        } else {
            fail_count++;
            // Print error occasionally
            if (fail_count % 100 == 0)
                std::cerr << "Thread " << id << " failed: " << (res ? res->status : -1) << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    int threads = 10;
    int requests = 100;
    
    if (argc > 1) threads = atoi(argv[1]);
    if (argc > 2) requests = atoi(argv[2]);
    if (argc > 3) target_port = atoi(argv[3]);
    
    std::cout << "========================================" << std::endl;
    std::cout << "Stress Test Tool (C++)" << std::endl;
    std::cout << "Target: " << target_ip << ":" << target_port << std::endl;
    std::cout << "Threads: " << threads << std::endl;
    std::cout << "Requests per thread: " << requests << std::endl;
    std::cout << "Total Requests: " << threads * requests << std::endl;
    std::cout << "========================================" << std::endl;
    
    std::vector<std::thread> pool;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < threads; ++i) {
        pool.emplace_back(Worker, i, requests);
    }
    
    for (auto &t : pool) {
        if(t.joinable()) t.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    
    double qps = (double)(success_count + fail_count) * 1000.0 / duration;
    double avg_latency = success_count > 0 ? (double)total_latency / success_count : 0.0;
    
    std::cout << "========================================" << std::endl;
    std::cout << "Results:" << std::endl;
    std::cout << "Time Taken: " << duration << " ms" << std::endl;
    std::cout << "Total Requests: " << (success_count + fail_count) << std::endl;
    std::cout << "Success: " << success_count << std::endl;
    std::cout << "Failed: " << fail_count << std::endl;
    std::cout << "Error Rate: " << (fail_count * 100.0 / (success_count + fail_count)) << "%" << std::endl;
    std::cout << "QPS: " << qps << std::endl;
    std::cout << "Avg Latency: " << avg_latency << " ms" << std::endl;
    std::cout << "========================================" << std::endl;
        
    return 0;
}
