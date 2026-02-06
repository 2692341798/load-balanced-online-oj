#pragma once

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h> // For unshare
#include <pwd.h>   // For getpwnam

#include "../comm/log.hpp"
#include "../comm/util.hpp"

namespace ns_runner
{
    using namespace ns_util;
    using namespace ns_log;

    class Runner
    {
    public:
        Runner() {}
        ~Runner() {}

    public:
        //提供设置进程占用资源大小的接口
        static void SetProcLimit(int _cpu_limit, int _mem_limit)
        {
            // 设置CPU时长
            struct rlimit cpu_rlimit;
            cpu_rlimit.rlim_max = RLIM_INFINITY;
            cpu_rlimit.rlim_cur = _cpu_limit;
            setrlimit(RLIMIT_CPU, &cpu_rlimit);

            // 设置内存大小
            struct rlimit mem_rlimit;
            mem_rlimit.rlim_max = RLIM_INFINITY;
            mem_rlimit.rlim_cur = _mem_limit * 1024; //转化成为KB
            setrlimit(RLIMIT_AS, &mem_rlimit);

            // 安全: 限制进程数量，防止Fork炸弹
            struct rlimit nproc_rlimit;
            nproc_rlimit.rlim_max = 200; // 允许一定的线程数(Java/Go需要)
            nproc_rlimit.rlim_cur = 200;
            setrlimit(RLIMIT_NPROC, &nproc_rlimit);
        }
        // 指明文件名即可，不需要代理路径，不需要带后缀
        /*******************************************
         * 返回值 > 0: 程序异常了，退出时收到了信号，返回值就是对应的信号编号
         * 返回值 == 0: 正常运行完毕的，结果保存到了对应的临时文件中
         * 返回值 < 0: 内部错误
         * 
         * cpu_limit: 该程序运行的时候，可以使用的最大cpu资源上限
         * mem_limit: 改程序运行的时候，可以使用的最大的内存大小(KB)
         * *****************************************/
        static int Run(const std::string &file_name, int cpu_limit, int mem_limit, const std::string &language = "C++")
        {
            /*********************************************
             * 程序运行：
             * 1. 代码跑完，结果正确
             * 2. 代码跑完，结果不正确
             * 3. 代码没跑完，异常了
             * Run需要考虑代码跑完，结果正确与否吗？？不考虑！
             * 结果正确与否：是由我们的测试用例决定的！
             * 我们只考虑：是否正确运行完毕
             *
             * 我们必须知道可执行程序是谁？
             * 一个程序在默认启动的时候
             * 标准输入: 不处理
             * 标准输出: 程序运行完成，输出结果是什么
             * 标准错误: 运行时错误信息
             * *******************************************/
            std::string _execute = PathUtil::Exe(file_name, language);
            std::string _stdin   = PathUtil::Stdin(file_name);
            std::string _stdout  = PathUtil::Stdout(file_name);
            std::string _stderr  = PathUtil::Stderr(file_name);

            umask(0);
            int _stdin_fd = open(_stdin.c_str(), O_CREAT|O_RDONLY, 0644);
            int _stdout_fd = open(_stdout.c_str(), O_CREAT|O_WRONLY, 0644);
            int _stderr_fd = open(_stderr.c_str(), O_CREAT|O_WRONLY, 0644);

            if(_stdin_fd < 0 || _stdout_fd < 0 || _stderr_fd < 0){
                LOG(ERROR) << "运行时打开标准文件失败" << "\n";
                return -1; //代表打开文件失败
            }            

            pid_t pid = fork();
            if (pid < 0)
            {
                LOG(ERROR) << "运行时创建子进程失败" << "\n";
                close(_stdin_fd);
                close(_stdout_fd);
                close(_stderr_fd);
                return -2; //代表创建子进程失败
            }
            else if (pid == 0)
            {
                dup2(_stdin_fd, 0);
                dup2(_stdout_fd, 1);
                dup2(_stderr_fd, 2);

                SetProcLimit(cpu_limit, mem_limit);

                // 安全增强: 网络隔离 (仅Linux)
                #ifdef __linux__
                if (unshare(CLONE_NEWNET) != 0) {
                    // LOG(WARNING) << "Failed to isolate network namespace" << "\n";
                    // 继续执行，因为非root环境下unshare可能会失败，视部署环境而定
                }
                #endif

                // 安全增强: 降权运行
                // 如果当前是root用户(uid=0)，则降级为nobody用户(通常uid=65534)
                if (getuid() == 0) {
                    struct passwd *nobody = getpwnam("nobody");
                    if (nobody) {
                        // 先设置GID，再设置UID
                        setgid(nobody->pw_gid);
                        setuid(nobody->pw_uid);
                    }
                }
                
                if (language == "C++") {
                    execl(_execute.c_str(), _execute.c_str(), nullptr);
                } else if (language == "Python") {
                    execlp("python3", "python3", _execute.c_str(), nullptr);
                } else if (language == "Java") {
                    std::string cp = ns_util::temp_path + file_name;
                    execlp("java", "java", "-cp", cp.c_str(), "Main", nullptr);
                }
                
                exit(1);
            }
            else
            {
                close(_stdin_fd);
                close(_stdout_fd);
                close(_stderr_fd);
                int status = 0;
                waitpid(pid, &status, 0);
                // 程序运行异常，一定是因为因为收到了信号！
                LOG(INFO) << "运行完毕, info: " << (status & 0x7F) << "\n"; 
                // return status & 0x7F;
                // Fix: Check exit code as well
                if (WIFEXITED(status)) {
                    int exit_code = WEXITSTATUS(status);
                    LOG(INFO) << "Program exited with code: " << exit_code << "\n";
                    if (exit_code == 0) return 0;
                    return -4; // Non-zero exit code
                } else {
                    LOG(INFO) << "Program terminated by signal: " << (status & 0x7F) << "\n";
                    return status & 0x7F;
                }
            }
        }
    };
}