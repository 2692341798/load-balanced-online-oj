        bool AddInlineComment(const std::string &user_id, const std::string &post_id, const std::string &content, const std::string &selected_text, const std::string &parent_id, std::string *json_out)
        {
            InlineComment c;
            c.user_id = user_id;
            c.post_id = post_id;
            c.content = content;
            c.selected_text = selected_text;
            c.parent_id = parent_id;
            
            if (model_.AddInlineComment(c)) {
                Json::Value res;
                res["status"] = 0;
                res["reason"] = "Success";
                
                *json_out = SerializeJson(res);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool GetInlineComments(const std::string &post_id, std::string *json_out)
        {
            std::vector<InlineComment> comments;
            if (model_.GetInlineComments(post_id, &comments)) {
                Json::Value root;
                root["status"] = 0;
                Json::Value list(Json::arrayValue);
                for (const auto &c : comments) {
                    Json::Value item;
                    item["id"] = c.id;
                    item["user_id"] = c.user_id;
                    item["username"] = c.username;
                    item["avatar"] = c.user_avatar;
                    item["post_id"] = c.post_id;
                    item["content"] = c.content;
                    item["selected_text"] = c.selected_text;
                    item["parent_id"] = c.parent_id;
                    item["created_at"] = c.created_at;
                    list.append(item);
                }
                root["data"] = list;
                
                *json_out = SerializeJson(root);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool DeleteInlineComment(const std::string &comment_id, const std::string &user_id, int role, std::string *json_out)
        {
             if (model_.DeleteInlineComment(comment_id, user_id, role)) {
                 Json::Value res;
                 res["status"] = 0;
                 res["reason"] = "Success";
                 
                 *json_out = SerializeJson(res);
                 return true;
             } else {
                 Json::Value res;
                 res["status"] = 1;
                 res["reason"] = "Failed to delete (Permission Denied or Not Found)";
                 
                 *json_out = SerializeJson(res);
                 return false;
             }
        }

        bool GetAllDiscussions(std::string *json_out)
        {
            std::vector<Discussion> discussions;
            if (model_.GetAllDiscussions(&discussions)) {
                Json::Value root;
                root["status"] = 0;
                Json::Value list(Json::arrayValue);
                for (const auto &d : discussions) {
                    Json::Value item;
                    item["id"] = d.id;
                    item["title"] = d.title;
                    item["summary"] = StringUtil::GetSummaryFromMarkdown(d.content);
                    item["author"] = d.author_name;
                    item["avatar"] = d.author_avatar;
                    item["date"] = d.created_at;
                    item["likes"] = d.likes;
                    item["views"] = d.views;
                    item["comments"] = d.comments_count;
                    item["isOfficial"] = d.is_official;
                    item["question_id"] = d.question_id;
                    item["question_title"] = d.question_title;
                    list.append(item);
                }
                root["data"] = list;
                
                *json_out = SerializeJson(root);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool GetDiscussion(const std::string &id, std::string *json_out)
        {
            Discussion d;
            if (model_.GetOneDiscussion(id, &d)) {
                Json::Value root;
                root["status"] = 0;
                Json::Value item;
                item["id"] = d.id;
                item["title"] = d.title;
                item["content"] = d.content;
                item["author"] = d.author_name;
                item["avatar"] = d.author_avatar;
                item["date"] = d.created_at;
                item["likes"] = d.likes;
                item["views"] = d.views;
                item["comments"] = d.comments_count;
                item["isOfficial"] = d.is_official;
                root["data"] = item;
                
                *json_out = SerializeJson(root);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Not Found";
                
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool AddDiscussion(const std::string &user_id, const std::string &title, const std::string &content, const std::string &question_id, std::string *json_out)
        {
            Discussion d;
            d.title = title;
            d.content = content;
            d.author_id = user_id;
            d.is_official = false;
            d.question_id = question_id;
            
            if (model_.AddDiscussion(d)) {
                Json::Value res;
                res["status"] = 0;
                res["reason"] = "Success";
                
                *json_out = SerializeJson(res);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool DeleteDiscussion(const std::string &discussion_id, const std::string &user_id, int role, std::string *json_out)
        {
            if (model_.DeleteDiscussion(discussion_id, user_id, role)) {
                Json::Value res;
                res["status"] = 0;
                res["reason"] = "Success";
                
                *json_out = SerializeJson(res);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Failed to delete (Permission Denied or Not Found)";
                
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool GetDiscussionsByQuestionId(const std::string &qid, std::string *json_out)
        {
            std::vector<Discussion> posts;
            if (model_.GetDiscussionsByQuestionId(qid, &posts)) {
                Json::Value root;
                root["status"] = 0;
                Json::Value list(Json::arrayValue);
                for (const auto &d : posts) {
                    Json::Value item;
                    item["id"] = d.id;
                    item["title"] = d.title;
                    item["content"] = d.content;
                    item["author"] = d.author_name;
                    item["avatar"] = d.author_avatar;
                    item["date"] = d.created_at;
                    item["likes"] = d.likes;
                    item["views"] = d.views;
                    item["comments"] = d.comments_count;
                    item["isOfficial"] = d.is_official;
                    item["question_id"] = d.question_id;
                    item["question_title"] = d.question_title;
                    list.append(item);
                }
                root["data"] = list;
                
                *json_out = SerializeJson(root);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool AddArticleComment(const std::string &user_id, const std::string &post_id, const std::string &content, std::string *json_out)
        {
            ArticleComment c;
            c.user_id = user_id;
            c.post_id = post_id;
            c.content = content;
            
            if (model_.AddArticleComment(c)) {
                Json::Value res;
                res["status"] = 0;
                res["reason"] = "Success";
                
                *json_out = SerializeJson(res);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                
                *json_out = SerializeJson(res);
                return false;
            }
        }

        bool GetArticleComments(const std::string &post_id, std::string *json_out)
        {
            std::vector<ArticleComment> comments;
            if (model_.GetArticleComments(post_id, &comments)) {
                Json::Value root;
                root["status"] = 0;
                Json::Value list(Json::arrayValue);
                for (const auto &c : comments) {
                    Json::Value item;
                    item["id"] = c.id;
                    item["user_id"] = c.user_id;
                    item["username"] = c.username;
                    item["post_id"] = c.post_id;
                    item["content"] = c.content;
                    item["avatar"] = c.user_avatar;
                    item["created_at"] = c.created_at;
                    item["likes"] = c.likes;
                    list.append(item);
                }
                root["data"] = list;
                
                *json_out = SerializeJson(root);
                return true;
            } else {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Error";
                
                *json_out = SerializeJson(res);
                return false;
            }
        }

