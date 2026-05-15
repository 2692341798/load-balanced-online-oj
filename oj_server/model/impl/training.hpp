        bool CreateTrainingList(const TrainingList &list, int *new_id) {
            if (list.author_id.empty()) {
                LOG(WARNING) << "CreateTrainingList failed: author_id is empty" << "\n";
                return false;
            }

            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                std::cerr << "Connect failed: " << mysql_error(my) << std::endl;
                return false;
            }

            auto escape = [&](const std::string &s) -> std::string {
                char *buf = new char[s.length() * 2 + 1];
                mysql_real_escape_string(my, buf, s.c_str(), s.length());
                std::string res(buf);
                delete[] buf;
                return res;
            };

            std::string sql = "INSERT INTO " + oj_training_lists + " (title, description, difficulty, tags, author_id, visibility) VALUES ('"
                + escape(list.title) + "', '"
                + escape(list.description) + "', '"
                + escape(list.difficulty) + "', '"
                + escape(list.tags) + "', "
                + list.author_id + ", '"
                + escape(list.visibility) + "')";

            if(0 != mysql_query(my, sql.c_str())) {
                std::string err_msg = mysql_error(my);
                LOG(WARNING) << sql << " execute error: " << err_msg << "\n";
                std::cerr << "SQL Error in CreateTrainingList: " << err_msg << "\nSQL: " << sql << std::endl;

                return false;
            }
            
            *new_id = (int)mysql_insert_id(my);

            return true;
        }

        bool UpdateTrainingList(const TrainingList &list) {
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

            std::string sql = "UPDATE " + oj_training_lists + " SET "
                + "title='" + escape(list.title) + "', "
                + "description='" + escape(list.description) + "', "
                + "difficulty='" + escape(list.difficulty) + "', "
                + "tags='" + escape(list.tags) + "', "
                + "visibility='" + escape(list.visibility) + "' "
                + "WHERE id=" + list.id;

            if(0 != mysql_query(my, sql.c_str())) {
                std::string err_msg = mysql_error(my);
                LOG(WARNING) << sql << " execute error: " << err_msg << "\n";
                std::cerr << "SQL Error in UpdateTrainingList: " << err_msg << "\nSQL: " << sql << std::endl;

                return false;
            }

            return true;
        }

        bool DeleteTrainingList(const std::string &id) {
            std::string sql = "DELETE FROM " + oj_training_lists + " WHERE id=" + id;
            return ExecuteSql(sql);
        }

        bool GetTrainingList(const std::string &id, TrainingList *list) {
            std::string sql = "SELECT t.id, t.title, t.description, t.difficulty, t.tags, t.author_id, t.visibility, t.created_at, t.updated_at, t.likes, t.collections, u.username, u.avatar, "
                              "(SELECT COUNT(*) FROM " + oj_training_list_items + " WHERE training_list_id = t.id) as problem_count "
                              "FROM " + oj_training_lists + " t "
                              "LEFT JOIN " + oj_users + " u ON t.author_id = u.id "
                              "WHERE t.id=" + id;
            
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }

            MYSQL_RES *res = mysql_store_result(my);
            if (mysql_num_rows(res) == 1) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row) {
                    list->id = row[0] ? row[0] : "";
                    list->title = row[1] ? row[1] : "";
                    list->description = row[2] ? row[2] : "";
                    list->difficulty = row[3] ? row[3] : "Unrated";
                    list->tags = row[4] ? row[4] : "[]";
                    list->author_id = row[5] ? row[5] : "";
                    list->visibility = row[6] ? row[6] : "public";
                    list->created_at = row[7] ? row[7] : "";
                    list->updated_at = row[8] ? row[8] : "";
                    list->likes = row[9] ? atoi(row[9]) : 0;
                    list->collections = row[10] ? atoi(row[10]) : 0;
                    list->author_name = row[11] ? row[11] : "Unknown";
                    list->author_avatar = row[12] ? row[12] : "";
                    list->problem_count = row[13] ? atoi(row[13]) : 0;
                    
                    mysql_free_result(res);

                    return true;
                }
            }
            mysql_free_result(res);

            return false;
        }

        bool GetTrainingLists(int page, int page_size, const std::string &visibility, const std::string &author_id, std::vector<TrainingList> *out, int *total) {
            int offset = (page - 1) * page_size;
            if (offset < 0) offset = 0;

            std::string where = " WHERE 1=1 ";
            if (!visibility.empty()) {
                where += " AND t.visibility='" + visibility + "' ";
            }
            if (!author_id.empty()) {
                where += " AND t.author_id=" + author_id + " ";
            }

            std::string count_sql = "SELECT COUNT(*) FROM " + oj_training_lists + " t " + where;
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

            std::string sql = "SELECT t.id, t.title, t.description, t.difficulty, t.tags, t.author_id, t.visibility, t.created_at, t.updated_at, t.likes, t.collections, u.username, u.avatar, "
                              "(SELECT COUNT(*) FROM " + oj_training_list_items + " WHERE training_list_id = t.id) as problem_count "
                              "FROM " + oj_training_lists + " t "
                              "LEFT JOIN " + oj_users + " u ON t.author_id = u.id "
                              + where + " ORDER BY t.created_at DESC LIMIT " + std::to_string(offset) + ", " + std::to_string(page_size);

            if(0 != mysql_query(my, sql.c_str())) {

                return false;
            }

            res = mysql_store_result(my);
            int rows = mysql_num_rows(res);

            for(int i = 0; i < rows; i++) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                TrainingList list;
                list.id = row[0] ? row[0] : "";
                list.title = row[1] ? row[1] : "";
                list.description = row[2] ? row[2] : "";
                list.difficulty = row[3] ? row[3] : "Unrated";
                list.tags = row[4] ? row[4] : "[]";
                list.author_id = row[5] ? row[5] : "";
                list.visibility = row[6] ? row[6] : "public";
                list.created_at = row[7] ? row[7] : "";
                list.updated_at = row[8] ? row[8] : "";
                list.likes = row[9] ? atoi(row[9]) : 0;
                list.collections = row[10] ? atoi(row[10]) : 0;
                list.author_name = row[11] ? row[11] : "Unknown";
                list.author_avatar = row[12] ? row[12] : "";
                list.problem_count = row[13] ? atoi(row[13]) : 0;
                out->push_back(list);
            }
            mysql_free_result(res);

            return true;
        }

        bool AddProblemToTrainingList(const std::string &list_id, const std::string &question_id) {
            std::string sql = "INSERT IGNORE INTO " + oj_training_list_items + " (training_list_id, question_id, order_index) "
                              "SELECT " + list_id + ", " + question_id + ", COALESCE(MAX(order_index), 0) + 1 FROM " + oj_training_list_items + " WHERE training_list_id=" + list_id;
            return ExecuteSql(sql);
        }

        bool RemoveProblemFromTrainingList(const std::string &list_id, const std::string &question_id) {
            std::string sql = "DELETE FROM " + oj_training_list_items + " WHERE training_list_id=" + list_id + " AND question_id=" + question_id;
            return ExecuteSql(sql);
        }

        bool ReorderTrainingListProblems(const std::string &list_id, const std::vector<std::string> &problem_ids) {
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                return false;
            }

            mysql_autocommit(my, 0);

            for (size_t i = 0; i < problem_ids.size(); ++i) {
                std::string sql = "UPDATE " + oj_training_list_items + " SET order_index=" + std::to_string(i + 1) + 
                                  " WHERE training_list_id=" + list_id + " AND question_id=" + problem_ids[i];
                if(0 != mysql_query(my, sql.c_str())) {
                    mysql_rollback(my);
                    mysql_autocommit(my, 1);
                    return false;
                }
            }

            mysql_commit(my);
            mysql_autocommit(my, 1);
            return true;
        }

        bool GetTrainingListProblems(const std::string &list_id, const std::string &user_id, std::vector<TrainingListItem> *out) {
            std::string user_status_sql = "";
            if (!user_id.empty()) {
                user_status_sql = ", (SELECT count(*) FROM " + oj_submissions + " WHERE user_id=" + user_id + " AND question_id=i.question_id AND result='0') as solved ";
            } else {
                user_status_sql = ", 0 as solved ";
            }

            std::string sql = "SELECT i.id, i.training_list_id, i.question_id, i.order_index, q.title, q.star " + user_status_sql +
                              "FROM " + oj_training_list_items + " i "
                              "LEFT JOIN " + oj_questions + " q ON i.question_id = q.number "
                              "WHERE i.training_list_id=" + list_id + " "
                              "ORDER BY i.order_index ASC";

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
                TrainingListItem item;
                item.id = row[0] ? row[0] : "";
                item.training_list_id = row[1] ? row[1] : "";
                item.question_id = row[2] ? row[2] : "";
                item.order_index = row[3] ? atoi(row[3]) : 0;
                item.question_title = row[4] ? row[4] : "";
                item.question_difficulty = row[5] ? row[5] : "";
                int solved = row[6] ? atoi(row[6]) : 0;
                item.user_status = (solved > 0) ? "Solved" : "Unsolved";
                
                out->push_back(item);
            }
            mysql_free_result(res);

            return true;
        }
