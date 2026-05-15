#pragma once

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "../../comm/httplib.h"
#include "../../comm/log.hpp"
#include "../../comm/util.hpp"
#include "../oj_model.hpp"

namespace ns_control
{
    using namespace std;
    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_model;
    using namespace httplib;

    std::string SerializeJson(const Json::Value &val) {
        Json::StreamWriterBuilder builder;
        builder["commentStyle"] = "None";
        builder["indentation"] = "";
        builder["emitUTF8"] = true;
        return Json::writeString(builder, val);
    }

    struct Session {
        User user;
        time_t expire_time;
    };

    class Machine
    {
    public:
        std::string ip;
        int port;
        uint64_t load;
        std::mutex *mtx;
    public:
        Machine() : ip(""), port(0), load(0), mtx(nullptr)
        {
        }
        ~Machine()
        {
        }

    public:
        void IncLoad()
        {
            if (mtx) mtx->lock();
            ++load;
            if (mtx) mtx->unlock();
        }
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
    class LoadBlance
    {
    private:
        std::vector<Machine> machines;
        std::vector<int> online;
        std::vector<int> offline;
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
}
