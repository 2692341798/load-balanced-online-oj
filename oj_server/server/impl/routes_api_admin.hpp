#pragma once

    svr.Post("/api/admin/register", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.AdminRegister(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Post("/api/admin/invitation_code/generate", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.GenerateInvitationCode(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Get("/api/admin/logs", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.GetLogs(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Get("/api/admin/stats", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.GetDashboardStats(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Get("/api/admin/questions", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.AllQuestionsAdmin(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Get(R"(/api/admin/question/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string number = req.matches[1];
        std::string json;
        ctrl.GetOneQuestionAdmin(number, req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Post("/api/admin/question", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.AddQuestion(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Post(R"(/api/admin/question/update/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string number = req.matches[1];
        std::string json;
        ctrl.UpdateQuestion(number, req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Post(R"(/api/admin/question/delete/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string number = req.matches[1];
        std::string json;
        ctrl.DeleteQuestion(number, req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Get("/api/admin/users", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.GetUsers(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Post("/api/admin/user/status", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.UpdateUserStatus(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Post("/api/admin/user/role", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.UpdateUserRole(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Post("/api/admin/user/reset_password", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.ResetUserPassword(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });
