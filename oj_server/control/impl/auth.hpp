        void RecoveryMachine()
        {
            load_blance_.OnlineMachine();
        }
        
        // Auth Logic
        std::string GenerateToken() {
            char buf[64];
            snprintf(buf, sizeof(buf), "%lx%lx", (unsigned long)time(nullptr), (unsigned long)rand());
            return std::string(buf);
        }

        bool CheckUserExists(const std::string &username) {
            std::string sql_check = "select id, username, password, email, nickname, phone, created_at, role from " + oj_users + " where username='" + username + "'";
            std::vector<User> users;
            if (!model_.QueryUserMySql(sql_check, &users)) {
                LOG(ERROR) << "检查用户是否存在时查询失败: " << username << "\n";
                return true; // 保守策略：查询失败时认为用户存在，避免重复注册
            }
            return !users.empty();
        }

        bool Register(const std::string &username, const std::string &password, const std::string &email, const std::string &nickname = "", const std::string &phone = "") {
            // 检查用户名是否已存在
            if (CheckUserExists(username)) {
                LOG(INFO) << "注册失败，用户名已存在: " << username << "\n";
                return false; // 用户已存在
            }
            
            // 执行注册
            bool result = model_.RegisterUser(username, password, email, nickname, phone);
            if (!result) {
                LOG(ERROR) << "注册失败，数据库错误: " << username << "\n";
            }
            return result;
        }

        bool Login(const std::string &username, const std::string &password, std::string *token) {
            User user;
            if (model_.LoginUser(username, password, &user)) {
                *token = GenerateToken();
                Session s;
                s.user = user;
                s.expire_time = time(nullptr) + 86400; // 1 day
                
                std::unique_lock<std::mutex> lock(session_mtx_);
                sessions_[*token] = s;
                return true;
            }
            return false;
        }

        bool AuthCheck(const Request &req, User *user) {
            if (req.has_header("Cookie")) {
                std::string cookie = req.get_header_value("Cookie");
                std::string key = "session_id=";
                size_t pos = cookie.find(key);
                if (pos != std::string::npos) {
                    std::string token = cookie.substr(pos + key.size());
                    size_t end = token.find(';');
                    if (end != std::string::npos) token = token.substr(0, end);
                    
                    std::unique_lock<std::mutex> lock(session_mtx_);
                    auto it = sessions_.find(token);
                    if (it != sessions_.end()) {
                        if (it->second.expire_time > time(nullptr)) {
                            *user = it->second.user;
                            return true;
                        } else {
                            sessions_.erase(it);
                        }
                    }
                }
            }
            return false;
        }

        bool AdminAuthCheck(const Request &req, User *user) {
            if (AuthCheck(req, user)) {
                return user->role == 1;
            }
            return false;
        }

        bool Logout(const Request &req) {
            if (req.has_header("Cookie")) {
                std::string cookie = req.get_header_value("Cookie");
                std::string key = "session_id=";
                size_t pos = cookie.find(key);
                if (pos != std::string::npos) {
                    std::string token = cookie.substr(pos + key.size());
                    size_t end = token.find(';');
                    if (end != std::string::npos) token = token.substr(0, end);
                    
                    std::unique_lock<std::mutex> lock(session_mtx_);
                    auto it = sessions_.find(token);
                    if (it != sessions_.end()) {
                        sessions_.erase(it);
                        return true;
                    }
                }
            }
            return false;
        }
