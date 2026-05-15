        bool DeleteInlineComment(const std::string &comment_id, const std::string &user_id, int role)
        {
            // First check if the comment belongs to the user or user is admin
            // Role: 1 is admin
            
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }
            
            std::string check_sql = "SELECT user_id FROM " + oj_inline_comments + " WHERE id=" + comment_id;
            if(0 != mysql_query(my, check_sql.c_str())) {

                return false;
            }
            
            MYSQL_RES *res = mysql_store_result(my);
            if(mysql_num_rows(res) == 0) {
                mysql_free_result(res);

                return false; // Not found
            }
            
            MYSQL_ROW row = mysql_fetch_row(res);
            std::string owner_id = row[0] ? row[0] : "";
            mysql_free_result(res);
            
            if (role != 1 && owner_id != user_id) {

                return false; // Not authorized
            }
            
            std::string sql = "DELETE FROM " + oj_inline_comments + " WHERE id=" + comment_id;
            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }

            return true;
        }

        bool AddInlineComment(const InlineComment &comment)
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
            
            std::string parent_val = comment.parent_id.empty() ? "0" : comment.parent_id;

            std::string sql = "INSERT INTO " + oj_inline_comments + " (user_id, post_id, content, selected_text, parent_id) VALUES ('"
                + comment.user_id + "', '"
                + comment.post_id + "', '"
                + escape(comment.content) + "', '"
                + escape(comment.selected_text) + "', "
                + parent_val + ")";
            
            if(0 != mysql_query(my, sql.c_str())) {
                LOG(WARNING) << sql << " execute error: " << mysql_error(my) << "\n";

                return false;
            }

            return true;
        }

        bool GetInlineComments(const std::string &post_id, std::vector<InlineComment> *out)
        {
            std::string sql = "SELECT c.id, c.user_id, u.username, c.post_id, c.content, c.selected_text, c.created_at, c.parent_id, u.avatar FROM " 
                              + oj_inline_comments + " c LEFT JOIN " + oj_users + " u ON c.user_id = u.id " 
                              + "WHERE c.post_id='" + post_id + "' ORDER BY c.created_at ASC";
            
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
                
                InlineComment c;
                c.id = row[0] ? row[0] : "";
                c.user_id = row[1] ? row[1] : "";
                c.username = row[2] ? row[2] : "Unknown";
                c.post_id = row[3] ? row[3] : "";
                c.content = row[4] ? row[4] : "";
                c.selected_text = row[5] ? row[5] : "";
                c.created_at = row[6] ? row[6] : "";
                c.parent_id = (fields > 7 && row[7]) ? row[7] : "0";
                c.user_avatar = (fields > 8 && row[8]) ? row[8] : "";
                
                out->push_back(c);
            }
            mysql_free_result(res);

            return true;
        }
