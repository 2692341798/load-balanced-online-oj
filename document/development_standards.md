# 在线评测系统开发规范

## 1. 概述

本文档定义了冻梨OJ（在线评测系统）的开发规范，包括代码风格、命名规范、设计原则、测试要求等，旨在确保代码质量、可维护性和团队协作效率。

### 1.1 目标
- 提高代码可读性和可维护性
- 确保代码质量和稳定性
- 促进团队协作和知识共享
- 降低技术债务和维护成本

### 1.2 适用范围
- 所有C++源代码文件
- 前端HTML/CSS/JavaScript代码
- 配置文件和脚本
- 文档和注释

## 2. 代码风格规范

### 2.1 C++代码风格

#### 2.1.1 命名规范

**文件命名**
```cpp
// 使用小写字母，下划线分隔
// 良好示例
oj_control.hpp
compile_run.cpp
util.hpp

// 避免使用
OJControl.hpp  // 驼峰命名
compile-run.cpp // 连字符
```

**类命名**
```cpp
// 使用大驼峰命名法（PascalCase）
class Control {
    // ...
};

class LoadBlance {
    // ...
};

class CompileAndRun {
    // ...
};
```

**函数命名**
```cpp
// 使用小驼峰命名法（camelCase）
class Control {
public:
    bool allQuestions(const Request& req, string* html);
    void judge(const string& number, const string& in_json, string* out_json);
    bool registerUser(const string& username, const string& password);
};
```

**变量命名**
```cpp
// 使用小写字母，下划线分隔
string user_name;
int cpu_limit;
uint64_t load_value;

// 类成员变量使用后缀_
class Machine {
private:
    std::string ip_;
    int port_;
    uint64_t load_;
    std::mutex* mtx_;
};
```

**常量命名**
```cpp
// 使用全大写字母，下划线分隔
const int MAX_RETRY_COUNT = 3;
const std::string SERVICE_MACHINE = "./conf/service_machine.conf";
const int SESSION_EXPIRE_TIME = 86400; // 24小时
```

#### 2.1.2 代码格式

**缩进和空格**
```cpp
// 使用4个空格缩进，不使用Tab
class Control {
public:
    bool allQuestions(const Request& req, string* html) {
        // 函数体缩进
        if (req.has_param("page")) {
            // 代码块缩进
            int page = std::stoi(req.get_param_value("page"));
            return true;
        }
        return false;
    }
};
```

**花括号风格**
```cpp
// 使用K&R风格（埃及括号）
class Control {
public:
    bool allQuestions(const Request& req, string* html) {
        if (req.has_param("page")) {
            int page = std::stoi(req.get_param_value("page"));
            return true;
        } else {
            return false;
        }
    }
};
```

**空格使用**
```cpp
// 关键字后加空格
if (condition) {
    // ...
}

for (int i = 0; i < count; ++i) {
    // ...
}

// 运算符两侧加空格
int result = a + b * c;
bool flag = (x > 0) && (y < 10);

// 函数调用不加空格
std::string name = getUserName(user_id);
bool success = validateInput(input);
```

#### 2.1.3 注释规范

**文件注释**
```cpp
/**
 * @file oj_control.hpp
 * @brief OJ主服务器业务逻辑控制器
 * @author 开发团队
 * @date 2026-01-08
 * @version 2.0.0
 * 
 * 主要负责用户请求处理、题目管理、代码评测调度等功能
 */
```

**函数注释**
```cpp
/**
 * @brief 获取所有题目列表
 * @param req HTTP请求对象
 * @param html 输出的HTML内容
 * @return bool 成功返回true，失败返回false
 * @note 需要用户已登录
 * @warning 输出内容较大时注意性能
 */
bool allQuestions(const Request& req, string* html);
```

