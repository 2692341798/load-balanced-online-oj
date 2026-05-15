        bool GetTotalUserCount(int *count) {
            std::string sql = "SELECT COUNT(*) FROM " + oj_users;
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){

                return false;
            }
            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            MYSQL_RES *res = mysql_store_result(my);
            if (res) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if (row) *count = atoi(row[0]);
                mysql_free_result(res);
            }

            return true;
        }

        bool GetTotalProblemCount(int *count) {
            std::string sql = "SELECT COUNT(*) FROM " + oj_questions;
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }
            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            MYSQL_RES *res = mysql_store_result(my);
            if (res) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if (row) *count = atoi(row[0]);
                mysql_free_result(res);
            }

            return true;
        }

        bool GetTotalSubmissionCount(int *count) {
            std::string sql = "SELECT COUNT(*) FROM " + oj_submissions;
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }
            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }
            MYSQL_RES *res = mysql_store_result(my);
            if (res) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if (row) *count = atoi(row[0]);
                mysql_free_result(res);
            }

            return true;
        }

        bool GetUserGrowthStats(int days, std::map<std::string, int>* stats) {
            std::string sql = "SELECT DATE(created_at) as date, COUNT(*) FROM " + oj_users + 
                              " WHERE created_at >= DATE_SUB(NOW(), INTERVAL " + std::to_string(days) + " DAY) GROUP BY date";
            
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
            
            for(int i = 0; i < rows; i++) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                std::string date = row[0] ? row[0] : "";
                int count = row[1] ? atoi(row[1]) : 0;
                (*stats)[date] = count;
            }
            mysql_free_result(res);

            return true;
        }

        bool GetSubmissionStats(std::map<std::string, int>* stats) {
            std::string sql = "SELECT result, COUNT(*) FROM " + oj_submissions + " GROUP BY result";
            
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
            
            for(int i = 0; i < rows; i++) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                std::string result = row[0] ? row[0] : "";
                int count = row[1] ? atoi(row[1]) : 0;
                (*stats)[result] = count;
            }
            mysql_free_result(res);

            return true;
        }

        bool GetDailyActivityStats(int days, std::map<std::string, int>* stats) {
            std::string sql = "SELECT DATE(created_at) as date, COUNT(DISTINCT user_id) FROM " + oj_submissions + 
                              " WHERE created_at >= DATE_SUB(NOW(), INTERVAL " + std::to_string(days) + " DAY) GROUP BY date";
            
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
            
            for(int i = 0; i < rows; i++) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                std::string date = row[0] ? row[0] : "";
                int count = row[1] ? atoi(row[1]) : 0;
                (*stats)[date] = count;
            }
            mysql_free_result(res);

            return true;
        }
