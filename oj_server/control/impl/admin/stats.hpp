#pragma once

        bool GetDashboardStats(const Request &req, std::string *json_out) {
            User user;
            if (!AdminAuthCheck(req, &user)) {
                Json::Value res;
                res["status"] = 403;
                res["reason"] = "Permission Denied";
                *json_out = SerializeJson(res);
                return false;
            }

            std::map<std::string, int> user_growth;
            std::map<std::string, int> submission_stats;
            std::map<std::string, int> daily_activity;
            int total_users = 0;
            int total_problems = 0;
            int total_submissions = 0;

            model_.GetUserGrowthStats(30, &user_growth);
            model_.GetSubmissionStats(&submission_stats);
            model_.GetDailyActivityStats(30, &daily_activity);
            model_.GetTotalUserCount(&total_users);
            model_.GetTotalProblemCount(&total_problems);
            model_.GetTotalSubmissionCount(&total_submissions);

            Json::Value root;
            root["status"] = 0;
            
            Json::Value data;

            Json::Value ug_json;
            for(auto const& kv : user_growth) {
                ug_json[kv.first] = kv.second;
            }
            data["user_growth"] = ug_json;

            Json::Value ss_json;
            for(auto const& kv : submission_stats) {
                if (kv.first == "0") ss_json["通过(AC)"] = kv.second;
                else if (kv.first == "-1") ss_json["未通过(WA)"] = kv.second;
                else ss_json["未知"] = ss_json.get("未知", 0).asInt() + kv.second;
            }
            data["submission_stats"] = ss_json;

            Json::Value da_json;
            for(auto const& kv : daily_activity) {
                da_json[kv.first] = kv.second;
            }
            data["daily_activity"] = da_json;
            
            data["total_users"] = total_users;
            data["total_problems"] = total_problems;
            data["total_submissions"] = total_submissions;

            root["data"] = data;

            *json_out = SerializeJson(root);
            return true;
        }
