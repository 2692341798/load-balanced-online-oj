        bool AddSubmission(const Submission &sub)
        {
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }
            
            char* escaped_content = new char[sub.content.length() * 2 + 1];
            mysql_real_escape_string(my, escaped_content, sub.content.c_str(), sub.content.length());
            
            std::string sql = "INSERT INTO " + oj_submissions + " (user_id, question_id, result, cpu_time, mem_usage, content, language) VALUES (";
            sql += "'" + sub.user_id + "', ";
            sql += "'" + sub.question_id + "', ";
            sql += "'" + sub.result + "', ";
            sql += std::to_string(sub.cpu_time) + ", ";
            sql += std::to_string(sub.mem_usage) + ", ";
            sql += "'";
            sql += escaped_content;
            sql += "', '";
            sql += sub.language;
            sql += "')";
            
            delete[] escaped_content;
            
            if(0 != mysql_query(my, sql.c_str())) {
                LOG(WARNING) << sql << " execute error: " << mysql_error(my) << "\n";

                return false;
            }

            return true;
        }
        
        bool GetSubmissions(const std::string &user_id, 
                            const std::string &question_id, 
                            const std::string &status,
                            const std::string &start_time,
                            const std::string &end_time,
                            const std::string &keyword,
                            int offset, int limit,
                            std::vector<Submission> *out,
                            int *total)
        {
            std::string where_clause = " WHERE 1=1 ";
            if(!user_id.empty()) where_clause += " AND s.user_id='" + user_id + "' ";
            if(!question_id.empty()) where_clause += " AND s.question_id='" + question_id + "' ";
            if(!status.empty()) where_clause += " AND s.result='" + status + "' ";
            if(!start_time.empty()) where_clause += " AND s.created_at >= '" + start_time + "' ";
            if(!end_time.empty()) where_clause += " AND s.created_at <= '" + end_time + "' ";
            if(!keyword.empty()) {
                where_clause += " AND s.content LIKE '%" + keyword + "%' "; 
            }

            std::string count_sql = "SELECT count(*) FROM " + oj_submissions + " s " + where_clause;
            
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            if(0 != mysql_query(my, count_sql.c_str())) {

                return false;
            }
            MYSQL_RES *res = mysql_store_result(my);
            if (res) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if (row) *total = atoi(row[0]);
                mysql_free_result(res);
            }

            std::string sql = "SELECT s.id, s.user_id, s.question_id, q.title, s.result, s.cpu_time, s.mem_usage, s.created_at, s.content FROM " 
                              + oj_submissions + " s LEFT JOIN " + oj_questions + " q ON s.question_id = q.number " 
                              + where_clause + " ORDER BY s.created_at DESC LIMIT " + std::to_string(offset) + ", " + std::to_string(limit);
            
            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            
            res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            int fields = mysql_num_fields(res);
            
            for(int i = 0; i < rows; i++)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                
                Submission s;
                s.id = row[0] ? row[0] : "";
                s.user_id = row[1] ? row[1] : "";
                s.question_id = row[2] ? row[2] : "";
                s.question_title = row[3] ? row[3] : "";
                s.result = row[4] ? row[4] : "";
                s.cpu_time = row[5] ? atoi(row[5]) : 0;
                s.mem_usage = row[6] ? atoi(row[6]) : 0;
                s.created_at = row[7] ? row[7] : "";
                if (fields > 8) s.content = row[8] ? row[8] : "";
                
                out->push_back(s);
            }
            mysql_free_result(res);

            return true;
        }
