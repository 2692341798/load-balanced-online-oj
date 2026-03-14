#include <gtest/gtest.h>
#define private public // Hack to access private members for testing
#include "oj_server/oj_control.hpp"
#undef private

using namespace ns_control;

class OJControlTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Ensure conf exists
        system("mkdir -p conf");
        system("echo '127.0.0.1:8081' > conf/service_machine.conf");
        system("echo '127.0.0.1:8082' >> conf/service_machine.conf");
    }
};

TEST_F(OJControlTest, LoadBalanceSmartChoice) {
    LoadBlance lb;
    int id = 0;
    Machine* m = nullptr;

    // 1. Initial Choice
    EXPECT_TRUE(lb.SmartChoice(&id, &m));
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->load, 0);
    
    // 2. Load Balancing
    // Machine 1 load increases
    m->IncLoad();
    
    int id2 = 0;
    Machine* m2 = nullptr;
    EXPECT_TRUE(lb.SmartChoice(&id2, &m2));
    ASSERT_NE(m2, nullptr);
    
    // Should choose different machine (assuming 2 machines in conf)
    // machine 1 load=1, machine 2 load=0. Should pick machine 2.
    if (lb.online.size() > 1) {
        EXPECT_NE(id, id2);
        EXPECT_EQ(m2->load, 0);
    }
    
    m->DecLoad();
}

TEST_F(OJControlTest, UserAuthFlow) {
    Control ctrl;
    std::string username = "auth_test_" + std::to_string(time(nullptr));
    std::string password = "password123";
    
    // 1. Register
    EXPECT_TRUE(ctrl.Register(username, password, "test@example.com"));
    
    // 2. Login
    std::string token;
    EXPECT_TRUE(ctrl.Login(username, password, &token));
    EXPECT_FALSE(token.empty());
    
    // 3. Auth Check
    Request req;
    req.set_header("Cookie", "session_id=" + token);
    
    User user;
    EXPECT_TRUE(ctrl.AuthCheck(req, &user));
    EXPECT_EQ(user.username, username);
    
    // 4. Logout
    EXPECT_TRUE(ctrl.Logout(req));
    
    // 5. Auth Check Fail
    EXPECT_FALSE(ctrl.AuthCheck(req, &user));
}

TEST_F(OJControlTest, AdminAuthCheck) {
    Control ctrl;
    // Assuming admin123 exists or we create one
    // In test environment, we might need to mock or insert admin directly
    // For now, register a normal user and check it fails admin check
    
    std::string username = "normal_user_" + std::to_string(time(nullptr));
    ctrl.Register(username, "pass", "email");
    
    std::string token;
    ctrl.Login(username, "pass", &token);
    
    Request req;
    req.set_header("Cookie", "session_id=" + token);
    
    User user;
    EXPECT_FALSE(ctrl.AdminAuthCheck(req, &user));
}