**代码注释**
```cpp
// 单行注释：解释代码意图，而不是描述代码本身
// 计算用户提交的代码行数，用于统计信息
int line_count = std::count(code.begin(), code.end(), '\n');

/*
 * 多行注释：用于复杂算法的解释
 * 负载均衡算法说明：
 * 1. 遍历所有在线的编译服务器
 * 2. 选择当前负载最小的服务器
 * 3. 更新该服务器的负载计数
 * 4. 返回选中的服务器信息
 */
bool smartChoice(int* id, Machine** m) {
    // 实现代码
}
```

### 2.2 前端代码风格

#### 2.2.1 HTML规范
```html
<!-- 使用语义化标签 -->
<nav class="navbar">
    <div class="navbar-brand">
        <a href="/">
            <span>&lt;/&gt;</span> 冻梨OJ
        </a>
    </div>
    <div class="navbar-links">
        <a href="/">首页</a>
        <a href="/all_questions">题库</a>
    </div>
</nav>

<!-- 属性使用双引号 -->
<input type="text" id="username" placeholder="请输入用户名" required>

<!-- 正确的缩进 -->
<div class="form-group">
    <label for="username">用户名</label>
    <input type="text" id="username" name="username" required>
</div>
```

#### 2.2.2 CSS规范
```css
/* 使用CSS变量 */
:root {
    --bg-color: #1a1a1a;
    --text-main: #ffffff;
    --accent-color: #ffa116;
}

/* 选择器命名 */
.navbar {
    display: flex;
    justify-content: space-between;
}

.navbar-brand {
    font-size: 1.5rem;
    font-weight: bold;
}

/* 属性顺序 */
.selector {
    /* 1. 布局属性 */
    display: flex;
    position: relative;
    
    /* 2. 盒模型属性 */
    width: 100%;
    padding: 20px;
    margin: 10px;
    
    /* 3. 视觉属性 */
    background: var(--bg-color);
    color: var(--text-main);
    border: 1px solid var(--border-color);
    
    /* 4. 文字属性 */
    font-size: 1rem;
    line-height: 1.5;
    
    /* 5. 动画属性 */
    transition: all 0.3s ease;
}
```

#### 2.2.3 JavaScript规范
```javascript
// 使用严格模式
'use strict';

// 变量命名使用camelCase
let userName = '';
let isLoggedIn = false;
let currentQuestion = null;

// 函数命名使用camelCase
function validateInput(input) {
    if (!input || input.length === 0) {
        return false;
    }
    return true;
}

// 常量使用全大写
const MAX_CODE_LENGTH = 65536;
const API_BASE_URL = '/api';

// 使用模板字符串
const url = `${API_BASE_URL}/login`;
const message = `欢迎，${userName}！`;
```

## 3. 设计原则

### 3.1 SOLID原则

#### 3.1.1 单一职责原则 (SRP)
```cpp
// 良好示例：每个类只负责一个职责
class LoadBlance {
    // 只负责负载均衡相关功能
public:
    bool smartChoice(int* id, Machine** m);
    void offlineMachine(int which);
private:
    std::vector<Machine> machines_;
    std::vector<int> online_;
};

class Model {
    // 只负责数据访问
public:
    bool getAllQuestions(std::vector<Question>* out);
    bool registerUser(const std::string& username, const std::string& password);
};
```

#### 3.1.2 开闭原则 (OCP)
```cpp
// 使用接口和抽象类实现扩展
class CompilerInterface {
public:
    virtual bool compile(const std::string& file_name) = 0;
    virtual ~CompilerInterface() = default;
};

class GccCompiler : public CompilerInterface {
public:
    bool compile(const std::string& file_name) override;
};

// 未来可以扩展支持其他编译器
class ClangCompiler : public CompilerInterface {
public:
    bool compile(const std::string& file_name) override;
};
```

#### 3.1.3 里氏替换原则 (LSP)
```cpp
// 确保子类可以替换父类
class Runner {
public:
    virtual int run(const std::string& file_name, int cpu_limit, int mem_limit);
};

class SafeRunner : public Runner {
public:
    int run(const std::string& file_name, int cpu_limit, int mem_limit) override {
        // 实现相同的行为契约
        return Runner::run(file_name, cpu_limit, mem_limit);
    }
};
```

#### 3.1.4 接口隔离原则 (ISP)
```cpp
// 将大接口拆分为小接口
class QuestionInterface {
public:
    virtual bool getQuestion(const std::string& number, Question* q) = 0;
    virtual bool getAllQuestions(std::vector<Question>* out) = 0;
};

class UserInterface {
public:
    virtual bool registerUser(const std::string& username, const std::string& password) = 0;
    virtual bool loginUser(const std::string& username, const std::string& password) = 0;
};

// Model类实现多个小接口
class Model : public QuestionInterface, public UserInterface {
    // 实现所有接口方法
};
```

#### 3.1.5 依赖倒置原则 (DIP)
```cpp
// 依赖抽象而不是具体实现
class Control {
private:
    std::unique_ptr<QuestionInterface> question_service_;
    std::unique_ptr<UserInterface> user_service_;
    
public:
    Control(std::unique_ptr<QuestionInterface> qs, 
            std::unique_ptr<UserInterface> us)
        : question_service_(std::move(qs)), 
          user_service_(std::move(us)) {}
};
```

### 3.2 其他设计原则

#### 3.2.1 DRY原则 (Don't Repeat Yourself)
```cpp
// 避免重复代码
// 不好的做法
bool validateUser1(const std::string& username) {
    if (username.empty()) return false;
    if (username.length() < 3) return false;
    if (username.length() > 20) return false;
    return true;
}

bool validateUser2(const std::string& username) {
    if (username.empty()) return false;
    if (username.length() < 3) return false;
    if (username.length() > 20) return false;
    return true;
}

// 好的做法
bool validateUsername(const std::string& username) {
    if (username.empty()) return false;
    if (username.length() < 3 || username.length() > 20) return false;
    return true;
}

// 在多个地方复用
bool registerUser(const std::string& username, const std::string& password) {
    if (!validateUsername(username)) return false;
    // ...
}

bool loginUser(const std::string& username, const std::string& password) {
    if (!validateUsername(username)) return false;
    // ...
}
```

#### 3.2.2 KISS原则 (Keep It Simple, Stupid)
```cpp
// 保持代码简单明了
// 不好的做法：过度复杂的实现
std::string getErrorMessage(int code) {
    std::map<int, std::string> error_map = {
        {-1, "代码为空"},
        {-2, "未知错误"},
        {-3, "编译错误"},
        {6, "内存超过限制"},
        {24, "CPU使用超时"}
    };
    
    auto it = error_map.find(code);
    return (it != error_map.end()) ? it->second : "未知错误";
}

// 好的做法：简单直接
std::string getErrorMessage(int code) {
    switch (code) {
        case -1: return "代码为空";
        case -2: return "未知错误";
        case -3: return "编译错误";
        case 6: return "内存超过限制";
        case 24: return "CPU使用超时";
        default: return "未知错误";
    }
}
```

#### 3.2.3 YAGNI原则 (You Aren't Gonna Need It)
```cpp
// 不要添加不需要的功能
// 不好的做法：过早优化和过度设计
class FutureFeature {
    // 现在不需要的复杂功能
    void quantumComputingSupport();
    void aiAssistedCoding();
    void blockchainVerification();
};

// 好的做法：专注于当前需求
class CompileAndRun {
public:
    // 只实现当前需要的功能
    void start(const std::string& in_json, std::string* out_json);
    void removeTempFile(const std::string& file_name);
};
```

## 4. 错误处理

### 4.1 异常处理

#### 4.1.1 使用异常而不是错误码
```cpp
// 不好的做法：使用错误码
int compile(const std::string& file_name) {
    if (file_name.empty()) return -1;
    if (!fileExists(file_name)) return -2;
    if (!hasPermission(file_name)) return -3;
    // ...
    return 0; // 成功
}

// 好的做法：使用异常
class CompileException : public std::exception {
private:
    std::string message_;
public:
    CompileException(const std::string& msg) : message_(msg) {}
    const char* what() const noexcept override {
        return message_.c_str();
    }
};

void compile(const std::string& file_name) {
    if (file_name.empty()) {
        throw CompileException("文件名不能为空");
    }
    if (!fileExists(file_name)) {
        throw CompileException("文件不存在: " + file_name);
    }
    if (!hasPermission(file_name)) {
        throw CompileException("没有文件访问权限: " + file_name);
    }
    // ...
}
```

#### 4.1.2 异常安全
```cpp
// 使用RAII确保资源正确释放
class FileGuard {
private:
    std::string file_name_;
    bool should_delete_;
    
public:
    FileGuard(const std::string& file_name, bool should_delete = true)
        : file_name_(file_name), should_delete_(should_delete) {}
    
    ~FileGuard() {
        if (should_delete_) {
            std::remove(file_name_.c_str());
        }
    }
    
    // 禁用拷贝
    FileGuard(const FileGuard&) = delete;
    FileGuard& operator=(const FileGuard&) = delete;
    
    // 允许移动
    FileGuard(FileGuard&& other) noexcept 
        : file_name_(std::move(other.file_name_)), 
          should_delete_(other.should_delete_) {
        other.should_delete_ = false;
    }
};

// 使用示例
void processFile(const std::string& file_name) {
    FileGuard guard(file_name); // 确保文件最终被删除
    
    // 处理文件...
    
    // 如果处理成功，取消删除
    guard.release(); // 需要添加release方法
}
```

### 4.2 错误传播

#### 4.2.1 合理的错误处理
```cpp
// Web服务层的错误处理
void judge(const std::string& number, const std::string& in_json, string* out_json) {
    try {
        Json::Reader reader;
        Json::Value in_value;
        
        if (!reader.parse(in_json, in_value)) {
            throw std::runtime_error("JSON解析失败");
        }
        
        std::string code = in_value["code"].asString();
        if (code.empty()) {
            throw std::runtime_error("提交的代码为空");
        }
        
        // 继续处理...
        
    } catch (const std::exception& e) {
        LOG(ERROR) << "代码评测异常: " << e.what();
        *out_json = R"({"status": -2, "reason": "")" + e.what() + R"("})";
    } catch (...) {
        LOG(ERROR) << "代码评测发生未知异常";
        *out_json = R"({"status": -2, "reason": "未知错误"})";
    }
}
```

## 5. 并发编程

### 5.1 线程安全

#### 5.1.1 互斥锁使用
```cpp
class LoadBlance {
private:
    std::vector<Machine> machines_;
    std::vector<int> online_;
    std::vector<int> offline_;
    mutable std::mutex mtx_; // mutable允许const函数加锁
    
public:
    bool smartChoice(int* id, Machine** m) {
        std::lock_guard<std::mutex> lock(mtx_);
        
        if (online_.empty()) {
            return false;
        }
        
        // 选择负载最小的机器
        int min_load_idx = 0;
        uint64_t min_load = machines_[online_[0]].load();
        
        for (size_t i = 1; i < online_.size(); ++i) {
            uint64_t current_load = machines_[online_[i]].load();
            if (current_load < min_load) {
                min_load = current_load;
                min_load_idx = i;
            }
        }
        
        *id = online_[min_load_idx];
        *m = &machines_[online_[min_load_idx]];
        
        // 增加负载
        machines_[online_[min_load_idx]].incLoad();
        
        return true;
    }
};
```

#### 5.1.2 原子操作
```cpp
#include <atomic>

class IdGenerator {
private:
    static std::atomic<uint64_t> counter_;
    
public:
    static uint64_t generate() {
        return counter_.fetch_add(1, std::memory_order_relaxed);
    }
};

// 初始化
std::atomic<uint64_t> IdGenerator::counter_{0};
```

