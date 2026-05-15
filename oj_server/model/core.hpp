    class MySQLConnectionPool {
    private:
        std::queue<MYSQL*> pool;
        std::mutex mtx;
        std::condition_variable cv;
        int current_size;
        int min_size;
        int max_size;

        MYSQL* CreateConnection() {
            MYSQL *my = mysql_init(nullptr);
            // Reconnect is important for long running process
            bool reconnect = true;
            mysql_options(my, MYSQL_OPT_RECONNECT, &reconnect);
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), port, nullptr, 0)){
                LOG(ERROR) << "Failed to connect to database in pool" << "\n";
                mysql_close(my);
                return nullptr;
            }
            if(0 != mysql_set_character_set(my, "utf8mb4")) {
                LOG(WARNING) << "mysql_set_character_set error: " << mysql_error(my) << "\n";
            }
            return my;
        }

    public:
        MySQLConnectionPool(int min_s = 5, int max_s = 20)
            : min_size(min_s), max_size(max_s), current_size(0) {
            for (int i = 0; i < min_size; ++i) {
                MYSQL* conn = CreateConnection();
                if (conn) {
                    pool.push(conn);
                    current_size++;
                }
            }
        }

        ~MySQLConnectionPool() {
            std::lock_guard<std::mutex> lock(mtx);
            while (!pool.empty()) {
                MYSQL* conn = pool.front();
                pool.pop();
                mysql_close(conn);
            }
        }

        MYSQL* GetConnection() {
            std::unique_lock<std::mutex> lock(mtx);
            if (pool.empty()) {
                if (current_size < max_size) {
                    MYSQL* conn = CreateConnection();
                    if (conn) {
                        current_size++;
                        return conn;
                    }
                }
                cv.wait(lock, [this] { return !pool.empty(); });
            }
            MYSQL* conn = pool.front();
            pool.pop();
            return conn;
        }

        void ReleaseConnection(MYSQL* conn) {
            if (!conn) return;
            // Check if connection is still alive, ping it. If dead, close it and decrement current_size
            if (mysql_ping(conn) != 0) {
                mysql_close(conn);
                std::lock_guard<std::mutex> lock(mtx);
                current_size--;
            } else {
                std::lock_guard<std::mutex> lock(mtx);
                pool.push(conn);
            }
            cv.notify_one();
        }
        
        static MySQLConnectionPool& GetInstance() {
            static MySQLConnectionPool instance(5, 20);
            return instance;
        }
    };

    class ConnectionGuard {
    private:
        MYSQL* conn;
    public:
        ConnectionGuard() {
            conn = MySQLConnectionPool::GetInstance().GetConnection();
        }
        ~ConnectionGuard() {
            MySQLConnectionPool::GetInstance().ReleaseConnection(conn);
        }
        MYSQL* get() { return conn; }
    };

    class Cache {
    private:
        std::unordered_map<std::string, Question> question_cache;
        std::vector<Question> all_questions_cache;
        bool all_questions_cached = false;
        std::mutex mtx;

    public:
        void SetQuestion(const std::string& number, const Question& q) {
            std::lock_guard<std::mutex> lock(mtx);
            question_cache[number] = q;
        }

        bool GetQuestion(const std::string& number, Question* q) {
            std::lock_guard<std::mutex> lock(mtx);
            if (question_cache.count(number)) {
                *q = question_cache[number];
                return true;
            }
            return false;
        }

        void SetAllQuestions(const std::vector<Question>& qs) {
            std::lock_guard<std::mutex> lock(mtx);
            all_questions_cache = qs;
            all_questions_cached = true;
        }

        bool GetAllQuestions(std::vector<Question>* qs) {
            std::lock_guard<std::mutex> lock(mtx);
            if (all_questions_cached) {
                *qs = all_questions_cache;
                return true;
            }
            return false;
        }

        void InvalidateAllQuestions() {
            std::lock_guard<std::mutex> lock(mtx);
            all_questions_cached = false;
        }

        void InvalidateQuestion(const std::string& number) {
            std::lock_guard<std::mutex> lock(mtx);
            question_cache.erase(number);
            all_questions_cached = false; // also invalidate all questions
        }
        
        static Cache& GetInstance() {
            static Cache instance;
            return instance;
        }
    };

