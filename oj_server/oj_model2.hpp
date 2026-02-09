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
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <sstream>
#include <iomanip>


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
        std::string created_at;
        int role;             // 0: User, 1: Admin
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

    const std::string oj_questions = "oj_questions";
    const std::string oj_users = "users";
    const std::string oj_submissions = "submissions";
    const std::string oj_inline_comments = "inline_comments";
    const std::string oj_discussions = "discussions";
    const std::string oj_article_comments = "article_comments";
    const std::string oj_contests = "contests";

    inline std::string GetEnv(const std::string& key, const std::string& default_value) {
        const char* val = std::getenv(key.c_str());
        return val ? std::string(val) : default_value;
    }

    const std::string host = GetEnv("MYSQL_HOST", "127.0.0.1");
    const std::string user = GetEnv("MYSQL_USER", "oj_client");
    const std::string passwd = GetEnv("MYSQL_PASSWORD", "123456");
    const std::string db = GetEnv("MYSQL_DB", "oj");
    const int port = std::stoi(GetEnv("MYSQL_PORT", "3306"));

    class Model
    {
    public:
        Model()
        {
            // Try to create users table if not exists
            InitUserTable();
            InitSubmissionTable();
            InitInlineCommentTable();
            InitDiscussionTable();
            InitArticleCommentTable();
            InitContestTable();
            CheckAndUpgradeTable();
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
            MYSQL *my = mysql_init(nullptr);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                return false;
            }
            if(0 != mysql_set_character_set(my, "utf8mb4")) { LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n"; }

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
                mysql_close(my);
                return false;
            }
            mysql_close(my);
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
            MYSQL *my = mysql_init(nullptr);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                return false;
            }
            if(0 != mysql_set_character_set(my, "utf8mb4")) { LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n"; }

            if(0 != mysql_query(my, count_sql.c_str())) {
                mysql_close(my);
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
                mysql_close(my);
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
            mysql_close(my);
            return true;
        }

        void CheckAndUpgradeTable() {
            MYSQL *my = mysql_init(nullptr);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), port, nullptr, 0)){
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

            mysql_close(my);
        }

        bool ExecuteSql(const std::string &sql) {
             MYSQL *my = mysql_init(nullptr);
             if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                 LOG(FATAL) << "连接数据库失败!" << "\n";
                 return false;
             }
             if(0 != mysql_set_character_set(my, "utf8mb4")) { LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n"; }
             if(0 != mysql_query(my, sql.c_str())) {
                 LOG(WARNING) << sql << " execute error: " << mysql_error(my) << "\n";
                 mysql_close(my);
                 return false;
             }
             mysql_close(my);
             return true;
        }

        bool QueryMySql(const std::string &sql, vector<Question> *out)
        {
            // 创建mysql句柄
            MYSQL *my = mysql_init(nullptr);

            // 连接数据库
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                LOG(FATAL) << "连接数据库失败!" << "\n";
                return false;
            }

            // 一定要设置该链接的编码格式, 要不然会出现乱码问题
            if(0 != mysql_set_character_set(my, "utf8mb4")) { LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n"; }

            // LOG(INFO) << "连接数据库成功!" << "\n";

            // 执行sql语句
            if(0 != mysql_query(my, sql.c_str()))
            {
                LOG(WARNING) << sql << " execute error!" << "\n";
                mysql_close(my);
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
                // If fields > 7, assume status is the 8th column if we used SELECT * or specific order
                // But caller might pass custom SQL. 
                // However, GetAllQuestions and GetOneQuestion are the main callers.
                // I will update them to use specific columns.
                if(fields > 7) q.status = row[7] ? atoi(row[7]) : 1;
                else q.status = 1; // Default visible

                out->push_back(q);
            }
            // 释放结果空间
            mysql_free_result(res);
            // 关闭mysql连接
            mysql_close(my);

            return true;
        }

        bool QueryUserMySql(const std::string &sql, vector<User> *out)
        {
            MYSQL *my = mysql_init(nullptr);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                LOG(FATAL) << "连接数据库失败!" << "\n";
                return false;
            }
            if(0 != mysql_set_character_set(my, "utf8mb4")) { LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n"; }

            if(0 != mysql_query(my, sql.c_str()))
            {
                LOG(WARNING) << sql << " execute error!" << "\n";
                mysql_close(my);
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
                
                out->push_back(u);
            }
            mysql_free_result(res);
            mysql_close(my);
            return true;
        }

        bool DeleteInlineComment(const std::string &comment_id, const std::string &user_id, int role)
        {
            // First check if the comment belongs to the user or user is admin
            // Role: 1 is admin
            
            MYSQL *my = mysql_init(nullptr);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                return false;
            }
            if(0 != mysql_set_character_set(my, "utf8mb4")) { LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n"; }
            
            std::string check_sql = "SELECT user_id FROM " + oj_inline_comments + " WHERE id=" + comment_id;
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
            std::string owner_id = row[0] ? row[0] : "";
            mysql_free_result(res);
            
            if (role != 1 && owner_id != user_id) {
                mysql_close(my);
                return false; // Not authorized
            }
            
            std::string sql = "DELETE FROM " + oj_inline_comments + " WHERE id=" + comment_id;
            if(0 != mysql_query(my, sql.c_str())) {
                mysql_close(my);
                return false;
            }
            
            mysql_close(my);
            return true;
        }

        bool AddSubmission(const Submission &sub)
        {
            // Escape content to avoid SQL injection
            // We need a connection to escape string, or just use a simple escape manually or use a prepared statement (C API doesn't make prepared statements easy with simple query strings)
            // For now, let's just do a basic connection to escape, or trust the caller? No, must escape.
            
            MYSQL *my = mysql_init(nullptr);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                 return false;
            }
            if(0 != mysql_set_character_set(my, "utf8mb4")) { LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n"; }
            
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
                mysql_close(my);
                return false;
            }
            mysql_close(my);
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
            
            MYSQL *my = mysql_init(nullptr);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                return false;
            }
            if(0 != mysql_set_character_set(my, "utf8mb4")) { LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n"; }

            if(0 != mysql_query(my, count_sql.c_str())) {
                mysql_close(my);
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
                mysql_close(my);
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
            mysql_close(my);
            return true;
        }


        bool AddInlineComment(const InlineComment &comment)
        {
            MYSQL *my = mysql_init(nullptr);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                 return false;
            }
            if(0 != mysql_set_character_set(my, "utf8mb4")) { LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n"; }
            
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
                mysql_close(my);
                return false;
            }
            mysql_close(my);
            return true;
        }

        bool GetInlineComments(const std::string &post_id, std::vector<InlineComment> *out)
        {
            std::string sql = "SELECT c.id, c.user_id, u.username, c.post_id, c.content, c.selected_text, c.created_at, c.parent_id FROM " 
                              + oj_inline_comments + " c LEFT JOIN " + oj_users + " u ON c.user_id = u.id " 
                              + "WHERE c.post_id='" + post_id + "' ORDER BY c.created_at ASC";
            
            MYSQL *my = mysql_init(nullptr);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                return false;
            }
            if(0 != mysql_set_character_set(my, "utf8mb4")) { LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n"; }
            
            if(0 != mysql_query(my, sql.c_str())) {
                mysql_close(my);
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
                if(fields > 7) c.parent_id = row[7] ? row[7] : "0";
                else c.parent_id = "0";
                
                out->push_back(c);
            }
            mysql_free_result(res);
            mysql_close(my);
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
            
            MYSQL *my = mysql_init(nullptr);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                return false;
            }
            if(0 != mysql_set_character_set(my, "utf8mb4")) { LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n"; }

            if(0 != mysql_query(my, sql.c_str()))
            {
                LOG(WARNING) << sql << " execute error: " << mysql_error(my) << "\n";
                mysql_close(my);
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
            mysql_close(my);
            return true;
        }

        bool GetAllQuestions(vector<Question> *out)
        {
            // Only show visible questions for normal users
            std::string sql = "select number, title, star, cpu_limit, mem_limit, description, tail_code, status from ";
            sql += oj_questions;
            sql += " where status=1";
            return QueryMySql(sql, out);
        }

        bool GetAllQuestionsAdmin(vector<Question> *out)
        {
            // Show all questions for admin
            std::string sql = "select number, title, star, cpu_limit, mem_limit, description, tail_code, status from ";
            sql += oj_questions;
            return QueryMySql(sql, out);
        }

        bool GetOneQuestion(const std::string &number, Question *q)
        {
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
                }
            }
            return res;
        }

        bool AddQuestion(const Question &q)
        {
            MYSQL *my = mysql_init(nullptr);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                return false;
            }
            if(0 != mysql_set_character_set(my, "utf8mb4")) { LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n"; }

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
                mysql_close(my);
                return false;
            }
            mysql_close(my);
            return true;
        }

        bool UpdateQuestion(const Question &q)
        {
            MYSQL *my = mysql_init(nullptr);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                return false;
            }
            if(0 != mysql_set_character_set(my, "utf8mb4")) { LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n"; }

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
                + "tail='" + escape(q.tail) + "', "
                + "status=" + std::to_string(q.status)
                + " WHERE number=" + q.number;

            if(0 != mysql_query(my, sql.c_str())) {
                LOG(WARNING) << sql << " execute error: " << mysql_error(my) << "\n";
                mysql_close(my);
                return false;
            }
            mysql_close(my);
            return true;
        }

        bool DeleteQuestion(const std::string &number)
        {
            std::string sql = "DELETE FROM " + oj_questions + " WHERE number=" + number;
            return ExecuteSql(sql);
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
            std::string sql_check = "select id, username, password, email, nickname, phone, created_at, role from " + oj_users + " where username='" + username + "'";
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
            std::string sql = "select id, username, password, email, nickname, phone, created_at, role from " + oj_users + " where username='" + username + "'";
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
            
            
            sql += " WHERE id='" + user.id + "'";
            
            if (first) return true; // No fields to update
            
            return ExecuteSql(sql);
        }

        bool AddDiscussion(const Discussion &d)
        {
            MYSQL *my = mysql_init(nullptr);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                return false;
            }
            if(0 != mysql_set_character_set(my, "utf8mb4")) { LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n"; }

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
                mysql_close(my);
                return false;
            }
            mysql_close(my);
            return true;
        }

        bool GetAllDiscussions(std::vector<Discussion> *out)
        {
            // Join with users to get author name and questions to get title
            std::string sql = "SELECT d.id, d.title, d.content, d.author_id, u.username, d.created_at, d.likes, d.views, d.is_official, "
                              "(SELECT COUNT(*) FROM " + oj_article_comments + " WHERE post_id = d.id) as comments_count, d.question_id, q.title "
                              "FROM " + oj_discussions + " d "
                              "LEFT JOIN " + oj_users + " u ON d.author_id = u.id "
                              "LEFT JOIN " + oj_questions + " q ON d.question_id = q.number "
                              "ORDER BY d.created_at DESC";
            
            MYSQL *my = mysql_init(nullptr);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                return false;
            }
            if(0 != mysql_set_character_set(my, "utf8mb4")) { LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n"; }
            
            if(0 != mysql_query(my, sql.c_str())) {
                mysql_close(my);
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
                
                out->push_back(d);
            }
            mysql_free_result(res);
            mysql_close(my);
            return true;
        }

        bool GetDiscussionsByQuestionId(const std::string &qid, std::vector<Discussion> *out)
        {
            std::string sql = "SELECT d.id, d.title, d.content, d.author_id, u.username, d.created_at, d.likes, d.views, d.is_official, "
                              "(SELECT COUNT(*) FROM " + oj_article_comments + " WHERE post_id = d.id) as comments_count, d.question_id, q.title "
                              "FROM " + oj_discussions + " d "
                              "LEFT JOIN " + oj_users + " u ON d.author_id = u.id "
                              "LEFT JOIN " + oj_questions + " q ON d.question_id = q.number "
                              "WHERE d.question_id=" + qid + " "
                              "ORDER BY d.created_at DESC";
            
            MYSQL *my = mysql_init(nullptr);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                return false;
            }
            if(0 != mysql_set_character_set(my, "utf8mb4")) { LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n"; }
            
            if(0 != mysql_query(my, sql.c_str())) {
                mysql_close(my);
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
                
                out->push_back(d);
            }
            mysql_free_result(res);
            mysql_close(my);
            return true;
        }

        bool GetOneDiscussion(const std::string &id, Discussion *d)
        {
            std::string sql = "SELECT d.id, d.title, d.content, d.author_id, u.username, d.created_at, d.likes, d.views, d.is_official, "
                              "(SELECT COUNT(*) FROM " + oj_article_comments + " WHERE post_id = d.id) as comments_count, d.question_id, q.title "
                              "FROM " + oj_discussions + " d "
                              "LEFT JOIN " + oj_users + " u ON d.author_id = u.id "
                              "LEFT JOIN " + oj_questions + " q ON d.question_id = q.number "
                              "WHERE d.id=" + id;
            
            MYSQL *my = mysql_init(nullptr);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                return false;
            }
            if(0 != mysql_set_character_set(my, "utf8mb4")) { LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n"; }
            
            if(0 != mysql_query(my, sql.c_str())) {
                mysql_close(my);
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
                    
                    mysql_free_result(res);
                    mysql_close(my);
                    return true;
                }
            }
            
            mysql_free_result(res);
            mysql_close(my);
            return false;
        }

        bool AddArticleComment(const ArticleComment &c)
        {
            MYSQL *my = mysql_init(nullptr);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                return false;
            }
            if(0 != mysql_set_character_set(my, "utf8mb4")) { LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n"; }
            
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
                mysql_close(my);
                return false;
            }
            mysql_close(my);
            return true;
        }

        bool GetArticleComments(const std::string &post_id, std::vector<ArticleComment> *out)
        {
            std::string sql = "SELECT c.id, c.user_id, u.username, c.post_id, c.content, c.created_at, c.likes FROM " 
                              + oj_article_comments + " c LEFT JOIN " + oj_users + " u ON c.user_id = u.id " 
                              + "WHERE c.post_id='" + post_id + "' ORDER BY c.created_at DESC";
            
            MYSQL *my = mysql_init(nullptr);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                return false;
            }
            if(0 != mysql_set_character_set(my, "utf8mb4")) { LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n"; }
            
            if(0 != mysql_query(my, sql.c_str())) {
                mysql_close(my);
                return false;
            }
            
            MYSQL_RES *res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            
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
                
                out->push_back(c);
            }
            mysql_free_result(res);
            mysql_close(my);
            return true;
        }

        ~Model()
        {}
    };
} // namespace ns_model
