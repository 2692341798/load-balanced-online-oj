#pragma once

        void LogAdminOp(const std::string &user_id, const std::string &action, const std::string &target, const std::string &details, const Request &req) {
             OperationLog log;
             log.user_id = user_id;
             log.action = action;
             log.target = target;
             log.details = details;
             
             if (req.has_header("X-Real-IP")) {
                 log.ip = req.get_header_value("X-Real-IP");
             } else if (req.has_header("X-Forwarded-For")) {
                 log.ip = req.get_header_value("X-Forwarded-For");
             } else {
                 log.ip = req.remote_addr;
             }
             model_.LogOperation(log);
        }

        bool GetLogs(const Request &req, std::string *json_out) {
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

            std::vector<OperationLog> logs;
            int total = 0;
            if (model_.GetLogs(page, page_size, keyword, &logs, &total)) {
                Json::Value root;
                root["status"] = 0;
                root["total"] = total;
                root["page"] = page;
                root["page_size"] = page_size;
                
                Json::Value list(Json::arrayValue);
                for (const auto &l : logs) {
                    Json::Value item;
                    item["id"] = l.id;
                    item["user_id"] = l.user_id;
                    item["username"] = l.username;
                    item["action"] = l.action;
                    item["target"] = l.target;
                    item["details"] = l.details;
                    item["ip"] = l.ip;
                    item["created_at"] = l.created_at;
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
