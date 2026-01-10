#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <mutex>
#include <cassert>
#include <json/json.h>
#include <unordered_map>
#include <ctime>

#include "../comm/util.hpp"
#include "../comm/log.hpp"
#include "../comm/httplib.h"
// #include "oj_model.hpp"
#include "oj_model2.hpp"
#include "oj_view.hpp"

namespace ns_control
{
    using namespace std;
    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_model;
    using namespace ns_view;
    using namespace httplib;

    // Session Management
    struct Session {
        User user;
        time_t expire_time;
    };

    // 提供服务的主机
    class Machine
    {
    public:
        std::string ip;  //编译服务的ip
        int port;        //编译服务的port
        uint64_t load;   //编译服务的负载
        std::mutex *mtx; // mutex禁止拷贝的，使用指针
    public:
        Machine() : ip(""), port(0), load(0), mtx(nullptr)
        {
        }
        ~Machine()
        {
        }

    public:
        // 提升主机负载
        void IncLoad()
        {
            if (mtx) mtx->lock();
            ++load;
            if (mtx) mtx->unlock();
        }
        // 减少主机负载
        void DecLoad()
        {
            if (mtx) mtx->lock();
            --load;
            if (mtx) mtx->unlock();
        }
        void ResetLoad()
        {
            if(mtx) mtx->lock();
            load = 0;
            if(mtx) mtx->unlock();
        }
        // 获取主机负载,没有太大的意义，只是为了统一接口
        uint64_t Load()
        {
            uint64_t _load = 0;
            if (mtx) mtx->lock();
            _load = load;
            if (mtx) mtx->unlock();

            return _load;
        }
    };

    const std::string service_machine = "./conf/service_machine.conf";
    // 负载均衡模块
    class LoadBlance
    {
    private:
        // 可以给我们提供编译服务的所有的主机
        // 每一台主机都有自己的下标，充当当前主机的id
        std::vector<Machine> machines;
        // 所有在线的主机id
        std::vector<int> online;
        // 所有离线的主机id
        std::vector<int> offline;
        // 保证LoadBlance它的数据安全
        std::mutex mtx;

    public:
        LoadBlance()
        {
            assert(LoadConf(service_machine));
            LOG(INFO) << "加载 " << service_machine << " 成功"
                      << "\n";
        }
        ~LoadBlance()
        {
        }

    public:
        bool LoadConf(const std::string &machine_conf)
        {
            std::ifstream in(machine_conf);
            if (!in.is_open())
            {
                LOG(FATAL) << " 加载: " << machine_conf << " 失败"
                           << "\n";
                return false;
            }
            std::string line;
            while (std::getline(in, line))
            {
                std::vector<std::string> tokens;
                StringUtil::SplitString(line, &tokens, ":");
                if (tokens.size() != 2)
                {
                    LOG(WARNING) << " 切分 " << line << " 失败"
                                 << "\n";
                    continue;
                }
                Machine m;
                m.ip = tokens[0];
                m.port = atoi(tokens[1].c_str());
                m.load = 0;
                m.mtx = new std::mutex();

                online.push_back(machines.size());
                machines.push_back(m);
            }

            in.close();
            return true;
        }
        // id: 输出型参数
        // m : 输出型参数
        bool SmartChoice(int *id, Machine **m)
        {
            // 1. 使用选择好的主机(更新该主机的负载)
            // 2. 我们需要可能离线该主机
            mtx.lock();
            // 负载均衡的算法
            // 1. 随机数+hash
            // 2. 轮询+hash
            int online_num = online.size();
            if (online_num == 0)
            {
                mtx.unlock();
                LOG(FATAL) << " 所有的后端编译主机已经离线, 请运维的同事尽快查看"
                           << "\n";
                return false;
            }
            // 通过遍历的方式，找到所有负载最小的机器
            *id = online[0];
            *m = &machines[online[0]];
            uint64_t min_load = machines[online[0]].Load();
            for (int i = 1; i < online_num; i++)
            {
                uint64_t curr_load = machines[online[i]].Load();
                if (min_load > curr_load)
                {
                    min_load = curr_load;
                    *id = online[i];
                    *m = &machines[online[i]];
                }
            }
            mtx.unlock();
            return true;
        }
        void OfflineMachine(int which)
        {
            mtx.lock();
            for(auto iter = online.begin(); iter != online.end(); )
            {
                if(*iter == which)
                {
                    machines[which].ResetLoad();
                    //要离线的主机已经找到啦
                    iter = online.erase(iter);
                    offline.push_back(which);
                    break;
                }
                else
                {
                    ++iter;
                }
            }
            mtx.unlock();
        }
        void OnlineMachine()
        {
            //我们统一上线，后面统一解决
            mtx.lock();
            online.insert(online.end(), offline.begin(), offline.end());
            offline.erase(offline.begin(), offline.end());
            mtx.unlock();

            LOG(INFO) << "所有的主机有上线啦!" << "\n";
        }
        //for test
        void ShowMachines()
        {
             mtx.lock();
             std::cout << "当前在线主机列表: ";
             for(auto &id : online)
             {
                 std::cout << id << " ";
             }
             std::cout << std::endl;
             std::cout << "当前离线主机列表: ";
             for(auto &id : offline)
             {
                 std::cout << id << " ";
             }
             std::cout << std::endl;
             mtx.unlock();
        }
    };

