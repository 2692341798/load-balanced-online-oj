        bool UpsertContest(const Contest &c) {
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

            std::string sql = "INSERT INTO " + oj_contests + " (contest_id, name, start_time, end_time, link, source, status, last_crawl_time) VALUES ('"
                + escape(c.contest_id) + "', '"
                + escape(c.name) + "', '"
                + c.start_time + "', '"
                + c.end_time + "', '"
                + escape(c.link) + "', '"
                + escape(c.source) + "', '"
                + c.status + "', NOW()) "
                + "ON DUPLICATE KEY UPDATE "
                + "name='" + escape(c.name) + "', "
                + "start_time='" + c.start_time + "', "
                + "end_time='" + c.end_time + "', "
                + "status='" + c.status + "', "
                + "last_crawl_time=NOW()";

            if(0 != mysql_query(my, sql.c_str())) {
                LOG(WARNING) << sql << " execute error: " << mysql_error(my) << "\n";

                return false;
            }

            return true;
        }

        bool GetContests(int page, int page_size, const std::string &status_filter, std::vector<Contest> *out, int *total) {
            int offset = (page - 1) * page_size;
            if (offset < 0) offset = 0;

            std::string where = " WHERE 1=1 ";
            if (!status_filter.empty()) {
                 where += " AND status='" + status_filter + "' ";
            }

            std::string count_sql = "SELECT COUNT(*) FROM " + oj_contests + where;
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

            std::string order_by = " ORDER BY FIELD(status, 'running', 'upcoming', 'ended'), "
                                   "CASE WHEN status = 'upcoming' THEN start_time END ASC, "
                                   "CASE WHEN status != 'upcoming' THEN start_time END DESC ";
            
            std::string sql = "SELECT id, contest_id, name, start_time, end_time, link, source, status, last_crawl_time FROM " 
                              + oj_contests + where + order_by + " LIMIT " + std::to_string(offset) + ", " + std::to_string(page_size);

            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            
            res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            
            for(int i = 0; i < rows; i++) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                Contest c;
                c.id = row[0] ? row[0] : "";
                c.contest_id = row[1] ? row[1] : "";
                c.name = row[2] ? row[2] : "";
                c.start_time = row[3] ? row[3] : "";
                c.end_time = row[4] ? row[4] : "";
                c.link = row[5] ? row[5] : "";
                c.source = row[6] ? row[6] : "";
                c.status = row[7] ? row[7] : "";
                c.last_crawl_time = row[8] ? row[8] : "";
                out->push_back(c);
            }
            mysql_free_result(res);

            return true;
        }
