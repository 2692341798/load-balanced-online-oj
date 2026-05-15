#pragma once

        bool UpdateUserStatus(const Request &req, std::string *json_out) {
           User user;
           if (!AdminAuthCheck(req, &user)) {
               Json::Value res;
               res["status"] = 403;
               res["reason"] = "Permission Denied";
               *json_out = SerializeJson(res);
               return false;
           }

           Json::Reader reader;
           Json::Value root;
           reader.parse(req.body, root);
           std::string id = root["id"].asString();
           int status = root["status"].asInt();

           if (model_.UpdateUserStatus(id, status)) {
               LogAdminOp(user.id, "Update User Status", "User " + id, "Changed status to " + std::to_string(status), req);
               Json::Value res;
               res["status"] = 0;
               res["reason"] = "Success";
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

        bool UpdateUserRole(const Request &req, std::string *json_out) {
           User user;
           if (!AdminAuthCheck(req, &user)) {
               Json::Value res;
               res["status"] = 403;
               res["reason"] = "Permission Denied";
               *json_out = SerializeJson(res);
               return false;
           }

           Json::Reader reader;
           Json::Value root;
           reader.parse(req.body, root);
           std::string id = root["id"].asString();
           int role = root["role"].asInt();

           if (model_.UpdateUserRole(id, role)) {
               LogAdminOp(user.id, "Update User Role", "User " + id, "Changed role to " + std::to_string(role), req);
               Json::Value res;
               res["status"] = 0;
               res["reason"] = "Success";
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

        std::string GenerateRandomPassword(int length) {
            const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
            std::string pwd;
            for (int i = 0; i < length; ++i) {
                pwd += chars[rand() % chars.length()];
            }
            return pwd;
        }

        bool ResetUserPassword(const Request &req, std::string *json_out) {
            User user;
            if (!AdminAuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            std::string id = root["id"].asString();

            std::string new_pwd = GenerateRandomPassword(8);
            std::string new_hash = model_.SHA256Hash(new_pwd);

            if (model_.ResetUserPassword(id, new_hash)) {
                LogAdminOp(user.id, "Reset Password", "User " + id, "Reset password", req);
                Json::Value res;
                res["status"] = 0;
                res["new_password"] = new_pwd;
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

        bool GetUsers(const Request &req, std::string *json_out) {
            User user;
            if (!AdminAuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            int page = 1;
            int page_size = 20;
            if (req.has_param("page")) page = std::stoi(req.get_param_value("page"));
            if (req.has_param("page_size")) page_size = std::stoi(req.get_param_value("page_size"));
            std::string keyword = req.get_param_value("keyword");

            std::vector<User> users;
            int total = 0;
            if (model_.GetUsers(page, page_size, keyword, &users, &total)) {
                Json::Value root;
                root["status"] = 0;
                root["total"] = total;
                root["page"] = page;
                root["page_size"] = page_size;
                
                Json::Value list(Json::arrayValue);
                for (const auto &u : users) {
                    Json::Value item;
                    item["id"] = u.id;
                    item["username"] = u.username;
                    item["email"] = u.email;
                    item["nickname"] = u.nickname;
                    item["phone"] = u.phone;
                    item["role"] = u.role;
                    item["status"] = u.status;
                    item["created_at"] = u.created_at;
                    list.append(item);
                }
                root["data"] = list;
                *json_out = SerializeJson(root);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                *json_out = SerializeJson(res);
                return false;
            }
        }
