#pragma once
//MySQL 版本
#include "../comm/util.hpp"
#include "../comm/log.hpp"
#include <mysql/mysql.h>
#include <openssl/sha.h>

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

// 根据题目list文件，加载所有的题目信息到内存中
// model: 主要用来和数据进行交互，对外提供访问数据的接口

namespace ns_model
{
    using namespace std;
    using namespace ns_log;
    using namespace ns_util;

    struct Question
    {
        std::string number; //题目编号，唯一
        std::string title;  //题目的标题
        std::string star;   //难度: 简单 中等 困难
        std::string desc;   //题目的描述
        std::string header; //题目预设给用户在线编辑器的代码
        std::string tail;   //题目的测试用例，需要和header拼接，形成完整代码
        int cpu_limit;      //题目的时间要求(S)
        int mem_limit;      //题目的空间要去(KB)
        std::string language_type;
        int status;         // 0: Hidden, 1: Visible
    };

    struct User
    {
        std::string id;
        std::string username;
        std::string password; // Hashed
        std::string email;
        std::string nickname;
        std::string phone;
        std::string avatar;   // Avatar URL
        std::string created_at;
        int role;             // 0: User, 1: Admin
        int status;           // 0: Normal, 1: Banned
    };

    struct InvitationCode
    {
        std::string id;
        std::string code;
        bool is_used;
        std::string used_by; // user_id
        std::string created_at;
    };

    struct OperationLog
    {
        std::string id;
        std::string user_id;
        std::string username; // join
        std::string action;
        std::string target;
        std::string details;
        std::string ip;
        std::string created_at;
    };

    struct Submission
    {
        std::string id;
        std::string user_id;
        std::string question_id;
        std::string question_title;
        std::string result; // "0": Success, "-1": Empty, "-3": Compile Error, etc.
        int cpu_time;
        int mem_usage;
        std::string created_at;
        std::string content; // Submission code/content
        std::string language;
    };

    struct InlineComment
    {
        std::string id;
        std::string user_id;
        std::string username; // Join query result
        std::string user_avatar; // Join query result
        std::string post_id;
        std::string content;
        std::string selected_text;
        std::string parent_id; // For nested replies
        std::string created_at;
    };

    struct Discussion
    {
        std::string id;
        std::string title;
        std::string content;
        std::string author_id;
        std::string author_name; // Join result
        std::string author_avatar; // Join result
        std::string question_id; // Related question ID (0 if none)
        std::string question_title; // Join result
        std::string created_at;
        int likes;
        int views;
        int comments_count;
        bool is_official;
    };

    struct ArticleComment
    {
        std::string id;
        std::string post_id;
        std::string user_id;
        std::string username; // Join result
        std::string user_avatar; // Join result
        std::string content;
        std::string created_at;
        int likes;
    };

    struct Contest
    {
        std::string id;
        std::string contest_id; // Platform ID
        std::string name;
        std::string start_time;
        std::string end_time;
        std::string link;
        std::string source;
        std::string status;
        std::string last_crawl_time;
    };

    struct TrainingList
    {
        std::string id;
        std::string title;
        std::string description;
        std::string difficulty; // Easy, Medium, Hard, Unrated
        std::string tags;       // JSON array
        std::string author_id;
        std::string visibility; // public, private
        std::string created_at;
        std::string updated_at;
        int likes;
        int collections;
        // Join fields
        std::string author_name;
        std::string author_avatar;
        int problem_count;
    };

    struct TrainingListItem
    {
        std::string id;
        std::string training_list_id;
        std::string question_id;
        int order_index;
        // Join fields
        std::string question_title;
        std::string question_difficulty;
        std::string user_status; // Solved status for current user
    };

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

    class MySQLConnectionPool {
    private:
        std::queue<MYSQL*> pool;
        std::mutex mtx;
        std::condition_variable cv;
        int current_size;
        int min_size;
        int max_size;

