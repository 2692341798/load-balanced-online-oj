#pragma once

        bool AllQuestionsAdmin(const Request &req, string *json)
        {
            User user;
            if (!AdminAuthCheck(req, &user)) {
                Json::Value root;
                root["status"] = 403;
                root["reason"] = "Permission Denied";
                
                *json = SerializeJson(root);
                return false;
            }

            int page = 1;
            int page_size = 20;
            if (req.has_param("page")) {
                try {
                    page = std::stoi(req.get_param_value("page"));
                } catch (...) { page = 1; }
            }
            if (req.has_param("page_size")) {
                try {
                    page_size = std::stoi(req.get_param_value("page_size"));
                } catch (...) { page_size = 20; }
            }
            if (page < 1) page = 1;
            if (page_size < 1) page_size = 20;

            vector<struct Question> all;
            int total = 0;
            if (model_.GetAllQuestionsAdmin(page, page_size, &all, &total))
            {
                Json::Value root;
                root["status"] = 0;
                root["total"] = total;
                root["page"] = page;
                root["page_size"] = page_size;

                Json::Value list(Json::arrayValue);
                for (const auto &q : all) {
                    Json::Value item;
                    item["number"] = q.number;
                    item["title"] = q.title;
                    item["star"] = q.star;
                    item["cpu_limit"] = q.cpu_limit;
                    item["mem_limit"] = q.mem_limit;
                    item["description"] = q.desc;
                    item["tail"] = q.tail;
                    item["status"] = q.status;
                    list.append(item);
                }
                root["data"] = list;
                
                *json = SerializeJson(root);
                return true;
            }
            else
            {
                Json::Value root;
                root["status"] = 1;
                root["reason"] = "Database Error";
                
                *json = SerializeJson(root);
                return false;
            }
        }

        bool GetOneQuestionAdmin(const string &number, const Request &req, string *json_out)
        {
            User user;
            if (!AdminAuthCheck(req, &user)) {
                Json::Value root;
                root["status"] = 403;
                root["reason"] = "Permission Denied";
                
                *json_out = SerializeJson(root);
                return false;
            }

            struct Question q;
            if (model_.GetOneQuestion(number, &q))
            {
                Json::Value root;
                root["status"] = 0;
                Json::Value item;
                item["number"] = q.number;
                item["title"] = q.title;
                item["star"] = q.star;
                item["cpu_limit"] = q.cpu_limit;
                item["mem_limit"] = q.mem_limit;
                item["description"] = q.desc;
                item["tail"] = q.tail;
                item["status"] = q.status;
                root["data"] = item;
                
                *json_out = SerializeJson(root);
                return true;
            }
            else
            {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Not Found";
                
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool AddQuestion(const Request &req, string *json)
        {
            User user;
            if (!AdminAuthCheck(req, &user)) {
                Json::Value root;
                root["status"] = 403;
                root["reason"] = "Permission Denied";
                
                *json = SerializeJson(root);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            
            struct Question q;
            q.title = root["title"].asString();
            q.star = root["star"].asString();
            q.desc = root["description"].asString();
            q.tail = root["tail"].asString();
            q.cpu_limit = root.get("cpu_limit", 1).asInt();
            q.mem_limit = root.get("mem_limit", 30000).asInt();
            q.status = root.get("status", 1).asInt();

            if (model_.AddQuestion(q)) {
                 LogAdminOp(user.id, "Add Question", "Question " + q.title, "Added new question", req);
                 Json::Value res;
                 res["status"] = 0;
                 res["reason"] = "Success";
                 
                 *json = SerializeJson(res);
                 return true;
            }
            
            Json::Value res;
            res["status"] = 1;
            res["reason"] = "Database Error";
            
            *json = SerializeJson(res);
            return false;
        }

        bool UpdateQuestion(const std::string &number, const Request &req, string *json)
        {
            User user;
            if (!AdminAuthCheck(req, &user)) {
                Json::Value root;
                root["status"] = 403;
                root["reason"] = "Permission Denied";
                
                *json = SerializeJson(root);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            
            struct Question q;
            q.number = number;
            q.title = root["title"].asString();
            q.star = root["star"].asString();
            q.desc = root["description"].asString();
            q.tail = root["tail"].asString();
            q.cpu_limit = root.get("cpu_limit", 1).asInt();
            q.mem_limit = root.get("mem_limit", 30000).asInt();
            q.status = root.get("status", 1).asInt();

            if (model_.UpdateQuestion(q)) {
                 LogAdminOp(user.id, "Update Question", "Question " + number, "Updated question " + number, req);
                 Json::Value res;
                 res["status"] = 0;
                 res["reason"] = "Success";
                 
                 *json = SerializeJson(res);
                 return true;
            }
            
            Json::Value res;
            res["status"] = 1;
            res["reason"] = "Database Error";
            
            *json = SerializeJson(res);
            return false;
        }

        bool DeleteQuestion(const std::string &number, const Request &req, string *json)
        {
            User user;
            if (!AdminAuthCheck(req, &user)) {
                Json::Value root;
                root["status"] = 403;
                root["reason"] = "Permission Denied";
                
                *json = SerializeJson(root);
                return false;
            }

            if (model_.DeleteQuestion(number)) {
                 LogAdminOp(user.id, "Delete Question", "Question " + number, "Deleted question " + number, req);
                 Json::Value res;
                 res["status"] = 0;
                 res["reason"] = "Success";
                 
                 *json = SerializeJson(res);
                 return true;
            }
            
            Json::Value res;
            res["status"] = 1;
            res["reason"] = "Database Error";
            
            *json = SerializeJson(res);
            return false;
        }
