        bool AddArticleComment(const ArticleComment &c)
        {
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }
            
            auto escape = [&](const std::string &s) -> std::string {
                char *buf = new char[s.length() * 2 + 1];
                mysql_real_escape_string(my, buf, s.c_str(), s.length());
                std::string res(buf);
                delete[] buf;
                return res;
            };
            
            std::string sql = "INSERT INTO " + oj_article_comments + " (user_id, post_id, content) VALUES ('"
                + c.user_id + "', '"
                + c.post_id + "', '"
                + escape(c.content) + "')";
            
            if(0 != mysql_query(my, sql.c_str())) {
                LOG(WARNING) << sql << " execute error: " << mysql_error(my) << "\n";

                return false;
            }

            return true;
        }

        bool GetArticleComments(const std::string &post_id, std::vector<ArticleComment> *out)
        {
            std::string sql = "SELECT c.id, c.user_id, u.username, c.post_id, c.content, c.created_at, c.likes, u.avatar FROM " 
                              + oj_article_comments + " c LEFT JOIN " + oj_users + " u ON c.user_id = u.id " 
                              + "WHERE c.post_id='" + post_id + "' ORDER BY c.created_at DESC";
            
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }
            
            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            
            MYSQL_RES *res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            int fields = mysql_num_fields(res);
            
            for(int i = 0; i < rows; i++)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                
                ArticleComment c;
                c.id = row[0] ? row[0] : "";
                c.user_id = row[1] ? row[1] : "";
                c.username = row[2] ? row[2] : "Unknown";
                c.post_id = row[3] ? row[3] : "";
                c.content = row[4] ? row[4] : "";
                c.created_at = row[5] ? row[5] : "";
                c.likes = row[6] ? atoi(row[6]) : 0;
                c.user_avatar = (fields > 7 && row[7]) ? row[7] : "";
                
                out->push_back(c);
            }
            mysql_free_result(res);

            return true;
        }
