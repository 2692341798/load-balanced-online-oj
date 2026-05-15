#pragma once

    svr.Post("/api/training/create", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.CreateTrainingList(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Post("/api/training/edit", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.UpdateTrainingList(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Post("/api/training/delete", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.DeleteTrainingList(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Post("/api/training/add_problem", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.AddProblemToTrainingList(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Post("/api/training/remove_problem", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.RemoveProblemFromTrainingList(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Post("/api/training/reorder", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.ReorderTrainingListProblems(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Get("/api/problems", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.GetQuestionsByPageJson(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Post("/api/training/add_problems", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.AddProblemsToTrainingList(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Get("/api/training/list", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.GetTrainingLists(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Get(R"(/api/training/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string id = req.matches[1];
        std::string json;
        ctrl.GetTrainingListDetailJson(id, req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });
