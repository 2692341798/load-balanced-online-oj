#pragma once

        bool AdminRegister(const Request &req, std::string *json_out) {
            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            std::string username = root["username"].asString();
            std::string password = root["password"].asString();
            std::string email = root["email"].asString();
            std::string invitation_code = root["invitation_code"].asString();

            if (username.empty() || password.empty() || email.empty() || invitation_code.empty()) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "All fields including invitation code are required";
                *json_out = SerializeJson(res);
                return false;
            }

            if (CheckUserExists(username)) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Username already exists";
                *json_out = SerializeJson(res);
                return false;
            }
            
            if (!model_.RegisterUser(username, password, email)) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error during Registration";
                *json_out = SerializeJson(res);
                return false;
            }

            std::string sql = "SELECT id, username, password, email, nickname, phone, created_at, role, avatar, status FROM " + oj_users + " WHERE username='" + username + "'";
            std::vector<User> users;
            model_.QueryUserMySql(sql, &users);
            if (users.empty()) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Failed to retrieve created user";
                *json_out = SerializeJson(res);
                return false;
            }
            User new_user = users[0];

            if (!model_.VerifyAndUseInvitationCode(invitation_code, new_user.id)) {
                std::string del_sql = "DELETE FROM " + oj_users + " WHERE id=" + new_user.id;
                model_.ExecuteSql(del_sql);
                
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Invalid or Used Invitation Code";
                *json_out = SerializeJson(res);
                return false;
            }

            model_.UpdateUserRole(new_user.id, 1);
            LogAdminOp(new_user.id, "Admin Register", "Self", "Registered as Admin with code " + invitation_code, req);

            Json::Value res;
            res["status"] = 0;
            res["reason"] = "Success";
            *json_out = SerializeJson(res);
            return true;
        }

        bool GenerateInvitationCode(const Request &req, std::string *json_out) {
            User user;
            if (!AdminAuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            std::string code = GenerateToken().substr(0, 16);
            const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            code = "";
            for (int i = 0; i < 16; ++i) {
                code += chars[rand() % chars.length()];
            }

            if (model_.GenerateInvitationCode(code)) {
                LogAdminOp(user.id, "Generate Invitation Code", code, "Generated new invitation code", req);
                Json::Value res;
                res["status"] = 0;
                res["code"] = code;
                *json_out = SerializeJson(res);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                *json_out = SerializeJson(res);
                return false;
            }
        }
