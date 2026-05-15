#pragma once

#include "../../comm/util.hpp"
#include "../../comm/log.hpp"
#include <mysql/mysql.h>
#include <openssl/sha.h>
#include "types.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <sstream>
#include <iomanip>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

namespace ns_model
{
    using namespace std;
    using namespace ns_log;
    using namespace ns_util;

    const std::string oj_questions = "oj_questions";
    const std::string oj_users = "users";
    const std::string oj_submissions = "submissions";
    const std::string oj_inline_comments = "inline_comments";
    const std::string oj_discussions = "discussions";
    const std::string oj_article_comments = "article_comments";
    const std::string oj_contests = "contests";
    const std::string oj_training_lists = "training_lists";
    const std::string oj_training_list_items = "training_list_items";
    const std::string oj_invitation_codes = "invitation_codes";
    const std::string oj_operation_logs = "operation_logs";

    inline std::string GetEnv(const std::string& key, const std::string& default_value) {
        const char* val = std::getenv(key.c_str());
        return val ? std::string(val) : default_value;
    }

    const std::string host = GetEnv("MYSQL_HOST", "127.0.0.1");
    const std::string user = GetEnv("MYSQL_USER", "oj_client");
    const std::string passwd = GetEnv("MYSQL_PASSWORD", "123456");
    const std::string db = GetEnv("MYSQL_DB", "oj");
    const int port = std::stoi(GetEnv("MYSQL_PORT", "3306"));

#include "core.hpp"

    class Model
    {
    public:
        Model()
        {
            LOG(INFO) << "Connecting to Database: " << host << ":" << port << " user=" << user << " db=" << db << "\n";
            InitUserTable();
            InitSubmissionTable();
            InitInlineCommentTable();
            InitDiscussionTable();
            InitArticleCommentTable();
            InitContestTable();
            InitTrainingListTable();
            InitTrainingListItemTable();
            InitInvitationCodeTable();
            InitOperationLogTable();
            CheckAndUpgradeTable();
            
            int total_users = 0;
            GetTotalUserCount(&total_users);
            LOG(INFO) << "Current Total Users: " << total_users << "\n";
        }
#include "impl/schema.hpp"
#include "impl/contests.hpp"

#include "impl/sql.hpp"

#include "impl/inline_comments.hpp"

#include "impl/submissions.hpp"

        bool GetUserSolvedStats(const std::string &user_id, std::unordered_map<std::string, int> *stats)
        {
            std::string sql = "SELECT q.star, COUNT(DISTINCT s.question_id) FROM " + oj_submissions + " s "
                              "JOIN " + oj_questions + " q ON s.question_id = q.number "
                              "WHERE s.user_id='" + user_id + "' AND s.result='0' "
                              "GROUP BY q.star";
            
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            if(0 != mysql_query(my, sql.c_str()))
            {
                LOG(WARNING) << sql << " execute error: " << mysql_error(my) << "\n";

                return false;
            }

            MYSQL_RES *res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            
            for(int i = 0; i < rows; i++)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                std::string difficulty = row[0] ? row[0] : "Unknown";
                int count = row[1] ? atoi(row[1]) : 0;
                (*stats)[difficulty] = count;
            }
            
            mysql_free_result(res);

            return true;
        }

#include "impl/questions.hpp"

#include "impl/users.hpp"

#include "impl/discussions.hpp"

#include "impl/article_comments.hpp"

#include "impl/training.hpp"

#include "impl/admin.hpp"

#include "impl/stats.hpp"

        ~Model()
        {}
    };
} // namespace ns_model
