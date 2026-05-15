#pragma once

    svr.Post("/api/inline_comment/add", [&ctrl](const Request &req, Response &resp){
        User user;
        Json::Value res_json;
        if (!ctrl.AuthCheck(req, &user)) {
             res_json["status"] = 401;
             res_json["reason"] = "请先登录";
             
             resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
             return;
        }

        Json::Reader reader;
        Json::Value root;
        reader.parse(req.body, root);
        std::string post_id = root["post_id"].asString();
        std::string content = root["content"].asString();
        std::string selected_text = root["selected_text"].asString();
        std::string parent_id = root.isMember("parent_id") ? root["parent_id"].asString() : "0";

        std::string json_out;
        ctrl.AddInlineComment(user.id, post_id, content, selected_text, parent_id, &json_out);
        resp.set_content(json_out, "application/json;charset=utf-8");
    });

    svr.Post("/api/inline_comment/delete", [&ctrl](const Request &req, Response &resp){
        User user;
        Json::Value res_json;
        if (!ctrl.AuthCheck(req, &user)) {
             res_json["status"] = 401;
             res_json["reason"] = "请先登录";
             
             resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
             return;
        }

        Json::Reader reader;
        Json::Value root;
        reader.parse(req.body, root);
        std::string comment_id = root["comment_id"].asString();

        std::string json_out;
        ctrl.DeleteInlineComment(comment_id, user.id, user.role, &json_out);
        resp.set_content(json_out, "application/json;charset=utf-8");
    });

    svr.Get(R"(/api/inline_comments/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string post_id = req.matches[1];
        std::string json_out;
        ctrl.GetInlineComments(post_id, &json_out);
        resp.set_content(json_out, "application/json;charset=utf-8");
    });

    svr.Get("/api/discussions", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.GetAllDiscussions(&json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Get(R"(/api/discussion/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string id = req.matches[1];
        std::string json;
        ctrl.GetDiscussion(id, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Post("/api/discussion", [&ctrl](const Request &req, Response &resp){
        User user;
        Json::Value res_json;
        if (!ctrl.AuthCheck(req, &user)) {
             res_json["status"] = 401;
             res_json["reason"] = "Unauthorized";
             
             resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
             return;
        }
        
        Json::Reader reader;
        Json::Value root;
        reader.parse(req.body, root);
        std::string title = root["title"].asString();
        std::string content = root["content"].asString();
        std::string question_id = root.isMember("question_id") ? root["question_id"].asString() : "0";
        
        std::string json;
        ctrl.AddDiscussion(user.id, title, content, question_id, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });
    
    svr.Post("/api/discussion/delete", [&ctrl](const Request &req, Response &resp){
        User user;
        Json::Value res_json;
        if (!ctrl.AuthCheck(req, &user)) {
             res_json["status"] = 401;
             res_json["reason"] = "Unauthorized";
             
             resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
             return;
        }
        
        Json::Reader reader;
        Json::Value root;
        reader.parse(req.body, root);
        std::string discussion_id = root["discussion_id"].asString();
        
        std::string json;
        ctrl.DeleteDiscussion(discussion_id, user.id, user.role, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });
    
    svr.Get(R"(/api/discussions/question/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string qid = req.matches[1];
        std::string json;
        ctrl.GetDiscussionsByQuestionId(qid, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Get(R"(/api/article_comments/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string id = req.matches[1];
        std::string json;
        ctrl.GetArticleComments(id, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Post("/api/article_comment/add", [&ctrl](const Request &req, Response &resp){
        User user;
        Json::Value res_json;
        if (!ctrl.AuthCheck(req, &user)) {
             res_json["status"] = 401;
             res_json["reason"] = "Unauthorized";
             
             resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
             return;
        }
        
        Json::Reader reader;
        Json::Value root;
        reader.parse(req.body, root);
        std::string post_id = root["post_id"].asString();
        std::string content = root["content"].asString();
        
        std::string json;
        ctrl.AddArticleComment(user.id, post_id, content, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });
