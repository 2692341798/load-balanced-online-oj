        void Judge(const std::string &number, const std::string in_json, std::string *out_json, const std::string &user_id = "")
        {
            struct Question q;
            if (!model_.GetOneQuestion(number, &q)) {
                 Json::Value err_res;
                 err_res["status"] = -2;
                 err_res["reason"] = "Question not found";
                 *out_json = SerializeJson(err_res);
                 return;
            }
            
            if (q.status == 0) {
                std::string sql = "select id, username, password, email, nickname, phone, created_at, role from " + oj_users + " where id='" + user_id + "'";
                std::vector<User> users;
                model_.QueryUserMySql(sql, &users);
                if (users.empty() || users[0].role != 1) {
                     Json::Value err_res;
                     err_res["status"] = -2;
                     err_res["reason"] = "Question is not published";
                     *out_json = SerializeJson(err_res);
                     return;
                }
            }

            Json::Reader reader;
            Json::Value in_value;
            reader.parse(in_json, in_value);
            std::string code = in_value["code"].asString();
            std::string language = in_value.isMember("language") ? in_value["language"].asString() : "C++";

            Json::Reader tail_reader;
            Json::Value cases;
            bool has_cases = tail_reader.parse(q.tail, cases) && cases.isArray();
            
            if (!has_cases) {
                 Json::Value single_case;
                 single_case["input"] = in_value["input"].asString(); 
                 single_case["expect"] = "";
                 cases = Json::Value(Json::arrayValue);
                 cases.append(single_case);
            }

            Json::Value result_cases(Json::arrayValue);
            bool all_passed = true;
            
            for (unsigned int i = 0; i < cases.size(); ++i) {
                Json::Value &one_case = cases[i];
                std::string input_data = one_case.isMember("input") ? one_case["input"].asString() : "";
                std::string expected_output = one_case.isMember("expect") ? one_case["expect"].asString() : "";

                Json::Value compile_value;
                compile_value["input"] = input_data;
                compile_value["code"] = code;
                compile_value["language"] = language;
                compile_value["cpu_limit"] = q.cpu_limit;
                compile_value["mem_limit"] = q.mem_limit;
                
                std::string compile_string = SerializeJson(compile_value);

                while(true) {
                    int id = 0;
                    Machine *m = nullptr;
                    if(!load_blance_.SmartChoice(&id, &m)) {
                         Json::Value err_res;
                         err_res["status"] = -2;
                         err_res["reason"] = "No available compile server";
                         *out_json = SerializeJson(err_res);
                         return;
                    }
                    
                    Client cli(m->ip, m->port);
                    cli.set_connection_timeout(1);
                    cli.set_read_timeout(5);
                    cli.set_write_timeout(2);

                    bool request_success = false;
                    int retry_count = 0;
                    bool case_completed = false;
                    
                    while (retry_count < 3) {
                        m->IncLoad();
                        auto res = cli.Post("/compile_and_run", compile_string, "application/json;charset=utf-8");
                        m->DecLoad();
                        if (res) {
                            request_success = true;
                            if(res->status == 200) {
                                Json::Reader resp_reader;
                                Json::Value resp_val;
                                resp_reader.parse(res->body, resp_val);
                                
                                if (resp_val["status"].asInt() != 0) {
                                    *out_json = res->body;
                                    return;
                                }
                                
                                std::string stdout_str = resp_val["stdout"].asString();
                                std::string trim_stdout = stdout_str; 
                                while(!trim_stdout.empty() && isspace(trim_stdout.back())) trim_stdout.pop_back();
                                std::string trim_expect = expected_output;
                                while(!trim_expect.empty() && isspace(trim_expect.back())) trim_expect.pop_back();
                                
                                bool pass = (trim_stdout == trim_expect);
                                if (!pass) all_passed = false;
                                
                                Json::Value case_res;
                                case_res["name"] = "Case " + std::to_string(i+1);
                                case_res["pass"] = pass;
                                case_res["input"] = input_data;
                                case_res["output"] = trim_stdout;
                                case_res["expected"] = trim_expect;
                                
                                result_cases.append(case_res);
                                case_completed = true;
                            }
                            break;
                        }
                        retry_count++;
                    }
                    
                    if (!request_success) {
                        load_blance_.OfflineMachine(id);
                    } else if (case_completed) {
                        break;
                    } else {
                        load_blance_.OfflineMachine(id);
                    }
                }
            }
            
            Json::Value final_res;
            final_res["status"] = 0; 
            final_res["reason"] = "";
            
            Json::Value stdout_json;
            stdout_json["cases"] = result_cases;
            
            Json::Value summary;
            summary["total"] = cases.size();
            int passed_cnt = 0;
            for(const auto& c : result_cases) if(c["pass"].asBool()) passed_cnt++;
            summary["passed"] = passed_cnt;
            if (passed_cnt == cases.size()) summary["overall"] = "All Passed";
            else summary["overall"] = std::to_string(passed_cnt) + "/" + std::to_string(cases.size()) + " Passed";
            
            stdout_json["summary"] = summary;
            
            final_res["stdout"] = SerializeJson(stdout_json);
            final_res["stderr"] = "";
            
            *out_json = SerializeJson(final_res);
            
            if (!user_id.empty()) {
                 Submission sub;
                 sub.user_id = user_id;
                 sub.question_id = number;
                 sub.result = (passed_cnt == cases.size()) ? "0" : "-1"; 
                 sub.content = code;
                 sub.language = language;
                 model_.AddSubmission(sub);
            }
        }

        void GenerateAiHint(const Request &req, std::string *json_out)
        {
            User user;
            if (!AuthCheck(req, &user)) {
                 Json::Value res;
                 res["status"] = 401;
                 res["reason"] = "Unauthorized";
                 *json_out = SerializeJson(res);
                 return;
            }

            Json::Reader reader;
            Json::Value root;
            reader.parse(req.body, root);
            
            std::string question_id = root.get("question_id", "").asString();
            std::string code = root.get("code", "").asString();
            std::string error_msg = root.get("error_msg", "").asString();
            std::string test_cases = root.get("test_cases", "").asString();

            if (question_id.empty() || code.empty() || error_msg.empty()) {
                Json::Value res;
                res["status"] = 1;
                res["reason"] = "Missing required fields";
                *json_out = SerializeJson(res);
                return;
            }

            struct Question q;
            if (!model_.GetOneQuestion(question_id, &q)) {
                 Json::Value res;
                 res["status"] = 1;
                 res["reason"] = "Question Not Found";
                 *json_out = SerializeJson(res);
                 return;
            }

            std::string hint;
            std::string error_detail;
            if (deepseek_api_.GenerateHint(q.desc, code, error_msg, test_cases, &hint, &error_detail)) {
                 Json::Value res;
                 res["status"] = 0;
                 res["hint"] = hint;
                 *json_out = SerializeJson(res);
            } else {
                 Json::Value res;
                 res["status"] = 1;
                 res["reason"] = "AI Service Unavailable" + (error_detail.empty() ? "" : ": " + error_detail);
                 *json_out = SerializeJson(res);
            }
        }

