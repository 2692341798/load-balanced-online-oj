        bool Home(const Request &req, string *html)
        {
             User user;
             AuthCheck(req, &user);
             view_.HomeHtml(html, &user);
             return true;
        }

        bool AllQuestions(const Request &req, string *html)
        {
            User user;
            AuthCheck(req, &user);

            int page = 1;
            int page_size = 40;
            if (req.has_param("page")) {
                try {
                    page = std::stoi(req.get_param_value("page"));
                    if (page < 1) page = 1;
                } catch (...) {
                    page = 1;
                }
            }

            bool ret = true;
            vector<struct Question> all;
            int total_count = 0;

            if (model_.GetQuestionsByPage(page, page_size, &all, &total_count))
            {
                int total_pages = (total_count + page_size - 1) / page_size;
                if (total_pages < 1) total_pages = 1;
                view_.AllExpandHtml(all, html, total_pages, page, &user);
            }
            else
            {
                *html = "获取题目失败, 形成题目列表失败";
                ret = false;
            }
            return ret;
        }

        bool Question(const string &number, const Request &req, string *html)
        {
            User user;
            AuthCheck(req, &user);

            bool ret = true;
            struct Question q;
            if (model_.GetOneQuestion(number, &q))
            {
                if (q.status == 0 && user.role != 1) {
                    *html = "指定题目: " + number + " 未发布!";
                    return false;
                }
                view_.OneExpandHtml(q, html, &user);
            }
            else
            {
                *html = "指定题目: " + number + " 不存在!";
                ret = false;
            }
            return ret;
        }

        bool GetQuestionJson(const string &number, string *json_out)
        {
            struct Question q;
            if (model_.GetOneQuestion(number, &q))
            {
                if (q.status == 0) {
                     Json::Value res;
                     res["status"] = 1;
                     res["reason"] = "Question hidden";
                     
                     *json_out = SerializeJson(res);
                     return false;
                }

                Json::Value root;
                root["status"] = 0;
                Json::Value data;
                data["number"] = q.number;
                data["title"] = q.title;
                data["star"] = q.star;
                data["cpu_limit"] = q.cpu_limit;
                data["mem_limit"] = q.mem_limit;
                root["data"] = data;
                
                
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

        bool DiscussionPage(const Request &req, string *html)
        {
            User user;
            AuthCheck(req, &user);
            view_.DiscussionHtml(html, &user);
            return true;
        }

        bool Contest(const Request &req, string *html)
        {
            User user;
            AuthCheck(req, &user);

            std::vector<ns_model::Contest> contests;
            view_.ContestHtml(contests, html, &user);
            return true;
        }

        bool GetProfile(const User &user, std::string *html)
        {
            std::unordered_map<std::string, int> stats;
            model_.GetUserSolvedStats(user.id, &stats);
            view_.ProfileExpandHtml(user, stats, html);
            return true;
        }

        bool GetProfileData(const User &user, std::string *json_out)
        {
            std::unordered_map<std::string, int> stats;
            if (!model_.GetUserSolvedStats(user.id, &stats)) {
                return false;
            }

            Json::Value root;
            root["status"] = 0;
            root["reason"] = "success";
            root["username"] = user.username;
            root["email"] = user.email;
            root["nickname"] = user.nickname;
            root["phone"] = user.phone;
            root["avatar"] = user.avatar;
            root["role"] = user.role;
            root["created_at"] = user.created_at;

            Json::Value stats_json;
            for (const auto &kv : stats) {
                stats_json[kv.first] = kv.second;
            }
            root["stats"] = stats_json;

            *json_out = SerializeJson(root);
            return true;
        }

        bool UpdateUserInfo(const std::string &user_id, const std::string &nickname, const std::string &email, const std::string &phone)
        {
            User user;
            user.id = user_id;
            user.nickname = nickname;
            user.email = email;
            user.phone = phone;

            if (!model_.UpdateUser(user)) {
                return false;
            }

            User refreshed;
            if (model_.GetUserById(user_id, &refreshed)) {
                std::unique_lock<std::mutex> lock(session_mtx_);
                for (auto &kv : sessions_) {
                    if (kv.second.user.id == user_id) {
                        kv.second.user = refreshed;
                    }
                }
            }

            return true;
        }
