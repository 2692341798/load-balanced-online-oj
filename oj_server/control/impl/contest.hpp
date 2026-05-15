        bool GetContestList(const Request &req, std::string *json_out)
        {
            int page = 1;
            int page_size = 5;
            if (req.has_param("page")) page = std::stoi(req.get_param_value("page"));
            if (req.has_param("size")) page_size = std::stoi(req.get_param_value("size"));
            std::string status = req.get_param_value("status");

#ifdef ENABLE_REDIS
            std::string cache_key = "contest:page:" + std::to_string(page) + ":size:" + std::to_string(page_size) + ":status:" + status;
            redisContext *c = redisConnect("127.0.0.1", 6379);
            if (c != NULL && c->err == 0) {
                 redisReply *reply = (redisReply*)redisCommand(c, "GET %s", cache_key.c_str());
                 if (reply != NULL && reply->type == REDIS_REPLY_STRING) {
                     *json_out = reply->str;
                     freeReplyObject(reply);
                     redisFree(c);
                     return true;
                 }
                 if(reply) freeReplyObject(reply);
            }
#endif

            std::vector<ns_model::Contest> contests;
            int total = 0;
            if (model_.GetContests(page, page_size, status, &contests, &total)) {
                 Json::Value root;
                 root["status"] = 0;
                 root["total"] = total;
                 root["total_pages"] = (total + page_size - 1) / page_size;
                 root["page"] = page;
                 
                 Json::Value list(Json::arrayValue);
                 for(const auto& cts : contests) {
                     Json::Value item;
                     item["name"] = cts.name;
                     item["start_time"] = cts.start_time;
                     item["end_time"] = cts.end_time;
                     item["status"] = cts.status;
                     item["link"] = cts.link;
                     item["source"] = cts.source;
                     list.append(item);
                 }
                 root["data"] = list;
                 
                 *json_out = SerializeJson(root);

#ifdef ENABLE_REDIS
                 if (c != NULL && c->err == 0) {
                     redisReply *reply = (redisReply*)redisCommand(c, "SETEX %s 300 %s", cache_key.c_str(), json_out->c_str());
                     if(reply) freeReplyObject(reply);
                     redisFree(c);
                 } else if (c) {
                     redisFree(c);
                 }
#endif

                 return true;
            } else {
#ifdef ENABLE_REDIS
                 if (c) redisFree(c);
#endif
                 Json::Value res;
                 res["status"] = 1;
                 res["reason"] = "Database Error";
                 
                 *json_out = SerializeJson(res);
                 return false;
            }
        }

