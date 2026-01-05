#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <atomic>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <boost/algorithm/string.hpp>

namespace ns_util
{
    class TimeUtil
    {
    public:
        static std::string GetTimeStamp()
        {
            struct timeval _time;
            gettimeofday(&_time, nullptr);
            return std::to_string(_time.tv_sec);
        }
        //获得毫秒时间戳
        static std::string GetTimeMs()
        {
            struct timeval _time;
            gettimeofday(&_time, nullptr);
            return std::to_string(_time.tv_sec * 1000 + _time.tv_usec / 1000);
        }
    };

    const std::string temp_path = "./temp/";

    class PathUtil
    {
    public:
        static std::string AddSuffix(const std::string &file_name, const std::string &suffix)
        {
            std::string path_name = temp_path;
            path_name += file_name;
            path_name += suffix;
            return path_name;
        }
        // 编译时需要有的临时文件
        // 构建源文件路径+后缀的完整文件名
        // 1234 -> ./temp/1234.cpp
        static std::string Src(const std::string &file_name)
        {
            return AddSuffix(file_name, ".cpp");
        }
        // 构建可执行程序的完整路径+后缀名
        static std::string Exe(const std::string &file_name)
        {
            return AddSuffix(file_name, ".exe");
        }
        static std::string CompilerError(const std::string &file_name)
        {
            return AddSuffix(file_name, ".compile_error");
        }

        // 运行时需要的临时文件
        static std::string Stdin(const std::string &file_name)
        {
            return AddSuffix(file_name, ".stdin");
        }
        static std::string Stdout(const std::string &file_name)
        {
            return AddSuffix(file_name, ".stdout");
        }
        // 构建该程序对应的标准错误完整的路径+后缀名
        static std::string Stderr(const std::string &file_name)
        {
            return AddSuffix(file_name, ".stderr");
        }
    };

    class FileUtil
    {
    public:
        static bool IsFileExists(const std::string &path_name)
        {
            struct stat st;
            if (stat(path_name.c_str(), &st) == 0)
            {
                //获取属性成功，文件已经存在
                return true;
            }

            return false;
        }
        static std::string UniqFileName()
        {
            static std::atomic_uint id(0);
            id++;
            // 毫秒级时间戳+原子性递增唯一值: 来保证唯一性
            std::string ms = TimeUtil::GetTimeMs();
            std::string uniq_id = std::to_string(id);
            return ms + "_" + uniq_id;
        }
        static bool WriteFile(const std::string &target, const std::string &content)
        {
            std::ofstream out(target);
            if (!out.is_open())
            {
                return false;
            }
            out.write(content.c_str(), content.size());
            out.close();
            return true;
        }
        static bool ReadFile(const std::string &target, std::string *content, bool keep = false)
        {
            (*content).clear();

            std::ifstream in(target);
            if (!in.is_open())
            {
                return false;
            }
            std::string line;
            // getline:不保存行分割符,有些时候需要保留\n,
            // getline内部重载了强制类型转化
            while (std::getline(in, line))
            {
                (*content) += line;
                (*content) += (keep ? "\n" : "");
            }
            in.close();
            return true;
        }
    };

    class StringUtil
    {
    public:
        /*************************************
         * str: 输入型，目标要切分的字符串
         * target: 输出型，保存切分完毕的结果
         * sep: 指定的分割符
         * **********************************/
        static void SplitString(const std::string &str, std::vector<std::string> *target, const std::string &sep)
        {
            //boost split
            boost::split((*target), str, boost::is_any_of(sep), boost::algorithm::token_compress_on);
        }
    };
}