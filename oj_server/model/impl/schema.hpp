        void InitInvitationCodeTable() {
            std::string sql = "CREATE TABLE IF NOT EXISTS `invitation_codes` ("
                              "`id` INT PRIMARY KEY AUTO_INCREMENT,"
                              "`code` VARCHAR(50) NOT NULL UNIQUE,"
                              "`is_used` TINYINT(1) DEFAULT 0,"
                              "`used_by` INT DEFAULT 0,"
                              "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
                              ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
            ExecuteSql(sql);
        }

        void InitOperationLogTable() {
            std::string sql = "CREATE TABLE IF NOT EXISTS `operation_logs` ("
                              "`id` INT PRIMARY KEY AUTO_INCREMENT,"
                              "`user_id` INT NOT NULL,"
                              "`action` VARCHAR(50) NOT NULL,"
                              "`target` VARCHAR(100),"
                              "`details` TEXT,"
                              "`ip` VARCHAR(50),"
                              "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                              "INDEX `idx_user_id` (`user_id`)"
                              ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
            ExecuteSql(sql);
        }

        void InitTrainingListTable() {
            std::string sql = "CREATE TABLE IF NOT EXISTS `training_lists` ("
                              "`id` INT PRIMARY KEY AUTO_INCREMENT,"
                              "`title` VARCHAR(255) NOT NULL,"
                              "`description` TEXT,"
                              "`difficulty` VARCHAR(50) DEFAULT 'Unrated',"
                              "`tags` TEXT,"
                              "`author_id` INT NOT NULL,"
                              "`visibility` ENUM('public', 'private') DEFAULT 'public',"
                              "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                              "`updated_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
                              "`likes` INT DEFAULT 0,"
                              "`collections` INT DEFAULT 0,"
                              "INDEX `idx_author_id` (`author_id`)"
                              // "FOREIGN KEY (`author_id`) REFERENCES `users`(`id`) ON DELETE CASCADE"
                              ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
            ExecuteSql(sql);
        }

        void InitTrainingListItemTable() {
            std::string sql = "CREATE TABLE IF NOT EXISTS `training_list_items` ("
                              "`id` INT PRIMARY KEY AUTO_INCREMENT,"
                              "`training_list_id` INT NOT NULL,"
                              "`question_id` INT NOT NULL,"
                              "`order_index` INT NOT NULL DEFAULT 0,"
                              "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                              // "FOREIGN KEY (`training_list_id`) REFERENCES `training_lists`(`id`) ON DELETE CASCADE,"
                              "INDEX `idx_list_id` (`training_list_id`),"
                              "INDEX `idx_question_id` (`question_id`),"
                              "UNIQUE KEY `unique_item` (`training_list_id`, `question_id`)"
                              ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
            ExecuteSql(sql);
        }

        void InitUserTable() {
            // 创建完整的表结构
            std::string sql = "CREATE TABLE IF NOT EXISTS `users` ("
                              "`id` int(11) NOT NULL AUTO_INCREMENT,"
                              "`username` varchar(50) NOT NULL UNIQUE,"
                              "`password` varchar(128) NOT NULL,"
                              "`email` varchar(100) DEFAULT NULL,"
                              "`nickname` varchar(100) DEFAULT NULL,"
                              "`phone` varchar(20) DEFAULT NULL,"
                              "`avatar` varchar(255) DEFAULT NULL,"
                              "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                              "`updated_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
                              "PRIMARY KEY (`id`)"
                              ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
            ExecuteSql(sql);
        }

        void InitSubmissionTable() {
            std::string sql = "CREATE TABLE IF NOT EXISTS `submissions` ("
                              "`id` int(11) NOT NULL AUTO_INCREMENT,"
                              "`user_id` int(11) NOT NULL,"
                              "`question_id` int(11) NOT NULL,"
                              "`result` varchar(10) NOT NULL,"
                              "`cpu_time` int(11) DEFAULT 0,"
                              "`mem_usage` int(11) DEFAULT 0,"
                              "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                              "`content` TEXT,"
                              "`language` varchar(20) DEFAULT 'cpp',"
                              "PRIMARY KEY (`id`),"
                              "INDEX `idx_user_id` (`user_id`),"
                              "INDEX `idx_question_id` (`question_id`)"
                              ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
            ExecuteSql(sql);
        }

        void InitInlineCommentTable() {
            std::string sql = "CREATE TABLE IF NOT EXISTS `inline_comments` ("
                              "`id` int(11) NOT NULL AUTO_INCREMENT,"
                              "`user_id` int(11) NOT NULL,"
                              "`post_id` int(11) NOT NULL,"
                              "`content` TEXT NOT NULL,"
                              "`selected_text` TEXT,"
                              "`parent_id` int(11) DEFAULT 0,"
                              "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                              "PRIMARY KEY (`id`),"
                              "INDEX `idx_post_id` (`post_id`)"
                              ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
            ExecuteSql(sql);
        }

        void InitDiscussionTable() {
            std::string sql = "CREATE TABLE IF NOT EXISTS `discussions` ("
                              "`id` int(11) NOT NULL AUTO_INCREMENT,"
                              "`title` varchar(255) NOT NULL,"
                              "`content` TEXT NOT NULL,"
                              "`author_id` int(11) NOT NULL,"
                              "`question_id` int(11) DEFAULT 0,"
                              "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                              "`likes` int(11) DEFAULT 0,"
                              "`views` int(11) DEFAULT 0,"
                              "`is_official` tinyint(1) DEFAULT 0,"
                              "PRIMARY KEY (`id`),"
                              "INDEX `idx_author_id` (`author_id`),"
                              "INDEX `idx_question_id` (`question_id`)"
                              ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
            ExecuteSql(sql);
        }

        void InitArticleCommentTable() {
            std::string sql = "CREATE TABLE IF NOT EXISTS `article_comments` ("
                              "`id` int(11) NOT NULL AUTO_INCREMENT,"
                              "`post_id` int(11) NOT NULL,"
                              "`user_id` int(11) NOT NULL,"
                              "`content` TEXT NOT NULL,"
                              "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                              "`likes` int(11) DEFAULT 0,"
                              "PRIMARY KEY (`id`),"
                              "INDEX `idx_post_id` (`post_id`),"
                              "INDEX `idx_user_id` (`user_id`)"
                              ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
            ExecuteSql(sql);
        }

        void InitContestTable() {
            std::string sql = "CREATE TABLE IF NOT EXISTS `contests` ("
                              "`id` int(11) NOT NULL AUTO_INCREMENT,"
                              "`contest_id` varchar(50) NOT NULL COMMENT 'Platform ID',"
                              "`name` varchar(255) NOT NULL,"
                              "`start_time` datetime NOT NULL,"
                              "`end_time` datetime NOT NULL,"
                              "`link` varchar(255) NOT NULL,"
                              "`source` varchar(50) DEFAULT 'Codeforces',"
                              "`status` varchar(20) DEFAULT 'upcoming',"
                              "`last_crawl_time` datetime DEFAULT NULL,"
                              "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                              "`updated_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
                              "PRIMARY KEY (`id`),"
                              "UNIQUE KEY `idx_source_id` (`source`, `contest_id`),"
                              "INDEX `idx_status_time` (`status`, `start_time` DESC)"
                              ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
            ExecuteSql(sql);
        }

        void CheckAndUpgradeTable() {
            ConnectionGuard guard;
            MYSQL *my = guard.get();
            if(!my){
                LOG(ERROR) << "Upgrade Check: Connect failed" << "\n";
                return;
            }

            // Check content column in submissions
            std::string check_content = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_submissions + "' AND COLUMN_NAME = 'content'";
            if(0 == mysql_query(my, check_content.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_submissions + " ADD COLUMN content TEXT";
                    LOG(INFO) << "Upgrading submissions table: adding content column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check language column in submissions
            std::string check_lang = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_submissions + "' AND COLUMN_NAME = 'language'";
            if(0 == mysql_query(my, check_lang.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_submissions + " ADD COLUMN language VARCHAR(20) DEFAULT 'cpp'";
                    LOG(INFO) << "Upgrading submissions table: adding language column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check role column in users
            std::string check_role = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_users + "' AND COLUMN_NAME = 'role'";
            if(0 == mysql_query(my, check_role.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_users + " ADD COLUMN role INT DEFAULT 0 COMMENT '0:User, 1:Admin'";
                    LOG(INFO) << "Upgrading users table: adding role column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check avatar column in users
            std::string check_avatar = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_users + "' AND COLUMN_NAME = 'avatar'";
            if(0 == mysql_query(my, check_avatar.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_users + " ADD COLUMN avatar VARCHAR(255) DEFAULT NULL";
                    LOG(INFO) << "Upgrading users table: adding avatar column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check created_at column in users
            std::string check_created = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_users + "' AND COLUMN_NAME = 'created_at'";
            if(0 == mysql_query(my, check_created.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_users + " ADD COLUMN created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP";
                    LOG(INFO) << "Upgrading users table: adding created_at column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check status column in oj_questions
            std::string check_status = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_questions + "' AND COLUMN_NAME = 'status'";
            if(0 == mysql_query(my, check_status.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_questions + " ADD COLUMN status INT DEFAULT 1 COMMENT '0:Hidden, 1:Visible'";
                    LOG(INFO) << "Upgrading oj_questions table: adding status column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check parent_id column in inline_comments
            std::string check_parent = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_inline_comments + "' AND COLUMN_NAME = 'parent_id'";
            if(0 == mysql_query(my, check_parent.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_inline_comments + " ADD COLUMN parent_id INT DEFAULT 0";
                    LOG(INFO) << "Upgrading inline_comments table: adding parent_id column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }
            
            // Check question_id column in discussions
            std::string check_qid = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_discussions + "' AND COLUMN_NAME = 'question_id'";
            if(0 == mysql_query(my, check_qid.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_discussions + " ADD COLUMN question_id INT DEFAULT 0, ADD INDEX idx_question_id (question_id)";
                    LOG(INFO) << "Upgrading discussions table: adding question_id column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check difficulty column in training_lists
            std::string check_tl_diff = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_training_lists + "' AND COLUMN_NAME = 'difficulty'";
            if(0 == mysql_query(my, check_tl_diff.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_training_lists + " ADD COLUMN difficulty VARCHAR(50) DEFAULT 'Unrated'";
                    LOG(INFO) << "Upgrading training_lists table: adding difficulty column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check tags column in training_lists
            std::string check_tl_tags = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_training_lists + "' AND COLUMN_NAME = 'tags'";
            if(0 == mysql_query(my, check_tl_tags.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_training_lists + " ADD COLUMN tags TEXT";
                    LOG(INFO) << "Upgrading training_lists table: adding tags column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check visibility column in training_lists
            std::string check_tl_vis = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_training_lists + "' AND COLUMN_NAME = 'visibility'";
            if(0 == mysql_query(my, check_tl_vis.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_training_lists + " ADD COLUMN visibility ENUM('public', 'private') DEFAULT 'public'";
                    LOG(INFO) << "Upgrading training_lists table: adding visibility column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check likes column in training_lists
            std::string check_tl_likes = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_training_lists + "' AND COLUMN_NAME = 'likes'";
            if(0 == mysql_query(my, check_tl_likes.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_training_lists + " ADD COLUMN likes INT DEFAULT 0";
                    LOG(INFO) << "Upgrading training_lists table: adding likes column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check collections column in training_lists
            std::string check_tl_col = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_training_lists + "' AND COLUMN_NAME = 'collections'";
            if(0 == mysql_query(my, check_tl_col.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_training_lists + " ADD COLUMN collections INT DEFAULT 0";
                    LOG(INFO) << "Upgrading training_lists table: adding collections column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check order_index column in training_list_items
            std::string check_tli_order = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_training_list_items + "' AND COLUMN_NAME = 'order_index'";
            if(0 == mysql_query(my, check_tli_order.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_training_list_items + " ADD COLUMN order_index INT NOT NULL DEFAULT 0";
                    LOG(INFO) << "Upgrading training_list_items table: adding order_index column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

            // Check status column in users
            std::string check_user_status = "SELECT count(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + db + "' AND TABLE_NAME = '" + oj_users + "' AND COLUMN_NAME = 'status'";
            if(0 == mysql_query(my, check_user_status.c_str())) {
                MYSQL_RES *res = mysql_store_result(my);
                MYSQL_ROW row = mysql_fetch_row(res);
                int count = row ? atoi(row[0]) : 0;
                mysql_free_result(res);
                
                if (count == 0) {
                    std::string alter_sql = "ALTER TABLE " + oj_users + " ADD COLUMN status INT DEFAULT 0 COMMENT '0:Normal, 1:Banned'";
                    LOG(INFO) << "Upgrading users table: adding status column" << "\n";
                    mysql_query(my, alter_sql.c_str());
                }
            }

        }
