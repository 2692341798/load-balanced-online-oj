        void SearchSubmissions(const Request &req, std::string *json_out)
        {
            std::string user_id = req.get_param_value("user_id");
            std::string question_id = req.get_param_value("question_id");
            std::string status = req.get_param_value("status");
            std::string start_time = req.get_param_value("start_time");
            std::string end_time = req.get_param_value("end_time");
            std::string keyword = req.get_param_value("keyword");
            
            int page = 1;
            int page_size = 20;
            if (req.has_param("page")) page = std::stoi(req.get_param_value("page"));
            if (req.has_param("page_size")) page_size = std::stoi(req.get_param_value("page_size"));
            if (page < 1) page = 1;
            if (page_size < 1) page_size = 20;
            if (page_size > 100) page_size = 100;
            
            int offset = (page - 1) * page_size;
            
            std::vector<Submission> submissions;
            int total = 0;
            
            if (model_.GetSubmissions(user_id, question_id, status, start_time, end_time, keyword, offset, page_size, &submissions, &total)) {
                Json::Value root;
                root["status"] = 0;
                root["total"] = total;
                root["page"] = page;
                root["page_size"] = page_size;
                
                Json::Value list(Json::arrayValue);
                for (const auto &s : submissions) {
                    Json::Value item;
                    item["id"] = s.id;
                    item["user_id"] = s.user_id;
                    item["question_id"] = s.question_id;
                    item["question_title"] = s.question_title;
                    item["result"] = s.result;
                    item["cpu_time"] = s.cpu_time;
                    item["mem_usage"] = s.mem_usage;
                    item["created_at"] = s.created_at;
                    item["content"] = s.content; 
                    list.append(item);
                }
                root["data"] = list;
                
                *json_out = SerializeJson(root);
            } else {
                Json::Value root;
                root["status"] = 1;
                root["reason"] = "Database Error";
                
                *json_out = SerializeJson(root);
            }
        }

