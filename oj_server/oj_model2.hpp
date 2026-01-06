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
    };

    struct User
    {
        std::string id;
        std::string username;
        std::string password; // Hashed
        std::string email;
    };

    const std::string oj_questions = "oj_questions";
    const std::string oj_users = "users";
    const std::string host = "127.0.0.1";
    const std::string user = "oj_client";
    const std::string passwd = "123456";
    const std::string db = "oj";
    const int port = 3306;

    class Model
    {
    public:
        Model()
        {
            // Try to create users table if not exists
            InitUserTable();
        }

        void InitUserTable() {
            std::string sql = "CREATE TABLE IF NOT EXISTS `users` ("
                              "`id` int(11) NOT NULL AUTO_INCREMENT,"
                              "`username` varchar(50) NOT NULL UNIQUE,"
                              "`password` varchar(128) NOT NULL,"
                              "`email` varchar(100) DEFAULT NULL,"
                              "`nickname` varchar(100) DEFAULT NULL,"
                              "`phone` varchar(20) DEFAULT NULL,"
                              "PRIMARY KEY (`id`)"
                              ") ENGINE=InnoDB DEFAULT CHARSET=utf8;";
            ExecuteSql(sql);
        }

        bool ExecuteSql(const std::string &sql) {
             MYSQL *my = mysql_init(nullptr);
             if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                 LOG(FATAL) << "连接数据库失败!" << "\n";
                 return false;
             }
             mysql_set_character_set(my, "utf8");
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
            mysql_set_character_set(my, "utf8");

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
            // int cols = mysql_num_fields(res); //获得列数量

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
                q.header = row[6] ? row[6] : "";
                q.tail = row[7] ? row[7] : "";

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
            mysql_set_character_set(my, "utf8");

            if(0 != mysql_query(my, sql.c_str()))
            {
                LOG(WARNING) << sql << " execute error!" << "\n";
                mysql_close(my);
                return false;
            }
            MYSQL_RES *res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            
            User u;
            for(int i = 0; i < rows; i++)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                u.id = row[0] ? row[0] : "";
                u.username = row[1] ? row[1] : "";
                u.password = row[2] ? row[2] : "";
                u.email = row[3] ? row[3] : "";
                out->push_back(u);
            }
            mysql_free_result(res);
            mysql_close(my);
            return true;
        }

        bool GetAllQuestions(vector<Question> *out)
        {
            std::string sql = "select * from ";
            sql += oj_questions;
            return QueryMySql(sql, out);
        }
        bool GetOneQuestion(const std::string &number, Question *q)
        {
            bool res = false;
            std::string sql = "select * from ";
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
            std::string sql_check = "select * from " + oj_users + " where username='" + username + "'";
            std::vector<User> users;
            if (QueryUserMySql(sql_check, &users) && !users.empty()) {
                return false; // User exists
            }

            // Insert
            std::string pwd_hash = SHA256Hash(password);
            std::string sql_insert = "insert into " + oj_users + " (username, password, email, nickname, phone) values ('" 
                                     + username + "', '" + pwd_hash + "', '" + email + "', '" + nickname + "', '" + phone + "')";
            return ExecuteSql(sql_insert);
        }

        bool LoginUser(const std::string &username, const std::string &password, User *user) {
            std::string sql = "select * from " + oj_users + " where username='" + username + "'";
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

        ~Model()
        {}
    };
} // namespace ns_model
