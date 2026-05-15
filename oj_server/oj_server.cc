#include <iostream>
#include <signal.h>
#include <clocale> // For setlocale
#include <json/json.h>   // 通过 -I/opt/homebrew/include 已能找到

#include "../comm/httplib.h"
#include "oj_control.hpp"

using namespace httplib;
using namespace ns_control;

static Control *ctrl_ptr = nullptr;

// Helper function to disable browser caching for dynamic pages
void SetNoCache(Response &resp) {
    resp.set_header("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
    resp.set_header("Pragma", "no-cache");
    resp.set_header("Expires", "0");
}

void Recovery(int signo)
{
    ctrl_ptr->RecoveryMachine();
}

int main()
{
    // Initialize random seed
    srand(time(nullptr));

    // Disable stdout buffering
    setbuf(stdout, NULL);
    std::cout << "[INFO] Server starting..." << std::endl;

    // Set locale to use environment variables (en_US.UTF-8 from Dockerfile)
    std::setlocale(LC_ALL, "en_US.UTF-8");

    // Log system locale
    const char* lang = std::getenv("LANG");
    const char* lc_all = std::getenv("LC_ALL");
    std::cout << "[INFO] Startup Locale Check: LANG=" << (lang ? lang : "null") 
              << ", LC_ALL=" << (lc_all ? lc_all : "null") << std::endl;

    signal(SIGQUIT, Recovery);

    // 用户请求的服务路由功能
    Server svr;
    
    // Configure httplib thread pool for I/O bound application
    // Set to a larger value (e.g. 500 threads) to handle concurrent connections effectively
    svr.new_task_queue = [] { return new httplib::ThreadPool(500); };

    Control ctrl;
    ctrl_ptr = &ctrl;
#include "server/impl/routes_pages.hpp"
#include "server/impl/routes_api_training.hpp"
#include "server/impl/routes_api_contests_questions_judge.hpp"
#include "server/impl/routes_api_auth.hpp"
#include "server/impl/routes_api_submissions_upload_ai.hpp"
#include "server/impl/routes_api_discussions_comments.hpp"
#include "server/impl/routes_api_admin.hpp"
#include "server/impl/server_static.hpp"
    return 0;
} 