### 5.2 避免死锁

#### 5.2.1 锁顺序
```cpp
// 不好的做法：可能导致死锁
void functionA() {
    std::lock_guard<std::mutex> lock1(mtx1_);
    std::lock_guard<std::mutex> lock2(mtx2_);
    // ...
}

void functionB() {
    std::lock_guard<std::mutex> lock2(mtx2_);
    std::lock_guard<std::mutex> lock1(mtx1_);
    // ...
}

// 好的做法：统一的锁顺序
void functionA() {
    std::lock_guard<std::mutex> lock1(mtx1_);
    std::lock_guard<std::mutex> lock2(mtx2_);
    // ...
}

void functionB() {
    std::lock_guard<std::mutex> lock1(mtx1_);
    std::lock_guard<std::mutex> lock2(mtx2_);
    // ...
}
```

#### 5.2.2 使用std::lock
```cpp
// 同时锁定多个互斥量
void transferMoney(Account& from, Account& to, double amount) {
    std::lock(from.mtx_, to.mtx_);
    std::lock_guard<std::mutex> lock1(from.mtx_, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(to.mtx_, std::adopt_lock);
    
    from.balance_ -= amount;
    to.balance_ += amount;
}
```

## 6. 性能优化

### 6.1 内存管理

#### 6.1.1 避免不必要的拷贝
```cpp
// 不好的做法：不必要的字符串拷贝
std::string getErrorMessage(int code) {
    std::string result = "";
    switch (code) {
        case -1: result = "代码为空"; break;
        case -2: result = "未知错误"; break;
        // ...
    }
    return result;
}

// 好的做法：返回字符串视图
std::string_view getErrorMessage(int code) {
    static const std::unordered_map<int, std::string> error_map = {
        {-1, "代码为空"},
        {-2, "未知错误"},
        {-3, "编译错误"},
        {6, "内存超过限制"},
        {24, "CPU使用超时"}
    };
    
    auto it = error_map.find(code);
    return (it != error_map.end()) ? it->second : "未知错误";
}
```

#### 6.1.2 使用移动语义
```cpp
// 在适当的地方使用std::move
class SessionManager {
private:
    std::unordered_map<std::string, Session> sessions_;
    
public:
    void addSession(std::string token, Session session) {
        // 使用移动语义避免拷贝
        sessions_[std::move(token)] = std::move(session);
    }
    
    Session getSession(const std::string& token) {
        auto it = sessions_.find(token);
        if (it != sessions_.end()) {
            // 返回副本，不移动
            return it->second;
        }
        throw std::runtime_error("会话不存在");
    }
};
```

### 6.2 算法优化

#### 6.2.1 选择合适的容器
```cpp
// 根据使用场景选择合适的容器
class QuestionCache {
private:
    // 需要快速查找，使用unordered_map
    std::unordered_map<std::string, Question> question_map_;
    
    // 需要保持顺序，使用vector
    std::vector<Question> question_list_;
    
    // 需要快速查找且保持顺序，使用vector + unordered_map
    std::vector<Question> ordered_questions_;
    std::unordered_map<std::string, size_t> question_index_;
    
public:
    void addQuestion(const Question& q) {
        question_map_[q.number] = q;
        question_list_.push_back(q);
        
        size_t index = ordered_questions_.size();
        ordered_questions_.push_back(q);
        question_index_[q.number] = index;
    }
    
    const Question* getQuestion(const std::string& number) const {
        auto it = question_map_.find(number);
        return (it != question_map_.end()) ? &it->second : nullptr;
    }
};
```

