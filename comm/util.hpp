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
#include <cctype>

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
            path_name += "/Main";
            path_name += suffix;
            return path_name;
        }
        // 编译时需要有的临时文件
        // 构建源文件路径+后缀的完整文件名
        // 1234 -> ./temp/1234/Main.cpp
        static std::string Src(const std::string &file_name, const std::string &language = "C++")
        {
            std::string ext = ".cpp";
            if (language == "Java") ext = ".java";
            else if (language == "Python") ext = ".py";
            return AddSuffix(file_name, ext);
        }
        // 构建可执行程序的完整路径+后缀名
        static std::string Exe(const std::string &file_name, const std::string &language = "C++")
        {
            if (language == "Java") return AddSuffix(file_name, ".class");
            if (language == "Python") return AddSuffix(file_name, ".py");
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
        static void SplitString(const std::string &str, std::vector<std::string> *target, const std::string &sep)
        {
            std::string::size_type start = 0;
            while (start < str.length())
            {
                auto pos = str.find_first_of(sep, start);
                if (pos == std::string::npos)
                {
                    if (start < str.length())
                    {
                        target->push_back(str.substr(start));
                    }
                    break;
                }
                
                if (pos > start)
                {
                    target->push_back(str.substr(start, pos - start));
                }
                start = pos + 1;
            }
        }

        static std::string GetSummaryFromMarkdown(const std::string &md, size_t max_len = 150)
        {
            std::string res;
            bool in_code_block = false;
            size_t n = md.size();
            for (size_t i = 0; i < n; ++i)
            {
                if (res.size() >= max_len)
                {
                    res += "...";
                    break;
                }

                // Check for code block ```
                if (i + 2 < n && md[i] == '`' && md[i+1] == '`' && md[i+2] == '`')
                {
                    in_code_block = !in_code_block;
                    i += 2; 
                    if (!in_code_block) res += " [代码] ";
                    continue;
                }

                if (in_code_block) continue;

                // Skip heading markers
                if (md[i] == '#') continue;
                
                // Collapse whitespace
                if (isspace(md[i]))
                {
                    if (!res.empty() && res.back() != ' ') res += ' ';
                }
                else
                {
                    res += md[i];
                }
            }
            if (res.empty()) return "点击查看详情";
            return res;
        }
    };
}