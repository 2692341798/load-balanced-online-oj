#pragma once

    svr.Get("/api/contests", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.GetContestList(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Get(R"(/api/question/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string number = req.matches[1];
        std::string json;
        ctrl.GetQuestionJson(number, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Post(R"(/judge/(\d+))", [&ctrl](const Request &req, Response &resp){
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
