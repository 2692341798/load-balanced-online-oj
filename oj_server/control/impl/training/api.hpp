#pragma once

        static std::string GetTrainingListIdCompat(const Json::Value &root)
        {
            std::string id = root.get("list_id", "").asString();
            if (id.empty()) id = root.get("training_list_id", "").asString();
            return id;
        }

        static Json::Value GetTrainingProblemIdsCompat(const Json::Value &root)
        {
            if (root.isMember("problems") && root["problems"].isArray()) return root["problems"];
            if (root.isMember("question_ids") && root["question_ids"].isArray()) return root["question_ids"];
            if (root.isMember("problem_ids") && root["problem_ids"].isArray()) return root["problem_ids"];
            return Json::Value(Json::arrayValue);
        }

        bool CreateTrainingList(const Request &req, std::string *json_out)
        {
            User user;
            if (!AuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 401;
                res["reason"] = "Unauthorized";
                *json_out = SerializeJson(res);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);

            TrainingList list;
            list.title = root["title"].asString();
            list.description = root.get("description", "").asString();
            list.difficulty = root.get("difficulty", "Unrated").asString();
            list.tags = root.get("tags", "[]").asString();
            list.visibility = root.get("visibility", "public").asString();
            list.author_id = user.id;

            int new_id = 0;
            if (model_.CreateTrainingList(list, &new_id)) {
                Json::Value res;
                res["status"] = 0;
                res["id"] = new_id;
                *json_out = SerializeJson(res);
                return true;
            }

            Json::Value res;
            res["status"] = 1;
            res["reason"] = "Database Error";
            *json_out = SerializeJson(res);
            return false;
        }

        bool UpdateTrainingList(const Request &req, std::string *json_out)
        {
            User user;
            if (!AuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 401;
                res["reason"] = "Unauthorized";
                *json_out = SerializeJson(res);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);

            TrainingList list;
            list.id = root["id"].asString();
            list.title = root["title"].asString();
            list.description = root.get("description", "").asString();
            list.difficulty = root.get("difficulty", "Unrated").asString();
            list.tags = root.get("tags", "[]").asString();
            list.visibility = root.get("visibility", "public").asString();

            TrainingList existing;
            if (!model_.GetTrainingList(list.id, &existing)) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Not Found";
                *json_out = SerializeJson(res);
                return false;
            }
            if (existing.author_id != user.id && user.role != 1) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            if (model_.UpdateTrainingList(list)) {
                Json::Value res;
                res["status"] = 0;
                res["reason"] = "Success";
                *json_out = SerializeJson(res);
                return true;
            }

            Json::Value res;
            res["status"] = 1;
            res["reason"] = "Database Error";
            *json_out = SerializeJson(res);
            return false;
        }

        bool DeleteTrainingList(const Request &req, std::string *json_out)
        {
            User user;
            if (!AuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 401;
                res["reason"] = "Unauthorized";
                *json_out = SerializeJson(res);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            std::string id = root["id"].asString();

            TrainingList existing;
            if (!model_.GetTrainingList(id, &existing)) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Not Found";
                *json_out = SerializeJson(res);
                return false;
            }
            if (existing.author_id != user.id && user.role != 1) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            if (model_.DeleteTrainingList(id)) {
                Json::Value res;
                res["status"] = 0;
                res["reason"] = "Success";
                *json_out = SerializeJson(res);
                return true;
            }

            Json::Value res;
            res["status"] = 1;
            res["reason"] = "Database Error";
            *json_out = SerializeJson(res);
            return false;
        }

        bool GetQuestionsByPageJson(const Request &req, std::string *json_out)
        {
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
            int total = 0;

            vector<struct Question> questions;
            if (model_.GetQuestionsByPage(page, page_size, &questions, &total)) {
                Json::Value root;
                root["status"] = 0;
                root["total"] = total;
                root["page"] = page;
                root["page_size"] = page_size;
                
                Json::Value list(Json::arrayValue);
                for (const auto &q : questions) {
                    Json::Value item;
                    item["number"] = q.number;
                    item["title"] = q.title;
                    item["star"] = q.star;
                    item["status"] = q.status;
                    list.append(item);
                }
                root["data"] = list;
                *json_out = SerializeJson(root);
                return true;
            }

            Json::Value res;
            res["status"] = 1;
            res["reason"] = "Database Error";
            *json_out = SerializeJson(res);
            return false;
        }

        bool AddProblemsToTrainingList(const Request &req, std::string *json_out)
        {
            User user;
            if (!AuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 401;
                res["reason"] = "Unauthorized";
                *json_out = SerializeJson(res);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            std::string list_id = GetTrainingListIdCompat(root);
            Json::Value problems = GetTrainingProblemIdsCompat(root);

            TrainingList existing;
            if (!model_.GetTrainingList(list_id, &existing)) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Not Found";
                *json_out = SerializeJson(res);
                return false;
            }
            if (existing.author_id != user.id && user.role != 1) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            int success = 0;
            for (const auto &p : problems) {
                std::string question_id = p.asString();
                if (model_.AddProblemToTrainingList(list_id, question_id)) {
                    success++;
                }
            }

            Json::Value res;
            res["status"] = 0;
            res["success_count"] = success;
            *json_out = SerializeJson(res);
            return true;
        }

        bool AddProblemToTrainingList(const Request &req, std::string *json_out)
        {
            User user;
            if (!AuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 401;
                res["reason"] = "Unauthorized";
                *json_out = SerializeJson(res);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            std::string list_id = GetTrainingListIdCompat(root);
            std::string question_id = root["question_id"].asString();

            TrainingList existing;
            if (!model_.GetTrainingList(list_id, &existing)) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Not Found";
                *json_out = SerializeJson(res);
                return false;
            }
            if (existing.author_id != user.id && user.role != 1) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            if (model_.AddProblemToTrainingList(list_id, question_id)) {
                Json::Value res;
                res["status"] = 0;
                res["reason"] = "Success";
                *json_out = SerializeJson(res);
                return true;
            }

            Json::Value res;
            res["status"] = 1;
            res["reason"] = "Database Error";
            *json_out = SerializeJson(res);
            return false;
        }

        bool RemoveProblemFromTrainingList(const Request &req, std::string *json_out)
        {
            User user;
            if (!AuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 401;
                res["reason"] = "Unauthorized";
                *json_out = SerializeJson(res);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            std::string list_id = GetTrainingListIdCompat(root);
            std::string question_id = root["question_id"].asString();

            TrainingList existing;
            if (!model_.GetTrainingList(list_id, &existing)) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Not Found";
                *json_out = SerializeJson(res);
                return false;
            }
            if (existing.author_id != user.id && user.role != 1) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            if (model_.RemoveProblemFromTrainingList(list_id, question_id)) {
                Json::Value res;
                res["status"] = 0;
                res["reason"] = "Success";
                *json_out = SerializeJson(res);
                return true;
            }

            Json::Value res;
            res["status"] = 1;
            res["reason"] = "Database Error";
            *json_out = SerializeJson(res);
            return false;
        }

        bool ReorderTrainingListProblems(const Request &req, std::string *json_out)
        {
            User user;
            if (!AuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 401;
                res["reason"] = "Unauthorized";
                *json_out = SerializeJson(res);
                return false;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            std::string list_id = GetTrainingListIdCompat(root);
            Json::Value problems = GetTrainingProblemIdsCompat(root);

            TrainingList existing;
            if (!model_.GetTrainingList(list_id, &existing)) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Not Found";
                *json_out = SerializeJson(res);
                return false;
            }
            if (existing.author_id != user.id && user.role != 1) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            std::vector<std::string> ids;
            for (const auto &p : problems) {
                ids.push_back(p.asString());
            }
            if (model_.ReorderTrainingListProblems(list_id, ids)) {
                Json::Value res;
                res["status"] = 0;
                res["reason"] = "Success";
                *json_out = SerializeJson(res);
                return true;
            }

            Json::Value res;
            res["status"] = 1;
            res["reason"] = "Database Error";
            *json_out = SerializeJson(res);
            return false;
        }

        bool GetTrainingLists(const Request &req, std::string *json_out)
        {
            int page = 1;
            int page_size = 10;
            std::string visibility = req.get_param_value("visibility");
            std::string author_id = req.get_param_value("author_id");
            if (req.has_param("page")) page = std::stoi(req.get_param_value("page"));
            if (req.has_param("page_size")) page_size = std::stoi(req.get_param_value("page_size"));

            std::vector<TrainingList> lists;
            int total = 0;
            if (model_.GetTrainingLists(page, page_size, visibility, author_id, &lists, &total)) {
                Json::Value root;
                root["status"] = 0;
                root["total"] = total;
                root["page"] = page;
                root["page_size"] = page_size;
                
                Json::Value arr(Json::arrayValue);
                for (const auto &l : lists) {
                    Json::Value item;
                    item["id"] = l.id;
                    item["title"] = l.title;
                    item["description"] = l.description;
                    item["difficulty"] = l.difficulty;
                    item["tags"] = l.tags;
                    item["author_id"] = l.author_id;
                    item["author_name"] = l.author_name;
                    item["author_avatar"] = l.author_avatar;
                    item["visibility"] = l.visibility;
                    item["created_at"] = l.created_at;
                    item["updated_at"] = l.updated_at;
                    item["likes"] = l.likes;
                    item["collections"] = l.collections;
                    item["problem_count"] = l.problem_count;
                    arr.append(item);
                }
                root["data"] = arr;
                *json_out = SerializeJson(root);
                return true;
            }

            Json::Value res;
            res["status"] = 1;
            res["reason"] = "Database Error";
            *json_out = SerializeJson(res);
            return false;
        }

        bool GetTrainingListDetailJson(const std::string &id, const Request &req, std::string *json_out)
        {
            User user;
            AuthCheck(req, &user);
            
            TrainingList list;
            if (!model_.GetTrainingList(id, &list)) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Not Found";
                *json_out = SerializeJson(res);
                return false;
            }

            if (list.visibility == "private") {
                if (user.id != list.author_id) {
                    Json::Value res;
                    res["status"] = 403;
                    res["reason"] = "Permission Denied";
                    *json_out = SerializeJson(res);
                    return false;
                }
            }

            std::vector<TrainingListItem> items;
            model_.GetTrainingListProblems(id, user.id, &items);

            Json::Value root;
            root["status"] = 0;

            Json::Value info;
            info["id"] = list.id;
            info["title"] = list.title;
            info["description"] = list.description;
            info["difficulty"] = list.difficulty;
            info["tags"] = list.tags;
            info["author_id"] = list.author_id;
            info["author_name"] = list.author_name;
            info["author_avatar"] = list.author_avatar;
            info["visibility"] = list.visibility;
            info["created_at"] = list.created_at;
            info["updated_at"] = list.updated_at;
            info["likes"] = list.likes;
            info["collections"] = list.collections;
            info["problem_count"] = list.problem_count;
            root["info"] = info;

            Json::Value arr(Json::arrayValue);
            for (const auto &it : items) {
                Json::Value item;
                item["id"] = it.id;
                item["training_list_id"] = it.training_list_id;
                item["question_id"] = it.question_id;
                item["order_index"] = it.order_index;
                item["question_title"] = it.question_title;
                item["question_difficulty"] = it.question_difficulty;
                item["user_status"] = it.user_status;
                arr.append(item);
            }
            root["items"] = arr;

            *json_out = SerializeJson(root);
            return true;
        }