#### 6.2.2 避免不必要的计算
```cpp
// 缓存计算结果
class LoadBlance {
private:
    std::vector<Machine> machines_;
    std::vector<int> online_;
    mutable std::mutex mtx_;
    
    // 缓存最小负载的机器索引
    mutable int min_load_idx_ = -1;
    mutable uint64_t min_load_value_ = UINT64_MAX;
    mutable bool cache_valid_ = false;
    
public:
    bool smartChoice(int* id, Machine** m) {
        std::lock_guard<std::mutex> lock(mtx_);
        
        if (online_.empty()) {
            return false;
        }
        
        // 如果缓存无效，重新计算
        if (!cache_valid_) {
            updateCache();
        }
        
        *id = online_[min_load_idx_];
        *m = &machines_[online_[min_load_idx_]];
        
        // 增加负载后使缓存失效
        machines_[online_[min_load_idx_]].incLoad();
        cache_valid_ = false;
        
        return true;
    }
    
private:
    void updateCache() {
        min_load_idx_ = 0;
        min_load_value_ = machines_[online_[0]].load();
        
        for (size_t i = 1; i < online_.size(); ++i) {
            uint64_t current_load = machines_[online_[i]].load();
            if (current_load < min_load_value_) {
                min_load_value_ = current_load;
                min_load_idx_ = i;
            }
        }
        
        cache_valid_ = true;
    }
};
```

## 7. 测试规范

### 7.1 单元测试

#### 7.1.1 测试结构
```cpp
// 使用Google Test框架
#include <gtest/gtest.h>

class LoadBlanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试前的准备工作
        load_blance_.loadConf("test_service.conf");
    }
    
    void TearDown() override {
        // 测试后的清理工作
    }
    
    LoadBlance load_blance_;
};

TEST_F(LoadBlanceTest, SmartChoiceReturnsValidMachine) {
    // 给定：有在线的机器
    ASSERT_FALSE(load_blance_.getOnlineMachines().empty());
    
    // 当：选择一个机器
    int id;
    Machine* machine;
    bool result = load_blance_.smartChoice(&id, &machine);
    
    // 那么：应该成功返回一个有效的机器
    EXPECT_TRUE(result);
    EXPECT_NE(id, -1);
    EXPECT_NE(machine, nullptr);
    EXPECT_GT(machine->load(), 0); // 负载应该增加
}

TEST_F(LoadBlanceTest, SmartChoiceWithNoOnlineMachines) {
    // 给定：没有在线的机器
    load_blance_.setAllMachinesOffline();
    
    // 当：尝试选择机器
    int id;
    Machine* machine;
    bool result = load_blance_.smartChoice(&id, &machine);
    
    // 那么：应该失败
    EXPECT_FALSE(result);
}
```

#### 7.1.2 测试命名
```cpp
// 测试函数命名规范：被测试的功能_条件_期望结果
TEST_F(LoadBlanceTest, SmartChoice_ReturnsMachineWithMinimumLoad);
TEST_F(LoadBlanceTest, SmartChoice_UpdatesMachineLoad);
TEST_F(LoadBlanceTest, SmartChoice_WithEmptyOnlineList_ReturnsFalse);
TEST_F(LoadBlanceTest, OfflineMachine_RemovesMachineFromOnlineList);
```

### 7.2 集成测试

#### 7.2.1 API测试
```cpp
// 使用httplib测试Web API
#include "httplib.h"
#include <gtest/gtest.h>

class ApiTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 启动测试服务器
        server_thread_ = std::thread([this]() {
            test_server_.listen("localhost", 8080);
        });
        
        // 等待服务器启动
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    void TearDown() override {
        test_server_.stop();
        server_thread_.join();
    }
    
    httplib::Server test_server_;
    std::thread server_thread_;
};

TEST_F(ApiTest, RegisterUserSuccessfully) {
    httplib::Client client("localhost", 8080);
    
    // 准备请求数据
    Json::Value request_data;
    request_data["username"] = "testuser";
    request_data["password"] = "testpass123";
    request_data["email"] = "test@example.com";
    
    Json::FastWriter writer;
    std::string json_str = writer.write(request_data);
    
    // 发送请求
    auto res = client.Post("/api/register", json_str, "application/json");
    
    // 验证响应
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    
    Json::Reader reader;
    Json::Value response_data;
    ASSERT_TRUE(reader.parse(res->body, response_data));
    
    EXPECT_EQ(response_data["status"].asInt(), 0);
    EXPECT_EQ(response_data["reason"].asString(), "success");
}
```

