#include <iostream>
#include <vector>
#include <cassert>
#include "contest_utils.hpp"

void TestSorting() {
    std::vector<ContestEntry> contests;
    
    // Create test data
    ContestEntry c1; c1.name = "C1"; c1.start_time = 100; c1.end_time = 200;
    ContestEntry c2; c2.name = "C2"; c2.start_time = 50;  c2.end_time = 150;
    ContestEntry c3; c3.name = "C3"; c3.start_time = 100; c3.end_time = 150; // Same start as C1, earlier end
    ContestEntry c4; c4.name = "C4"; c4.start_time = 200; c4.end_time = 300;

    contests.push_back(c1);
    contests.push_back(c2);
    contests.push_back(c3);
    contests.push_back(c4);

    std::cout << "Before sort:" << std::endl;
    for(const auto& c : contests) std::cout << c.name << " " << c.start_time << " " << c.end_time << std::endl;

    SortContests(contests);

    std::cout << "After sort:" << std::endl;
    for(const auto& c : contests) std::cout << c.name << " " << c.start_time << " " << c.end_time << std::endl;

    // Assertions
    // Order should be: C2 (50), C3 (100, 150), C1 (100, 200), C4 (200)
    assert(contests[0].name == "C2");
    assert(contests[1].name == "C3");
    assert(contests[2].name == "C1");
    assert(contests[3].name == "C4");

    std::cout << "TestSorting Passed!" << std::endl;
}

int main() {
    TestSorting();
    return 0;
}
