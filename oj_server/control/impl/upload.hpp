        bool UploadImage(const Request &req, std::string *json_out)
        {
            if (!req.has_file("image")) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "No image file uploaded";
                
                *json_out = SerializeJson(res);
                return false;
            }

            const auto &file = req.get_file_value("image");
            std::string filename = file.filename;
            std::string content = file.content;
            
            // Generate unique filename
            size_t ext_pos = filename.find_last_of('.');
            std::string ext = (ext_pos != std::string::npos) ? filename.substr(ext_pos) : ".jpg";
            std::string new_filename = std::to_string(time(nullptr)) + "_" + std::to_string(rand()) + ext;
            
            // Save path
            std::string path = "./uploads/" + new_filename;
            
            // Create uploads directory if not exists
            // Assuming wwwroot exists, mkdir uploads
            system("mkdir -p ./uploads");
            
            std::ofstream out(path, std::ios::binary);
            if (!out.is_open()) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Failed to save file";
                
                *json_out = SerializeJson(res);
                return false;
            }
            out.write(content.c_str(), content.size());
            out.close();
            
            Json::Value res;
            res["status"] = 0;
            res["url"] = "/uploads/" + new_filename;
            
            *json_out = SerializeJson(res);
            return true;
        }

        bool UploadAvatar(const Request &req, const std::string &user_id, std::string *json_out)
        {
            if (!req.has_file("avatar")) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "No avatar file uploaded";
                *json_out = SerializeJson(res);
                return false;
            }

            const auto &file = req.get_file_value("avatar");
            std::string filename = file.filename;
            std::string content = file.content;
            
            // Check file size (2MB limit)
            if (content.size() > 2 * 1024 * 1024) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "File size exceeds 2MB limit";
                *json_out = SerializeJson(res);
                return false;
            }

            // Check file type
            std::string ext;
            size_t ext_pos = filename.find_last_of('.');
            if (ext_pos != std::string::npos) {
                ext = filename.substr(ext_pos);
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            }
            
            if (ext != ".jpg" && ext != ".jpeg" && ext != ".png" && ext != ".gif") {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Invalid file type. Only JPG, PNG, GIF are allowed.";
                *json_out = SerializeJson(res);
                return false;
            }

            // Generate unique filename: avatar_<user_id>_<timestamp><ext>
            std::string new_filename = "avatar_" + user_id + "_" + std::to_string(time(nullptr)) + ext;
            std::string path = "./uploads/avatars/" + new_filename;
            
            // Create directory
            system("mkdir -p ./uploads/avatars");
            
            // Save file
            std::ofstream out(path, std::ios::binary);
            if (!out.is_open()) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Failed to save file";
                *json_out = SerializeJson(res);
                return false;
            }
            out.write(content.c_str(), content.size());
            out.close();
            
            std::string avatar_url = "/uploads/avatars/" + new_filename;

            // Update Database
            User u;
            u.id = user_id;
            u.avatar = avatar_url;
            if (model_.UpdateUser(u)) {
                LOG(INFO) << "User " << user_id << " avatar updated in DB to " << avatar_url << "\n";
                // Update Session Cache
                std::unique_lock<std::mutex> lock(session_mtx_);
                bool old_avatar_deleted = false;
                int updated_sessions = 0;
                for (auto &kv : sessions_) {
                    if (kv.second.user.id == user_id) {
                        // Delete old avatar file if exists and not default
                        // Only delete once to avoid redundant filesystem calls
                        if (!old_avatar_deleted && !kv.second.user.avatar.empty()) {
                            std::string old_path = "." + kv.second.user.avatar;
                            // Basic check to ensure we don't delete something outside uploads/avatars
                            if (old_path.find("/uploads/avatars/") != std::string::npos) {
                                remove(old_path.c_str());
                                old_avatar_deleted = true;
                            }
                        }
                        
                        kv.second.user.avatar = avatar_url;
                        updated_sessions++;
                        // Do NOT break here! Update ALL sessions for this user.
                    }
                }
                LOG(INFO) << "Updated " << updated_sessions << " sessions for user " << user_id << "\n";
                
                Json::Value res;
                res["status"] = 0;
                res["url"] = avatar_url;
                *json_out = SerializeJson(res);
                return true;
            } else {
                // If DB update fails, maybe delete the uploaded file?
                remove(path.c_str());
                
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Database Update Failed";
                *json_out = SerializeJson(res);
                return false;
            }
        }

