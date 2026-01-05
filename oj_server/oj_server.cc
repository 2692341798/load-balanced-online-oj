#include <iostream>
#include <signal.h>
#include <json/json.h>   // 通过 -I/opt/homebrew/include 已能找到

#include "../comm/httplib.h"
#include "oj_control.hpp"

using namespace httplib;
using namespace ns_control;

static Control *ctrl_ptr = nullptr;

void Recovery(int signo)
{
    ctrl_ptr->RecoveryMachine();
}

int main()
{
    signal(SIGQUIT, Recovery);

    //用户请求的服务路由功能
    Server svr;

    Control ctrl;
    ctrl_ptr = &ctrl;

    // 获取所有的题目列表
    svr.Get("/all_questions", [&ctrl](const Request &req, Response &resp){
        //返回一张包含有所有题目的html网页
        std::string html;
        ctrl.AllQuestions(&html);
        //用户看到的是什么呢？？网页数据 + 拼上了题目相关的数据
        resp.set_content(html, "text/html; charset=utf-8");
    });

    // 用户要根据题目编号，获取题目的内容
    // /question/100 -> 正则匹配
    // R"()", 原始字符串raw string,保持字符串内容的原貌，不用做相关的转义
    svr.Get(R"(/question/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string number = req.matches[1];
        std::string html;
        ctrl.Question(number, &html);
        resp.set_content(html, "text/html; charset=utf-8");
    });

    // 用户提交代码，使用我们的判题功能(1. 每道题的测试用例 2. compile_and_run)
    svr.Post(R"(/judge/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string number = req.matches[1];
        std::string result_json;
        try {
            ctrl.Judge(number, req.body, &result_json);
            resp.set_content(result_json, "application/json;charset=utf-8");
        } catch (const std::exception &e) {
            Json::Value err;
            err["status"] = -1;
            err["reason"] = std::string("判题异常: ") + e.what();
            Json::FastWriter w;
            resp.set_content(w.write(err), "application/json;charset=utf-8");
        } catch (...) {
            Json::Value err;
            err["status"] = -1;
            err["reason"] = "判题时发生未知错误";
            Json::FastWriter w;
            resp.set_content(w.write(err), "application/json;charset=utf-8");
        }
    });
    
    svr.set_base_dir("./wwwroot");
    svr.listen("0.0.0.0", 8080);
    return 0;
}