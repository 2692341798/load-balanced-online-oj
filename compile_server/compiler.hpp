#pragma once

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../comm/util.hpp"
#include "../comm/log.hpp"

// 只负责进行代码的编译

namespace ns_compiler
{
    // 引入路径拼接功能
    using namespace ns_util;
    using namespace ns_log;

    class Compiler
    {
    public:
        Compiler()
        {}
        ~Compiler()
        {}
        //返回值：编译成功：true，否则：false
        //输入参数：编译的文件名
        //file_name: 1234
        //1234 -> ./temp/1234.cpp
        //1234 -> ./temp/1234.exe
        //1234 -> ./temp/1234.stderr
        static bool Compile(const std::string &file_name)
        {
            pid_t pid = fork();
            if(pid < 0)
            {
                LOG(ERROR) << "内部错误，创建子进程失败" << "\n";
                return false;
            }
            else if (pid == 0)
            {
                umask(0);
                int _stderr = open(PathUtil::CompilerError(file_name).c_str(), O_CREAT | O_WRONLY, 0644);
                if(_stderr < 0){
                    LOG(WARNING) << "没有成功形成stderr文件" << "\n";
                    exit(1);
                }
                //重定向标准错误到_stderr
                dup2(_stderr, 2);
                
                //程序替换，并不影响进程的文件描述符表
                //子进程: 调用编译器，完成对代码的编译工作
                //g++ -o target src -std=c++11
                execlp("g++", "g++", "-o", PathUtil::Exe(file_name).c_str(),\
                PathUtil::Src(file_name).c_str(), "-D", "COMPILER_ONLINE","-std=c++11",  nullptr/*不要忘记*/);
                LOG(ERROR) << "启动编译器g++失败，可能是参数错误" << "\n";
                exit(2);
            }
            else{
                waitpid(pid, nullptr, 0);
                //编译是否成功,就看有没有形成对应的可执行程序
                if(FileUtil::IsFileExists(PathUtil::Exe(file_name))){
                    LOG(INFO) << PathUtil::Src(file_name) << " 编译成功!" << "\n";
                    return true;
                }
            }
            LOG(ERROR) << "编译失败，没有形成可执行程序" << "\n";
            return false;
        }
    };
}