        std::string SHA256Hash(const std::string &str) {
             unsigned char hash[SHA256_DIGEST_LENGTH];
             SHA256_CTX sha256;
             SHA256_Init(&sha256);
             SHA256_Update(&sha256, str.c_str(), str.size());
             SHA256_Final(hash, &sha256);
             std::stringstream ss;
             for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
             {
                 ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
             }
             return ss.str();
        }

        bool RegisterUser(const std::string &username, const std::string &password, const std::string &email, const std::string &nickname = "", const std::string &phone = "") {
            // Check if exists
            std::string sql_check = "select id, username, password, email, nickname, phone, created_at, role, avatar, status from " + oj_users + " where username='" + username + "'";
            std::vector<User> users;
            if (!QueryUserMySql(sql_check, &users)) {
                LOG(ERROR) << "查询用户失败，数据库错误: " << username << "\n";
                return false; // Database error
            }
            
            if (!users.empty()) {
                LOG(INFO) << "用户名已存在: " << username << "\n";
                return false; // User exists
            }

            // Insert
            std::string pwd_hash = SHA256Hash(password);
            std::string sql_insert = "insert into " + oj_users + " (username, password, email, nickname, phone) values ('" 
                                     + username + "', '" + pwd_hash + "', '" + email + "', '" + nickname + "', '" + phone + "')";
            bool result = ExecuteSql(sql_insert);
            if (result) {
                LOG(INFO) << "用户注册成功: " << username << "\n";
            } else {
                LOG(ERROR) << "用户注册失败，数据库插入错误: " << username << "\n";
            }
            return result;
        }

        bool LoginUser(const std::string &username, const std::string &password, User *user) {
            std::string sql = "select id, username, password, email, nickname, phone, created_at, role, avatar, status from " + oj_users + " where username='" + username + "'";
            std::vector<User> users;
            if (QueryUserMySql(sql, &users) && users.size() == 1) {
                std::string pwd_hash = SHA256Hash(password);
                if (users[0].password == pwd_hash) {
                    *user = users[0];
                    return true;
                }
            }
            return false;
        }

        bool UpdateUser(const User &user) {
            std::string sql = "UPDATE " + oj_users + " SET ";
            bool first = true;
            if (!user.nickname.empty()) {
                sql += "nickname='" + user.nickname + "'";
                first = false;
            }
            if (!user.email.empty()) {
                if (!first) sql += ", ";
                sql += "email='" + user.email + "'";
                first = false;
            }
            if (!user.phone.empty()) {
                if (!first) sql += ", ";
                sql += "phone='" + user.phone + "'";
                first = false;
            }
            if (!user.avatar.empty()) {
                if (!first) sql += ", ";
                sql += "avatar='" + user.avatar + "'";
                first = false;
            }
            
            
            sql += " WHERE id='" + user.id + "'";
            
            if (first) return true; // No fields to update
            
            return ExecuteSql(sql);
        }

        bool GetUsers(int page, int page_size, const std::string &keyword, std::vector<User> *out, int *total) {
            int offset = (page - 1) * page_size;
            if (offset < 0) offset = 0;

            std::string where = " WHERE 1=1 ";
            if (!keyword.empty()) {
                where += " AND (username LIKE '%" + keyword + "%' OR nickname LIKE '%" + keyword + "%' OR email LIKE '%" + keyword + "%') ";
            }

            // Count
            std::string count_sql = "SELECT COUNT(*) FROM " + oj_users + where;
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

            // Fetch
            std::string sql = "SELECT id, username, password, email, nickname, phone, created_at, role, avatar, status FROM " 
                              + oj_users + where + " ORDER BY created_at DESC LIMIT " + std::to_string(offset) + ", " + std::to_string(page_size);

            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            
            res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            
            for(int i = 0; i < rows; i++) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                User u;
                u.id = row[0] ? row[0] : "";
                u.username = row[1] ? row[1] : "";
                u.password = row[2] ? row[2] : "";
                u.email = row[3] ? row[3] : "";
                u.nickname = row[4] ? row[4] : "";
                u.phone = row[5] ? row[5] : "";
                u.created_at = row[6] ? row[6] : "";
                u.role = row[7] ? atoi(row[7]) : 0;
                u.avatar = row[8] ? row[8] : "";
                u.status = row[9] ? atoi(row[9]) : 0;
                out->push_back(u);
            }
            mysql_free_result(res);

            return true;
        }

        bool UpdateUserStatus(const std::string &id, int status) {
            std::string sql = "UPDATE " + oj_users + " SET status=" + std::to_string(status) + " WHERE id=" + id;
            return ExecuteSql(sql);
        }

        bool UpdateUserRole(const std::string &id, int role) {
            std::string sql = "UPDATE " + oj_users + " SET role=" + std::to_string(role) + " WHERE id=" + id;
            return ExecuteSql(sql);
        }

        bool ResetUserPassword(const std::string &id, const std::string &new_password_hash) {
            std::string sql = "UPDATE " + oj_users + " SET password='" + new_password_hash + "' WHERE id=" + id;
            return ExecuteSql(sql);
        }

        bool GetUserById(const std::string &id, User *user) {
            std::string sql = "SELECT id, username, password, email, nickname, phone, created_at, role, avatar, status FROM " + oj_users + " WHERE id=" + id;
            std::vector<User> users;
            if (QueryUserMySql(sql, &users) && !users.empty()) {
                *user = users[0];
                return true;
            }
            return false;
        }

        // Invitation Code Methods
        bool VerifyAndUseInvitationCode(const std::string &code, const std::string &user_id) {
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

            std::string safe_code = escape(code);
            std::string sql_check = "SELECT id, is_used FROM " + oj_invitation_codes + " WHERE code='" + safe_code + "'";
            
            if(0 != mysql_query(my, sql_check.c_str())) {

                return false;
            }

            MYSQL_RES *res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            if (rows == 0) {
                mysql_free_result(res);

                return false; // Code not found
            }

            MYSQL_ROW row = mysql_fetch_row(res);
            int is_used = row[1] ? atoi(row[1]) : 1;
            std::string id = row[0] ? row[0] : "0";
            mysql_free_result(res);

            if (is_used) {

                return false; // Code already used
            }

            // Mark as used
            std::string sql_update = "UPDATE " + oj_invitation_codes + " SET is_used=1, used_by=" + user_id + " WHERE id=" + id;
            if(0 != mysql_query(my, sql_update.c_str())) {

                return false;
            }

            return true;
        }

        bool GenerateInvitationCode(const std::string &code) {
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

            std::string sql = "INSERT INTO " + oj_invitation_codes + " (code) VALUES ('" + escape(code) + "')";
            
            bool ret = true;
            if(0 != mysql_query(my, sql.c_str())) {
                ret = false;
            }

            return ret;
        }