#### 7.2.2 端到端测试
```javascript
// 使用Playwright进行端到端测试
const { test, expect } = require('@playwright/test');

test.describe('用户注册和登录流程', () => {
    test.beforeEach(async ({ page }) => {
        await page.goto('http://localhost:8080/login');
    });
    
    test('用户能够成功注册', async ({ page }) => {
        // 点击注册标签
        await page.click('text=注册');
        
        // 填写注册表单
        await page.fill('#register-username', 'newuser');
        await page.fill('#register-password', 'newpass123');
        await page.fill('#register-email', 'newuser@example.com');
        
        // 提交表单
        await page.click('#register-submit');
        
        // 验证注册成功
        await expect(page.locator('.success-message')).toContainText('注册成功');
    });
    
    test('用户能够成功登录', async ({ page }) => {
        // 填写登录表单
        await page.fill('#login-username', 'testuser');
        await page.fill('#login-password', 'testpass123');
        
        // 提交表单
        await page.click('#login-submit');
        
        // 验证登录成功并重定向到题库
        await expect(page).toHaveURL('http://localhost:8080/all_questions');
        await expect(page.locator('.user-profile')).toContainText('testuser');
    });
    
    test('用户能够提交代码并获得评测结果', async ({ page }) => {
        // 先登录
        await page.fill('#login-username', 'testuser');
        await page.fill('#login-password', 'testpass123');
        await page.click('#login-submit');
        
        // 进入题目详情页
        await page.goto('http://localhost:8080/question/1');
        
        // 输入代码
        await page.fill('#code-editor', '#include<iostream>\nint main(){std::cout<<"Hello"<<std::endl;return 0;}');
        
        // 提交代码
        await page.click('#submit-code');
        
        // 等待评测结果
        await expect(page.locator('.result-status')).toContainText('编译运行成功');
    });
});
```

## 8. 代码审查

### 8.1 审查清单

#### 8.1.1 功能性检查
- [ ] 代码是否实现了预期的功能？
- [ ] 边界条件是否处理正确？
- [ ] 错误处理是否完善？
- [ ] 是否存在潜在的内存泄漏？
- [ ] 线程安全问题是否考虑？

#### 8.1.2 代码质量检查
- [ ] 命名是否清晰且符合规范？
- [ ] 注释是否充分且准确？
- [ ] 代码结构是否清晰？
- [ ] 是否存在重复代码？
- [ ] 是否遵循设计原则？

#### 8.1.3 性能检查
- [ ] 是否存在不必要的拷贝？
- [ ] 算法复杂度是否合理？
- [ ] 资源使用是否高效？
- [ ] 是否存在性能瓶颈？

#### 8.1.4 安全检查
- [ ] 输入验证是否充分？
- [ ] 是否存在缓冲区溢出风险？
- [ ] 敏感信息是否得到保护？
- [ ] 权限控制是否正确？

### 8.2 审查流程

#### 8.2.1 提交前自检
```bash
# 代码格式检查
clang-format -i src/*.cpp src/*.hpp

# 静态分析
cppcheck --enable=all src/

# 编译警告检查
g++ -Wall -Wextra -Werror -std=c++11 src/*.cpp

# 单元测试
make test

# 代码覆盖率
gcov src/*.cpp
```

#### 8.2.2 同行审查
1. **创建Pull Request**: 描述清楚修改内容和原因
2. **自动检查**: CI/CD系统自动运行测试和静态分析
3. **人工审查**: 至少一名团队成员进行代码审查
4. **修改完善**: 根据审查意见修改代码
5. **合并代码**: 审查通过后合并到主分支

## 9. 文档规范

### 9.1 代码文档

