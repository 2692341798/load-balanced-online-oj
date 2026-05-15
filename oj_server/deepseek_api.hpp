#pragma once

#include <string>
#include <vector>
#include <json/json.h>
#include "../comm/log.hpp"
#include <cstdio>
#include <memory>
#include <array>
#include <cstdlib>
#include <cctype>
#include <sys/wait.h>

namespace ns_deepseek {

using namespace ns_log;

class DeepSeekApi {
public:
    DeepSeekApi()
    {
        const char* key = std::getenv("DEEPSEEK_API_KEY");
        api_key_ = key ? std::string(key) : "";

        const char* url = std::getenv("DEEPSEEK_API_URL");
        base_url_ = url ? std::string(url) : "https://api.deepseek.com/chat/completions";

        auto trim = [](std::string* s) {
            if (!s) return;
            size_t start = 0;
            while (start < s->size() && std::isspace(static_cast<unsigned char>((*s)[start]))) start++;
            if (start > 0) s->erase(0, start);
            while (!s->empty() && std::isspace(static_cast<unsigned char>(s->back()))) s->pop_back();
        };
        trim(&api_key_);
        trim(&base_url_);

        while (!base_url_.empty() && base_url_.back() == '/') base_url_.pop_back();
    }

    // Generic method to call Chat Completion API
    bool CreateChatCompletion(const Json::Value& messages, std::string* out_content, std::string* out_error) {
        if (api_key_.empty()) {
            if (out_error) *out_error = "未配置 DeepSeek API Key";
            return false;
        }
        if (base_url_.empty()) {
            if (out_error) *out_error = "未配置 DeepSeek API URL";
            return false;
        }

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
        std::string command = "curl -sS -X POST '" + base_url_ + "'" +
                              " --connect-timeout 5 --max-time 30" +
                              " --retry 2 --retry-all-errors --retry-delay 0" +
                              " -w '\n%{http_code}'" +
                              " -H 'Content-Type: application/json'" + 
                              " -H 'Authorization: Bearer " + api_key_ + "'" + 
                              " -d '" + escaped_body + "'" +
                              " 2>&1";

        LOG(INFO) << "Calling DeepSeek API..." << std::endl;

        std::string response_str;
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            LOG(ERROR) << "Failed to run curl command" << std::endl;
            if (out_error) *out_error = "Failed to run curl command";
            return false;
        }

        char buffer[128];
        while (fgets(buffer, 128, pipe) != NULL) {
            response_str += buffer;
        }
        int status = pclose(pipe);
        int exit_code = status;
        if (WIFEXITED(status)) exit_code = WEXITSTATUS(status);
        else if (WIFSIGNALED(status)) exit_code = 128 + WTERMSIG(status);

        if (exit_code != 0) {
             LOG(ERROR) << "Curl command failed with exit code " << exit_code << std::endl;
             if (out_error) {
                 std::string snippet = response_str;
                 while (!snippet.empty() && (snippet.back() == '\n' || snippet.back() == '\r')) snippet.pop_back();
                 if (snippet.size() > 512) snippet = snippet.substr(snippet.size() - 512);
                 *out_error = "Curl command failed (exit " + std::to_string(exit_code) + ")" + (snippet.empty() ? "" : ": " + snippet);
             }
             return false;
        }

        std::string body_str;
        std::string http_code_str;
        {
            size_t pos = response_str.rfind('\n');
            if (pos != std::string::npos) {
                body_str = response_str.substr(0, pos);
                http_code_str = response_str.substr(pos + 1);
            } else {
                body_str = response_str;
            }

            while (!body_str.empty() && (body_str.back() == '\n' || body_str.back() == '\r')) body_str.pop_back();
            while (!http_code_str.empty() && (http_code_str.back() == '\n' || http_code_str.back() == '\r')) http_code_str.pop_back();
        }

        if (body_str.empty()) {
            if (out_error) {
                if (!http_code_str.empty()) *out_error = "DeepSeek API 返回为空 (HTTP " + http_code_str + ")";
                else *out_error = "DeepSeek API 返回为空";
            }
            return false;
        }

        // Parse response
        Json::Reader reader;
        Json::Value response_json;
        if (!reader.parse(body_str, response_json)) {
            std::string snippet = body_str;
            if (snippet.size() > 512) snippet = snippet.substr(0, 512);
            LOG(ERROR) << "Failed to parse API response: " << snippet << std::endl;
            if (out_error) *out_error = "Failed to parse API response";
            return false;
        }

        if (!http_code_str.empty() && http_code_str != "200") {
            if (response_json.isMember("error") && response_json["error"].isMember("message")) {
                if (out_error) *out_error = response_json["error"]["message"].asString();
            } else {
                if (out_error) *out_error = "DeepSeek API HTTP 状态 " + http_code_str;
            }
            return false;
        }

        if (response_json.isMember("error")) {
             LOG(ERROR) << "API Error: " << response_json["error"]["message"].asString() << std::endl;
             if (out_error) *out_error = response_json["error"]["message"].asString();
             return false;
        }

        if (response_json.isMember("choices") && response_json["choices"].size() > 0) {
            *out_content = response_json["choices"][0]["message"]["content"].asString();
            return true;
        }

        LOG(ERROR) << "Unexpected API response format: " << response_str << std::endl;
        if (out_error) *out_error = "Unexpected API response format";
        return false;
    }

    // Specific method for generating hints
    bool GenerateHint(const std::string& problem_desc, 
                     const std::string& user_code, 
                     const std::string& error_msg, 
                     const std::string& test_cases, 
                     std::string* out_hint,
                     std::string* out_error = nullptr) {
        
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

        return CreateChatCompletion(messages, out_hint, out_error);
    }

private:
    std::string api_key_;
    std::string base_url_;
};

} // namespace ns_deepseek
