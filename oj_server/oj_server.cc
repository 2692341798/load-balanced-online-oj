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

    // 4. 配置路由
    // 4.1 首页
    svr.Get("/", [&ctrl](const Request &req, Response &resp){
        resp.set_redirect("/all_questions");
    });

    // 获取所有的题目列表
    svr.Get("/all_questions", [&ctrl](const Request &req, Response &resp){
        // 权限检查
        User user;
        if (!ctrl.AuthCheck(req, &user)) {
            resp.set_redirect("/login");
            return;
        }

        //返回一张包含有所有题目的html网页
        std::string html;
        ctrl.AllQuestions(req, &html);
        //用户看到的是什么呢？？网页数据 + 拼上了题目相关的数据
        resp.set_content(html, "text/html; charset=utf-8");
    });

    // 用户要根据题目编号，获取题目的内容
    // /question/100 -> 正则匹配
    // R"()", 原始字符串raw string,保持字符串内容的原貌，不用做相关的转义
    svr.Get(R"(/question/(\d+))", [&ctrl](const Request &req, Response &resp){
        // 权限检查
        User user;
        if (!ctrl.AuthCheck(req, &user)) {
            resp.set_redirect("/login");
            return;
        }

        std::string number = req.matches[1];
        std::string html;
        ctrl.Question(number, req, &html);
        resp.set_content(html, "text/html; charset=utf-8");
    });

    // 用户提交代码，使用我们的判题功能(1. 每道题的测试用例 2. compile_and_run)
    svr.Post(R"(/judge/(\d+))", [&ctrl](const Request &req, Response &resp){
        // 权限检查
        User user;
        if (!ctrl.AuthCheck(req, &user)) {
             Json::Value err;
             err["status"] = -1;
             err["reason"] = "请先登录";
             Json::FastWriter w;
             resp.set_content(w.write(err), "application/json;charset=utf-8");
             return;
        }

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

    // Login Page
    svr.Get("/login", [](const Request &req, Response &resp){
        std::ifstream in("./template_html/login.html");
        if(in.is_open()) {
            std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            resp.set_content(content, "text/html; charset=utf-8");
            in.close();
        } else {
            resp.set_content("Login Page Not Found", "text/plain");
        }
    });

    // API Login
    svr.Post("/api/login", [&ctrl](const Request &req, Response &resp){
        Json::Reader reader;
        Json::Value root;
        reader.parse(req.body, root);
        std::string username = root["username"].asString();
        std::string password = root["password"].asString();
        std::string token;
        
        Json::Value res_json;
        if(ctrl.Login(username, password, &token)) {
            res_json["status"] = 0;
            res_json["reason"] = "success";
            resp.set_header("Set-Cookie", "session_id=" + token + "; Path=/; Max-Age=86400; HttpOnly");
        } else {
            res_json["status"] = 1;
            res_json["reason"] = "用户名或密码错误";
        }
        Json::FastWriter w;
        resp.set_content(w.write(res_json), "application/json;charset=utf-8");
    });

    // API Register
    svr.Post("/api/register", [&ctrl](const Request &req, Response &resp){
        Json::Reader reader;
        Json::Value root;
        reader.parse(req.body, root);
        std::string username = root["username"].asString();
        std::string password = root["password"].asString();
        std::string email = root["email"].asString();
        std::string nickname = root.get("nickname", "").asString();
        std::string phone = root.get("phone", "").asString();
        
        Json::Value res_json;
        
        // 参数验证
        if(username.empty() || password.empty() || email.empty()) {
            res_json["status"] = 1;
            res_json["reason"] = "注册失败：用户名、密码和邮箱不能为空";
        } else if(username.length() < 3 || username.length() > 20) {
            res_json["status"] = 1;
            res_json["reason"] = "注册失败：用户名长度必须在3-20个字符之间";
        } else if(password.length() < 6 || password.length() > 30) {
            res_json["status"] = 1;
            res_json["reason"] = "注册失败：密码长度必须在6-30个字符之间";
        } else if(!ctrl.CheckUserExists(username) && ctrl.Register(username, password, email, nickname, phone)) {
            res_json["status"] = 0;
            res_json["reason"] = "success";
        } else if(ctrl.CheckUserExists(username)) {
            res_json["status"] = 1;
            res_json["reason"] = "注册失败：用户名已存在";
        } else {
            res_json["status"] = 1;
            res_json["reason"] = "注册失败：数据库错误";
        }
        
        Json::FastWriter w;
        resp.set_content(w.write(res_json), "application/json;charset=utf-8");
    });

    // API Check User Status
    svr.Get("/api/user", [&ctrl](const Request &req, Response &resp){
        Json::Value res_json;
        User user;
        if (ctrl.AuthCheck(req, &user)) {
            res_json["status"] = 0;
            res_json["username"] = user.username;
            res_json["email"] = user.email;
        } else {
            res_json["status"] = 1;
            res_json["reason"] = "未登录";
        }
        Json::FastWriter w;
        resp.set_content(w.write(res_json), "application/json;charset=utf-8");
    });

    // API Logout
    svr.Get("/api/logout", [&ctrl](const Request &req, Response &resp){
        ctrl.Logout(req);
        Json::Value res_json;
        res_json["status"] = 0;
        res_json["reason"] = "success";
        resp.set_header("Set-Cookie", "session_id=; Path=/; Max-Age=0; HttpOnly");
        Json::FastWriter w;
        resp.set_content(w.write(res_json), "application/json;charset=utf-8");
    });
    
    
    svr.set_base_dir("./wwwroot");
    svr.set_mount_point("/css", "./css");
    svr.listen("0.0.0.0", 8080);
    return 0;
} 
