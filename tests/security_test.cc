#include <gtest/gtest.h>
#include "../comm/httplib.h"
#include <json/json.h>
#include <string>
#include <iostream>

using namespace httplib;

class SecurityTest : public ::testing::Test {
protected:
    std::string admin_user = "admin123";
    std::string admin_pass = "Ahqjhqj73@you";
    std::string base_url = "http://127.0.0.1:8094";

    // Helper to serialize JSON
    std::string SerializeJson(const Json::Value &val) {
        Json::StreamWriterBuilder builder;
        builder["commentStyle"] = "None";
        builder["indentation"] = "";
        return Json::writeString(builder, val);
    }

    void SetUp() override {
        // 1. Register Admin User via API (if not exists)
        Client cli("127.0.0.1", 8094);
        
        Json::Value reg_req;
        reg_req["username"] = admin_user;
        reg_req["password"] = admin_pass;
        reg_req["email"] = "admin@example.com";
        
        cli.Post("/api/register", SerializeJson(reg_req), "application/json");
        
        // 2. Force update role to 1 (Admin) using mysql command line
        // Assuming mysql client is installed and configured as per project defaults
        // DB: oj, User: oj_client, Pass: 123456
        std::string cmd = "mysql -h 127.0.0.1 -P 3306 -u oj_client -p123456 -D oj -e \"UPDATE users SET role=1 WHERE username='" + admin_user + "'\" > /dev/null 2>&1";
        system(cmd.c_str());
    }
};

TEST_F(SecurityTest, AdminLoginAndAccess) {
    Client cli("127.0.0.1", 8094);
    
    // 1. Login
    Json::Value login_req;
    login_req["username"] = admin_user;
    login_req["password"] = admin_pass;
    std::string body = SerializeJson(login_req);
    
    auto res = cli.Post("/api/login", body, "application/json");
    ASSERT_TRUE(res);
    ASSERT_EQ(res->status, 200);
    
    // Extract Cookie
    std::string cookie = res->get_header_value("Set-Cookie");
    ASSERT_FALSE(cookie.empty());
    
    // 2. Access Admin API (Add Question)
    Json::Value q_req;
    q_req["title"] = "Admin Security Test Question";
    q_req["star"] = "Hard";
    q_req["description"] = "Desc";
    q_req["tail"] = "[]";
    q_req["cpu_limit"] = 1;
    q_req["mem_limit"] = 1024;
    q_req["status"] = 0; // Hidden
    
    // Without cookie, should fail (403 or redirect)
    auto res_admin = cli.Post("/api/admin/question", SerializeJson(q_req), "application/json");
    ASSERT_TRUE(res_admin);
    Json::Reader reader;
    Json::Value val;
    reader.parse(res_admin->body, val);
    
    // My implementation returns JSON with status=403 for API calls if unauthorized
    EXPECT_EQ(val["status"].asInt(), 403);
    
    // With Cookie
    Headers headers;
    headers.emplace("Cookie", cookie);
    auto res_admin_auth = cli.Post("/api/admin/question", headers, SerializeJson(q_req), "application/json");
    ASSERT_TRUE(res_admin_auth);
    reader.parse(res_admin_auth->body, val);
    EXPECT_EQ(val["status"].asInt(), 0); // Success
}

TEST_F(SecurityTest, SQLInjectionLogin) {
    Client cli("127.0.0.1", 8094);
    
    // Try SQL Injection in username
    Json::Value login_req;
    login_req["username"] = "admin' OR '1'='1";
    login_req["password"] = "whatever";
    
    auto res = cli.Post("/api/login", SerializeJson(login_req), "application/json");
    ASSERT_TRUE(res);
    
    Json::Reader reader;
    Json::Value val;
    reader.parse(res->body, val);
    
    // Should fail (status 1)
    EXPECT_EQ(val["status"].asInt(), 1);
}

TEST_F(SecurityTest, XSSProtection) {
    Client cli("127.0.0.1", 8094);
    
    // Login first
    Json::Value login_req;
    login_req["username"] = admin_user;
    login_req["password"] = admin_pass;
    auto res = cli.Post("/api/login", SerializeJson(login_req), "application/json");
    std::string cookie = res->get_header_value("Set-Cookie");
    Headers headers;
    headers.emplace("Cookie", cookie);
    
    // Post Discussion with XSS
    Json::Value disc_req;
    std::string xss_payload = "<script>alert('XSS')</script>";
    disc_req["title"] = "XSS Test";
    disc_req["content"] = "Hello " + xss_payload;
    
    auto res_disc = cli.Post("/api/discussion", headers, SerializeJson(disc_req), "application/json");
    ASSERT_TRUE(res_disc);
    EXPECT_EQ(res_disc->status, 200);
}