    // 这是我们的核心业务逻辑的控制器
    class Control
    {
    private:
        Model model_; //提供后台数据
        View view_;   //提供html渲染功能
        LoadBlance load_blance_; //核心负载均衡器
        
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
        void RecoveryMachine()
        {
            load_blance_.OnlineMachine();
        }
        
        // Auth Logic
        std::string GenerateToken() {
            char buf[64];
            snprintf(buf, sizeof(buf), "%lx%lx", (unsigned long)time(nullptr), (unsigned long)rand());
            return std::string(buf);
        }

        bool CheckUserExists(const std::string &username) {
            std::string sql_check = "select * from " + oj_users + " where username='" + username + "'";
            std::vector<User> users;
            if (!model_.QueryUserMySql(sql_check, &users)) {
                LOG(ERROR) << "检查用户是否存在时查询失败: " << username << "\n";
                return true; // 保守策略：查询失败时认为用户存在，避免重复注册
            }
            return !users.empty();
        }

        bool Register(const std::string &username, const std::string &password, const std::string &email, const std::string &nickname = "", const std::string &phone = "") {
            // 检查用户名是否已存在
            if (CheckUserExists(username)) {
                LOG(INFO) << "注册失败，用户名已存在: " << username << "\n";
                return false; // 用户已存在
            }
            
            // 执行注册
            bool result = model_.RegisterUser(username, password, email, nickname, phone);
            if (!result) {
                LOG(ERROR) << "注册失败，数据库错误: " << username << "\n";
            }
            return result;
        }

        bool Login(const std::string &username, const std::string &password, std::string *token) {
            User user;
            if (model_.LoginUser(username, password, &user)) {
                *token = GenerateToken();
                Session s;
                s.user = user;
                s.expire_time = time(nullptr) + 86400; // 1 day
                
                std::unique_lock<std::mutex> lock(session_mtx_);
                sessions_[*token] = s;
                return true;
            }
            return false;
        }

        bool AuthCheck(const Request &req, User *user) {
            if (req.has_header("Cookie")) {
                std::string cookie = req.get_header_value("Cookie");
                std::string key = "session_id=";
                size_t pos = cookie.find(key);
                if (pos != std::string::npos) {
                    std::string token = cookie.substr(pos + key.size());
                    size_t end = token.find(';');
                    if (end != std::string::npos) token = token.substr(0, end);
                    
                    std::unique_lock<std::mutex> lock(session_mtx_);
                    auto it = sessions_.find(token);
                    if (it != sessions_.end()) {
                        if (it->second.expire_time > time(nullptr)) {
                            *user = it->second.user;
                            return true;
                        } else {
                            sessions_.erase(it);
                        }
                    }
                }
            }
            return false;
        }

        bool Logout(const Request &req) {
            if (req.has_header("Cookie")) {
                std::string cookie = req.get_header_value("Cookie");
                std::string key = "session_id=";
                size_t pos = cookie.find(key);
                if (pos != std::string::npos) {
                    std::string token = cookie.substr(pos + key.size());
                    size_t end = token.find(';');
                    if (end != std::string::npos) token = token.substr(0, end);
                    
                    std::unique_lock<std::mutex> lock(session_mtx_);
                    auto it = sessions_.find(token);
                    if (it != sessions_.end()) {
                        sessions_.erase(it);
                        return true;
                    }
                }
            }
            return false;
        }