        MYSQL* CreateConnection() {
            MYSQL *my = mysql_init(nullptr);
            // Reconnect is important for long running process
            bool reconnect = true;
            mysql_options(my, MYSQL_OPT_RECONNECT, &reconnect);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), port, nullptr, 0)){
                LOG(ERROR) << "Failed to connect to database in pool" << "\n";
                mysql_close(my);
                return nullptr;
            }
            if(0 != mysql_set_character_set(my, "utf8mb4")) {
                LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n";
            }
            return my;
        }

    public:
        MySQLConnectionPool(int min_s = 5, int max_s = 20)
            : min_size(min_s), max_size(max_s), current_size(0) {
            for (int i = 0; i < min_size; ++i) {
                MYSQL* conn = CreateConnection();
                if (conn) {
                    pool.push(conn);
                    current_size++;
                }
            }
        }

        ~MySQLConnectionPool() {
            std::lock_guard<std::mutex> lock(mtx);
            while (!pool.empty()) {
                MYSQL* conn = pool.front();
                pool.pop();
                mysql_close(conn);
            }
        }

        MYSQL* GetConnection() {
            std::unique_lock<std::mutex> lock(mtx);
            if (pool.empty()) {
                if (current_size < max_size) {
                    MYSQL* conn = CreateConnection();
                    if (conn) {
                        current_size++;
                        return conn;
                    }
                }
                cv.wait(lock, [this] { return !pool.empty(); });
            }
            MYSQL* conn = pool.front();
            pool.pop();
            return conn;
        }

        void ReleaseConnection(MYSQL* conn) {
            if (!conn) return;
            // Check if connection is still alive, ping it. If dead, close it and decrement current_size
            if (mysql_ping(conn) != 0) {
                mysql_close(conn);
                std::lock_guard<std::mutex> lock(mtx);
                current_size--;
            } else {
                std::lock_guard<std::mutex> lock(mtx);
                pool.push(conn);
            }
            cv.notify_one();
        }
        
        static MySQLConnectionPool& GetInstance() {
            static MySQLConnectionPool instance(5, 20);
            return instance;
        }
    };

    class ConnectionGuard {
    private:
        MYSQL* conn;
    public:
        ConnectionGuard() {
            conn = MySQLConnectionPool::GetInstance().GetConnection();
        }
        ~ConnectionGuard() {
            MySQLConnectionPool::GetInstance().ReleaseConnection(conn);
        }
        MYSQL* get() { return conn; }
    };

    class Cache {
    private:
        std::unordered_map<std::string, Question> question_cache;
        std::vector<Question> all_questions_cache;
        bool all_questions_cached = false;
        std::mutex mtx;

    public:
        void SetQuestion(const std::string& number, const Question& q) {
            std::lock_guard<std::mutex> lock(mtx);
            question_cache[number] = q;
        }

        bool GetQuestion(const std::string& number, Question* q) {
            std::lock_guard<std::mutex> lock(mtx);
            if (question_cache.count(number)) {
                *q = question_cache[number];
                return true;
            }
            return false;
        }

        void SetAllQuestions(const std::vector<Question>& qs) {
            std::lock_guard<std::mutex> lock(mtx);
            all_questions_cache = qs;
            all_questions_cached = true;
        }

        bool GetAllQuestions(std::vector<Question>* qs) {
            std::lock_guard<std::mutex> lock(mtx);
            if (all_questions_cached) {
                *qs = all_questions_cache;
                return true;
            }
            return false;
        }

        void InvalidateAllQuestions() {
            std::lock_guard<std::mutex> lock(mtx);
            all_questions_cached = false;
        }

        void InvalidateQuestion(const std::string& number) {
            std::lock_guard<std::mutex> lock(mtx);
            question_cache.erase(number);
            all_questions_cached = false; // also invalidate all questions
        }
        
        static Cache& GetInstance() {
            static Cache instance;
            return instance;
        }
    };

    class Model
    {
    public:
        Model()
        {
            LOG(INFO) << "Connecting to Database: " << host << ":" << port << " user=" << user << " db=" << db << "\n";
            // Try to create users table if not exists
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

        void InitInvitationCodeTable() {
            std::string sql = "CREATE TABLE IF NOT EXISTS `invitation_codes` ("
                              "`id` INT PRIMARY KEY AUTO_INCREMENT,"
                              "`code` VARCHAR(50) NOT NULL UNIQUE,"
                              "`is_used` TINYINT(1) DEFAULT 0,"
                              "`used_by` INT DEFAULT 0,"
                              "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
                              ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
            ExecuteSql(sql);
        }

        void InitOperationLogTable() {
            std::string sql = "CREATE TABLE IF NOT EXISTS `operation_logs` ("
                              "`id` INT PRIMARY KEY AUTO_INCREMENT,"
                              "`user_id` INT NOT NULL,"
                              "`action` VARCHAR(50) NOT NULL,"
                              "`target` VARCHAR(100),"
                              "`details` TEXT,"
                              "`ip` VARCHAR(50),"
                              "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                              "INDEX `idx_user_id` (`user_id`)"
                              ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
            ExecuteSql(sql);
        }

        void InitTrainingListTable() {
            std::string sql = "CREATE TABLE IF NOT EXISTS `training_lists` ("
                              "`id` INT PRIMARY KEY AUTO_INCREMENT,"
                              "`title` VARCHAR(255) NOT NULL,"
                              "`description` TEXT,"
                              "`difficulty` VARCHAR(50) DEFAULT 'Unrated',"
                              "`tags` TEXT,"
                              "`author_id` INT NOT NULL,"
                              "`visibility` ENUM('public', 'private') DEFAULT 'public',"
                              "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                              "`updated_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
                              "`likes` INT DEFAULT 0,"
                              "`collections` INT DEFAULT 0,"
                              "INDEX `idx_author_id` (`author_id`)"
                              // "FOREIGN KEY (`author_id`) REFERENCES `users`(`id`) ON DELETE CASCADE"
                              ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
            ExecuteSql(sql);
        }

        void InitTrainingListItemTable() {
            std::string sql = "CREATE TABLE IF NOT EXISTS `training_list_items` ("
                              "`id` INT PRIMARY KEY AUTO_INCREMENT,"
                              "`training_list_id` INT NOT NULL,"
                              "`question_id` INT NOT NULL,"
                              "`order_index` INT NOT NULL DEFAULT 0,"
                              "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                              // "FOREIGN KEY (`training_list_id`) REFERENCES `training_lists`(`id`) ON DELETE CASCADE,"
                              "INDEX `idx_list_id` (`training_list_id`),"
                              "INDEX `idx_question_id` (`question_id`),"
                              "UNIQUE KEY `unique_item` (`training_list_id`, `question_id`)"
                              ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
            ExecuteSql(sql);
        }

        void InitUserTable() {
            // 创建完整的表结构
            std::string sql = "CREATE TABLE IF NOT EXISTS `users` ("
                              "`id` int(11) NOT NULL AUTO_INCREMENT,"
                              "`username` varchar(50) NOT NULL UNIQUE,"
                              "`password` varchar(128) NOT NULL,"
                              "`email` varchar(100) DEFAULT NULL,"
                              "`nickname` varchar(100) DEFAULT NULL,"
                              "`phone` varchar(20) DEFAULT NULL,"
                              "`avatar` varchar(255) DEFAULT NULL,"
                              "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                              "`updated_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
                              "PRIMARY KEY (`id`)"
                              ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
            ExecuteSql(sql);
        }

        void InitSubmissionTable() {
            std::string sql = "CREATE TABLE IF NOT EXISTS `submissions` ("
                              "`id` int(11) NOT NULL AUTO_INCREMENT,"
                              "`user_id` int(11) NOT NULL,"
                              "`question_id` int(11) NOT NULL,"
                              "`result` varchar(10) NOT NULL,"
                              "`cpu_time` int(11) DEFAULT 0,"
                              "`mem_usage` int(11) DEFAULT 0,"
                              "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                              "`content` TEXT,"
                              "`language` varchar(20) DEFAULT 'cpp',"
                              "PRIMARY KEY (`id`),"
                              "INDEX `idx_user_id` (`user_id`),"
                              "INDEX `idx_question_id` (`question_id`)"
                              ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
            ExecuteSql(sql);
        }

        void InitInlineCommentTable() {
            std::string sql = "CREATE TABLE IF NOT EXISTS `inline_comments` ("
                              "`id` int(11) NOT NULL AUTO_INCREMENT,"
                              "`user_id` int(11) NOT NULL,"
                              "`post_id` int(11) NOT NULL,"
                              "`content` TEXT NOT NULL,"
                              "`selected_text` TEXT,"
                              "`parent_id` int(11) DEFAULT 0,"
                              "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                              "PRIMARY KEY (`id`),"
                              "INDEX `idx_post_id` (`post_id`)"
                              ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
            ExecuteSql(sql);
        }

        void InitDiscussionTable() {
            std::string sql = "CREATE TABLE IF NOT EXISTS `discussions` ("
                              "`id` int(11) NOT NULL AUTO_INCREMENT,"
                              "`title` varchar(255) NOT NULL,"
                              "`content` TEXT NOT NULL,"
                              "`author_id` int(11) NOT NULL,"
                              "`question_id` int(11) DEFAULT 0,"
                              "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                              "`likes` int(11) DEFAULT 0,"
                              "`views` int(11) DEFAULT 0,"
                              "`is_official` tinyint(1) DEFAULT 0,"
                              "PRIMARY KEY (`id`),"
                              "INDEX `idx_author_id` (`author_id`),"
                              "INDEX `idx_question_id` (`question_id`)"
                              ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
            ExecuteSql(sql);
        }

        void InitArticleCommentTable() {
            std::string sql = "CREATE TABLE IF NOT EXISTS `article_comments` ("
                              "`id` int(11) NOT NULL AUTO_INCREMENT,"
                              "`post_id` int(11) NOT NULL,"
                              "`user_id` int(11) NOT NULL,"
                              "`content` TEXT NOT NULL,"
                              "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                              "`likes` int(11) DEFAULT 0,"
                              "PRIMARY KEY (`id`),"
                              "INDEX `idx_post_id` (`post_id`),"
                              "INDEX `idx_user_id` (`user_id`)"
                              ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
            ExecuteSql(sql);
        }

        void InitContestTable() {
            std::string sql = "CREATE TABLE IF NOT EXISTS `contests` ("
                              "`id` int(11) NOT NULL AUTO_INCREMENT,"
                              "`contest_id` varchar(50) NOT NULL COMMENT 'Platform ID',"
                              "`name` varchar(255) NOT NULL,"
                              "`start_time` datetime NOT NULL,"
                              "`end_time` datetime NOT NULL,"
                              "`link` varchar(255) NOT NULL,"
                              "`source` varchar(50) DEFAULT 'Codeforces',"
                              "`status` varchar(20) DEFAULT 'upcoming',"
                              "`last_crawl_time` datetime DEFAULT NULL,"
                              "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                              "`updated_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
                              "PRIMARY KEY (`id`),"
                              "UNIQUE KEY `idx_source_id` (`source`, `contest_id`),"
                              "INDEX `idx_status_time` (`status`, `start_time` DESC)"
                              ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
            ExecuteSql(sql);
        }

        bool UpsertContest(const Contest &c) {
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            auto escape = [&](const std::string &s) -> std::string {
                char *buf = new char[s.length() * 2 + 1];
                mysql_real_escape_string(my, buf, s.c_str(), s.length());
                std::string res(buf);
                delete[] buf;
                return res;
            };

            // INSERT ... ON DUPLICATE KEY UPDATE
            std::string sql = "INSERT INTO " + oj_contests + " (contest_id, name, start_time, end_time, link, source, status, last_crawl_time) VALUES ('"
                + escape(c.contest_id) + "', '"
                + escape(c.name) + "', '"
                + c.start_time + "', '"
                + c.end_time + "', '"
                + escape(c.link) + "', '"
                + escape(c.source) + "', '"
                + c.status + "', NOW()) "
                + "ON DUPLICATE KEY UPDATE "
                + "name='" + escape(c.name) + "', "
                + "start_time='" + c.start_time + "', "
                + "end_time='" + c.end_time + "', "
                + "status='" + c.status + "', "
                + "last_crawl_time=NOW()";

            if(0 != mysql_query(my, sql.c_str())) {
                LOG(WARNING) << sql << " execute error: " << mysql_error(my) << "\n";

                return false;
            }

            return true;
        }

        bool GetContests(int page, int page_size, const std::string &status_filter, std::vector<Contest> *out, int *total) {
            int offset = (page - 1) * page_size;
            if (offset < 0) offset = 0;

            std::string where = " WHERE 1=1 ";
            if (!status_filter.empty()) {
                // Assuming status_filter is safe or validated by controller
                // Better escape it too
                 // But here I don't have connection open yet. 
                 // status_filter is enum like, so minimal risk if controller checks.
                 // I'll trust controller for now or escape inside.
                 where += " AND status='" + status_filter + "' ";
            }

            // Count
            std::string count_sql = "SELECT COUNT(*) FROM " + oj_contests + where;
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            if(0 != mysql_query(my, count_sql.c_str())) {

                return false;
            }
            MYSQL_RES *res = mysql_store_result(my);
            if (res) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if (row) *total = atoi(row[0]);
                mysql_free_result(res);
            }

            // Fetch
            // Sort logic: 
            // 1. Status priority: Running > Upcoming > Ended
            // 2. Upcoming: Sort by start_time ASC (Nearest future contest first)
            // 3. Running/Ended: Sort by start_time DESC (Most recent first)
            std::string order_by = " ORDER BY FIELD(status, 'running', 'upcoming', 'ended'), "
                                   "CASE WHEN status = 'upcoming' THEN start_time END ASC, "
                                   "CASE WHEN status != 'upcoming' THEN start_time END DESC ";
            
            std::string sql = "SELECT id, contest_id, name, start_time, end_time, link, source, status, last_crawl_time FROM " 
                              + oj_contests + where + order_by + " LIMIT " + std::to_string(offset) + ", " + std::to_string(page_size);

            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            
            res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            
            for(int i = 0; i < rows; i++) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                Contest c;
                c.id = row[0] ? row[0] : "";
                c.contest_id = row[1] ? row[1] : "";
                c.name = row[2] ? row[2] : "";
                c.start_time = row[3] ? row[3] : "";
                c.end_time = row[4] ? row[4] : "";
                c.link = row[5] ? row[5] : "";
                c.source = row[6] ? row[6] : "";
                c.status = row[7] ? row[7] : "";
                c.last_crawl_time = row[8] ? row[8] : "";
                out->push_back(c);
            }
            mysql_free_result(res);

            return true;
        }

        void CheckAndUpgradeTable() {
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                LOG(ERROR) << "Upgrade Check: Connect failed" << "\n";
                return;
            }

            // Check content column in submissions
            std::string check_content = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_submissions + "' AND COLUMN_NAME = 'content'";
            if(0 == mysql_query(my, check_content.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_submissions + " ADD COLUMN content TEXT";
                    LOG(INFO) << "Upgrading submissions table: adding content column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check language column in submissions
            std::string check_lang = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_submissions + "' AND COLUMN_NAME = 'language'";
            if(0 == mysql_query(my, check_lang.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_submissions + " ADD COLUMN language VARCHAR(20) DEFAULT 'cpp'";
                    LOG(INFO) << "Upgrading submissions table: adding language column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check role column in users
            std::string check_role = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_users + "' AND COLUMN_NAME = 'role'";
            if(0 == mysql_query(my, check_role.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_users + " ADD COLUMN role INT DEFAULT 0 COMMENT '0:User, 1:Admin'";
                    LOG(INFO) << "Upgrading users table: adding role column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check avatar column in users
            std::string check_avatar = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_users + "' AND COLUMN_NAME = 'avatar'";
            if(0 == mysql_query(my, check_avatar.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_users + " ADD COLUMN avatar VARCHAR(255) DEFAULT NULL";
                    LOG(INFO) << "Upgrading users table: adding avatar column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check created_at column in users
            std::string check_created = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_users + "' AND COLUMN_NAME = 'created_at'";
            if(0 == mysql_query(my, check_created.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_users + " ADD COLUMN created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP";
                    LOG(INFO) << "Upgrading users table: adding created_at column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check status column in oj_questions
            std::string check_status = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_questions + "' AND COLUMN_NAME = 'status'";
            if(0 == mysql_query(my, check_status.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_questions + " ADD COLUMN status INT DEFAULT 1 COMMENT '0:Hidden, 1:Visible'";
                    LOG(INFO) << "Upgrading oj_questions table: adding status column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check parent_id column in inline_comments
            std::string check_parent = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_inline_comments + "' AND COLUMN_NAME = 'parent_id'";
            if(0 == mysql_query(my, check_parent.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_inline_comments + " ADD COLUMN parent_id INT DEFAULT 0";
                    LOG(INFO) << "Upgrading inline_comments table: adding parent_id column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }
            
            // Check question_id column in discussions
            std::string check_qid = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_discussions + "' AND COLUMN_NAME = 'question_id'";
            if(0 == mysql_query(my, check_qid.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_discussions + " ADD COLUMN question_id INT DEFAULT 0, ADD INDEX idx_question_id (question_id)";
                    LOG(INFO) << "Upgrading discussions table: adding question_id column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check difficulty column in training_lists
            std::string check_tl_diff = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_training_lists + "' AND COLUMN_NAME = 'difficulty'";
            if(0 == mysql_query(my, check_tl_diff.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_training_lists + " ADD COLUMN difficulty VARCHAR(50) DEFAULT 'Unrated'";
                    LOG(INFO) << "Upgrading training_lists table: adding difficulty column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check tags column in training_lists
            std::string check_tl_tags = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_training_lists + "' AND COLUMN_NAME = 'tags'";
            if(0 == mysql_query(my, check_tl_tags.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_training_lists + " ADD COLUMN tags TEXT";
                    LOG(INFO) << "Upgrading training_lists table: adding tags column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check visibility column in training_lists
            std::string check_tl_vis = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_training_lists + "' AND COLUMN_NAME = 'visibility'";
            if(0 == mysql_query(my, check_tl_vis.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_training_lists + " ADD COLUMN visibility ENUM('public', 'private') DEFAULT 'public'";
                    LOG(INFO) << "Upgrading training_lists table: adding visibility column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check likes column in training_lists
            std::string check_tl_likes = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_training_lists + "' AND COLUMN_NAME = 'likes'";
            if(0 == mysql_query(my, check_tl_likes.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_training_lists + " ADD COLUMN likes INT DEFAULT 0";
                    LOG(INFO) << "Upgrading training_lists table: adding likes column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check collections column in training_lists
            std::string check_tl_col = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_training_lists + "' AND COLUMN_NAME = 'collections'";
            if(0 == mysql_query(my, check_tl_col.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_training_lists + " ADD COLUMN collections INT DEFAULT 0";
                    LOG(INFO) << "Upgrading training_lists table: adding collections column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check order_index column in training_list_items
            std::string check_tli_order = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_training_list_items + "' AND COLUMN_NAME = 'order_index'";
            if(0 == mysql_query(my, check_tli_order.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_training_list_items + " ADD COLUMN order_index INT NOT NULL DEFAULT 0";
                    LOG(INFO) << "Upgrading training_list_items table: adding order_index column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check status column in users
            std::string check_user_status = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_users + "' AND COLUMN_NAME = 'status'";
            if(0 == mysql_query(my, check_user_status.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_users + " ADD COLUMN status INT DEFAULT 0 COMMENT '0:Normal, 1:Banned'";
                    LOG(INFO) << "Upgrading users table: adding status column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

        }

        bool ExecuteSql(const std::string &sql) {
             ConnectionGuard guard;
             MYSQL *my = guard.get();
             if (!my) return false;
             if(0 != mysql_query(my, sql.c_str())) {
                 std::string err_msg = mysql_error(my);
                 LOG(WARNING) << sql << " execute error: " << err_msg << "\n";
                 std::cerr << "SQL Execute Error: " << err_msg << "\nSQL: " << sql << std::endl;
                 return false;
             }
             return true;
        }

        bool QueryMySql(const std::string &sql, vector<Question> *out)
        {
             ConnectionGuard guard;
             MYSQL *my = guard.get();
             if (!my) return false;

            // 执行sql语句
            if(0 != mysql_query(my, sql.c_str()))
            {
                LOG(WARNING) << sql << " execute error!" << "\n";
                return false;
            }

            // 提取结果
            MYSQL_RES *res = mysql_store_result(my);

            // 分析结果
            int rows = mysql_num_rows(res); //获得行数量
            int fields = mysql_num_fields(res); //获得列数量

            Question q;

            for(int i = 0; i < rows; i++)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                
                q.number = row[0] ? row[0] : "";
                q.title = row[1] ? row[1] : "";
                q.star = row[2] ? row[2] : "";
                q.cpu_limit = row[3] ? atoi(row[3]) : 0;
                q.mem_limit = row[4] ? atoi(row[4]) : 0;
                q.desc = row[5] ? row[5] : "";
                q.tail = row[6] ? row[6] : "";
                if(fields > 7) q.status = row[7] ? atoi(row[7]) : 1;
                else q.status = 1; // Default visible

                out->push_back(q);
            }
            // 释放结果空间
            mysql_free_result(res);

            return true;
        }

        bool QueryUserMySql(const std::string &sql, vector<User> *out)
        {
             ConnectionGuard guard;
             MYSQL *my = guard.get();
             if (!my) return false;

            if(0 != mysql_query(my, sql.c_str()))
            {
                LOG(WARNING) << sql << " execute error!" << "\n";
                return false;
            }
            MYSQL_RES *res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            int fields = mysql_num_fields(res);
            
            User u;
            for(int i = 0; i < rows; i++)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                u.id = row[0] ? row[0] : "";
                u.username = row[1] ? row[1] : "";
                u.password = row[2] ? row[2] : "";
                u.email = row[3] ? row[3] : "";
                if(fields > 4) u.nickname = row[4] ? row[4] : "";
                if(fields > 5) u.phone = row[5] ? row[5] : "";
                if(fields > 6) u.created_at = row[6] ? row[6] : "";
                if(fields > 7) u.role = row[7] ? atoi(row[7]) : 0;
                else u.role = 0;
                if(fields > 8) u.avatar = row[8] ? row[8] : "";
                if(fields > 9) u.status = row[9] ? atoi(row[9]) : 0;
                else u.status = 0;
                
                out->push_back(u);
            }
            mysql_free_result(res);
            return true;
        }

        bool DeleteInlineComment(const std::string &comment_id, const std::string &user_id, int role)
        {
            // First check if the comment belongs to the user or user is admin
            // Role: 1 is admin
            
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }
            
            std::string check_sql = "SELECT user_id FROM " + oj_inline_comments + " WHERE id=" + comment_id;
            if(0 != mysql_query(my, check_sql.c_str())) {

                return false;
            }
            
            MYSQL_RES *res = mysql_store_result(my);
            if(mysql_num_rows(res) == 0) {
                mysql_free_result(res);

                return false; // Not found
            }
            
            MYSQL_ROW row = mysql_fetch_row(res);
            std::string owner_id = row[0] ? row[0] : "";
            mysql_free_result(res);
            
            if (role != 1 && owner_id != user_id) {

                return false; // Not authorized
            }
            
            std::string sql = "DELETE FROM " + oj_inline_comments + " WHERE id=" + comment_id;
            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }

            return true;
        }

        bool AddSubmission(const Submission &sub)
        {
            // Escape content to avoid SQL injection
            // We need a connection to escape string, or just use a simple escape manually or use a prepared statement (C API doesn't make prepared statements easy with simple query strings)
            // For now, let's just do a basic connection to escape, or trust the caller? No, must escape.
            
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }
            
            char* escaped_content = new char[sub.content.length() * 2 + 1];
            mysql_real_escape_string(my, escaped_content, sub.content.c_str(), sub.content.length());
            
            std::string sql = "INSERT INTO " + oj_submissions + " (user_id, question_id, result, cpu_time, mem_usage, content, language) VALUES (";
            sql += "'" + sub.user_id + "', ";
            sql += "'" + sub.question_id + "', ";
            sql += "'" + sub.result + "', ";
            sql += std::to_string(sub.cpu_time) + ", ";
            sql += std::to_string(sub.mem_usage) + ", ";
            sql += "'";
            sql += escaped_content;
            sql += "', '";
            sql += sub.language;
            sql += "')";
            
            delete[] escaped_content;
            
            if(0 != mysql_query(my, sql.c_str())) {
                LOG(WARNING) << sql << " execute error: " << mysql_error(my) << "\n";

                return false;
            }

            return true;
        }
        
        // Get Submissions with filters and pagination
        // Filters: user_id, question_id, status, start_time, end_time, keyword (in content or question_id)
        bool GetSubmissions(const std::string &user_id, 
                            const std::string &question_id, 
                            const std::string &status,
                            const std::string &start_time,
                            const std::string &end_time,
                            const std::string &keyword,
                            int offset, int limit,
                            std::vector<Submission> *out,
                            int *total)
        {
            std::string where_clause = " WHERE 1=1 ";
            if(!user_id.empty()) where_clause += " AND s.user_id='" + user_id + "' ";
            if(!question_id.empty()) where_clause += " AND s.question_id='" + question_id + "' ";
            if(!status.empty()) where_clause += " AND s.result='" + status + "' ";
            if(!start_time.empty()) where_clause += " AND s.created_at >= '" + start_time + "' ";
            if(!end_time.empty()) where_clause += " AND s.created_at <= '" + end_time + "' ";
            if(!keyword.empty()) {
                // keyword search in question_id or content (if content is searchable, but content is large)
                // user requirement: "Content keywords"
                where_clause += " AND s.content LIKE '%" + keyword + "%' "; 
            }

            // Count total first
            std::string count_sql = "SELECT count(*) FROM " + oj_submissions + " s " + where_clause;
            
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            if(0 != mysql_query(my, count_sql.c_str())) {

                return false;
            }
            MYSQL_RES *res = mysql_store_result(my);
            if (res) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if (row) *total = atoi(row[0]);
                mysql_free_result(res);
            }

            // Fetch data
            std::string sql = "SELECT s.id, s.user_id, s.question_id, q.title, s.result, s.cpu_time, s.mem_usage, s.created_at, s.content FROM " 
                              + oj_submissions + " s LEFT JOIN " + oj_questions + " q ON s.question_id = q.number " 
                              + where_clause + " ORDER BY s.created_at DESC LIMIT " + std::to_string(offset) + ", " + std::to_string(limit);
            
            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            
            res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            int fields = mysql_num_fields(res);
            
            for(int i = 0; i < rows; i++)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                
                Submission s;
                s.id = row[0] ? row[0] : "";
                s.user_id = row[1] ? row[1] : "";
                s.question_id = row[2] ? row[2] : "";
                s.question_title = row[3] ? row[3] : "";
                s.result = row[4] ? row[4] : "";
                s.cpu_time = row[5] ? atoi(row[5]) : 0;
                s.mem_usage = row[6] ? atoi(row[6]) : 0;
                s.created_at = row[7] ? row[7] : "";
                if (fields > 8) s.content = row[8] ? row[8] : "";
                
                out->push_back(s);
            }
            mysql_free_result(res);

            return true;
        }


        bool AddInlineComment(const InlineComment &comment)
        {
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }
            
            auto escape = [&](const std::string &s) -> std::string {
                char *buf = new char[s.length() * 2 + 1];
                mysql_real_escape_string(my, buf, s.c_str(), s.length());
                std::string res(buf);
                delete[] buf;
                return res;
            };
            
            std::string parent_val = comment.parent_id.empty() ? "0" : comment.parent_id;

            std::string sql = "INSERT INTO " + oj_inline_comments + " (user_id, post_id, content, selected_text, parent_id) VALUES ('"
                + comment.user_id + "', '"
                + comment.post_id + "', '"
                + escape(comment.content) + "', '"
                + escape(comment.selected_text) + "', "
                + parent_val + ")";
            
            if(0 != mysql_query(my, sql.c_str())) {
                LOG(WARNING) << sql << " execute error: " << mysql_error(my) << "\n";

                return false;
            }

            return true;
        }

        bool GetInlineComments(const std::string &post_id, std::vector<InlineComment> *out)
        {
            std::string sql = "SELECT c.id, c.user_id, u.username, c.post_id, c.content, c.selected_text, c.created_at, c.parent_id, u.avatar FROM " 
                              + oj_inline_comments + " c LEFT JOIN " + oj_users + " u ON c.user_id = u.id " 
                              + "WHERE c.post_id='" + post_id + "' ORDER BY c.created_at ASC";
            
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }
            
            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            
            MYSQL_RES *res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            int fields = mysql_num_fields(res);
            
            for(int i = 0; i < rows; i++)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                
                InlineComment c;
                c.id = row[0] ? row[0] : "";
                c.user_id = row[1] ? row[1] : "";
                c.username = row[2] ? row[2] : "Unknown";
                c.post_id = row[3] ? row[3] : "";
                c.content = row[4] ? row[4] : "";
                c.selected_text = row[5] ? row[5] : "";
                c.created_at = row[6] ? row[6] : "";
                c.parent_id = (fields > 7 && row[7]) ? row[7] : "0";
                c.user_avatar = (fields > 8 && row[8]) ? row[8] : "";
                
                out->push_back(c);
            }
            mysql_free_result(res);

            return true;
        }

        // Return map: Difficulty -> Count
        bool GetUserSolvedStats(const std::string &user_id, std::unordered_map<std::string, int> *stats)
        {
            // Join submissions and questions to get difficulty of solved problems
            // Distinct question_id to count each problem once
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

        bool GetAllQuestions(vector<Question> *out)
        {
            if (Cache::GetInstance().GetAllQuestions(out)) {
                return true;
            }
            // Only show visible questions for normal users
            std::string sql = "select number, title, star, cpu_limit, mem_limit, description, tail_code, status from ";
            sql += oj_questions;
            sql += " where status=1";
            bool ret = QueryMySql(sql, out);
            if (ret) {
                Cache::GetInstance().SetAllQuestions(*out);
            }
            return ret;
        }

        bool GetQuestionsByPage(int page, int page_size, vector<Question> *out, int *total)
        {
            int offset = (page - 1) * page_size;
            if (offset < 0) offset = 0;

            // 1. Get total count
            std::string count_sql = "select count(*) from " + oj_questions + " where status=1";
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            if(0 != mysql_query(my, count_sql.c_str())) {

                return false;
            }
            MYSQL_RES *res = mysql_store_result(my);
            if (res) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if (row) *total = atoi(row[0]);
                mysql_free_result(res);
            }

            // 2. Get paginated data
            // Note: number is varchar, so CAST AS UNSIGNED for correct numeric sorting
            std::string sql = "select number, title, star, cpu_limit, mem_limit, description, tail_code, status from " + oj_questions + " where status=1 ORDER BY CAST(number AS UNSIGNED) ASC LIMIT " + std::to_string(offset) + ", " + std::to_string(page_size);
            
            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            
            res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            int fields = mysql_num_fields(res);
            
            for(int i = 0; i < rows; i++)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                
                Question q;
                q.number = row[0] ? row[0] : "";
                q.title = row[1] ? row[1] : "";
                q.star = row[2] ? row[2] : "";
                q.cpu_limit = row[3] ? atoi(row[3]) : 0;
                q.mem_limit = row[4] ? atoi(row[4]) : 0;
                q.desc = row[5] ? row[5] : "";
                q.tail = row[6] ? row[6] : "";
                if(fields > 7) q.status = row[7] ? atoi(row[7]) : 1;
                else q.status = 1;

                out->push_back(q);
            }
            mysql_free_result(res);

            return true;
        }

        bool GetAllQuestionsAdmin(int page, int page_size, vector<Question> *out, int *total)
        {
            int offset = (page - 1) * page_size;
            if (offset < 0) offset = 0;

            // 1. Get total count
            std::string count_sql = "select count(*) from " + oj_questions;
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            if(0 != mysql_query(my, count_sql.c_str())) {

                return false;
            }
            MYSQL_RES *res = mysql_store_result(my);
            if (res) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if (row) *total = atoi(row[0]);
                mysql_free_result(res);
            }

            // 2. Get paginated data
            // Show all questions for admin
            std::string sql = "select number, title, star, cpu_limit, mem_limit, description, tail_code, status from " + oj_questions + " ORDER BY CAST(number AS UNSIGNED) ASC LIMIT " + std::to_string(offset) + ", " + std::to_string(page_size);
            
            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            
            res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            int fields = mysql_num_fields(res);
            
            for(int i = 0; i < rows; i++)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                
                Question q;
                q.number = row[0] ? row[0] : "";
                q.title = row[1] ? row[1] : "";
                q.star = row[2] ? row[2] : "";
                q.cpu_limit = row[3] ? atoi(row[3]) : 0;
                q.mem_limit = row[4] ? atoi(row[4]) : 0;
                q.desc = row[5] ? row[5] : "";
                q.tail = row[6] ? row[6] : "";
                if(fields > 7) q.status = row[7] ? atoi(row[7]) : 1;
                else q.status = 1;

                out->push_back(q);
            }
            mysql_free_result(res);

            return true;
        }

        bool GetOneQuestion(const std::string &number, Question *q)
        {
            if (Cache::GetInstance().GetQuestion(number, q)) {
                return true;
            }
            bool res = false;
            std::string sql = "select number, title, star, cpu_limit, mem_limit, description, tail_code, status from ";
            sql += oj_questions;
            sql += " where number=";
            sql += number;
            vector<Question> result;
            if(QueryMySql(sql, &result))
            {
                if(result.size() == 1){
                    *q = result[0];
                    res = true;
                    Cache::GetInstance().SetQuestion(number, *q);
                }
            }
            return res;
        }

        bool AddQuestion(const Question &q)
        {
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if (!my) return false;

            // Escape strings
            auto escape = [&](const std::string &s) -> std::string {
                char *buf = new char[s.length() * 2 + 1];
                mysql_real_escape_string(my, buf, s.c_str(), s.length());
                std::string res(buf);
                delete[] buf;
                return res;
            };

            std::string sql = "INSERT INTO " + oj_questions + " (title, star, cpu_limit, mem_limit, description, tail_code, status) VALUES ('"
                + escape(q.title) + "', '"
                + escape(q.star) + "', "
                + std::to_string(q.cpu_limit) + ", "
                + std::to_string(q.mem_limit) + ", '"
                + escape(q.desc) + "', '"
                + escape(q.tail) + "', "
                + std::to_string(q.status) + ")";

            if(0 != mysql_query(my, sql.c_str())) {
                LOG(WARNING) << sql << " execute error: " << mysql_error(my) << "\n";
                return false;
            }
            Cache::GetInstance().InvalidateAllQuestions();
            return true;
        }

        bool UpdateQuestion(const Question &q)
        {
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if (!my) return false;

            auto escape = [&](const std::string &s) -> std::string {
                char *buf = new char[s.length() * 2 + 1];
                mysql_real_escape_string(my, buf, s.c_str(), s.length());
                std::string res(buf);
                delete[] buf;
                return res;
            };

            std::string sql = "UPDATE " + oj_questions + " SET "
                + "title='" + escape(q.title) + "', "
                + "star='" + escape(q.star) + "', "
                + "cpu_limit=" + std::to_string(q.cpu_limit) + ", "
                + "mem_limit=" + std::to_string(q.mem_limit) + ", "
                + "description='" + escape(q.desc) + "', "
                + "tail_code='" + escape(q.tail) + "', "
                + "status=" + std::to_string(q.status)
                + " WHERE number=" + q.number;

            if(0 != mysql_query(my, sql.c_str())) {
                LOG(WARNING) << sql << " execute error: " << mysql_error(my) << "\n";
                return false;
            }
            Cache::GetInstance().InvalidateQuestion(q.number);
            return true;
        }

        bool DeleteQuestion(const std::string &number)
        {
            std::string sql = "DELETE FROM " + oj_questions + " WHERE number=" + number;
            bool ret = ExecuteSql(sql);
            if (ret) {
                Cache::GetInstance().InvalidateQuestion(number);
            }
            return ret;
        }

        // User Auth Methods
        std::string SHA256Hash(const std::string &str) {
             unsigned char hash[SHA256_DIGEST_LENGTH];
             SHA256_CTX sha256;
             SHA256_Init(&sha256);
             SHA256_Update(&sha256, str.c_str(), str.size());
             SHA256_Final(hash, &sha256);
             std::stringstream ss;
             for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
             {
                 ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
             }
             return ss.str();
        }

        bool RegisterUser(const std::string &username, const std::string &password, const std::string &email, const std::string &nickname = "", const std::string &phone = "") {
            // Check if exists
            std::string sql_check = "select id, username, password, email, nickname, phone, created_at, role, avatar, status from " + oj_users + " where username='" + username + "'";
            std::vector<User> users;
            if (!QueryUserMySql(sql_check, &users)) {
                LOG(ERROR) << "查询用户失败，数据库错误: " << username << "\n";
                return false; // Database error
            }
            
            if (!users.empty()) {
                LOG(INFO) << "用户名已存在: " << username << "\n";
                return false; // User exists
            }

            // Insert
            std::string pwd_hash = SHA256Hash(password);
            std::string sql_insert = "insert into " + oj_users + " (username, password, email, nickname, phone) values ('" 
                                     + username + "', '" + pwd_hash + "', '" + email + "', '" + nickname + "', '" + phone + "')";
            bool result = ExecuteSql(sql_insert);
            if (result) {
                LOG(INFO) << "用户注册成功: " << username << "\n";
            } else {
                LOG(ERROR) << "用户注册失败，数据库插入错误: " << username << "\n";
            }
            return result;
        }

        bool LoginUser(const std::string &username, const std::string &password, User *user) {
            std::string sql = "select id, username, password, email, nickname, phone, created_at, role, avatar, status from " + oj_users + " where username='" + username + "'";
            std::vector<User> users;
            if (QueryUserMySql(sql, &users) && users.size() == 1) {
                std::string pwd_hash = SHA256Hash(password);
                if (users[0].password == pwd_hash) {
                    *user = users[0];
                    return true;
                }
            }
            return false;
        }

        bool UpdateUser(const User &user) {
            std::string sql = "UPDATE " + oj_users + " SET ";
            bool first = true;
            if (!user.nickname.empty()) {
                sql += "nickname='" + user.nickname + "'";
                first = false;
            }
            if (!user.email.empty()) {
                if (!first) sql += ", ";
                sql += "email='" + user.email + "'";
                first = false;
            }
            if (!user.phone.empty()) {
                if (!first) sql += ", ";
                sql += "phone='" + user.phone + "'";
                first = false;
            }
            if (!user.avatar.empty()) {
                if (!first) sql += ", ";
                sql += "avatar='" + user.avatar + "'";
                first = false;
            }
            
            
            sql += " WHERE id='" + user.id + "'";
            
            if (first) return true; // No fields to update
            
            return ExecuteSql(sql);
        }

        bool AddDiscussion(const Discussion &d)
        {
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            auto escape = [&](const std::string &s) -> std::string {
                char *buf = new char[s.length() * 2 + 1];
                mysql_real_escape_string(my, buf, s.c_str(), s.length());
                std::string res(buf);
                delete[] buf;
                return res;
            };

            std::string sql = "INSERT INTO " + oj_discussions + " (title, content, author_id, is_official, question_id) VALUES ('"
                + escape(d.title) + "', '"
                + escape(d.content) + "', '"
                + d.author_id + "', "
                + (d.is_official ? "1" : "0") + ", "
                + (d.question_id.empty() ? "0" : d.question_id) + ")";

            if(0 != mysql_query(my, sql.c_str())) {
                LOG(WARNING) << sql << " execute error: " << mysql_error(my) << "\n";

                return false;
            }

            return true;
        }

        bool DeleteDiscussion(const std::string &discussion_id, const std::string &user_id, int role)
        {
            MYSQL *my = mysql_init(nullptr);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                mysql_close(my);
                return false;
            }
            if(0 != mysql_set_character_set(my, "utf8mb4")) { LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n"; }
            
            std::string check_sql = "SELECT author_id FROM " + oj_discussions + " WHERE id=" + discussion_id;
            if(0 != mysql_query(my, check_sql.c_str())) {
                mysql_close(my);
                return false;
            }
            
            MYSQL_RES *res = mysql_store_result(my);
            if(mysql_num_rows(res) == 0) {
                mysql_free_result(res);
                mysql_close(my);
                return false; // Not found
            }
            
            MYSQL_ROW row = mysql_fetch_row(res);
            std::string author_id = row[0] ? row[0] : "";
            mysql_free_result(res);
            
            if (role != 1 && author_id != user_id) {
                mysql_close(my);
                return false; // Not authorized
            }
            
            std::string sql = "DELETE FROM " + oj_discussions + " WHERE id=" + discussion_id;
            if(0 != mysql_query(my, sql.c_str())) {
                mysql_close(my);
                return false;
            }
            
            // Also delete associated comments
            std::string del_inline_comments_sql = "DELETE FROM " + oj_inline_comments + " WHERE post_id=" + discussion_id;
            mysql_query(my, del_inline_comments_sql.c_str());
            
            std::string del_article_comments_sql = "DELETE FROM " + oj_article_comments + " WHERE post_id=" + discussion_id;
            mysql_query(my, del_article_comments_sql.c_str());
            
            mysql_close(my);
            return true;
        }

        bool GetAllDiscussions(std::vector<Discussion> *out)
        {
            // Join with users to get author name and questions to get title
            std::string sql = "SELECT d.id, d.title, d.content, d.author_id, u.username, d.created_at, d.likes, d.views, d.is_official, "
                              "(SELECT COUNT(*) FROM " + oj_article_comments + " WHERE post_id = d.id) as comments_count, d.question_id, q.title, u.avatar "
                              "FROM " + oj_discussions + " d "
                              "LEFT JOIN " + oj_users + " u ON d.author_id = u.id "
                              "LEFT JOIN " + oj_questions + " q ON d.question_id = q.number "
                              "ORDER BY d.created_at DESC";
            
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }
            
            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            
            MYSQL_RES *res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            int fields = mysql_num_fields(res);
            
            for(int i = 0; i < rows; i++)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                
                Discussion d;
                d.id = row[0] ? row[0] : "";
                d.title = row[1] ? row[1] : "";
                d.content = row[2] ? row[2] : "";
                d.author_id = row[3] ? row[3] : "";
                d.author_name = row[4] ? row[4] : "Unknown";
                d.created_at = row[5] ? row[5] : "";
                d.likes = row[6] ? atoi(row[6]) : 0;
                d.views = row[7] ? atoi(row[7]) : 0;
                d.is_official = (row[8] && atoi(row[8]) == 1);
                d.comments_count = row[9] ? atoi(row[9]) : 0;
                d.question_id = row[10] ? row[10] : "0";
                if (fields > 11) d.question_title = row[11] ? row[11] : "";
                d.author_avatar = (fields > 12 && row[12]) ? row[12] : "";
                
                out->push_back(d);
            }
            mysql_free_result(res);

            return true;
        }

        bool GetDiscussionsByQuestionId(const std::string &qid, std::vector<Discussion> *out)
        {
            std::string sql = "SELECT d.id, d.title, d.content, d.author_id, u.username, d.created_at, d.likes, d.views, d.is_official, "
                              "(SELECT COUNT(*) FROM " + oj_article_comments + " WHERE post_id = d.id) as comments_count, d.question_id, q.title, u.avatar "
                              "FROM " + oj_discussions + " d "
                              "LEFT JOIN " + oj_users + " u ON d.author_id = u.id "
                              "LEFT JOIN " + oj_questions + " q ON d.question_id = q.number "
                              "WHERE d.question_id=" + qid + " "
                              "ORDER BY d.created_at DESC";
            
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }
            
            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            
            MYSQL_RES *res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            int fields = mysql_num_fields(res);
            
            for(int i = 0; i < rows; i++)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                
                Discussion d;
                d.id = row[0] ? row[0] : "";
                d.title = row[1] ? row[1] : "";
                d.content = row[2] ? row[2] : "";
                d.author_id = row[3] ? row[3] : "";
                d.author_name = row[4] ? row[4] : "Unknown";
                d.created_at = row[5] ? row[5] : "";
                d.likes = row[6] ? atoi(row[6]) : 0;
                d.views = row[7] ? atoi(row[7]) : 0;
                d.is_official = (row[8] && atoi(row[8]) == 1);
                d.comments_count = row[9] ? atoi(row[9]) : 0;
                d.question_id = row[10] ? row[10] : "0";
                if (fields > 11) d.question_title = row[11] ? row[11] : "";
                d.author_avatar = (fields > 12 && row[12]) ? row[12] : "";
                
                out->push_back(d);
            }
            mysql_free_result(res);

            return true;
        }

        bool GetOneDiscussion(const std::string &id, Discussion *d)
        {
            std::string sql = "SELECT d.id, d.title, d.content, d.author_id, u.username, d.created_at, d.likes, d.views, d.is_official, "
                              "(SELECT COUNT(*) FROM " + oj_article_comments + " WHERE post_id = d.id) as comments_count, d.question_id, q.title, u.avatar "
                              "FROM " + oj_discussions + " d "
                              "LEFT JOIN " + oj_users + " u ON d.author_id = u.id "
                              "LEFT JOIN " + oj_questions + " q ON d.question_id = q.number "
                              "WHERE d.id=" + id;
            
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }
            
            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            
            MYSQL_RES *res = mysql_store_result(my);
            int fields = mysql_num_fields(res);
            if (mysql_num_rows(res) == 1) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row) {
                    d->id = row[0] ? row[0] : "";
                    d->title = row[1] ? row[1] : "";
                    d->content = row[2] ? row[2] : "";
                    d->author_id = row[3] ? row[3] : "";
                    d->author_name = row[4] ? row[4] : "Unknown";
                    d->created_at = row[5] ? row[5] : "";
                    d->likes = row[6] ? atoi(row[6]) : 0;
                    d->views = row[7] ? atoi(row[7]) : 0;
                    d->is_official = (row[8] && atoi(row[8]) == 1);
                    d->comments_count = row[9] ? atoi(row[9]) : 0;
                    d->question_id = row[10] ? row[10] : "0";
                    if (fields > 11) d->question_title = row[11] ? row[11] : "";
                    d->author_avatar = (fields > 12 && row[12]) ? row[12] : "";
                    
                    mysql_free_result(res);

                    return true;
                }
            }
            
            mysql_free_result(res);

            return false;
        }

        bool AddArticleComment(const ArticleComment &c)
        {
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }
            
            auto escape = [&](const std::string &s) -> std::string {
                char *buf = new char[s.length() * 2 + 1];
                mysql_real_escape_string(my, buf, s.c_str(), s.length());
                std::string res(buf);
                delete[] buf;
                return res;
            };
            
            std::string sql = "INSERT INTO " + oj_article_comments + " (user_id, post_id, content) VALUES ('"
                + c.user_id + "', '"
                + c.post_id + "', '"
                + escape(c.content) + "')";
            
            if(0 != mysql_query(my, sql.c_str())) {
                LOG(WARNING) << sql << " execute error: " << mysql_error(my) << "\n";

                return false;
            }

            return true;
        }

        bool GetArticleComments(const std::string &post_id, std::vector<ArticleComment> *out)
        {
            std::string sql = "SELECT c.id, c.user_id, u.username, c.post_id, c.content, c.created_at, c.likes, u.avatar FROM " 
                              + oj_article_comments + " c LEFT JOIN " + oj_users + " u ON c.user_id = u.id " 
                              + "WHERE c.post_id='" + post_id + "' ORDER BY c.created_at DESC";
            
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }
            
            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            
            MYSQL_RES *res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            int fields = mysql_num_fields(res);
            
            for(int i = 0; i < rows; i++)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                
                ArticleComment c;
                c.id = row[0] ? row[0] : "";
                c.user_id = row[1] ? row[1] : "";
                c.username = row[2] ? row[2] : "Unknown";
                c.post_id = row[3] ? row[3] : "";
                c.content = row[4] ? row[4] : "";
                c.created_at = row[5] ? row[5] : "";
                c.likes = row[6] ? atoi(row[6]) : 0;
                c.user_avatar = (fields > 7 && row[7]) ? row[7] : "";
                
                out->push_back(c);
            }
            mysql_free_result(res);

            return true;
        }

        // Training List Methods

        bool CreateTrainingList(const TrainingList &list, int *new_id) {
            if (list.author_id.empty()) {
                LOG(WARNING) << "CreateTrainingList failed: author_id is empty" << "\n";
                return false;
            }

            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                std::cerr << "Connect failed: " << mysql_error(my) << std::endl;
                return false;
            }

            auto escape = [&](const std::string &s) -> std::string {
                char *buf = new char[s.length() * 2 + 1];
                mysql_real_escape_string(my, buf, s.c_str(), s.length());
                std::string res(buf);
                delete[] buf;
                return res;
            };

            std::string sql = "INSERT INTO " + oj_training_lists + " (title, description, difficulty, tags, author_id, visibility) VALUES ('"
                + escape(list.title) + "', '"
                + escape(list.description) + "', '"
                + escape(list.difficulty) + "', '"
                + escape(list.tags) + "', "
                + list.author_id + ", '"
                + escape(list.visibility) + "')";

            if(0 != mysql_query(my, sql.c_str())) {
                std::string err_msg = mysql_error(my);
                LOG(WARNING) << sql << " execute error: " << err_msg << "\n";
                std::cerr << "SQL Error in CreateTrainingList: " << err_msg << "\nSQL: " << sql << std::endl;

                return false;
            }
            
            *new_id = (int)mysql_insert_id(my);

            return true;
        }

        bool UpdateTrainingList(const TrainingList &list) {
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            auto escape = [&](const std::string &s) -> std::string {
                char *buf = new char[s.length() * 2 + 1];
                mysql_real_escape_string(my, buf, s.c_str(), s.length());
                std::string res(buf);
                delete[] buf;
                return res;
            };

            std::string sql = "UPDATE " + oj_training_lists + " SET "
                + "title='" + escape(list.title) + "', "
                + "description='" + escape(list.description) + "', "
                + "difficulty='" + escape(list.difficulty) + "', "
                + "tags='" + escape(list.tags) + "', "
                + "visibility='" + escape(list.visibility) + "' "
                + "WHERE id=" + list.id;

            if(0 != mysql_query(my, sql.c_str())) {
                std::string err_msg = mysql_error(my);
                LOG(WARNING) << sql << " execute error: " << err_msg << "\n";
                std::cerr << "SQL Error in UpdateTrainingList: " << err_msg << "\nSQL: " << sql << std::endl;

                return false;
            }

            return true;
        }

        bool DeleteTrainingList(const std::string &id) {
            std::string sql = "DELETE FROM " + oj_training_lists + " WHERE id=" + id;
            return ExecuteSql(sql);
        }

        bool GetTrainingList(const std::string &id, TrainingList *list) {
            std::string sql = "SELECT t.id, t.title, t.description, t.difficulty, t.tags, t.author_id, t.visibility, t.created_at, t.updated_at, t.likes, t.collections, u.username, u.avatar, "
                              "(SELECT COUNT(*) FROM " + oj_training_list_items + " WHERE training_list_id = t.id) as problem_count "
                              "FROM " + oj_training_lists + " t "
                              "LEFT JOIN " + oj_users + " u ON t.author_id = u.id "
                              "WHERE t.id=" + id;
            
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }

            MYSQL_RES *res = mysql_store_result(my);
            if (mysql_num_rows(res) == 1) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row) {
                    list->id = row[0] ? row[0] : "";
                    list->title = row[1] ? row[1] : "";
                    list->description = row[2] ? row[2] : "";
                    list->difficulty = row[3] ? row[3] : "Unrated";
                    list->tags = row[4] ? row[4] : "[]";
                    list->author_id = row[5] ? row[5] : "";
                    list->visibility = row[6] ? row[6] : "public";
                    list->created_at = row[7] ? row[7] : "";
                    list->updated_at = row[8] ? row[8] : "";
                    list->likes = row[9] ? atoi(row[9]) : 0;
                    list->collections = row[10] ? atoi(row[10]) : 0;
                    list->author_name = row[11] ? row[11] : "Unknown";
                    list->author_avatar = row[12] ? row[12] : "";
                    list->problem_count = row[13] ? atoi(row[13]) : 0;
                    
                    mysql_free_result(res);

                    return true;
                }
            }
            mysql_free_result(res);

            return false;
        }

        bool GetTrainingLists(int page, int page_size, const std::string &visibility, const std::string &author_id, std::vector<TrainingList> *out, int *total) {
            int offset = (page - 1) * page_size;
            if (offset < 0) offset = 0;

            std::string where = " WHERE 1=1 ";
            if (!visibility.empty()) {
                where += " AND t.visibility='" + visibility + "' ";
            }
            if (!author_id.empty()) {
                where += " AND t.author_id=" + author_id + " ";
            }

            // Count
            std::string count_sql = "SELECT COUNT(*) FROM " + oj_training_lists + " t " + where;
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            if(0 != mysql_query(my, count_sql.c_str())) {

                return false;
            }
            MYSQL_RES *res = mysql_store_result(my);
            if (res) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if (row) *total = atoi(row[0]);
                mysql_free_result(res);
            }

            // Fetch
            std::string sql = "SELECT t.id, t.title, t.description, t.difficulty, t.tags, t.author_id, t.visibility, t.created_at, t.updated_at, t.likes, t.collections, u.username, u.avatar, "
                              "(SELECT COUNT(*) FROM " + oj_training_list_items + " WHERE training_list_id = t.id) as problem_count "
                              "FROM " + oj_training_lists + " t "
                              "LEFT JOIN " + oj_users + " u ON t.author_id = u.id "
                              + where + " ORDER BY t.created_at DESC LIMIT " + std::to_string(offset) + ", " + std::to_string(page_size);

            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }

            res = mysql_store_result(my);
            int rows = mysql_num_rows(res);

            for(int i = 0; i < rows; i++) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                TrainingList list;
                list.id = row[0] ? row[0] : "";
                list.title = row[1] ? row[1] : "";
                list.description = row[2] ? row[2] : "";
                list.difficulty = row[3] ? row[3] : "Unrated";
                list.tags = row[4] ? row[4] : "[]";
                list.author_id = row[5] ? row[5] : "";
                list.visibility = row[6] ? row[6] : "public";
                list.created_at = row[7] ? row[7] : "";
                list.updated_at = row[8] ? row[8] : "";
                list.likes = row[9] ? atoi(row[9]) : 0;
                list.collections = row[10] ? atoi(row[10]) : 0;
                list.author_name = row[11] ? row[11] : "Unknown";
                list.author_avatar = row[12] ? row[12] : "";
                list.problem_count = row[13] ? atoi(row[13]) : 0;
                out->push_back(list);
            }
            mysql_free_result(res);

            return true;
        }

        bool AddProblemToTrainingList(const std::string &list_id, const std::string &question_id) {
            // Check if exists first to avoid error? Or just let INSERT fail (UNIQUE constraint)
            // But we need to handle "already exists" gracefully?
            // INSERT IGNORE? Or just INSERT and catch error.
            std::string sql = "INSERT IGNORE INTO " + oj_training_list_items + " (training_list_id, question_id, order_index) "
                              "SELECT " + list_id + ", " + question_id + ", COALESCE(MAX(order_index), 0) + 1 FROM " + oj_training_list_items + " WHERE training_list_id=" + list_id;
            return ExecuteSql(sql);
        }

        bool RemoveProblemFromTrainingList(const std::string &list_id, const std::string &question_id) {
            std::string sql = "DELETE FROM " + oj_training_list_items + " WHERE training_list_id=" + list_id + " AND question_id=" + question_id;
            return ExecuteSql(sql);
        }

        bool ReorderTrainingListProblems(const std::string &list_id, const std::vector<std::string> &problem_ids) {
            // This is tricky. Multiple updates.
            // Transaction recommended.
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            mysql_autocommit(my, 0); // Start transaction

            for (size_t i = 0; i < problem_ids.size(); ++i) {
                std::string sql = "UPDATE " + oj_training_list_items + " SET order_index=" + std::to_string(i + 1) + 
                                  " WHERE training_list_id=" + list_id + " AND question_id=" + problem_ids[i];
                if(0 != mysql_query(my, sql.c_str())) {
                    mysql_rollback(my);
                    mysql_autocommit(my, 1); // Reset autocommit
                    return false;
                }
            }

            mysql_commit(my);
            mysql_autocommit(my, 1); // Reset autocommit
            return true;
        }

        bool GetTrainingListProblems(const std::string &list_id, const std::string &user_id, std::vector<TrainingListItem> *out) {
            // Need to join with oj_questions to get title/difficulty
            // And join with submissions (or use subquery) to get user status.
            // User status: Solved (0), Attempted (non-0), Unsolved (no submission).
            // For simplicity, just check if solved.
            
            std::string user_status_sql = "";
            if (!user_id.empty()) {
                user_status_sql = ", (SELECT count(*) FROM " + oj_submissions + " WHERE user_id=" + user_id + " AND question_id=i.question_id AND result='0') as solved ";
            } else {
                user_status_sql = ", 0 as solved ";
            }

            std::string sql = "SELECT i.id, i.training_list_id, i.question_id, i.order_index, q.title, q.star " + user_status_sql +
                              "FROM " + oj_training_list_items + " i "
                              "LEFT JOIN " + oj_questions + " q ON i.question_id = q.number "
                              "WHERE i.training_list_id=" + list_id + " "
                              "ORDER BY i.order_index ASC";

            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }

            MYSQL_RES *res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            
            for(int i = 0; i < rows; i++) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                TrainingListItem item;
                item.id = row[0] ? row[0] : "";
                item.training_list_id = row[1] ? row[1] : "";
                item.question_id = row[2] ? row[2] : "";
                item.order_index = row[3] ? atoi(row[3]) : 0;
                item.question_title = row[4] ? row[4] : "";
                item.question_difficulty = row[5] ? row[5] : "";
                int solved = row[6] ? atoi(row[6]) : 0;
                item.user_status = (solved > 0) ? "Solved" : "Unsolved";
                
                out->push_back(item);
            }
            mysql_free_result(res);

            return true;
        }

        bool GetUsers(int page, int page_size, const std::string &keyword, std::vector<User> *out, int *total) {
            int offset = (page - 1) * page_size;
            if (offset < 0) offset = 0;

            std::string where = " WHERE 1=1 ";
            if (!keyword.empty()) {
                where += " AND (username LIKE '%" + keyword + "%' OR nickname LIKE '%" + keyword + "%' OR email LIKE '%" + keyword + "%') ";
            }

            // Count
            std::string count_sql = "SELECT COUNT(*) FROM " + oj_users + where;
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            if(0 != mysql_query(my, count_sql.c_str())) {

                return false;
            }
            MYSQL_RES *res = mysql_store_result(my);
            if (res) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if (row) *total = atoi(row[0]);
                mysql_free_result(res);
            }

            // Fetch
            std::string sql = "SELECT id, username, password, email, nickname, phone, created_at, role, avatar, status FROM " 
                              + oj_users + where + " ORDER BY created_at DESC LIMIT " + std::to_string(offset) + ", " + std::to_string(page_size);

            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            
            res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            
            for(int i = 0; i < rows; i++) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                User u;
                u.id = row[0] ? row[0] : "";
                u.username = row[1] ? row[1] : "";
                u.password = row[2] ? row[2] : "";
                u.email = row[3] ? row[3] : "";
                u.nickname = row[4] ? row[4] : "";
                u.phone = row[5] ? row[5] : "";
                u.created_at = row[6] ? row[6] : "";
                u.role = row[7] ? atoi(row[7]) : 0;
                u.avatar = row[8] ? row[8] : "";
                u.status = row[9] ? atoi(row[9]) : 0;
                out->push_back(u);
            }
            mysql_free_result(res);

            return true;
        }

        bool UpdateUserStatus(const std::string &id, int status) {
            std::string sql = "UPDATE " + oj_users + " SET status=" + std::to_string(status) + " WHERE id=" + id;
            return ExecuteSql(sql);
        }

        bool UpdateUserRole(const std::string &id, int role) {
            std::string sql = "UPDATE " + oj_users + " SET role=" + std::to_string(role) + " WHERE id=" + id;
            return ExecuteSql(sql);
        }

        bool ResetUserPassword(const std::string &id, const std::string &new_password_hash) {
            std::string sql = "UPDATE " + oj_users + " SET password='" + new_password_hash + "' WHERE id=" + id;
            return ExecuteSql(sql);
        }

        bool GetUserById(const std::string &id, User *user) {
            std::string sql = "SELECT id, username, password, email, nickname, phone, created_at, role, avatar, status FROM " + oj_users + " WHERE id=" + id;
            std::vector<User> users;
            if (QueryUserMySql(sql, &users) && !users.empty()) {
                *user = users[0];
                return true;
            }
            return false;
        }

        // Invitation Code Methods
        bool VerifyAndUseInvitationCode(const std::string &code, const std::string &user_id) {
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            auto escape = [&](const std::string &s) -> std::string {
                char *buf = new char[s.length() * 2 + 1];
                mysql_real_escape_string(my, buf, s.c_str(), s.length());
                std::string res(buf);
                delete[] buf;
                return res;
            };

            std::string safe_code = escape(code);
            std::string sql_check = "SELECT id, is_used FROM " + oj_invitation_codes + " WHERE code='" + safe_code + "'";
            
            if(0 != mysql_query(my, sql_check.c_str())) {

                return false;
            }

            MYSQL_RES *res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            if (rows == 0) {
                mysql_free_result(res);

                return false; // Code not found
            }

            MYSQL_ROW row = mysql_fetch_row(res);
            int is_used = row[1] ? atoi(row[1]) : 1;
            std::string id = row[0] ? row[0] : "0";
            mysql_free_result(res);

            if (is_used) {

                return false; // Code already used
            }

            // Mark as used
            std::string sql_update = "UPDATE " + oj_invitation_codes + " SET is_used=1, used_by=" + user_id + " WHERE id=" + id;
            if(0 != mysql_query(my, sql_update.c_str())) {

                return false;
            }

            return true;
        }

        bool GenerateInvitationCode(const std::string &code) {
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            auto escape = [&](const std::string &s) -> std::string {
                char *buf = new char[s.length() * 2 + 1];
                mysql_real_escape_string(my, buf, s.c_str(), s.length());
                std::string res(buf);
                delete[] buf;
                return res;
            };

            std::string sql = "INSERT INTO " + oj_invitation_codes + " (code) VALUES ('" + escape(code) + "')";
            
            bool ret = true;
            if(0 != mysql_query(my, sql.c_str())) {
                ret = false;
            }

            return ret;
        }

        // Operation Log Methods
        bool LogOperation(const OperationLog &log) {
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            auto escape = [&](const std::string &s) -> std::string {
                char *buf = new char[s.length() * 2 + 1];
                mysql_real_escape_string(my, buf, s.c_str(), s.length());
                std::string res(buf);
                delete[] buf;
                return res;
            };

            std::string sql = "INSERT INTO " + oj_operation_logs + " (user_id, action, target, details, ip) VALUES ('"
                              + log.user_id + "', '"
                              + escape(log.action) + "', '"
                              + escape(log.target) + "', '"
                              + escape(log.details) + "', '"
                              + escape(log.ip) + "')";

            bool ret = true;
            if(0 != mysql_query(my, sql.c_str())) {
                LOG(WARNING) << "LogOperation failed: " << mysql_error(my) << "\n";
                ret = false;
            }

            return ret;
        }

        bool GetLogs(int page, int page_size, const std::string &keyword, std::vector<OperationLog> *out, int *total) {
            int offset = (page - 1) * page_size;
            if (offset < 0) offset = 0;

            std::string where = " WHERE 1=1 ";
            if (!keyword.empty()) {
                where += " AND (action LIKE '%" + keyword + "%' OR target LIKE '%" + keyword + "%' OR details LIKE '%" + keyword + "%') ";
            }

            // Count
            std::string count_sql = "SELECT COUNT(*) FROM " + oj_operation_logs + where;
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            if(0 != mysql_query(my, count_sql.c_str())) {

                return false;
            }
            MYSQL_RES *res = mysql_store_result(my);
            if (res) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if (row) *total = atoi(row[0]);
                mysql_free_result(res);
            }

            // Fetch with user join
            std::string sql = "SELECT l.id, l.user_id, u.username, l.action, l.target, l.details, l.ip, l.created_at FROM " 
                              + oj_operation_logs + " l LEFT JOIN " + oj_users + " u ON l.user_id = u.id "
                              + where + " ORDER BY l.created_at DESC LIMIT " + std::to_string(offset) + ", " + std::to_string(page_size);

            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            
            res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            
            for(int i = 0; i < rows; i++) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                OperationLog l;
                l.id = row[0] ? row[0] : "";
                l.user_id = row[1] ? row[1] : "";
                l.username = row[2] ? row[2] : "Unknown";
                l.action = row[3] ? row[3] : "";
                l.target = row[4] ? row[4] : "";
                l.details = row[5] ? row[5] : "";
                l.ip = row[6] ? row[6] : "";
                l.created_at = row[7] ? row[7] : "";
                out->push_back(l);
            }
            mysql_free_result(res);

            return true;
        }

        // Statistics Methods
        bool GetTotalUserCount(int *count) {
            std::string sql = "SELECT COUNT(*) FROM " + oj_users;
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){

                return false;
            }
            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            MYSQL_RES *res = mysql_store_result(my);
            if (res) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if (row) *count = atoi(row[0]);
                mysql_free_result(res);
            }

            return true;
        }

        bool GetTotalProblemCount(int *count) {
            std::string sql = "SELECT COUNT(*) FROM " + oj_questions;
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }
            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            MYSQL_RES *res = mysql_store_result(my);
            if (res) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if (row) *count = atoi(row[0]);
                mysql_free_result(res);
            }

            return true;
        }

        bool GetTotalSubmissionCount(int *count) {
            std::string sql = "SELECT COUNT(*) FROM " + oj_submissions;
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }
            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            MYSQL_RES *res = mysql_store_result(my);
            if (res) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if (row) *count = atoi(row[0]);
                mysql_free_result(res);
            }

            return true;
        }

        bool GetUserGrowthStats(int days, std::map<std::string, int>* stats) {
            std::string sql = "SELECT DATE(created_at) as date, COUNT(*) FROM " + oj_users + 
                              " WHERE created_at >= DATE_SUB(NOW(), INTERVAL " + std::to_string(days) + " DAY) GROUP BY date";
            
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            if(0 != mysql_query(my, sql.c_str())) {
                return false;
            }
            
            MYSQL_RES *res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            
            for(int i = 0; i < rows; i++) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                std::string date = row[0] ? row[0] : "";
                int count = row[1] ? atoi(row[1]) : 0;
                (*stats)[date] = count;
            }
            mysql_free_result(res);

            return true;
        }

        bool GetSubmissionStats(std::map<std::string, int>* stats) {
            std::string sql = "SELECT result, COUNT(*) FROM " + oj_submissions + " GROUP BY result";
            
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            
            MYSQL_RES *res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            
            for(int i = 0; i < rows; i++) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                std::string result = row[0] ? row[0] : "";
                int count = row[1] ? atoi(row[1]) : 0;
                (*stats)[result] = count;
            }
            mysql_free_result(res);

            return true;
        }

        bool GetDailyActivityStats(int days, std::map<std::string, int>* stats) {
            std::string sql = "SELECT DATE(created_at) as date, COUNT(DISTINCT user_id) FROM " + oj_submissions + 
                              " WHERE created_at >= DATE_SUB(NOW(), INTERVAL " + std::to_string(days) + " DAY) GROUP BY date";
            
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            
            MYSQL_RES *res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            
            for(int i = 0; i < rows; i++) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                std::string date = row[0] ? row[0] : "";
                int count = row[1] ? atoi(row[1]) : 0;
                (*stats)[date] = count;
            }
            mysql_free_result(res);

            return true;
        }

        ~Model()
        {}
    };
} // namespace ns_model
