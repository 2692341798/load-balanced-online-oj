        bool ExecuteSql(const std::string &sql) {
             ConnectionGuard guard;
             MYSQL *my = guard.get();
             if (!my) return false;
             if(0 != mysql_query(my, sql.c_str())) {
                 std::string err_msg = mysql_error(my);
                 LOG(WARNING) << sql << " execute error: " << err_msg << "\n";
                 std::cerr << "SQL Execute Error: " << err_msg << "\nSQL: " << sql << std::endl;
                 return false;
             }
             return true;
        }

        bool QueryMySql(const std::string &sql, vector<Question> *out)
        {
             ConnectionGuard guard;
             MYSQL *my = guard.get();
             if (!my) return false;

            // 执行sql语句
            if(0 != mysql_query(my, sql.c_str()))
            {
                LOG(WARNING) << sql << " execute error!" << "\n";
                return false;
            }

            // 提取结果
            MYSQL_RES *res = mysql_store_result(my);

            // 分析结果
            int rows = mysql_num_rows(res); //获得行数量
            int fields = mysql_num_fields(res); //获得列数量

            Question q;

            for(int i = 0; i < rows; i++)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                
                q.number = row[0] ? row[0] : "";
                q.title = row[1] ? row[1] : "";
                q.star = row[2] ? row[2] : "";
                q.cpu_limit = row[3] ? atoi(row[3]) : 0;
                q.mem_limit = row[4] ? atoi(row[4]) : 0;
                q.desc = row[5] ? row[5] : "";
                q.tail = row[6] ? row[6] : "";
                if(fields > 7) q.status = row[7] ? atoi(row[7]) : 1;
                else q.status = 1; // Default visible

                out->push_back(q);
            }
            // 释放结果空间
            mysql_free_result(res);

            return true;
        }

        bool QueryUserMySql(const std::string &sql, vector<User> *out)
        {
             ConnectionGuard guard;
             MYSQL *my = guard.get();
             if (!my) return false;

            if(0 != mysql_query(my, sql.c_str()))
            {
                LOG(WARNING) << sql << " execute error!" << "\n";
                return false;
            }
            MYSQL_RES *res = mysql_store_result(my);
            int rows = mysql_num_rows(res);
            int fields = mysql_num_fields(res);
            
            User u;
            for(int i = 0; i < rows; i++)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row == nullptr) continue;
                u.id = row[0] ? row[0] : "";
                u.username = row[1] ? row[1] : "";
                u.password = row[2] ? row[2] : "";
                u.email = row[3] ? row[3] : "";
                if(fields > 4) u.nickname = row[4] ? row[4] : "";
                if(fields > 5) u.phone = row[5] ? row[5] : "";
                if(fields > 6) u.created_at = row[6] ? row[6] : "";
                if(fields > 7) u.role = row[7] ? atoi(row[7]) : 0;
                else u.role = 0;
                if(fields > 8) u.avatar = row[8] ? row[8] : "";
                if(fields > 9) u.status = row[9] ? atoi(row[9]) : 0;
                else u.status = 0;
                
                out->push_back(u);
            }
            mysql_free_result(res);
            return true;
        }
