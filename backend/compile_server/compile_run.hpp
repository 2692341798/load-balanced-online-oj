#pragma once

#include "compiler.hpp"
#include "runner.hpp"
#include "../comm/log.hpp"
#include "../comm/util.hpp"

#include <signal.h>
#include <unistd.h>
#include <json/json.h>

namespace ns_compile_and_run
{
    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_compiler;
    using namespace ns_runner;

    class CompileAndRun
    {
    public:
        static void RemoveTempFile(const std::string &file_name, const std::string &language = "C++")
        {
            //清理文件的个数是不确定的，但是有哪些我们是知道的
            std::string _src = PathUtil::Src(file_name, language);
            if(FileUtil::IsFileExists(_src)) unlink(_src.c_str());

            std::string _compiler_error = PathUtil::CompilerError(file_name);
            if(FileUtil::IsFileExists(_compiler_error)) unlink(_compiler_error.c_str());

            std::string _execute = PathUtil::Exe(file_name, language);
            if(FileUtil::IsFileExists(_execute)) unlink(_execute.c_str());

            std::string _stdin = PathUtil::Stdin(file_name);
            if(FileUtil::IsFileExists(_stdin)) unlink(_stdin.c_str());

            std::string _stdout = PathUtil::Stdout(file_name);
            if(FileUtil::IsFileExists(_stdout)) unlink(_stdout.c_str());

            std::string _stderr = PathUtil::Stderr(file_name);
            if(FileUtil::IsFileExists(_stderr)) unlink(_stderr.c_str());
            
            std::string dir = ns_util::temp_path + file_name;
            rmdir(dir.c_str());
        }
        // code > 0 : 进程收到了信号导致异常奔溃
        // code < 0 : 整个过程非运行报错(代码为空，编译报错等)
        // code = 0 : 整个过程全部完成
        //待完善
        static std::string CodeToDesc(int code, const std::string &file_name)
        {
            std::string desc;
            switch (code)
            {
            case 0:
                desc = "编译运行成功";
                break;
            case -1:
                desc = "提交的代码是空";
                break;
            case -2:
                desc = "系统错误";
                break;
            case -3:
                // desc = "代码编译的时候发生了错误";
                FileUtil::ReadFile(PathUtil::CompilerError(file_name), &desc, true);
                break;
            case -4:
                desc = "测试用例未通过";
                break;
            case SIGABRT: // 6
                desc = "内存超过范围";
                break;
            case SIGXCPU: // 24
                desc = "CPU使用超时";
                break;
            case SIGFPE: // 8
                desc = "浮点数溢出";
                break;
            default:
                desc = "未知: " + std::to_string(code);
                break;
            }

            return desc;
        }
        static std::string CodeToCategory(int code)
        {
            if (code == 0) return "成功";
            if (code == -1) return "提交错误";
            if (code == -2) return "系统错误";
            if (code == -3) return "编译错误";
            if (code == -4) return "答案错误";
            if (code == SIGABRT) return "内存超限";
            if (code == SIGXCPU) return "时间超限";
            if (code == SIGFPE) return "浮点溢出";
            if (code > 0) return "运行时错误";
            return "未知错误";
        }

