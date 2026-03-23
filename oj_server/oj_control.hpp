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
#include <cstdio>
#include <thread>
#include <chrono>

#include "../comm/util.hpp"
#include "../comm/log.hpp"
#include "../comm/httplib.h"
#include "oj_model.hpp"
#include "oj_view.hpp"
#include "deepseek_api.hpp"
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

    // Helper for UTF-8 JSON
    std::string SerializeJson(const Json::Value &val) {
        Json::StreamWriterBuilder builder;
        builder["commentStyle"] = "None";
        builder["indentation"] = "";
        builder["emitUTF8"] = true;
        return Json::writeString(builder, val);
    }

    // Session Management
    struct Session {
        User user;
        time_t expire_time;
    };
    /*
    struct Session_Old {
        User user;
        time_t expire_time;




    };
    */

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

        bool is_running_;
        std::thread heartbeat_thread_;

    public:
        LoadBlance() : is_running_(true)
        {
            assert(LoadConf(service_machine));
            LOG(INFO) << "加载 " << service_machine << " 成功"
                      << "\n";
            heartbeat_thread_ = std::thread(&LoadBlance::Heartbeat, this);
        }
        ~LoadBlance()
        {
            is_running_ = false;
            if (heartbeat_thread_.joinable()) {
                heartbeat_thread_.join();
            }
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
            mtx.lock();
            int online_num = online.size();
            if (online_num == 0)
            {
                mtx.unlock();
                LOG(FATAL) << " 所有的后端编译主机已经离线, 请运维的同事尽快查看\n";
                return false;
            }
            
            // 找到所有负载最小的机器，使用数组记录所有拥有最小负载的主机下标
            uint64_t min_load = machines[online[0]].Load();
            std::vector<int> min_load_machines;
            min_load_machines.push_back(online[0]);

            for (int i = 1; i < online_num; i++)
            {
                uint64_t curr_load = machines[online[i]].Load();
                if (curr_load < min_load)
                {
                    min_load = curr_load;
                    min_load_machines.clear();
                    min_load_machines.push_back(online[i]);
                }
                else if (curr_load == min_load)
                {
                    min_load_machines.push_back(online[i]);
                }
            }
            
            // 如果有多个最小负载相同的机器，随机选一个以避免并发请求聚集在同一台机器
            int random_idx = rand() % min_load_machines.size();
            *id = min_load_machines[random_idx];
            *m = &machines[*id];
            
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
        
        void Heartbeat() {
            while (is_running_) {
                std::vector<int> current_offline;
                std::vector<int> current_online;
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    current_offline = offline;
                    current_online = online;
                }

                // Check offline machines
                for (int id : current_offline) {
                    Machine& m = machines[id];
                    httplib::Client cli(m.ip, m.port);
                    cli.set_connection_timeout(1);
                    cli.set_read_timeout(1);
                    auto res = cli.Get("/ping");
                    if (res && res->status == 200) {
                        std::lock_guard<std::mutex> lock(mtx);
                        auto it = std::find(offline.begin(), offline.end(), id);
                        if (it != offline.end()) {
                            offline.erase(it);
                            online.push_back(id);
                            LOG(INFO) << "编译服务器 " << m.ip << ":" << m.port << " 恢复在线" << "\n";
                        }
                    }
                }

                // Check online machines
                for (int id : current_online) {
                    Machine& m = machines[id];
                    httplib::Client cli(m.ip, m.port);
                    cli.set_connection_timeout(1);
                    cli.set_read_timeout(1);
                    auto res = cli.Get("/ping");
                    if (!res || res->status != 200) {
                        std::lock_guard<std::mutex> lock(mtx);
                        auto it = std::find(online.begin(), online.end(), id);
                        if (it != online.end()) {
                            machines[id].ResetLoad();
                            online.erase(it);
                            offline.push_back(id);
                            LOG(WARNING) << "编译服务器 " << m.ip << ":" << m.port << " 心跳检测失败，已下线" << "\n";
                        }
                    }
                }

                std::this_thread::sleep_for(std::chrono::seconds(3));
            }
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

        bool UploadImage(const Request &req, std::string *json_out)
        {
            if (!req.has_file("image")) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "No image file uploaded";
                
                *json_out = SerializeJson(res);
                return false;
            }

            const auto &file = req.get_file_value("image");
            std::string filename = file.filename;
            std::string content = file.content;
            
            // Generate unique filename
            size_t ext_pos = filename.find_last_of('.');
            std::string ext = (ext_pos != std::string::npos) ? filename.substr(ext_pos) : ".jpg";
            std::string new_filename = std::to_string(time(nullptr)) + "_" + std::to_string(rand()) + ext;
            
            // Save path
            std::string path = "./uploads/" + new_filename;
            
            // Create uploads directory if not exists
            // Assuming wwwroot exists, mkdir uploads
            system("mkdir -p ./uploads");
            
            std::ofstream out(path, std::ios::binary);
            if (!out.is_open()) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Failed to save file";
                
                *json_out = SerializeJson(res);
                return false;
            }
            out.write(content.c_str(), content.size());
            out.close();
            
            Json::Value res;
            res["status"] = 0;
            res["url"] = "/uploads/" + new_filename;
            
            *json_out = SerializeJson(res);
            return true;
        }

        bool UploadAvatar(const Request &req, const std::string &user_id, std::string *json_out)
        {
            if (!req.has_file("avatar")) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "No avatar file uploaded";
                *json_out = SerializeJson(res);
                return false;
            }

            const auto &file = req.get_file_value("avatar");
            std::string filename = file.filename;
            std::string content = file.content;
            
            // Check file size (2MB limit)
            if (content.size() > 2 * 1024 * 1024) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "File size exceeds 2MB limit";
                *json_out = SerializeJson(res);
                return false;
            }

            // Check file type
            std::string ext;
            size_t ext_pos = filename.find_last_of('.');
            if (ext_pos != std::string::npos) {
                ext = filename.substr(ext_pos);
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            }
            
            if (ext != ".jpg" && ext != ".jpeg" && ext != ".png" && ext != ".gif") {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Invalid file type. Only JPG, PNG, GIF are allowed.";
                *json_out = SerializeJson(res);
                return false;
            }

            // Generate unique filename: avatar_<user_id>_<timestamp><ext>
            std::string new_filename = "avatar_" + user_id + "_" + std::to_string(time(nullptr)) + ext;
            std::string path = "./uploads/avatars/" + new_filename;
            
            // Create directory
            system("mkdir -p ./uploads/avatars");
            
            // Save file
            std::ofstream out(path, std::ios::binary);
            if (!out.is_open()) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Failed to save file";
                *json_out = SerializeJson(res);
                return false;
            }
            out.write(content.c_str(), content.size());
            out.close();
            
            std::string avatar_url = "/uploads/avatars/" + new_filename;

            // Update Database
            User u;
            u.id = user_id;
            u.avatar = avatar_url;
            if (model_.UpdateUser(u)) {
                LOG(INFO) << "User " << user_id << " avatar updated in DB to " << avatar_url << "\n";
                // Update Session Cache
                std::unique_lock<std::mutex> lock(session_mtx_);
                bool old_avatar_deleted = false;
                int updated_sessions = 0;
                for (auto &kv : sessions_) {
                    if (kv.second.user.id == user_id) {
                        // Delete old avatar file if exists and not default
                        // Only delete once to avoid redundant filesystem calls
                        if (!old_avatar_deleted && !kv.second.user.avatar.empty()) {
                            std::string old_path = "." + kv.second.user.avatar;
                            // Basic check to ensure we don't delete something outside uploads/avatars
                            if (old_path.find("/uploads/avatars/") != std::string::npos) {
                                remove(old_path.c_str());
                                old_avatar_deleted = true;
                            }
                        }
                        
                        kv.second.user.avatar = avatar_url;
                        updated_sessions++;
                        // Do NOT break here! Update ALL sessions for this user.
                    }
                }
                LOG(INFO) << "Updated " << updated_sessions << " sessions for user " << user_id << "\n";
                
                Json::Value res;
                res["status"] = 0;
                res["url"] = avatar_url;
                *json_out = SerializeJson(res);
                return true;
            } else {
                // If DB update fails, maybe delete the uploaded file?
                remove(path.c_str());
                
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Update Failed";
                *json_out = SerializeJson(res);
                return false;
            }
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
            std::string sql_check = "select id, username, password, email, nickname, phone, created_at, role from " + oj_users + " where username='" + username + "'";
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

        bool AdminAuthCheck(const Request &req, User *user) {
            if (AuthCheck(req, user)) {
                return user->role == 1;
            }
            return false;
        }

        bool AllQuestionsAdmin(const Request &req, string *json)
        {
            User user;
            if (!AdminAuthCheck(req, &user)) {
                Json::Value root;
                root["status"] = 403;
                root["reason"] = "Permission Denied";
                
                *json = SerializeJson(root);
                return false;
            }

            int page = 1;
            int page_size = 20;
            if (req.has_param("page")) {
                try {
                    page = std::stoi(req.get_param_value("page"));
                } catch (...) { page = 1; }
            }
            if (req.has_param("page_size")) {
                try {
                    page_size = std::stoi(req.get_param_value("page_size"));
                } catch (...) { page_size = 20; }
            }
            if (page < 1) page = 1;
            if (page_size < 1) page_size = 20;

            vector<struct Question> all;
            int total = 0;
            if (model_.GetAllQuestionsAdmin(page, page_size, &all, &total))
            {
                Json::Value root;
                root["status"] = 0;
                root["total"] = total;
                root["page"] = page;
                root["page_size"] = page_size;

                Json::Value list;
                for (const auto &q : all) {
                    Json::Value item;
                    item["number"] = q.number;
                    item["title"] = q.title;
                    item["star"] = q.star;
                    item["cpu_limit"] = q.cpu_limit;
                    item["mem_limit"] = q.mem_limit;
                    item["description"] = q.desc;
                    item["tail"] = q.tail;
                    item["status"] = q.status;
                    list.append(item);
                }
                root["data"] = list;
                
                *json = SerializeJson(root);
                return true;
            }
            else
            {
                Json::Value root;
                root["status"] = 1;
                root["reason"] = "Database Error";
                
                *json = SerializeJson(root);
                return false;
            }
        }

        bool AddQuestion(const Request &req, string *json)
        {
            User user;
            if (!AdminAuthCheck(req, &user)) {
                Json::Value root;
                root["status"] = 403;
                root["reason"] = "Permission Denied";
                
                *json = SerializeJson(root);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            
            struct Question q;
            q.title = root["title"].asString();
            q.star = root["star"].asString();
            q.desc = root["description"].asString();
            q.tail = root["tail"].asString(); // Expecting JSON string of test cases
            q.cpu_limit = root.get("cpu_limit", 1).asInt();
            q.mem_limit = root.get("mem_limit", 30000).asInt();
            q.status = root.get("status", 1).asInt();

            if (model_.AddQuestion(q)) {
                 LogAdminOp(user.id, "Add Question", "Question " + q.title, "Added new question", req);
                 Json::Value res;
                 res["status"] = 0;
                 res["reason"] = "Success";
                 
                 *json = SerializeJson(res);
                 return true;
            }
            
            Json::Value res;
            res["status"] = 1;
            res["reason"] = "Database Error";
            
            *json = SerializeJson(res);
            return false;
        }

        bool UpdateQuestion(const std::string &number, const Request &req, string *json)
        {
            User user;
            if (!AdminAuthCheck(req, &user)) {
                Json::Value root;
                root["status"] = 403;
                root["reason"] = "Permission Denied";
                
                *json = SerializeJson(root);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            
            struct Question q;
            q.number = number;
            q.title = root["title"].asString();
            q.star = root["star"].asString();
            q.desc = root["description"].asString();
            q.tail = root["tail"].asString();
            q.cpu_limit = root.get("cpu_limit", 1).asInt();
            q.mem_limit = root.get("mem_limit", 30000).asInt();
            q.status = root.get("status", 1).asInt();

            if (model_.UpdateQuestion(q)) {
                 LogAdminOp(user.id, "Update Question", "Question " + number, "Updated question " + number, req);
                 Json::Value res;
                 res["status"] = 0;
                 res["reason"] = "Success";
                 
                 *json = SerializeJson(res);
                 return true;
            }
            
            Json::Value res;
            res["status"] = 1;
            res["reason"] = "Database Error";
            
            *json = SerializeJson(res);
            return false;
        }

        bool DeleteQuestion(const std::string &number, const Request &req, string *json)
        {
            User user;
            if (!AdminAuthCheck(req, &user)) {
                Json::Value root;
                root["status"] = 403;
                root["reason"] = "Permission Denied";
                
                *json = SerializeJson(root);
                return false;
            }

            if (model_.DeleteQuestion(number)) {
                 LogAdminOp(user.id, "Delete Question", "Question " + number, "Deleted question " + number, req);
                 Json::Value res;
                 res["status"] = 0;
                 res["reason"] = "Success";
                 
                 *json = SerializeJson(res);
                 return true;
            }
            
            Json::Value res;
            res["status"] = 1;
            res["reason"] = "Database Error";
            
            *json = SerializeJson(res);
            return false;
        }

        // Helper for Logging
        void LogAdminOp(const std::string &user_id, const std::string &action, const std::string &target, const std::string &details, const Request &req) {
             OperationLog log;
             log.user_id = user_id;
             log.action = action;
             log.target = target;
             log.details = details;
             
             if (req.has_header("X-Real-IP")) {
                 log.ip = req.get_header_value("X-Real-IP");
             } else if (req.has_header("X-Forwarded-For")) {
                 log.ip = req.get_header_value("X-Forwarded-For");
             } else {
                 log.ip = req.remote_addr;
             }
             model_.LogOperation(log);
        }

        /* Removed GetUsers from Session */

        bool UpdateUserStatus(const Request &req, std::string *json_out) {
            User user;
            if (!AdminAuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            std::string id = root["id"].asString();
            int status = root["status"].asInt();

            if (model_.UpdateUserStatus(id, status)) {
                LogAdminOp(user.id, "Update User Status", "User " + id, "Changed status to " + std::to_string(status), req);
                Json::Value res;
                res["status"] = 0;
                res["reason"] = "Success";
                *json_out = SerializeJson(res);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool UpdateUserRole(const Request &req, std::string *json_out) {
            User user;
            if (!AdminAuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            std::string id = root["id"].asString();
            int role = root["role"].asInt();

            if (model_.UpdateUserRole(id, role)) {
                LogAdminOp(user.id, "Update User Role", "User " + id, "Changed role to " + std::to_string(role), req);
                Json::Value res;
                res["status"] = 0;
                res["reason"] = "Success";
                *json_out = SerializeJson(res);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                *json_out = SerializeJson(res);
                return false;
            }
        }

        std::string GenerateRandomPassword(int length) {
            const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
            std::string pwd;
            for (int i = 0; i < length; ++i) {
                pwd += chars[rand() % chars.length()];
            }
            return pwd;
        }

        bool ResetUserPassword(const Request &req, std::string *json_out) {
            User user;
            if (!AdminAuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            std::string id = root["id"].asString();

            std::string new_pwd = GenerateRandomPassword(8);
            std::string new_hash = model_.SHA256Hash(new_pwd);

            if (model_.ResetUserPassword(id, new_hash)) {
                LogAdminOp(user.id, "Reset Password", "User " + id, "Reset password", req);
                Json::Value res;
                res["status"] = 0;
                res["new_password"] = new_pwd;
                *json_out = SerializeJson(res);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                *json_out = SerializeJson(res);
                return false;
            }
        }

        // Admin Registration with Invitation Code
        bool AdminRegister(const Request &req, std::string *json_out) {
            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            std::string username = root["username"].asString();
            std::string password = root["password"].asString();
            std::string email = root["email"].asString();
            std::string invitation_code = root["invitation_code"].asString();

            if (username.empty() || password.empty() || email.empty() || invitation_code.empty()) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "All fields including invitation code are required";
                *json_out = SerializeJson(res);
                return false;
            }

            // Check user existence first
            if (CheckUserExists(username)) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Username already exists";
                *json_out = SerializeJson(res);
                return false;
            }

            // Register user first (status 0, role 1)
            // But wait, VerifyAndUseInvitationCode needs user_id.
            // So we register user first? If code is invalid, we should not register user or rollback.
            // Better: Verify code exists and unused first (without marking used), then register, then mark used.
            // However, race condition?
            // `VerifyAndUseInvitationCode` does check and update atomically (if possible) or sequentially.
            // Let's change `VerifyAndUseInvitationCode`? No, it's already implemented to check and use.
            
            // Revised flow:
            // 1. Register User (Role 1).
            // 2. Get User ID.
            // 3. VerifyAndUseInvitationCode(code, user_id).
            // 4. If code fails, DELETE user and return error.
            
            if (!model_.RegisterUser(username, password, email)) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error during Registration";
                *json_out = SerializeJson(res);
                return false;
            }

            // Get the user just created
            std::string sql = "SELECT id, username, password, email, nickname, phone, created_at, role, avatar, status FROM " + oj_users + " WHERE username='" + username + "'";
            std::vector<User> users;
            model_.QueryUserMySql(sql, &users);
            if (users.empty()) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Failed to retrieve created user";
                *json_out = SerializeJson(res);
                return false;
            }
            User new_user = users[0];

            // Verify Code
            if (!model_.VerifyAndUseInvitationCode(invitation_code, new_user.id)) {
                // Failed: Code invalid or used. Delete user.
                std::string del_sql = "DELETE FROM " + oj_users + " WHERE id=" + new_user.id;
                model_.ExecuteSql(del_sql);
                
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Invalid or Used Invitation Code";
                *json_out = SerializeJson(res);
                return false;
            }

            // Update Role to Admin (1)
            model_.UpdateUserRole(new_user.id, 1);
            
            // Log Self
            LogAdminOp(new_user.id, "Admin Register", "Self", "Registered as Admin with code " + invitation_code, req);

            Json::Value res;
            res["status"] = 0;
            res["reason"] = "Success";
            *json_out = SerializeJson(res);
            return true;
        }

        bool GenerateInvitationCode(const Request &req, std::string *json_out) {
            User user;
            if (!AdminAuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            // Generate Random Code
            std::string code = GenerateToken().substr(0, 16); // Reusing GenerateToken or make new one
            // Simple random string
            const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            code = "";
            for (int i = 0; i < 16; ++i) {
                code += chars[rand() % chars.length()];
            }

            if (model_.GenerateInvitationCode(code)) {
                LogAdminOp(user.id, "Generate Invitation Code", code, "Generated new invitation code", req);
                Json::Value res;
                res["status"] = 0;
                res["code"] = code;
                *json_out = SerializeJson(res);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool GetLogs(const Request &req, std::string *json_out) {
            User user;
            if (!AdminAuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            int page = 1;
            int page_size = 20;
            if (req.has_param("page")) page = std::stoi(req.get_param_value("page"));
            if (req.has_param("page_size")) page_size = std::stoi(req.get_param_value("page_size"));
            std::string keyword = req.get_param_value("keyword");

            std::vector<OperationLog> logs;
            int total = 0;
            if (model_.GetLogs(page, page_size, keyword, &logs, &total)) {
                Json::Value root;
                root["status"] = 0;
                root["total"] = total;
                root["page"] = page;
                root["page_size"] = page_size;
                
                Json::Value list;
                for (const auto &l : logs) {
                    Json::Value item;
                    item["id"] = l.id;
                    item["user_id"] = l.user_id;
                    item["username"] = l.username;
                    item["action"] = l.action;
                    item["target"] = l.target;
                    item["details"] = l.details;
                    item["ip"] = l.ip;
                    item["created_at"] = l.created_at;
                    list.append(item);
                }
                root["data"] = list;
                *json_out = SerializeJson(root);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                *json_out = SerializeJson(res);
                return false;
            }
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
        bool Home(const Request &req, string *html)
        {
             User user;
             AuthCheck(req, &user);
             view_.HomeHtml(html, &user);
             return true;
        }

        bool AllQuestions(const Request &req, string *html)
        {
            User user;
            AuthCheck(req, &user);

            // Pagination parameters
            int page = 1;
            int page_size = 40; // Changed to 40 per page
            if (req.has_param("page")) {
                try {
                    page = std::stoi(req.get_param_value("page"));
                    if (page < 1) page = 1;
                } catch (...) {
                    page = 1;
                }
            }

            bool ret = true;
            vector<struct Question> all;
            int total_count = 0;

            if (model_.GetQuestionsByPage(page, page_size, &all, &total_count))
            {
                int total_pages = (total_count + page_size - 1) / page_size;
                if (total_pages < 1) total_pages = 1; // At least 1 page even if empty

                // 获取题目信息成功，将所有的题目数据构建成网页
                view_.AllExpandHtml(all, html, total_pages, page, &user);
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
                // Check visibility
                if (q.status == 0 && user.role != 1) {
                    *html = "指定题目: " + number + " 未发布!";
                    return false;
                }
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

        bool GetQuestionJson(const string &number, string *json_out)
        {
            struct Question q;
            if (model_.GetOneQuestion(number, &q))
            {
                // Note: We might want to check visibility here too, but for basic info (title/star) it might be okay?
                // For consistency, let's assume public API only returns visible ones unless we pass auth (which we don't here easily)
                // However, GetOneQuestion returns it regardless of status.
                // Let's filter out hidden ones if we want to be strict, but for now just return it.
                // Or better, check status.
                if (q.status == 0) {
                     Json::Value res;
                     res["status"] = 1;
                     res["reason"] = "Question hidden";
                     
                     *json_out = SerializeJson(res);
                     return false;
                }

                Json::Value root;
                root["status"] = 0;
                Json::Value data;
                data["number"] = q.number;
                data["title"] = q.title;
                data["star"] = q.star;
                data["cpu_limit"] = q.cpu_limit;
                data["mem_limit"] = q.mem_limit;
                // Don't return full description/tail/header to save bandwidth if only needed for metadata
                root["data"] = data;
                
                
                *json_out = SerializeJson(root);
                return true;
            }
            else
            {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Not Found";
                
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool DiscussionPage(const Request &req, string *html)
        {
            User user;
            AuthCheck(req, &user);
            view_.DiscussionHtml(html, &user);
            return true;
        }

        bool GetContestList(const Request &req, std::string *json_out)
        {
            int page = 1;
            int page_size = 5;
            if (req.has_param("page")) page = std::stoi(req.get_param_value("page"));
            if (req.has_param("size")) page_size = std::stoi(req.get_param_value("size"));
            std::string status = req.get_param_value("status");

#ifdef ENABLE_REDIS
            // Redis Cache
            std::string cache_key = "contest:page:" + std::to_string(page) + ":size:" + std::to_string(page_size) + ":status:" + status;
            redisContext *c = redisConnect("127.0.0.1", 6379);
            if (c != NULL && c->err == 0) {
                 redisReply *reply = (redisReply*)redisCommand(c, "GET %s", cache_key.c_str());
                 if (reply != NULL && reply->type == REDIS_REPLY_STRING) {
                     *json_out = reply->str;
                     freeReplyObject(reply);
                     redisFree(c);
                     return true;
                 }
                 if(reply) freeReplyObject(reply);
            }
#endif

            std::vector<ns_model::Contest> contests;
            int total = 0;
            if (model_.GetContests(page, page_size, status, &contests, &total)) {
                 Json::Value root;
                 root["status"] = 0;
                 root["total"] = total;
                 root["total_pages"] = (total + page_size - 1) / page_size;
                 root["page"] = page;
                 
                 Json::Value list;
                 for(const auto& c : contests) {
                     Json::Value item;
                     item["name"] = c.name;
                     item["start_time"] = c.start_time;
                     item["end_time"] = c.end_time;
                     item["status"] = c.status;
                     item["link"] = c.link;
                     item["source"] = c.source;
                     list.append(item);
                 }
                 root["data"] = list;
                 
                 
                 *json_out = SerializeJson(root);

#ifdef ENABLE_REDIS
                 // Set Cache
                 if (c != NULL && c->err == 0) {
                     redisReply *reply = (redisReply*)redisCommand(c, "SETEX %s 300 %s", cache_key.c_str(), json_out->c_str());
                     if(reply) freeReplyObject(reply);
                     redisFree(c);
                 } else if (c) {
                     redisFree(c);
                 }
#endif

                 return true;
            } else {
#ifdef ENABLE_REDIS
                 if (c) redisFree(c);
#endif
                 Json::Value res;
                 res["status"] = 1;
                 res["reason"] = "Database Error";
                 
                 *json_out = SerializeJson(res);
                 return false;
            }
        }

        bool Contest(const Request &req, string *html)
        {
            User user;
            AuthCheck(req, &user);

            std::vector<ns_model::Contest> contests;
            // Empty list for initial render, JS will fetch data
            view_.ContestHtml(contests, html, &user);
            return true;
        }

        // code: #include...
        // input: ""
        void Judge(const std::string &number, const std::string in_json, std::string *out_json, const std::string &user_id = "")
        {
            // 0. 根据题目编号，直接拿到对应的题目细节
            struct Question q;
            if (!model_.GetOneQuestion(number, &q)) {
                 Json::Value err_res;
                 err_res["status"] = -2;
                 err_res["reason"] = "Question not found";
                 
                 *out_json = SerializeJson(err_res);
                 return;
            }
            
            // Check visibility (unless it's an internal call or admin)
            // But Judge doesn't have easy access to User role unless passed or checked.
            // We can check user_id if provided.
            if (q.status == 0) {
                // If user_id is provided, check their role? 
                // Model doesn't support GetUserById efficiently (only by username in Login/Register, or QueryUserMySql).
                // But we can assume if it's hidden, regular users can't judge it.
                // However, Admins might want to test their hidden questions.
                // For now, let's allow judging if they know the ID, or block it.
                // Requirement: "Ensure only published questions are visible to customers".
                // Judging implies visibility of content/result.
                // I'll block it if status is 0, UNLESS I can verify admin.
                // Since I can't easily verify admin here without refactoring Judge signature or logic, 
                // and considering Admin likely tests via the same interface, 
                // I will add a check: if status==0, check if user is admin.
                
                // Fetch user to check role
                std::string sql = "select id, username, password, email, nickname, phone, created_at, role from " + oj_users + " where id='" + user_id + "'";
                std::vector<User> users;
                model_.QueryUserMySql(sql, &users);
                if (users.empty() || users[0].role != 1) {
                     Json::Value err_res;
                     err_res["status"] = -2;
                     err_res["reason"] = "Question is not published";
                     
                     *out_json = SerializeJson(err_res);
                     return;
                }
            }

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
                
                
                std::string compile_string = SerializeJson(compile_value);

                // 3. Load Balance & Request
                while(true) {
                    int id = 0;
                    Machine *m = nullptr;
                    if(!load_blance_.SmartChoice(&id, &m)) {
                         // System Error
                         Json::Value err_res;
                         err_res["status"] = -2;
                         err_res["reason"] = "No available compile server";
                         
                         *out_json = SerializeJson(err_res);
                         return;
                    }
                    
                    Client cli(m->ip, m->port);
                    // Add appropriate timeouts to avoid indefinite blocking
                    cli.set_connection_timeout(1);
                    cli.set_read_timeout(5);
                    cli.set_write_timeout(2);

                    // Add simple retry logic at the request level
                    bool request_success = false;
                    int retry_count = 0;
                    bool case_completed = false;
                    
                    while (retry_count < 3) {
                        m->IncLoad();
                        auto res = cli.Post("/compile_and_run", compile_string, "application/json;charset=utf-8");
                        m->DecLoad();
                        if (res) {
                            request_success = true;
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
                                case_completed = true;
                            }
                            break; // Received response, stop retrying this machine
                        }
                        retry_count++;
                    }
                    
                    if (!request_success) {
                        load_blance_.OfflineMachine(id);
                    } else if (case_completed) {
                        break; // Success for this case, move to next case
                    } else {
                        // Received response but status != 200 (e.g. 500 Internal Server Error)
                        // It's a server error, we should probably try another machine.
                        // Or if it's the only machine, we might want to return an error eventually.
                        // For now, let's offline it temporarily or just retry another machine.
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
            
            
            final_res["stdout"] = SerializeJson(stdout_json);
            final_res["stderr"] = "";
            
            *out_json = SerializeJson(final_res);
            
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

        void GenerateAiHint(const Request &req, std::string *json_out)
        {
            // 1. Auth Check
            User user;
            if (!AuthCheck(req, &user)) {
                 Json::Value res;
                 res["status"] = 401;
                 res["reason"] = "Unauthorized";
                 *json_out = SerializeJson(res);
                 return;
            }

            // 2. Parse Body
            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            
            std::string question_id = root.get("question_id", "").asString();
            std::string code = root.get("code", "").asString();
            std::string error_msg = root.get("error_msg", "").asString();
            std::string test_cases = root.get("test_cases", "").asString();

            if (question_id.empty() || code.empty() || error_msg.empty()) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Missing required fields";
                *json_out = SerializeJson(res);
                return;
            }

            // 3. Get Question Details
            struct Question q;
            if (!model_.GetOneQuestion(question_id, &q)) {
                 Json::Value res;
                 res["status"] = 1;
                 res["reason"] = "Question Not Found";
                 *json_out = SerializeJson(res);
                 return;
            }

            // 4. Call DeepSeek
            std::string hint;
            if (deepseek_api_.GenerateHint(q.desc, code, error_msg, test_cases, &hint)) {
                 Json::Value res;
                 res["status"] = 0;
                 res["hint"] = hint;
                 *json_out = SerializeJson(res);
            } else {
                 Json::Value res;
                 res["status"] = 1;
                 res["reason"] = "AI Service Unavailable";
                 *json_out = SerializeJson(res);
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
                
                
                *json_out = SerializeJson(root);
            } else {
                Json::Value root;
                root["status"] = 1;
                root["reason"] = "Database Error";
                
                *json_out = SerializeJson(root);
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
            root["avatar"] = user.avatar;
            root["created_at"] = user.created_at;
            root["role"] = user.role; // Add role to response
            
            Json::Value stats_json;
            for(auto &kv : stats) {
                stats_json[kv.first] = kv.second;
            }
            root["stats"] = stats_json;
            
            
            *json = SerializeJson(root);
            return true;
        }

        bool UpdateUserInfo(const std::string &user_id, const std::string &nickname, const std::string &email, const std::string &phone) {
            User u;
            u.id = user_id;
            u.nickname = nickname;
            u.email = email;
            u.phone = phone;
            // Avatar is handled by UploadAvatar
            
            // 更新数据库
            if (model_.UpdateUser(u)) {
                // 更新内存中的 Session (如果存在)
                std::unique_lock<std::mutex> lock(session_mtx_);
                for (auto &kv : sessions_) {
                    if (kv.second.user.id == user_id) {
                        if (!nickname.empty()) kv.second.user.nickname = nickname;
                        if (!email.empty()) kv.second.user.email = email;
                        if (!phone.empty()) kv.second.user.phone = phone;
                        // Do NOT break here! Update ALL sessions for this user.
                    }
                }
                return true;
            }
            return false;
        }

        bool AddInlineComment(const std::string &user_id, const std::string &post_id, const std::string &content, const std::string &selected_text, const std::string &parent_id, std::string *json_out)
        {
            InlineComment c;
            c.user_id = user_id;
            c.post_id = post_id;
            c.content = content;
            c.selected_text = selected_text;
            c.parent_id = parent_id;
            
            if (model_.AddInlineComment(c)) {
                Json::Value res;
                res["status"] = 0;
                res["reason"] = "Success";
                
                *json_out = SerializeJson(res);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool GetInlineComments(const std::string &post_id, std::string *json_out)
        {
            std::vector<InlineComment> comments;
            if (model_.GetInlineComments(post_id, &comments)) {
                Json::Value root;
                root["status"] = 0;
                Json::Value list;
                for (const auto &c : comments) {
                    Json::Value item;
                    item["id"] = c.id;
                    item["user_id"] = c.user_id;
                    item["username"] = c.username;
                    item["avatar"] = c.user_avatar;
                    item["post_id"] = c.post_id;
                    item["content"] = c.content;
                    item["selected_text"] = c.selected_text;
                    item["parent_id"] = c.parent_id;
                    item["created_at"] = c.created_at;
                    list.append(item);
                }
                root["data"] = list;
                
                *json_out = SerializeJson(root);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool DeleteInlineComment(const std::string &comment_id, const std::string &user_id, int role, std::string *json_out)
        {
             if (model_.DeleteInlineComment(comment_id, user_id, role)) {
                 Json::Value res;
                 res["status"] = 0;
                 res["reason"] = "Success";
                 
                 *json_out = SerializeJson(res);
                 return true;
             } else {
                 Json::Value res;
                 res["status"] = 1;
                 res["reason"] = "Failed to delete (Permission Denied or Not Found)";
                 
                 *json_out = SerializeJson(res);
                 return false;
             }
        }

        bool GetAllDiscussions(std::string *json_out)
        {
            std::vector<Discussion> discussions;
            if (model_.GetAllDiscussions(&discussions)) {
                Json::Value root;
                root["status"] = 0;
                Json::Value list;
                for (const auto &d : discussions) {
                    Json::Value item;
                    item["id"] = d.id;
                    item["title"] = d.title;
                    item["summary"] = StringUtil::GetSummaryFromMarkdown(d.content);
                    item["author"] = d.author_name;
                    item["avatar"] = d.author_avatar;
                    std::cout << "DEBUG: " << d.author_name << " avatar=" << d.author_avatar << std::endl;
                    item["date"] = d.created_at;
                    item["likes"] = d.likes;
                    item["views"] = d.views;
                    item["comments"] = d.comments_count;
                    item["isOfficial"] = d.is_official;
                    item["question_id"] = d.question_id;
                    item["question_title"] = d.question_title;
                    list.append(item);
                }
                root["data"] = list;
                
                *json_out = SerializeJson(root);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool GetDiscussion(const std::string &id, std::string *json_out)
        {
            Discussion d;
            if (model_.GetOneDiscussion(id, &d)) {
                Json::Value root;
                root["status"] = 0;
                Json::Value item;
                item["id"] = d.id;
                item["title"] = d.title;
                item["content"] = d.content;
                item["author"] = d.author_name;
                item["avatar"] = d.author_avatar;
                item["date"] = d.created_at;
                item["likes"] = d.likes;
                item["views"] = d.views;
                item["comments"] = d.comments_count;
                item["isOfficial"] = d.is_official;
                root["data"] = item;
                
                *json_out = SerializeJson(root);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Not Found";
                
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool AddDiscussion(const std::string &user_id, const std::string &title, const std::string &content, const std::string &question_id, std::string *json_out)
        {
            Discussion d;
            d.title = title;
            d.content = content;
            d.author_id = user_id;
            d.is_official = false; // Default false
            d.question_id = question_id;
            
            if (model_.AddDiscussion(d)) {
                Json::Value res;
                res["status"] = 0;
                res["reason"] = "Success";
                
                *json_out = SerializeJson(res);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool GetDiscussionsByQuestionId(const std::string &qid, std::string *json_out)
        {
            std::vector<Discussion> posts;
            if (model_.GetDiscussionsByQuestionId(qid, &posts)) {
                Json::Value root;
                root["status"] = 0;
                Json::Value list;
                for (const auto &d : posts) {
                    Json::Value item;
                    item["id"] = d.id;
                    item["title"] = d.title;
                    item["content"] = d.content;
                    item["author"] = d.author_name;
                    item["avatar"] = d.author_avatar;
                    item["date"] = d.created_at;
                    item["likes"] = d.likes;
                    item["views"] = d.views;
                    item["comments"] = d.comments_count;
                    item["isOfficial"] = d.is_official;
                    item["question_id"] = d.question_id;
                    item["question_title"] = d.question_title;
                    list.append(item);
                }
                root["data"] = list;
                
                *json_out = SerializeJson(root);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool AddArticleComment(const std::string &user_id, const std::string &post_id, const std::string &content, std::string *json_out)
        {
            ArticleComment c;
            c.user_id = user_id;
            c.post_id = post_id;
            c.content = content;
            
            if (model_.AddArticleComment(c)) {
                Json::Value res;
                res["status"] = 0;
                res["reason"] = "Success";
                
                *json_out = SerializeJson(res);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool GetArticleComments(const std::string &post_id, std::string *json_out)
        {
            std::vector<ArticleComment> comments;
            if (model_.GetArticleComments(post_id, &comments)) {
                Json::Value root;
                root["status"] = 0;
                Json::Value list;
                for (const auto &c : comments) {
                    Json::Value item;
                    item["id"] = c.id;
                    item["user_id"] = c.user_id;
                    item["username"] = c.username;
                    item["post_id"] = c.post_id;
                    item["content"] = c.content;
                    item["avatar"] = c.user_avatar;
                    item["created_at"] = c.created_at;
                    item["likes"] = c.likes;
                    list.append(item);
                }
                root["data"] = list;
                
                *json_out = SerializeJson(root);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                
                *json_out = SerializeJson(res);
                return false;
            }
        }

        // Training List Pages

        bool TrainingListPage(const Request &req, std::string *html)
        {
            User user;
            AuthCheck(req, &user);
            
            // Just render the page structure, JS will fetch data
            view_.TrainingListHtml(html, &user);
            return true;
        }

        bool TrainingDetail(const std::string &id, const Request &req, std::string *html)
        {
            User user;
            AuthCheck(req, &user);
            
            TrainingList list;
            if (!model_.GetTrainingList(id, &list)) {
                *html = "Training List Not Found";
                return false;
            }

            // Check visibility
            if (list.visibility == "private") {
                if (user.id != list.author_id) {
                    *html = "Access Denied (Private List)";
                    return false;
                }
            }

            std::vector<TrainingListItem> items;
            model_.GetTrainingListProblems(id, user.id, &items);

            view_.TrainingDetailHtml(list, items, html, &user);
            return true;
        }

        bool LoginPage(const Request &req, std::string *html)
        {
            // Just render the page
            view_.LoginHtml(html);
            return true;
        }

        // Training List APIs

        bool CreateTrainingList(const Request &req, std::string *json_out) {
            User user;
            if (!AuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 401;
                res["reason"] = "Unauthorized";
                *json_out = SerializeJson(res);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            
            TrainingList list;
            list.title = root["title"].asString();
            list.description = root.get("description", "").asString();
            list.difficulty = root.get("difficulty", "Unrated").asString();
            list.tags = root.get("tags", "[]").asString(); // Should be JSON string
            list.visibility = root.get("visibility", "public").asString();
            list.author_id = user.id;

            int new_id = 0;
            if (model_.CreateTrainingList(list, &new_id)) {
                Json::Value res;
                res["status"] = 0;
                res["id"] = new_id;
                *json_out = SerializeJson(res);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool UpdateTrainingList(const Request &req, std::string *json_out) {
            User user;
            if (!AuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 401;
                res["reason"] = "Unauthorized";
                *json_out = SerializeJson(res);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            
            std::string id = root["id"].asString();
            TrainingList list;
            if (!model_.GetTrainingList(id, &list)) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "List Not Found";
                *json_out = SerializeJson(res);
                return false;
            }

            if (list.author_id != user.id) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            if (root.isMember("title")) list.title = root["title"].asString();
            if (root.isMember("description")) list.description = root["description"].asString();
            if (root.isMember("difficulty")) list.difficulty = root["difficulty"].asString();
            if (root.isMember("tags")) list.tags = root["tags"].asString();
            if (root.isMember("visibility")) list.visibility = root["visibility"].asString();

            if (model_.UpdateTrainingList(list)) {
                Json::Value res;
                res["status"] = 0;
                *json_out = SerializeJson(res);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool DeleteTrainingList(const Request &req, std::string *json_out) {
            User user;
            if (!AuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 401;
                res["reason"] = "Unauthorized";
                *json_out = SerializeJson(res);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            std::string id = root["id"].asString();

            TrainingList list;
            if (!model_.GetTrainingList(id, &list)) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "List Not Found";
                *json_out = SerializeJson(res);
                return false;
            }

            if (list.author_id != user.id) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            if (model_.DeleteTrainingList(id)) {
                Json::Value res;
                res["status"] = 0;
                *json_out = SerializeJson(res);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool GetQuestionsByPageJson(const Request &req, std::string *json_out)
        {
            int page = 1;
            int page_size = 20;
            if (req.has_param("page")) {
                try {
                    page = std::stoi(req.get_param_value("page"));
                } catch (...) { page = 1; }
            }
            if (req.has_param("page_size")) {
                try {
                    page_size = std::stoi(req.get_param_value("page_size"));
                } catch (...) { page_size = 20; }
            }
            if (page < 1) page = 1;
            if (page_size < 1) page_size = 20;
            
            std::vector<struct Question> questions;
            int total = 0;
            
            if (model_.GetQuestionsByPage(page, page_size, &questions, &total)) {
                Json::Value root;
                root["status"] = 0;
                root["total"] = total;
                root["page"] = page;
                root["page_size"] = page_size;
                
                Json::Value list(Json::arrayValue);
                for (const auto &q : questions) {
                    Json::Value item;
                    item["number"] = q.number;
                    item["title"] = q.title;
                    item["star"] = q.star;
                    list.append(item);
                }
                root["data"] = list;
                
                *json_out = SerializeJson(root);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool AddProblemsToTrainingList(const Request &req, std::string *json_out) {
            User user;
            if (!AuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 401;
                res["reason"] = "Unauthorized";
                *json_out = SerializeJson(res);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            std::string list_id = root["training_list_id"].asString();
            Json::Value question_ids = root["question_ids"];

            if (list_id.empty()) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Training list ID cannot be empty";
                *json_out = SerializeJson(res);
                return false;
            }

            if (!question_ids.isArray()) {
                 Json::Value res;
                res["status"] = 1;
                res["reason"] = "Invalid question_ids format";
                *json_out = SerializeJson(res);
                return false;
            }
            
            for (const auto& qid : question_ids) {
                if (qid.asString().empty()) {
                    Json::Value res;
                    res["status"] = 1;
                    res["reason"] = "Question ID cannot be empty string";
                    *json_out = SerializeJson(res);
                    return false;
                }
            }

            TrainingList list;
            if (!model_.GetTrainingList(list_id, &list)) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "List Not Found";
                *json_out = SerializeJson(res);
                return false;
            }

            if (list.author_id != user.id) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            int success_count = 0;
            for (const auto& qid : question_ids) {
                if (model_.AddProblemToTrainingList(list_id, qid.asString())) {
                    success_count++;
                }
            }

            Json::Value res;
            res["status"] = 0;
            res["added_count"] = success_count;
            *json_out = SerializeJson(res);
            return true;
        }

        bool AddProblemToTrainingList(const Request &req, std::string *json_out) {
            User user;
            if (!AuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 401;
                res["reason"] = "Unauthorized";
                *json_out = SerializeJson(res);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            std::string list_id = root["training_list_id"].asString();
            std::string question_id = root["question_id"].asString();

            if (list_id.empty()) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Training list ID cannot be empty";
                *json_out = SerializeJson(res);
                return false;
            }

            if (question_id.empty()) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Question ID cannot be empty";
                *json_out = SerializeJson(res);
                return false;
            }

            TrainingList list;
            if (!model_.GetTrainingList(list_id, &list)) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "List Not Found";
                *json_out = SerializeJson(res);
                return false;
            }

            if (list.author_id != user.id) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            if (model_.AddProblemToTrainingList(list_id, question_id)) {
                Json::Value res;
                res["status"] = 0;
                *json_out = SerializeJson(res);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error or Already Exists";
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool RemoveProblemFromTrainingList(const Request &req, std::string *json_out) {
            User user;
            if (!AuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 401;
                res["reason"] = "Unauthorized";
                *json_out = SerializeJson(res);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            std::string list_id = root["training_list_id"].asString();
            std::string question_id = root["question_id"].asString();

            TrainingList list;
            if (!model_.GetTrainingList(list_id, &list)) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "List Not Found";
                *json_out = SerializeJson(res);
                return false;
            }

            if (list.author_id != user.id) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            if (model_.RemoveProblemFromTrainingList(list_id, question_id)) {
                Json::Value res;
                res["status"] = 0;
                *json_out = SerializeJson(res);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool ReorderTrainingListProblems(const Request &req, std::string *json_out) {
            User user;
            if (!AuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 401;
                res["reason"] = "Unauthorized";
                *json_out = SerializeJson(res);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            std::string list_id = root["training_list_id"].asString();
            Json::Value problem_ids_json = root["problem_ids"];
            
            std::vector<std::string> problem_ids;
            if (problem_ids_json.isArray()) {
                for (const auto &id : problem_ids_json) {
                    problem_ids.push_back(id.asString());
                }
            }

            TrainingList list;
            if (!model_.GetTrainingList(list_id, &list)) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "List Not Found";
                *json_out = SerializeJson(res);
                return false;
            }

            if (list.author_id != user.id) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            if (model_.ReorderTrainingListProblems(list_id, problem_ids)) {
                Json::Value res;
                res["status"] = 0;
                *json_out = SerializeJson(res);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool GetTrainingLists(const Request &req, std::string *json_out) {
            // Public API (or with auth for private lists?)
            // Spec says: Filters: Difficulty, Tags, Author.
            // And user's own lists.
            
            User user;
            AuthCheck(req, &user); // Optional auth
            
            int page = 1;
            int page_size = 20;
            if (req.has_param("page")) page = std::stoi(req.get_param_value("page"));
            if (req.has_param("limit")) page_size = std::stoi(req.get_param_value("limit"));
            
            std::string visibility = req.get_param_value("visibility");
            std::string author_id = req.get_param_value("author_id");
            
            // If fetching "my lists", visibility can be empty (all) but author_id must be me.
            if (author_id == "me") {
                if (user.id.empty()) {
                    Json::Value res;
                    res["status"] = 401;
                    res["reason"] = "Unauthorized";
                    *json_out = SerializeJson(res);
                    return false;
                }
                author_id = user.id;
                // visibility can be ignored or set to empty to get all
                visibility = ""; 
            } else {
                // Fetching public lists
                visibility = "public";
            }

            std::vector<TrainingList> lists;
            int total = 0;
            if (model_.GetTrainingLists(page, page_size, visibility, author_id, &lists, &total)) {
                Json::Value root;
                root["status"] = 0;
                root["total"] = total;
                root["page"] = page;
                
                Json::Value data;
                for (const auto &l : lists) {
                    Json::Value item;
                    item["id"] = l.id;
                    item["title"] = l.title;
                    item["description"] = l.description;
                    item["difficulty"] = l.difficulty;
                    item["author_name"] = l.author_name;
                    item["author_avatar"] = l.author_avatar;
                    item["problem_count"] = l.problem_count;
                    item["likes"] = l.likes;
                    item["collections"] = l.collections;
                    item["created_at"] = l.created_at;
                    data.append(item);
                }
                root["data"] = data;
                
                *json_out = SerializeJson(root);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool GetTrainingListDetailJson(const std::string &id, const Request &req, std::string *json_out) {
            // For API calls to get detail
            User user;
            AuthCheck(req, &user);

            TrainingList list;
            if (!model_.GetTrainingList(id, &list)) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Not Found";
                *json_out = SerializeJson(res);
                return false;
            }

            if (list.visibility == "private" && list.author_id != user.id) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            std::vector<TrainingListItem> items;
            model_.GetTrainingListProblems(id, user.id, &items);

            Json::Value root;
            root["status"] = 0;
            Json::Value data;
            data["id"] = list.id;
            data["title"] = list.title;
            data["description"] = list.description;
            data["difficulty"] = list.difficulty;
            data["tags"] = list.tags;
            data["author_id"] = list.author_id;
            data["author_name"] = list.author_name;
            data["visibility"] = list.visibility;
            
            Json::Value problems;
            for (const auto &item : items) {
                Json::Value p;
                p["id"] = item.question_id;
                p["title"] = item.question_title;
                p["difficulty"] = item.question_difficulty;
                p["status"] = item.user_status;
                problems.append(p);
            }
            data["problems"] = problems;
            root["data"] = data;

            *json_out = SerializeJson(root);
            return true;
        }

        bool GetUsers(const Request &req, std::string *json_out) {
            User user;
            if (!AdminAuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            int page = 1;
            int page_size = 20;
            if (req.has_param("page")) page = std::stoi(req.get_param_value("page"));
            if (req.has_param("page_size")) page_size = std::stoi(req.get_param_value("page_size"));
            std::string keyword = req.get_param_value("keyword");

            std::vector<User> users;
            int total = 0;
            if (model_.GetUsers(page, page_size, keyword, &users, &total)) {
                Json::Value root;
                root["status"] = 0;
                root["total"] = total;
                root["page"] = page;
                root["page_size"] = page_size;
                
                Json::Value list;
                for (const auto &u : users) {
                    Json::Value item;
                    item["id"] = u.id;
                    item["username"] = u.username;
                    item["email"] = u.email;
                    item["nickname"] = u.nickname;
                    item["phone"] = u.phone;
                    item["role"] = u.role;
                    item["status"] = u.status;
                    item["created_at"] = u.created_at;
                    list.append(item);
                }
                root["data"] = list;
                *json_out = SerializeJson(root);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool GetDashboardStats(const Request &req, std::string *json_out) {
            User user;
            if (!AdminAuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            std::map<std::string, int> user_growth;
            std::map<std::string, int> submission_stats;
            std::map<std::string, int> daily_activity;
            int total_users = 0;
            int total_problems = 0;
            int total_submissions = 0;

            // Get stats
            model_.GetUserGrowthStats(30, &user_growth);
            model_.GetSubmissionStats(&submission_stats);
            model_.GetDailyActivityStats(30, &daily_activity);
            model_.GetTotalUserCount(&total_users);
            model_.GetTotalProblemCount(&total_problems);
            model_.GetTotalSubmissionCount(&total_submissions);

            Json::Value root;
            root["status"] = 0;
            
            Json::Value data;

            Json::Value ug_json;
            for(auto const& kv : user_growth) {
                ug_json[kv.first] = kv.second;
            }
            data["user_growth"] = ug_json;

            Json::Value ss_json;
            for(auto const& kv : submission_stats) {
                if (kv.first == "0") ss_json["通过(AC)"] = kv.second;
                else if (kv.first == "-1") ss_json["未通过(WA)"] = kv.second;
                else ss_json["未知"] = ss_json.get("未知", 0).asInt() + kv.second;
            }
            data["submission_stats"] = ss_json;

            Json::Value da_json;
            for(auto const& kv : daily_activity) {
                da_json[kv.first] = kv.second;
            }
            data["daily_activity"] = da_json;
            
            data["total_users"] = total_users;
            data["total_problems"] = total_problems;
            data["total_submissions"] = total_submissions;

            root["data"] = data;

            *json_out = SerializeJson(root);
            return true;
        }

    };
} // namespace ns_control
