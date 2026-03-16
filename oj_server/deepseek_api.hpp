#pragma once

#include <string>
#include <vector>
#include <json/json.h>
#include "../comm/log.hpp"
#include <cstdio>
#include <memory>
#include <array>

namespace ns_deepseek {

using namespace ns_log;

class DeepSeekApi {
public:
    DeepSeekApi() : api_key_("sk-ec8f7b7814974227a6ca9b0eecd2e678"), base_url_("https://api.deepseek.com/chat/completions") {}

    // Generic method to call Chat Completion API
    bool CreateChatCompletion(const Json::Value& messages, std::string* out_content) {
        Json::Value root;
        root["model"] = "deepseek-chat";
        root["messages"] = messages;
        root["stream"] = false;

        Json::FastWriter writer;
        std::string request_body = writer.write(root);
        
        // Escape single quotes for shell command compatibility
        std::string escaped_body;
        for (char c : request_body) {
            if (c == '\'') escaped_body += "'\\''";
            else escaped_body += c;
        }

        // Construct curl command
        // Note: In production, consider using a proper HTTP client library to avoid shell injection risks
        // or ensure strict validation of inputs. Here we assume internal usage is relatively safe.
        std::string command = "curl -s -X POST " + base_url_ + 
                              " -H 'Content-Type: application/json'" + 
                              " -H 'Authorization: Bearer " + api_key_ + "'" + 
                              " -d '" + escaped_body + "'";

        LOG(INFO) << "Calling DeepSeek API..." << std::endl;

        std::string response_str;
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            LOG(ERROR) << "Failed to run curl command" << std::endl;
            return false;
        }

        char buffer[128];
        while (fgets(buffer, 128, pipe) != NULL) {
            response_str += buffer;
        }
        int return_code = pclose(pipe);

        if (return_code != 0) {
             LOG(ERROR) << "Curl command failed with code " << return_code << std::endl;
             return false;
        }

        // Parse response
        Json::Reader reader;
        Json::Value response_json;
        if (!reader.parse(response_str, response_json)) {
            LOG(ERROR) << "Failed to parse API response: " << response_str << std::endl;
            return false;
        }

        if (response_json.isMember("error")) {
             LOG(ERROR) << "API Error: " << response_json["error"]["message"].asString() << std::endl;
             return false;
        }

        if (response_json.isMember("choices") && response_json["choices"].size() > 0) {
            *out_content = response_json["choices"][0]["message"]["content"].asString();
            return true;
        }

        LOG(ERROR) << "Unexpected API response format: " << response_str << std::endl;
        return false;
    }

    // Specific method for generating hints
    bool GenerateHint(const std::string& problem_desc, 
                     const std::string& user_code, 
                     const std::string& error_msg, 
                     const std::string& test_cases, 
                     std::string* out_hint) {
        
        Json::Value messages(Json::arrayValue);
        
        Json::Value system_msg;
        system_msg["role"] = "system";
        system_msg["content"] = "你是一个编程在线评测系统(Online Judge)的智能助手。 "
                                "你的任务是根据用户的代码、题目描述和报错信息，提供有帮助的提示。 "
                                "请不要直接给出完整的正确代码，而是解释错误原因并建议如何修复。 "
                                "请使用Markdown格式回答。 "
                                "如果是编译错误，请解释编译器输出的含义。 "
                                "如果是逻辑错误（Wrong Answer），请尝试找出逻辑漏洞。 "
                                "请务必使用中文回答。";
        messages.append(system_msg);

        std::string user_content = "Problem Description:\n" + problem_desc + "\n\n";
        user_content += "User Code:\n```\n" + user_code + "\n```\n\n";
        user_content += "Error Message/Test Result:\n" + error_msg + "\n\n";
        if (!test_cases.empty()) {
            user_content += "Failed Test Cases:\n" + test_cases + "\n\n";
        }
        user_content += "请提供一个简短的提示来帮助我修复这个问题。";

        Json::Value user_msg;
        user_msg["role"] = "user";
        user_msg["content"] = user_content;
        messages.append(user_msg);

        return CreateChatCompletion(messages, out_hint);
    }

private:
    std::string api_key_;
    std::string base_url_;
};

} // namespace ns_deepseek
