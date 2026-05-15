#pragma once

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

        auto is_valid_email = [](const std::string &s) -> bool {
            if (s.empty() || s.size() > 100) return false;
            size_t at_pos = std::string::npos;
            for (size_t i = 0; i < s.size(); ++i) {
                unsigned char c = static_cast<unsigned char>(s[i]);
                if (c <= 0x20 || c == 0x7f) return false;
                if (s[i] == '@') {
                    if (at_pos != std::string::npos) return false;
                    at_pos = i;
                }
                if (i > 0 && s[i] == '.' && s[i - 1] == '.') return false;
            }
            if (at_pos == std::string::npos || at_pos == 0 || at_pos + 1 >= s.size()) return false;

            const std::string domain = s.substr(at_pos + 1);
            const size_t dot_pos = domain.find('.');
            if (dot_pos == std::string::npos || dot_pos == 0 || dot_pos + 1 >= domain.size()) return false;
            return true;
        };
        
        if(username.empty() || password.empty() || email.empty()) {
            res_json["status"] = 1;
            res_json["reason"] = "注册失败：用户名、密码和邮箱不能为空";
        } else if(username.length() < 3 || username.length() > 20) {
            res_json["status"] = 1;
            res_json["reason"] = "注册失败：用户名长度必须在3-20个字符之间";
        } else if(password.length() < 6 || password.length() > 30) {
            res_json["status"] = 1;
            res_json["reason"] = "注册失败：密码长度必须在6-30个字符之间";
        } else if(!is_valid_email(email)) {
            res_json["status"] = 1;
            res_json["reason"] = "注册失败：邮箱格式不正确";
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

    svr.Get("/api/user", [&ctrl](const Request &req, Response &resp){
        Json::Value res_json;
        User user;
        if (ctrl.AuthCheck(req, &user)) {
            res_json["status"] = 0;
            res_json["username"] = user.username;
            res_json["email"] = user.email;
            res_json["avatar"] = user.avatar;
        } else {
            res_json["status"] = 1;
            res_json["reason"] = "未登录";
        }
        
        resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
    });

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

    svr.Post("/api/user/update", [&ctrl](const Request &req, Response &resp){
        User user;
        Json::Value res_json;
        if (!ctrl.AuthCheck(req, &user)) {
            res_json["status"] = 401;
            res_json["reason"] = "Unauthorized";
            
            resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
            return;
        }
        
        std::string nickname, email, phone;
        if (req.has_header("Content-Type") && req.get_header_value("Content-Type").find("application/json") != std::string::npos) {
             Json::Reader reader;
             Json::Value val;
             reader.parse(req.body, val);
             nickname = val.get("nickname", "").asString();
             email = val.get("email", "").asString();
             phone = val.get("phone", "").asString();
        } else {
             if (req.has_param("nickname")) nickname = req.get_param_value("nickname");
             if (req.has_param("email")) email = req.get_param_value("email");
             if (req.has_param("phone")) phone = req.get_param_value("phone");
        }
        
        if (ctrl.UpdateUserInfo(user.id, nickname, email, phone)) {
            res_json["status"] = 0;
            res_json["reason"] = "Success";
        } else {
            res_json["status"] = 500;
            res_json["reason"] = "Internal Server Error";
        }
        
        resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
    });

    svr.Get("/api/logout", [&ctrl](const Request &req, Response &resp){
        ctrl.Logout(req);
        Json::Value res_json;
        res_json["status"] = 0;
        res_json["reason"] = "success";
        resp.set_header("Set-Cookie", "session_id=; Path=/; Max-Age=0; HttpOnly");
        
        resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
    });
