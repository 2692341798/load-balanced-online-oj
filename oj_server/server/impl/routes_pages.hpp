#pragma once

    svr.Get("/", [&ctrl](const Request &req, Response &resp){
        SetNoCache(resp);
        std::string html;
        ctrl.Home(req, &html);
        resp.set_content(html, "text/html; charset=utf-8");
    });

    svr.Get("/all_questions", [&ctrl](const Request &req, Response &resp){
        SetNoCache(resp);
        User user;
        if (!ctrl.AuthCheck(req, &user)) {
            resp.set_redirect("/login");
            return;
        }

        std::string html;
        ctrl.AllQuestions(req, &html);
        resp.set_content(html, "text/html; charset=utf-8");
    });

    svr.Get("/discussion", [&ctrl](const Request &req, Response &resp){
        SetNoCache(resp);
        User user;
        if (!ctrl.AuthCheck(req, &user)) {
            resp.set_redirect("/login");
            return;
        }
        
        std::string html;
        ctrl.DiscussionPage(req, &html);
        resp.set_content(html, "text/html; charset=utf-8");
    });

    svr.Get("/contest", [&ctrl](const Request &req, Response &resp){
        SetNoCache(resp);
        std::string html;
        ctrl.Contest(req, &html);
        resp.set_content(html, "text/html; charset=utf-8");
    });

    svr.Get("/training", [&ctrl](const Request &req, Response &resp){
        SetNoCache(resp);
        std::string html;
        if (ctrl.TrainingListPage(req, &html)) {
            resp.set_content(html, "text/html; charset=utf-8");
        } else {
            resp.set_content(html, "text/plain; charset=utf-8");
        }
    });

    svr.Get(R"(/training/(\d+))", [&ctrl](const Request &req, Response &resp){
        SetNoCache(resp);
        std::string id = req.matches[1];
        std::string html;
        if (ctrl.TrainingDetail(id, req, &html)) {
            resp.set_content(html, "text/html; charset=utf-8");
        } else {
            resp.set_content(html, "text/plain; charset=utf-8");
        }
    });

    svr.Get(R"(/question/(\d+))", [&ctrl](const Request &req, Response &resp){
        SetNoCache(resp);
        User user;
        if (!ctrl.AuthCheck(req, &user)) {
            resp.set_redirect("/login");
            return;
        }

        std::string number = req.matches[1];
        std::string html;
        ctrl.Question(number, req, &html);
        resp.set_content(html, "text/html; charset=utf-8");
    });

    svr.Get("/login", [&ctrl](const Request &req, Response &resp){
        SetNoCache(resp);
        std::string html;
        if (ctrl.LoginPage(req, &html)) {
            resp.set_content(html, "text/html; charset=utf-8");
        } else {
            resp.set_content("Login Page Not Found", "text/plain");
        }
    });

    svr.Get("/profile", [&ctrl](const Request &req, Response &resp){
        SetNoCache(resp);
        User user;
        if (!ctrl.AuthCheck(req, &user)) {
            resp.set_redirect("/login");
            return;
        }

        std::string html;
        if (ctrl.GetProfile(user, &html)) {
            resp.set_content(html, "text/html; charset=utf-8");
        } else {
            resp.set_content("获取个人中心失败", "text/plain; charset=utf-8");
        }
    });

    svr.Get("/admin", [](const Request &req, Response &resp){
        resp.set_redirect("/admin/index.html");
    });
