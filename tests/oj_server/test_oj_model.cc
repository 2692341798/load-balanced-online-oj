#include <gtest/gtest.h>
#include "oj_server/oj_model.hpp"
#include <string>
#include <vector>

using namespace ns_model;

class OJModelTest : public ::testing::Test {
protected:
    Model model;

    void SetUp() override {
        // Code here will be called immediately after the constructor (right
        // before each test).
    }

    void TearDown() override {
        // Code here will be called immediately after each test (right
        // before the destructor).
    }
};

// Test User Operations
TEST_F(OJModelTest, UserOperations) {
    std::string username = "test_user_" + std::to_string(time(nullptr));
    std::string password = "password123";
    std::string email = "test@example.com";

    // 1. Register
    EXPECT_TRUE(model.RegisterUser(username, password, email));

    // 2. Login
    User user;
    EXPECT_TRUE(model.LoginUser(username, password, &user));
    EXPECT_EQ(user.username, username);
    EXPECT_EQ(user.email, email);

    // 3. Update
    user.nickname = "UpdatedNick";
    EXPECT_TRUE(model.UpdateUser(user));
    
    User updated_user;
    EXPECT_TRUE(model.LoginUser(username, password, &updated_user));
    EXPECT_EQ(updated_user.nickname, "UpdatedNick");
}

// Test Question Operations
TEST_F(OJModelTest, QuestionOperations) {
    Question q;
    q.title = "Test Question " + std::to_string(time(nullptr));
    q.star = "Easy";
    q.desc = "Description";
    q.header = "Header";
    q.tail = "Tail";
    q.cpu_limit = 1;
    q.mem_limit = 1024;
    q.status = 1;

    // 1. Add
    EXPECT_TRUE(model.AddQuestion(q));

    // 2. Get All
    std::vector<Question> questions;
    EXPECT_TRUE(model.GetAllQuestions(&questions));
    bool found = false;
    for (const auto& question : questions) {
        if (question.title == q.title) {
            found = true;
            q.number = question.number; // Capture the ID
            break;
        }
    }
    EXPECT_TRUE(found);

    // 3. Get One
    Question q2;
    EXPECT_TRUE(model.GetOneQuestion(q.number, &q2));
    EXPECT_EQ(q2.title, q.title);

    // 4. Delete (Cleanup)
    EXPECT_TRUE(model.DeleteQuestion(q.number));
}

// Test Training List Operations
TEST_F(OJModelTest, TrainingListOperations) {
    TrainingList list;
    list.title = "Test List";
    list.description = "Desc";
    list.author_id = "1"; // Assuming admin/user 1 exists
    list.difficulty = "Easy";
    list.visibility = "public";

    int list_id = 0;
    // 1. Create
    EXPECT_TRUE(model.CreateTrainingList(list, &list_id));
    EXPECT_GT(list_id, 0);

    // 2. Get
    TrainingList fetched;
    EXPECT_TRUE(model.GetTrainingList(std::to_string(list_id), &fetched));
    EXPECT_EQ(fetched.title, "Test List");

    // 3. Delete
    EXPECT_TRUE(model.DeleteTrainingList(std::to_string(list_id)));
}
