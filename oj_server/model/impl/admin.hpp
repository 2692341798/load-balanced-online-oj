        bool LogOperation(const OperationLog &log) {
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

            std::string sql = "INSERT INTO " + oj_operation_logs + " (user_id, action, target, details, ip) VALUES ('"
                              + log.user_id + "', '"
                              + escape(log.action) + "', '"
                              + escape(log.target) + "', '"
                              + escape(log.details) + "', '"
                              + escape(log.ip) + "')";

            bool ret = true;
            if(0 != mysql_query(my, sql.c_str())) {
                LOG(WARNING) << "LogOperation failed: " << mysql_error(my) << "\n";
                ret = false;
            }

            return ret;
        }

        bool GetLogs(int page, int page_size, const std::string &keyword, std::vector<OperationLog> *out, int *total) {
            int offset = (page - 1) * page_size;
            if (offset < 0) offset = 0;

            std::string where = " WHERE 1=1 ";
            if (!keyword.empty()) {
                where += " AND (action LIKE '%" + keyword + "%' OR target LIKE '%" + keyword + "%' OR details LIKE '%" + keyword + "%') ";
            }

            std::string count_sql = "SELECT COUNT(*) FROM " + oj_operation_logs + where;
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

            std::string sql = "SELECT l.id, l.user_id, u.username, l.action, l.target, l.details, l.ip, l.created_at FROM " 
                              + oj_operation_logs + " l LEFT JOIN " + oj_users + " u ON l.user_id = u.id "
                              + where + " ORDER BY l.created_at DESC LIMIT " + std::to_string(offset) + ", " + std::to_string(page_size);

            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            
            res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            
            for(int i = 0; i < rows; i++) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                OperationLog l;
                l.id = row[0] ? row[0] : "";
                l.user_id = row[1] ? row[1] : "";
                l.username = row[2] ? row[2] : "Unknown";
                l.action = row[3] ? row[3] : "";
                l.target = row[4] ? row[4] : "";
                l.details = row[5] ? row[5] : "";
                l.ip = row[6] ? row[6] : "";
                l.created_at = row[7] ? row[7] : "";
                out->push_back(l);
            }
            mysql_free_result(res);

            return true;
        }
