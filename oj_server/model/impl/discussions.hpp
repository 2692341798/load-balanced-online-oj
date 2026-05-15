        bool AddDiscussion(const Discussion &d)
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

            std::string sql = "INSERT INTO " + oj_discussions + " (title, content, author_id, is_official, question_id) VALUES ('"
                + escape(d.title) + "', '"
                + escape(d.content) + "', '"
                + d.author_id + "', "
                + (d.is_official ? "1" : "0") + ", "
                + (d.question_id.empty() ? "0" : d.question_id) + ")";

            if(0 != mysql_query(my, sql.c_str())) {
                LOG(WARNING) << sql << " execute error: " << mysql_error(my) << "\n";

                return false;
            }

            return true;
        }

        bool DeleteDiscussion(const std::string &discussion_id, const std::string &user_id, int role)
        {
            MYSQL *my = mysql_init(nullptr);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(),db.c_str(),port, nullptr, 0)){
                mysql_close(my);
                return false;
            }
            if(0 != mysql_set_character_set(my, "utf8mb4")) { LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n"; }
            
            std::string check_sql = "SELECT author_id FROM " + oj_discussions + " WHERE id=" + discussion_id;
            if(0 != mysql_query(my, check_sql.c_str())) {
                mysql_close(my);
                return false;
            }
            
            MYSQL_RES *res = mysql_store_result(my);
            if(mysql_num_rows(res) == 0) {
                mysql_free_result(res);
                mysql_close(my);
                return false; // Not found
            }
            
            MYSQL_ROW row = mysql_fetch_row(res);
            std::string author_id = row[0] ? row[0] : "";
            mysql_free_result(res);
            
            if (role != 1 && author_id != user_id) {
                mysql_close(my);
                return false; // Not authorized
            }
            
            std::string sql = "DELETE FROM " + oj_discussions + " WHERE id=" + discussion_id;
            if(0 != mysql_query(my, sql.c_str())) {
                mysql_close(my);
                return false;
            }
            
            // Also delete associated comments
            std::string del_inline_comments_sql = "DELETE FROM " + oj_inline_comments + " WHERE post_id=" + discussion_id;
            mysql_query(my, del_inline_comments_sql.c_str());
            
            std::string del_article_comments_sql = "DELETE FROM " + oj_article_comments + " WHERE post_id=" + discussion_id;
            mysql_query(my, del_article_comments_sql.c_str());
            
            mysql_close(my);
            return true;
        }

        bool GetAllDiscussions(std::vector<Discussion> *out)
        {
            // Join with users to get author name and questions to get title
            std::string sql = "SELECT d.id, d.title, d.content, d.author_id, u.username, d.created_at, d.likes, d.views, d.is_official, "
                              "(SELECT COUNT(*) FROM " + oj_article_comments + " WHERE post_id = d.id) as comments_count, d.question_id, q.title, u.avatar "
                              "FROM " + oj_discussions + " d "
                              "LEFT JOIN " + oj_users + " u ON d.author_id = u.id "
                              "LEFT JOIN " + oj_questions + " q ON d.question_id = q.number "
                              "ORDER BY d.created_at DESC";
            
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
                
                Discussion d;
                d.id = row[0] ? row[0] : "";
                d.title = row[1] ? row[1] : "";
                d.content = row[2] ? row[2] : "";
                d.author_id = row[3] ? row[3] : "";
                d.author_name = row[4] ? row[4] : "Unknown";
                d.created_at = row[5] ? row[5] : "";
                d.likes = row[6] ? atoi(row[6]) : 0;
                d.views = row[7] ? atoi(row[7]) : 0;
                d.is_official = (row[8] && atoi(row[8]) == 1);
                d.comments_count = row[9] ? atoi(row[9]) : 0;
                d.question_id = row[10] ? row[10] : "0";
                if (fields > 11) d.question_title = row[11] ? row[11] : "";
                d.author_avatar = (fields > 12 && row[12]) ? row[12] : "";
                
                out->push_back(d);
            }
            mysql_free_result(res);

            return true;
        }

        bool GetDiscussionsByQuestionId(const std::string &qid, std::vector<Discussion> *out)
        {
            std::string sql = "SELECT d.id, d.title, d.content, d.author_id, u.username, d.created_at, d.likes, d.views, d.is_official, "
                              "(SELECT COUNT(*) FROM " + oj_article_comments + " WHERE post_id = d.id) as comments_count, d.question_id, q.title, u.avatar "
                              "FROM " + oj_discussions + " d "
                              "LEFT JOIN " + oj_users + " u ON d.author_id = u.id "
                              "LEFT JOIN " + oj_questions + " q ON d.question_id = q.number "
                              "WHERE d.question_id=" + qid + " "
                              "ORDER BY d.created_at DESC";
            
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
                
                Discussion d;
                d.id = row[0] ? row[0] : "";
                d.title = row[1] ? row[1] : "";
                d.content = row[2] ? row[2] : "";
                d.author_id = row[3] ? row[3] : "";
                d.author_name = row[4] ? row[4] : "Unknown";
                d.created_at = row[5] ? row[5] : "";
                d.likes = row[6] ? atoi(row[6]) : 0;
                d.views = row[7] ? atoi(row[7]) : 0;
                d.is_official = (row[8] && atoi(row[8]) == 1);
                d.comments_count = row[9] ? atoi(row[9]) : 0;
                d.question_id = row[10] ? row[10] : "0";
                if (fields > 11) d.question_title = row[11] ? row[11] : "";
                d.author_avatar = (fields > 12 && row[12]) ? row[12] : "";
                
                out->push_back(d);
            }
            mysql_free_result(res);

            return true;
        }

        bool GetOneDiscussion(const std::string &id, Discussion *d)
        {
            std::string sql = "SELECT d.id, d.title, d.content, d.author_id, u.username, d.created_at, d.likes, d.views, d.is_official, "
                              "(SELECT COUNT(*) FROM " + oj_article_comments + " WHERE post_id = d.id) as comments_count, d.question_id, q.title, u.avatar "
                              "FROM " + oj_discussions + " d "
                              "LEFT JOIN " + oj_users + " u ON d.author_id = u.id "
                              "LEFT JOIN " + oj_questions + " q ON d.question_id = q.number "
                              "WHERE d.id=" + id;
            
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }
            
            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            
            MYSQL_RES *res = mysql_store_result(my);
            int fields = mysql_num_fields(res);
            if (mysql_num_rows(res) == 1) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row) {
                    d->id = row[0] ? row[0] : "";
                    d->title = row[1] ? row[1] : "";
                    d->content = row[2] ? row[2] : "";
                    d->author_id = row[3] ? row[3] : "";
                    d->author_name = row[4] ? row[4] : "Unknown";
                    d->created_at = row[5] ? row[5] : "";
                    d->likes = row[6] ? atoi(row[6]) : 0;
                    d->views = row[7] ? atoi(row[7]) : 0;
                    d->is_official = (row[8] && atoi(row[8]) == 1);
                    d->comments_count = row[9] ? atoi(row[9]) : 0;
                    d->question_id = row[10] ? row[10] : "0";
                    if (fields > 11) d->question_title = row[11] ? row[11] : "";
                    d->author_avatar = (fields > 12 && row[12]) ? row[12] : "";
                    
                    mysql_free_result(res);

                    return true;
                }
            }
            
            mysql_free_result(res);

            return false;
        }
