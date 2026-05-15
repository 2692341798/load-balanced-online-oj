        bool GetAllQuestions(vector<Question> *out)
        {
            if (Cache::GetInstance().GetAllQuestions(out)) {
                return true;
            }
            // Only show visible questions for normal users
            std::string sql = "select number, title, star, cpu_limit, mem_limit, description, tail_code, status from ";
            sql += oj_questions;
            sql += " where status=1";
            bool ret = QueryMySql(sql, out);
            if (ret) {
                Cache::GetInstance().SetAllQuestions(*out);
            }
            return ret;
        }

        bool GetQuestionsByPage(int page, int page_size, vector<Question> *out, int *total)
        {
            int offset = (page - 1) * page_size;
            if (offset < 0) offset = 0;

            // 1. Get total count
            std::string count_sql = "select count(*) from " + oj_questions + " where status=1";
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

            // 2. Get paginated data
            // Note: number is varchar, so CAST AS UNSIGNED for correct numeric sorting
            std::string sql = "select number, title, star, cpu_limit, mem_limit, description, tail_code, status from " + oj_questions + " where status=1 ORDER BY CAST(number AS UNSIGNED) ASC LIMIT " + std::to_string(offset) + ", " + std::to_string(page_size);
            
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
                
                Question q;
                q.number = row[0] ? row[0] : "";
                q.title = row[1] ? row[1] : "";
                q.star = row[2] ? row[2] : "";
                q.cpu_limit = row[3] ? atoi(row[3]) : 0;
                q.mem_limit = row[4] ? atoi(row[4]) : 0;
                q.desc = row[5] ? row[5] : "";
                q.tail = row[6] ? row[6] : "";
                if(fields > 7) q.status = row[7] ? atoi(row[7]) : 1;
                else q.status = 1;

                out->push_back(q);
            }
            mysql_free_result(res);

            return true;
        }

        bool GetAllQuestionsAdmin(int page, int page_size, vector<Question> *out, int *total)
        {
            int offset = (page - 1) * page_size;
            if (offset < 0) offset = 0;

            // 1. Get total count
            std::string count_sql = "select count(*) from " + oj_questions;
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

            // 2. Get paginated data
            // Show all questions for admin
            std::string sql = "select number, title, star, cpu_limit, mem_limit, description, tail_code, status from " + oj_questions + " ORDER BY CAST(number AS UNSIGNED) ASC LIMIT " + std::to_string(offset) + ", " + std::to_string(page_size);
            
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
                
                Question q;
                q.number = row[0] ? row[0] : "";
                q.title = row[1] ? row[1] : "";
                q.star = row[2] ? row[2] : "";
                q.cpu_limit = row[3] ? atoi(row[3]) : 0;
                q.mem_limit = row[4] ? atoi(row[4]) : 0;
                q.desc = row[5] ? row[5] : "";
                q.tail = row[6] ? row[6] : "";
                if(fields > 7) q.status = row[7] ? atoi(row[7]) : 1;
                else q.status = 1;

                out->push_back(q);
            }
            mysql_free_result(res);

            return true;
        }

        bool GetOneQuestion(const std::string &number, Question *q)
        {
            if (Cache::GetInstance().GetQuestion(number, q)) {
                return true;
            }
            bool res = false;
            std::string sql = "select number, title, star, cpu_limit, mem_limit, description, tail_code, status from ";
            sql += oj_questions;
            sql += " where number=";
            sql += number;
            vector<Question> result;
            if(QueryMySql(sql, &result))
            {
                if(result.size() == 1){
                    *q = result[0];
                    res = true;
                    Cache::GetInstance().SetQuestion(number, *q);
                }
            }
            return res;
        }

        bool AddQuestion(const Question &q)
        {
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if (!my) return false;

            // Escape strings
            auto escape = [&](const std::string &s) -> std::string {
                char *buf = new char[s.length() * 2 + 1];
                mysql_real_escape_string(my, buf, s.c_str(), s.length());
                std::string res(buf);
                delete[] buf;
                return res;
            };

            std::string sql = "INSERT INTO " + oj_questions + " (title, star, cpu_limit, mem_limit, description, tail_code, status) VALUES ('"
                + escape(q.title) + "', '"
                + escape(q.star) + "', "
                + std::to_string(q.cpu_limit) + ", "
                + std::to_string(q.mem_limit) + ", '"
                + escape(q.desc) + "', '"
                + escape(q.tail) + "', "
                + std::to_string(q.status) + ")";

            if(0 != mysql_query(my, sql.c_str())) {
                LOG(WARNING) << sql << " execute error: " << mysql_error(my) << "\n";
                return false;
            }
            Cache::GetInstance().InvalidateAllQuestions();
            return true;
        }

        bool UpdateQuestion(const Question &q)
        {
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if (!my) return false;

            auto escape = [&](const std::string &s) -> std::string {
                char *buf = new char[s.length() * 2 + 1];
                mysql_real_escape_string(my, buf, s.c_str(), s.length());
                std::string res(buf);
                delete[] buf;
                return res;
            };

            std::string sql = "UPDATE " + oj_questions + " SET "
                + "title='" + escape(q.title) + "', "
                + "star='" + escape(q.star) + "', "
                + "cpu_limit=" + std::to_string(q.cpu_limit) + ", "
                + "mem_limit=" + std::to_string(q.mem_limit) + ", "
                + "description='" + escape(q.desc) + "', "
                + "tail_code='" + escape(q.tail) + "', "
                + "status=" + std::to_string(q.status)
                + " WHERE number=" + q.number;

            if(0 != mysql_query(my, sql.c_str())) {
                LOG(WARNING) << sql << " execute error: " << mysql_error(my) << "\n";
                return false;
            }
            Cache::GetInstance().InvalidateQuestion(q.number);
            return true;
        }

        bool DeleteQuestion(const std::string &number)
        {
            std::string sql = "DELETE FROM " + oj_questions + " WHERE number=" + number;
            bool ret = ExecuteSql(sql);
            if (ret) {
                Cache::GetInstance().InvalidateQuestion(number);
            }
            return ret;
        }
