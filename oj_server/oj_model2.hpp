#pragma once
//MySQL 版本
#include "../comm/util.hpp"
#include "../comm/log.hpp"
#include <mysql/mysql.h>

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <cstdlib>
#include <cassert>


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

    const std::string oj_questions = "oj_questions";
    const std::string host = "127.0.0.1";
    const std::string user = "oj_client";
    const std::string passwd = "123456";
    const std::string db = "oj";
    const int port = 3306;

    class Model
    {
    public:
        Model()
        {}
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

            LOG(INFO) << "连接数据库成功!" << "\n";

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
            int cols = mysql_num_fields(res); //获得列数量

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
        ~Model()
        {}
    };
} // namespace ns_model