        //根据题目数据构建网页
        // html: 输出型参数
        bool AllQuestions(const Request &req, string *html)
        {
            User user;
            AuthCheck(req, &user);

            bool ret = true;
            vector<struct Question> all;
            if (model_.GetAllQuestions(&all))
            {
                sort(all.begin(), all.end(), [](const struct Question &q1, const struct Question &q2){
                    return atoi(q1.number.c_str()) < atoi(q2.number.c_str());
                });
                // 获取题目信息成功，将所有的题目数据构建成网页
                view_.AllExpandHtml(all, html, &user);
            }
            else
            {
                *html = "获取题目失败, 形成题目列表失败";
                ret = false;
            }
            return ret;
        }
        bool Question(const string &number, const Request &req, string *html)
        {
            User user;
            AuthCheck(req, &user);

            bool ret = true;
            struct Question q;
            if (model_.GetOneQuestion(number, &q))
            {
                // 获取指定题目信息成功，将所有的题目数据构建成网页
                view_.OneExpandHtml(q, html, &user);
            }
            else
            {
                *html = "指定题目: " + number + " 不存在!";
                ret = false;
            }
            return ret;
        }
        // code: #include...
        // input: ""
        void Judge(const std::string &number, const std::string in_json, std::string *out_json, const std::string &user_id = "")
        {
            // 0. 根据题目编号，直接拿到对应的题目细节
            struct Question q;
            model_.GetOneQuestion(number, &q);

            // 1. in_json进行反序列化
            Json::Reader reader;
            Json::Value in_value;
            reader.parse(in_json, in_value);
            std::string code = in_value["code"].asString();
            std::string language = in_value.isMember("language") ? in_value["language"].asString() : "C++";

            // 2. Parse test cases from q.tail (JSON format)
            Json::Reader tail_reader;
            Json::Value cases;
            bool has_cases = tail_reader.parse(q.tail, cases) && cases.isArray();
            
            if (!has_cases) {
                 Json::Value single_case;
                 single_case["input"] = in_value["input"].asString(); 
                 single_case["expect"] = ""; // No expectation if not provided
                 cases = Json::Value(Json::arrayValue);
                 cases.append(single_case);
            }

            Json::Value result_cases(Json::arrayValue);
            bool all_passed = true;
            
            for (unsigned int i = 0; i < cases.size(); ++i) {
                Json::Value &one_case = cases[i];
                std::string input_data = one_case.isMember("input") ? one_case["input"].asString() : "";
                std::string expected_output = one_case.isMember("expect") ? one_case["expect"].asString() : "";

                Json::Value compile_value;
                compile_value["input"] = input_data;
                compile_value["code"] = code;
                compile_value["language"] = language;
                compile_value["cpu_limit"] = q.cpu_limit;
                compile_value["mem_limit"] = q.mem_limit;
                
                Json::FastWriter writer;
                std::string compile_string = writer.write(compile_value);

                // 3. Load Balance & Request
                while(true) {
                    int id = 0;
                    Machine *m = nullptr;
                    if(!load_blance_.SmartChoice(&id, &m)) {
                         // System Error
                         Json::Value err_res;
                         err_res["status"] = -2;
                         err_res["reason"] = "No available compile server";
                         Json::FastWriter w;
                         *out_json = w.write(err_res);
                         return;
                    }
                    
                    Client cli(m->ip, m->port);
                    m->IncLoad();
                    auto res = cli.Post("/compile_and_run", compile_string, "application/json;charset=utf-8");
                    m->DecLoad();
                    
                    if(res) {
                        if(res->status == 200) {
                            Json::Reader resp_reader;
                            Json::Value resp_val;
                            resp_reader.parse(res->body, resp_val);
                            
                            // Check if compile error or runtime error
                            if (resp_val["status"].asInt() != 0) {
                                *out_json = res->body; // Return error immediately
                                return;
                            }
                            
                            // Check output
                            std::string stdout_str = resp_val["stdout"].asString();
                            std::string trim_stdout = stdout_str; 
                            while(!trim_stdout.empty() && isspace(trim_stdout.back())) trim_stdout.pop_back();
                            std::string trim_expect = expected_output;
                            while(!trim_expect.empty() && isspace(trim_expect.back())) trim_expect.pop_back();
                            
                            bool pass = (trim_stdout == trim_expect);
                            if (!pass) all_passed = false;
                            
                            Json::Value case_res;
                            case_res["name"] = "Case " + std::to_string(i+1);
                            case_res["pass"] = pass;
                            case_res["input"] = input_data;
                            case_res["output"] = trim_stdout;
                            case_res["expected"] = trim_expect;
                            // Add time/mem if available in future
                            
                            result_cases.append(case_res);
                            break; // Success for this case
                        }
                    } else {
                        load_blance_.OfflineMachine(id);
                    }
                }
            }
            
            // 4. Aggregate results
            Json::Value final_res;
            final_res["status"] = 0; 
            final_res["reason"] = "";
            
            Json::Value stdout_json;
            stdout_json["cases"] = result_cases;
            
            // Summary
            Json::Value summary;
            summary["total"] = cases.size();
            int passed_cnt = 0;
            for(const auto& c : result_cases) if(c["pass"].asBool()) passed_cnt++;
            summary["passed"] = passed_cnt;
            if (passed_cnt == cases.size()) summary["overall"] = "All Passed";
            else summary["overall"] = std::to_string(passed_cnt) + "/" + std::to_string(cases.size()) + " Passed";
            
            stdout_json["summary"] = summary;
            
            Json::FastWriter w;
            final_res["stdout"] = w.write(stdout_json);
            final_res["stderr"] = "";
            
            *out_json = w.write(final_res);
            
            // Record Submission
             if (!user_id.empty()) {
                 Submission sub;
                 sub.user_id = user_id;
                 sub.question_id = number;
                 sub.result = (passed_cnt == cases.size()) ? "0" : "-1"; 
                 sub.content = code;
                 sub.language = language;
                 model_.AddSubmission(sub);
            }
        }

