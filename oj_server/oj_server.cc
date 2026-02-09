#include <iostream>
#include <signal.h>
#include <clocale> // For setlocale
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
    // Set locale to use environment variables (en_US.UTF-8 from Dockerfile)
    std::setlocale(LC_ALL, "en_US.UTF-8");

    // Log system locale
    const char* lang = std::getenv("LANG");
    const char* lc_all = std::getenv("LC_ALL");
    std::cout << "[INFO] Startup Locale Check: LANG=" << (lang ? lang : "null") 
              << ", LC_ALL=" << (lc_all ? lc_all : "null") << std::endl;

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

    // 讨论区页面
    svr.Get("/discussion", [&ctrl](const Request &req, Response &resp){
        // 权限检查
        User user;
        if (!ctrl.AuthCheck(req, &user)) {
            resp.set_redirect("/login");
            return;
        }
        
        std::string html;
        ctrl.DiscussionPage(req, &html);
        resp.set_content(html, "text/html; charset=utf-8");
    });

    // 竞赛页面
    svr.Get("/contest", [&ctrl](const Request &req, Response &resp){
        // 权限检查 (可选，是否需要登录才能看？通常竞赛列表是公开的，但为了保持一致性可以要求登录)
        // Codeforces allows viewing contests without login.
        // But my implementation of `Contest` calls `AuthCheck` to get user info for navbar.
        // It doesn't enforce login.
        // However, `DiscussionPage` enforced login in the snippet above (lines 51-57).
        // Let's keep it open but show user info if logged in.
        // Wait, `DiscussionPage` in `oj_server.cc` (line 54) checks auth and redirects if not logged in.
        // `Contest` in `oj_control` calls `AuthCheck` but ignores return value (just populates user).
        // So I can just call `ctrl.Contest`.
        
        std::string html;
        ctrl.Contest(req, &html);
        resp.set_content(html, "text/html; charset=utf-8");
    });

    // API Contests
    svr.Get("/api/contests", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.GetContestList(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
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

    // API Get Single Question (JSON)
    svr.Get(R"(/api/question/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string number = req.matches[1];
        std::string json;
        ctrl.GetQuestionJson(number, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    // 用户提交代码，使用我们的判题功能(1. 每道题的测试用例 2. compile_and_run)
    svr.Post(R"(/judge/(\d+))", [&ctrl](const Request &req, Response &resp){
        // 权限检查
        User user;
        if (!ctrl.AuthCheck(req, &user)) {
             Json::Value err;
             err["status"] = -1;
             err["reason"] = "请先登录";
             
             resp.set_content(SerializeJson(err), "application/json;charset=utf-8");
             return;
        }

        std::string number = req.matches[1];
        std::string result_json;
        try {
            ctrl.Judge(number, req.body, &result_json, user.id);
            resp.set_content(result_json, "application/json;charset=utf-8");
        } catch (const std::exception &e) {
            Json::Value err;
            err["status"] = -1;
            err["reason"] = std::string("判题异常: ") + e.what();
            
            resp.set_content(SerializeJson(err), "application/json;charset=utf-8");
        } catch (...) {
            Json::Value err;
            err["status"] = -1;
            err["reason"] = "判题时发生未知错误";
            
            resp.set_content(SerializeJson(err), "application/json;charset=utf-8");
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
        
        resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
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
        
        
        resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
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
        
        resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
    });

    // User Profile Page
    svr.Get("/profile", [&ctrl](const Request &req, Response &resp){
        // 权限检查
        User user;
        if (!ctrl.AuthCheck(req, &user)) {
            resp.set_redirect("/login");
            return;
        }

        std::string html;
        if (ctrl.GetProfile(user, &html)) {
            resp.set_content(html, "text/html; charset=utf-8");
        } else {
            resp.set_content("获取个人中心失败", "text/plain; charset=utf-8");
        }
    });

    // API User Profile
    svr.Get("/api/profile", [&ctrl](const Request &req, Response &resp){
        User user;
        Json::Value res_json;
        if (ctrl.AuthCheck(req, &user)) {
             std::string data;
             if(ctrl.GetProfileData(user, &data)) {
                 resp.set_content(data, "application/json;charset=utf-8");
                 return;
             } else {
                 res_json["status"] = 1;
                 res_json["reason"] = "获取数据失败";
             }
        } else {
            res_json["status"] = 1;
            res_json["reason"] = "未登录";
        }
        
        resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
    });

    // API Update User Profile
    svr.Post("/api/user/update", [&ctrl](const Request &req, Response &resp){
        // 1. 验证登录
        User user;
        Json::Value res_json;
        if (!ctrl.AuthCheck(req, &user)) {
            res_json["status"] = 401;
            res_json["reason"] = "Unauthorized";
            
            resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
            return;
        }
        
        // 2. 解析参数
        std::string nickname, email, phone;
        if (req.has_header("Content-Type") && req.get_header_value("Content-Type").find("application/json") != std::string::npos) {
             Json::Reader reader;
             Json::Value val;
             reader.parse(req.body, val);
             nickname = val.get("nickname", "").asString();
             email = val.get("email", "").asString();
             phone = val.get("phone", "").asString();
        } else {
             // Fallback
             if (req.has_param("nickname")) nickname = req.get_param_value("nickname");
             if (req.has_param("email")) email = req.get_param_value("email");
             if (req.has_param("phone")) phone = req.get_param_value("phone");
        }
        
        // 3. 调用 Control 更新
        if (ctrl.UpdateUserInfo(user.id, nickname, email, phone)) {
            res_json["status"] = 0;
            res_json["reason"] = "Success";
        } else {
            res_json["status"] = 500;
            res_json["reason"] = "Internal Server Error";
        }
        
        resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
    });

    // API Logout
    svr.Get("/api/logout", [&ctrl](const Request &req, Response &resp){
        ctrl.Logout(req);
        Json::Value res_json;
        res_json["status"] = 0;
        res_json["reason"] = "success";
        resp.set_header("Set-Cookie", "session_id=; Path=/; Max-Age=0; HttpOnly");
        
        resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
    });
    
    // API Search Submissions
    svr.Get("/api/submissions", [&ctrl](const Request &req, Response &resp){
        User user;
        Json::Value res_json;
        if (!ctrl.AuthCheck(req, &user)) {
             res_json["status"] = 401;
             res_json["reason"] = "Unauthorized";
             
             resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
             return;
        }
        
        std::string json_out;
        ctrl.SearchSubmissions(req, &json_out);
        resp.set_content(json_out, "application/json;charset=utf-8");
    });

    // API Add Inline Comment
    svr.Post("/api/inline_comment/add", [&ctrl](const Request &req, Response &resp){
        User user;
        Json::Value res_json;
        if (!ctrl.AuthCheck(req, &user)) {
             res_json["status"] = 401;
             res_json["reason"] = "请先登录";
             
             resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
             return;
        }

        Json::Reader reader;
        Json::Value root;
        reader.parse(req.body, root);
        std::string post_id = root["post_id"].asString();
        std::string content = root["content"].asString();
        std::string selected_text = root["selected_text"].asString();
        std::string parent_id = root.isMember("parent_id") ? root["parent_id"].asString() : "0";

        std::string json_out;
        ctrl.AddInlineComment(user.id, post_id, content, selected_text, parent_id, &json_out);
        resp.set_content(json_out, "application/json;charset=utf-8");
    });

    // API Delete Inline Comment
    svr.Post("/api/inline_comment/delete", [&ctrl](const Request &req, Response &resp){
        User user;
        Json::Value res_json;
        if (!ctrl.AuthCheck(req, &user)) {
             res_json["status"] = 401;
             res_json["reason"] = "请先登录";
             
             resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
             return;
        }

        Json::Reader reader;
        Json::Value root;
        reader.parse(req.body, root);
        std::string comment_id = root["comment_id"].asString();

        std::string json_out;
        ctrl.DeleteInlineComment(comment_id, user.id, user.role, &json_out);
        resp.set_content(json_out, "application/json;charset=utf-8");
    });

    // API Get Inline Comments
    svr.Get(R"(/api/inline_comments/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string post_id = req.matches[1];
        std::string json_out;
        ctrl.GetInlineComments(post_id, &json_out);
        resp.set_content(json_out, "application/json;charset=utf-8");
    });
    
    // --- Admin APIs ---
    svr.Get("/api/admin/questions", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.AllQuestionsAdmin(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Post("/api/admin/question", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.AddQuestion(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Post(R"(/api/admin/question/update/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string number = req.matches[1];
        std::string json;
        ctrl.UpdateQuestion(number, req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Post(R"(/api/admin/question/delete/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string number = req.matches[1];
        std::string json;
        ctrl.DeleteQuestion(number, req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    // Admin Page Redirect
    svr.Get("/admin", [](const Request &req, Response &resp){
        resp.set_redirect("/admin/index.html");
    });
    
    // --- Discussion & Article Comments APIs ---
    
    // Get Discussion List
    svr.Get("/api/discussions", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.GetAllDiscussions(&json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    // Get Single Discussion
    svr.Get(R"(/api/discussion/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string id = req.matches[1];
        std::string json;
        ctrl.GetDiscussion(id, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    // Add Discussion
    svr.Post("/api/discussion", [&ctrl](const Request &req, Response &resp){
        User user;
        Json::Value res_json;
        if (!ctrl.AuthCheck(req, &user)) {
             res_json["status"] = 401;
             res_json["reason"] = "Unauthorized";
             
             resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
             return;
        }
        
        Json::Reader reader;
        Json::Value root;
        reader.parse(req.body, root);
        std::string title = root["title"].asString();
        std::string content = root["content"].asString();
        std::string question_id = root.isMember("question_id") ? root["question_id"].asString() : "0";
        
        std::string json;
        ctrl.AddDiscussion(user.id, title, content, question_id, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });
    
    // Get Discussions by Question ID
    svr.Get(R"(/api/discussions/question/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string qid = req.matches[1];
        std::string json;
        ctrl.GetDiscussionsByQuestionId(qid, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    // Get Article Comments
    svr.Get(R"(/api/article_comments/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string id = req.matches[1];
        std::string json;
        ctrl.GetArticleComments(id, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    // Add Article Comment
    svr.Post("/api/article_comment/add", [&ctrl](const Request &req, Response &resp){
        User user;
        Json::Value res_json;
        if (!ctrl.AuthCheck(req, &user)) {
             res_json["status"] = 401;
             res_json["reason"] = "Unauthorized";
             
             resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
             return;
        }
        
        Json::Reader reader;
        Json::Value root;
        reader.parse(req.body, root);
        std::string post_id = root["post_id"].asString();
        std::string content = root["content"].asString();
        
        std::string json;
        ctrl.AddArticleComment(user.id, post_id, content, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    // API Upload Image
    svr.Post("/api/upload_image", [&ctrl](const Request &req, Response &resp){
        User user;
        Json::Value res_json;
        if (!ctrl.AuthCheck(req, &user)) {
             res_json["status"] = 401;
             res_json["reason"] = "Unauthorized";
             
             resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
             return;
        }
        
        std::string json;
        ctrl.UploadImage(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.set_base_dir("./wwwroot");
    svr.set_mount_point("/css", "./css");
    svr.set_mount_point("/uploads", "./wwwroot/uploads"); // Serve uploaded files
    svr.listen("0.0.0.0", 8088);
    return 0;
} 
