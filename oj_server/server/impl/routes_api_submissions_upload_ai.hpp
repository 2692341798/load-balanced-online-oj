#pragma once

    svr.Get("/api/submissions", [&ctrl](const Request &req, Response &resp){
        User user;
        Json::Value res_json;
        if (!ctrl.AuthCheck(req, &user)) {
             res_json["status"] = 401;
             res_json["reason"] = "Unauthorized";
             
             resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
             return;
        }
        
        std::string json_out;
        ctrl.SearchSubmissions(req, &json_out);
        resp.set_content(json_out, "application/json;charset=utf-8");
    });

    svr.Post("/api/upload_image", [&ctrl](const Request &req, Response &resp){
        User user;
        Json::Value res_json;
        if (!ctrl.AuthCheck(req, &user)) {
             res_json["status"] = 401;
             res_json["reason"] = "Unauthorized";
             
             resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
             return;
        }
        
        std::string json;
        ctrl.UploadImage(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Post("/api/upload_avatar", [&ctrl](const Request &req, Response &resp){
        User user;
        Json::Value res_json;
        if (!ctrl.AuthCheck(req, &user)) {
             res_json["status"] = 401;
             res_json["reason"] = "Unauthorized";
             
             resp.set_content(SerializeJson(res_json), "application/json;charset=utf-8");
             return;
        }
        
        std::string json;
        ctrl.UploadAvatar(req, user.id, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });

    svr.Post("/api/generate_hint", [&ctrl](const Request &req, Response &resp){
        std::string json;
        ctrl.GenerateAiHint(req, &json);
        resp.set_content(json, "application/json;charset=utf-8");
    });
