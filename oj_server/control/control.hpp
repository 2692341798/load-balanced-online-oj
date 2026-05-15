#pragma once

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "../../comm/httplib.h"
#include "../../comm/log.hpp"
#include "../../comm/util.hpp"
#include "../deepseek_api.hpp"
#include "../oj_model.hpp"
#include "../oj_view.hpp"
#include "types.hpp"

#ifdef ENABLE_REDIS
#include <hiredis/hiredis.h>
#endif

namespace ns_control
{
    using namespace std;
    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_model;
    using namespace ns_view;
    using namespace httplib;

    class Control
    {
    private:
        Model model_;
        View view_;
        LoadBlance load_blance_;
        ns_deepseek::DeepSeekApi deepseek_api_;
        
        std::unordered_map<std::string, Session> sessions_;
        std::mutex session_mtx_;

    public:
        Control()
        {
        }
        ~Control()
        {
        }

    public:
#include "impl/upload.hpp"
#include "impl/auth.hpp"
#include "impl/admin.hpp"
#include "impl/pages.hpp"
#include "impl/contest.hpp"
#include "impl/judge.hpp"
#include "impl/submissions.hpp"
#include "impl/discussions.hpp"
#include "impl/training.hpp"
    };
}

