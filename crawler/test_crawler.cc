#include <iostream>
#include <cassert>
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

int main() {
    TestParse();
    return 0;
}