        void SearchSubmissions(const Request &req, std::string *json_out)
        {
            // Parse parameters
            std::string user_id = req.get_param_value("user_id");
            std::string question_id = req.get_param_value("question_id");
            std::string status = req.get_param_value("status");
            std::string start_time = req.get_param_value("start_time");
            std::string end_time = req.get_param_value("end_time");
            std::string keyword = req.get_param_value("keyword");
            
            int page = 1;
            int page_size = 20;
            if (req.has_param("page")) page = std::stoi(req.get_param_value("page"));
            if (req.has_param("page_size")) page_size = std::stoi(req.get_param_value("page_size"));
            if (page < 1) page = 1;
            if (page_size < 1) page_size = 20;
            if (page_size > 100) page_size = 100;
            
            int offset = (page - 1) * page_size;
            
            std::vector<Submission> submissions;
            int total = 0;
            
            if (model_.GetSubmissions(user_id, question_id, status, start_time, end_time, keyword, offset, page_size, &submissions, &total)) {
                Json::Value root;
                root["status"] = 0;
                root["total"] = total;
                root["page"] = page;
                root["page_size"] = page_size;
                
                Json::Value list;
                for (const auto &s : submissions) {
                    Json::Value item;
                    item["id"] = s.id;
                    item["user_id"] = s.user_id;
                    item["question_id"] = s.question_id;
                    item["question_title"] = s.question_title;
                    item["result"] = s.result;
                    item["cpu_time"] = s.cpu_time;
                    item["mem_usage"] = s.mem_usage;
                    item["created_at"] = s.created_at;
                    item["content"] = s.content; 
                    list.append(item);
                }
                root["data"] = list;
                
                Json::FastWriter w;
                *json_out = w.write(root);
            } else {
                Json::Value root;
                root["status"] = 1;
                root["reason"] = "Database Error";
                Json::FastWriter w;
                *json_out = w.write(root);
            }
        }

        bool GetProfile(const User &user, string *html)
        {
            std::unordered_map<std::string, int> stats;
            // Ensure we have full user details (AuthCheck might only populate basic info depending on implementation)
            // But AuthCheck gets user from Session, which gets it from LoginUser, which calls QueryUserMySql.
            // And I updated QueryUserMySql to fetch all fields. So 'user' should be complete.
            
            if (model_.GetUserSolvedStats(user.id, &stats)) {
                view_.ProfileExpandHtml(user, stats, html);
                return true;
            }
            return false;
        }

        bool GetProfileData(const User &user, std::string *json)
        {
            std::unordered_map<std::string, int> stats;
            model_.GetUserSolvedStats(user.id, &stats);
            
            Json::Value root;
            root["status"] = 0;
            root["reason"] = "success";
            root["username"] = user.username;
            root["email"] = user.email;
            root["nickname"] = user.nickname;
            root["phone"] = user.phone;
            root["created_at"] = user.created_at;
            
            Json::Value stats_json;
            for(auto &kv : stats) {
                stats_json[kv.first] = kv.second;
            }
            root["stats"] = stats_json;
            
            Json::FastWriter w;
            *json = w.write(root);
            return true;
        }

        bool UpdateUserInfo(const std::string &user_id, const std::string &nickname, const std::string &email, const std::string &phone) {
            User u;
            u.id = user_id;
            u.nickname = nickname;
            u.email = email;
            u.phone = phone;
            
            // 更新数据库
            if (model_.UpdateUser(u)) {
                // 更新内存中的 Session (如果存在)
                std::unique_lock<std::mutex> lock(session_mtx_);
                for (auto &kv : sessions_) {
                    if (kv.second.user.id == user_id) {
                        if (!nickname.empty()) kv.second.user.nickname = nickname;
                        if (!email.empty()) kv.second.user.email = email;
                        if (!phone.empty()) kv.second.user.phone = phone;
                        break;
                    }
                }
                return true;
            }
            return false;
        }
    };
} // namespace ns_control