        /***************************************
         * 输入:
         * code： 用户提交的代码
         * input: 用户给自己提交的代码对应的输入，不做处理
         * cpu_limit: 时间要求
         * mem_limit: 空间要求
         *
         * 输出:
         * 必填
         * status: 状态码
         * reason: 请求结果
         * 选填：
         * stdout: 我的程序运行完的结果
         * stderr: 我的程序运行完的错误结果
         *
         * 参数：
         * in_json: {"code": "#include...", "input": "","cpu_limit":1, "mem_limit":10240}
         * out_json: {"status":"0", "reason":"","stdout":"","stderr":"",}
         * ************************************/
        static void Start(const std::string &in_json, std::string *out_json)
        {
            Json::Value in_value;
            Json::Reader reader;
            reader.parse(in_json, in_value); //最后在处理差错问题

            std::string code = in_value["code"].asString();
            std::string input = in_value["input"].asString();
            int cpu_limit = in_value["cpu_limit"].asInt();
            int mem_limit = in_value["mem_limit"].asInt();
            std::string language = in_value.isMember("language") ? in_value["language"].asString() : "C++";

            // 安全检查: 限制资源最大值，防止DoS攻击
            if (cpu_limit <= 0 || cpu_limit > 30) {
                LOG(WARNING) << "Invalid cpu_limit: " << cpu_limit << ", clamped to 30s" << "\n";
                cpu_limit = 30;
            }
            if (mem_limit <= 0 || mem_limit > 512 * 1024) { // 512MB
                LOG(WARNING) << "Invalid mem_limit: " << mem_limit << ", clamped to 512MB" << "\n";
                mem_limit = 512 * 1024;
            }

            int status_code = 0;
            Json::Value out_value;
            int run_result = 0;
            std::string file_name; //需要内部形成的唯一文件名
            std::string dir;

            if (code.size() == 0)
            {
                status_code = -1; //代码为空
                goto END;
            }
            // 形成的文件名只具有唯一性，没有目录没有后缀
            // 毫秒级时间戳+原子性递增唯一值: 来保证唯一性
            file_name = FileUtil::UniqFileName();
            // Create directory
            dir = ns_util::temp_path + file_name;
            if (mkdir(dir.c_str(), 0755) != 0) {
                LOG(ERROR) << "创建临时目录失败: " << dir << " errno: " << errno << "\n";
                status_code = -2;
                goto END;
            }
            
            //形成临时src文件
            if (!FileUtil::WriteFile(PathUtil::Src(file_name, language), code))
            {
                LOG(ERROR) << "写入源文件失败: " << PathUtil::Src(file_name, language) << "\n";
                status_code = -2; //未知错误
                goto END;
            }

            // 写入输入数据到 stdin 文件
            LOG(INFO) << "Writing input to stdin, size: " << input.size() << " Content: " << input << "\n";
            if (!FileUtil::WriteFile(PathUtil::Stdin(file_name), input)) {
                LOG(ERROR) << "写入Stdin文件失败: " << PathUtil::Stdin(file_name) << "\n";
                status_code = -2;
                goto END;
            }
            chmod(PathUtil::Stdin(file_name).c_str(), 0644); // Ensure permissions

            if (!Compiler::Compile(file_name, language))
            {
                //编译失败
                status_code = -3; //代码编译的时候发生了错误
                goto END;
            }

            // 安全: 确保生成的程序对nobody用户是可读/可执行的
            // 因为Runner中会降权执行
            {
                std::string _exe_path = PathUtil::Exe(file_name, language);
                if (FileUtil::IsFileExists(_exe_path)) {
                    chmod(_exe_path.c_str(), 0755);
                }
            }

            run_result = Runner::Run(file_name, cpu_limit, mem_limit, language);
            if (run_result < 0)
            {
                if (run_result == -4) {
                    status_code = -4; // Runtime Error (Non-zero exit)
                } else {
                    status_code = -2; //系统错误
                }
            }
            else if (run_result > 0)
            {
                //程序运行崩溃了
                status_code = run_result;
            }
            else
            {
                //运行成功
                status_code = 0;
            }
        END:
            out_value["status"] = status_code;
            out_value["reason"] = CodeToDesc(status_code, file_name);
            out_value["category"] = CodeToCategory(status_code);
            if (status_code == -2)
            {
                if (run_result == -1) out_value["error_detail"] = "运行时打开标准文件失败";
                else if (run_result == -2) out_value["error_detail"] = "运行时创建子进程失败";
                else out_value["error_detail"] = "未知系统错误";
            }
            if (status_code > 0)
            {
                out_value["signal"] = status_code;
            }
            
            // Always try to read stdout/stderr to provide more info
            std::string _stdout;
            FileUtil::ReadFile(PathUtil::Stdout(file_name), &_stdout, true);
            out_value["stdout"] = _stdout;

            std::string _stderr;
            FileUtil::ReadFile(PathUtil::Stderr(file_name), &_stderr, true);
            out_value["stderr"] = _stderr;

            Json::StyledWriter writer;
            *out_json = writer.write(out_value);

            RemoveTempFile(file_name, language);
        }
    };
}