#### 9.1.1 函数文档
```cpp
/**
 * @brief 获取所有题目列表
 * @param req HTTP请求对象，包含分页参数等
 * @param html 输出的HTML内容，由模板引擎渲染
 * @return bool 成功返回true，失败返回false
 * @note 需要用户已登录，会自动进行权限检查
 * @warning 输出内容较大时可能影响性能，建议实现分页
 * @see Question, Model::getAllQuestions()
 * @since v1.0.0
 */
bool allQuestions(const Request& req, string* html);
```

#### 9.1.2 类文档
```cpp
/**
 * @class LoadBlance
 * @brief 负载均衡器，负责编译服务器的选择和状态管理
 * 
 * 该类实现了最小负载优先的负载均衡算法，支持：
 * - 动态添加/移除编译服务器
 * - 服务器健康状态监控
 * - 负载统计和报告
 * 
 * @note 线程安全，内部使用互斥锁保护共享数据
 * @warning 所有操作都是阻塞的，在高并发场景下可能成为瓶颈
 */
class LoadBlance {
    // ...
};
```

### 9.2 架构文档

#### 9.2.1 模块说明
```markdown
## 负载均衡模块 (LoadBlance)

### 职责
- 管理编译服务器列表
- 监控服务器健康状态
- 实现负载均衡算法
- 提供服务器选择接口

### 设计决策
- 使用最小负载优先算法
- 线程安全设计
- 支持动态扩缩容
- 故障自动转移

### 性能考虑
- O(n)时间复杂度选择服务器
- 使用缓存优化频繁查询
- 锁粒度控制
```

#### 9.2.2 API文档
```markdown
## 用户认证API

### POST /api/register
用户注册接口

#### 请求参数
| 参数名 | 类型 | 必需 | 描述 |
|--------|------|------|------|
| username | string | 是 | 用户名，3-20个字符 |
| password | string | 是 | 密码，6-30个字符 |
| email | string | 是 | 邮箱地址 |

#### 响应格式
```json
{
    "status": 0,
    "reason": "success"
}
```

#### 错误码
- `status: 1` - 注册失败，原因在reason字段
```

## 10. 持续集成

### 10.1 CI/CD配置

#### 10.1.1 GitHub Actions配置
```yaml
name: CI

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  build:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y build-essential libjsoncpp-dev libctemplate-dev libmysqlclient-dev
    
    - name: Build
      run: |
        make clean
        make
    
    - name: Run tests
      run: |
        make test
    
    - name: Run static analysis
      run: |
        cppcheck --enable=all --error-exitcode=1 src/
    
    - name: Check code format
      run: |
        find src -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i --dry-run --Werror
```

#### 10.1.2 质量门禁
```yaml
# 质量检查配置
quality-gates:
  code-coverage:
    minimum: 80%
  
  static-analysis:
    max-warnings: 0
    max-errors: 0
  
  code-format:
    enforce: true
  
  test-pass-rate:
    minimum: 100%
  
  performance:
    max-response-time: 5s
    max-memory-usage: 1GB
```

### 10.2 发布流程

#### 10.2.1 版本管理
```bash
# 语义化版本号：主版本号.次版本号.修订号
# MAJOR.MINOR.PATCH

# 创建新版本
git tag -a v2.1.0 -m "Release version 2.1.0"
git push origin v2.1.0

# 自动生成变更日志
git log --pretty=format:"- %s" v2.0.0..v2.1.0 > CHANGELOG.md
```

#### 10.2.2 发布检查清单
- [ ] 所有测试通过
- [ ] 代码覆盖率达标
- [ ] 静态分析无错误
- [ ] 代码格式正确
- [ ] 文档已更新
- [ ] 变更日志已生成
- [ ] 版本号已更新
- [ ] 发布说明已准备

---

**文档版本**: v0.2.6  
**最后更新时间**: 2026-01-10  
**维护团队**: 在线评测系统开发团队