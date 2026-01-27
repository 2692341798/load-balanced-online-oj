# 在线评测系统架构设计文档

## 1. 系统概述

### 1.1 项目背景
负载均衡式在线评测系统是一个专为算法竞赛和编程练习设计的分布式评测平台。系统采用C++开发，支持多编译服务器负载均衡，能够高效处理代码编译、运行和评测任务。系统还包含完整的用户认证系统和现代化的Web界面。

### 1.2 核心功能
- **在线代码评测**: 支持C/C++代码的编译、运行和自动化测试
- **负载均衡**: 多编译服务器分布式处理，提高系统吞吐量
- **用户认证系统**: 完整的用户注册、登录、会话管理功能
- **题目管理**: 支持算法题目的创建、编辑和分类管理
- **现代化Web界面**: 响应式设计，支持深色主题
- **实时评测**: 快速反馈代码运行结果和测试用例通过情况
- **错误处理**: 完善的异常捕获和用户友好的错误提示

### 1.3 技术栈
- **后端**: C++11, 多线程编程, Socket网络编程, JSON处理
- **Web框架**: httplib.h（轻量级C++ HTTP服务器）
- **数据库**: MySQL 8.0+
- **模板引擎**: CTemplate
- **编译器**: GCC/G++
- **构建工具**: GNU Make
- **操作系统**: Linux/macOS
- **测试框架**: Playwright（端到端测试）

## 2. 整体架构

### 2.1 架构概览

```
┌─────────────────────────────────────────────────────────────┐
│                    Web前端界面                            │
│              (HTML/CSS/JS + 响应式设计)                    │
└─────────────────────┬───────────────────────────────────────┘
                      │ HTTP请求
┌─────────────────────┴───────────────────────────────────────┐
│                    OJ主服务器                              │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐         │
│  │   路由控制   │ │   业务逻辑   │ │   视图渲染   │         │
│  │ oj_control  │ │  控制器层    │ │ oj_view     │         │
│  └──────┬──────┘ └──────┬──────┘ └──────┬──────┘         │
│         │               │               │                  │
│  ┌──────┴──────┐ ┌──────┴──────┐ ┌──────┴──────┐         │
│  │   数据模型   │ │  负载均衡器  │ │   会话管理   │         │
│  │ oj_model2   │ │LoadBlance   │ │  Session     │         │
│  └──────┬──────┘ └──────┬──────┘ └──────┬──────┘         │
└────────┼────────┼────────┼──────────────────────────────┘
         │        │        │
         │        │        │
┌────────┼────────┼────────┼──────────────────────────────┐
│        │        │        │     编译服务器集群            │
│  ┌─────┴───┐ ┌──┴─────┐ ┌─┴──────┐                   │
│  │编译服务器1│ │编译服务器2│ │编译服务器3│                   │
│  │  端口8081 │ │ 端口8082 │ │ 端口8083 │                   │
│  └─────┬───┘ └──┬─────┘ └─┬──────┘                   │
│        │        │        │                           │
│  ┌─────┴────────┴────────┴─────┐                     │
│  │      编译运行核心模块         │                     │
│  │   compile_run + compiler     │                     │
│  │   + runner + 临时文件管理    │                     │
│  └──────────────────────────────┘                     │
└─────────────────────────────────────────────────────────┘
                      │
              ┌───────┴───────┐
              │    MySQL数据库  │
              │  题目/用户数据  │
              └───────────────┘
```

### 2.2 架构特点
- **分布式设计**: 主服务器与编译服务器分离，支持水平扩展
- **负载均衡**: 智能选择负载最低的编译服务器处理请求
- **模块化**: 各模块职责单一，便于维护和扩展
- **无状态**: 编译服务器无状态设计，支持动态扩缩容
- **用户认证**: 完整的用户注册、登录、会话管理功能
- **前后端分离**: 现代化的前端界面与后端API分离

## 3. 核心模块设计

### 3.1 OJ主服务器 (oj_server)

#### 3.1.1 职责边界
- **HTTP服务**: 提供Web界面和API接口
- **业务逻辑**: 处理用户请求、题目管理、代码提交
- **负载调度**: 智能分发编译任务到后端服务器
- **会话管理**: 维护用户登录状态和权限控制
- **数据交互**: 与MySQL数据库进行数据持久化
- **用户认证**: 处理用户注册、登录和密码验证
- **管理员系统**: 题目的增删改查和发布管理

#### 3.1.2 核心组件

**Control类 (oj_control.hpp)**
```cpp
class Control {
    // 核心业务逻辑控制器
    Model model_;              // 数据模型
    View view_;                // 视图渲染
    LoadBlance load_blance_;   // 负载均衡器
    std::unordered_map<std::string, Session> sessions_;  // 会话管理
    
public:
    // 主要业务方法
    bool AllQuestions(const Request &req, string *html);     // 获取题目列表
    bool Question(const string &number, const Request &req, string *html); // 获取题目详情
    void Judge(const std::string &number, const std::string in_json, std::string *out_json); // 代码评测
    bool Register(const std::string &username, const std::string &password, const std::string &email, const std::string &nickname, const std::string &phone); // 用户注册
    bool Login(const std::string &username, const std::string &password, std::string *token); // 用户登录
    bool AuthCheck(const Request &req, User *user); // 权限检查
    bool CheckUserExists(const std::string &username); // 检查用户是否存在
    bool GetProfile(const User &user, string *html); // 获取个人中心页面
    bool GetProfileData(const User &user, std::string *json); // 获取个人中心数据API

    // 管理员相关方法
    bool AdminAllQuestions(string *json); // 获取所有题目（含未发布）
    bool AddQuestion(const string &json_str); // 添加题目
    bool UpdateQuestion(const string &id, const string &json_str); // 更新题目
    bool DeleteQuestion(const string &id); // 删除题目
};
```

**LoadBlance类 (负载均衡器)**
```cpp
class LoadBlance {
private:
    std::vector<Machine> machines;     // 编译服务器列表
    std::vector<int> online;           // 在线服务器ID
    std::vector<int> offline;          // 离线服务器ID
    std::mutex mtx;                    // 线程安全保护
    
public:
    bool SmartChoice(int *id, Machine **m);    // 智能选择最优服务器
    void OfflineMachine(int which);            // 标记服务器离线
    void OnlineMachine();                      // 恢复服务器在线
    bool LoadConf(const std::string &machine_conf); // 加载配置
};
```

**Machine类 (编译服务器)**
```cpp
class Machine {
public:
    std::string ip;                    // 服务器IP地址
    int port;                          // 服务器端口
    uint64_t load;                     // 当前负载数
    std::mutex *mtx;                   // 负载计数保护锁
    
public:
    void IncLoad();                    // 增加负载
    void DecLoad();                    // 减少负载
    void ResetLoad();                  // 重置负载
    uint64_t Load();                   // 获取当前负载
};
```

**Session结构体 (会话管理)**
```cpp
struct Session {
    User user;            // 用户信息
    time_t expire_time;   // 过期时间戳
};
```

### 3.2 编译服务器 (compile_server)

#### 3.2.1 职责边界
- **代码编译**: 接收用户代码并进行语法检查
- **程序运行**: 在安全环境中执行编译后的程序
- **资源限制**: 控制CPU时间和内存使用量
- **结果收集**: 收集编译错误、运行输出和错误信息
- **临时文件**: 管理编译过程中的临时文件
- **JSON测试用例评测**: 基于JSON格式的测试用例进行评测，逐个用例比较输入输出

#### 3.2.2 核心组件

**CompileAndRun类 (compile_run.hpp)**
```cpp
class CompileAndRun {
public:
    static void Start(const std::string &in_json, std::string *out_json);
    static void RemoveTempFile(const std::string &file_name);
    static std::string CodeToDesc(int code, const std::string &file_name);
};
```

**Compiler类 (compiler.hpp)**
```cpp
class Compiler {
public:
    static bool Compile(const std::string &file_name);
    // 编译指定文件，生成可执行程序
};
```

**Runner类 (runner.hpp)**
```cpp
class Runner {
public:
    static int Run(const std::string &file_name, int cpu_limit, int mem_limit);
    // 运行程序并返回结果状态码
};
```

### 3.3 公共组件 (comm)

#### 3.3.1 工具类 (util.hpp)
```cpp
namespace ns_util {
    class TimeUtil;      // 时间戳工具
    class PathUtil;      // 文件路径工具
    class FileUtil;      // 文件操作工具
    class StringUtil;    // 字符串处理工具
}
```

#### 3.3.2 日志系统 (log.hpp)
```cpp
namespace ns_log {
    // 日志级别定义
    enum {
        INFO = 0,
        DEBUG = 1,
        WARNING = 2,
        ERROR = 3,
        FATAL = 4
    };
    
    class Log {
        // 线程安全的日志记录器
    };
}

### 3.4 多语言评测引擎 (Multi-language Judge Engine)

#### 3.4.1 支持语言
- **C++**: 使用G++编译器，支持LeetCode模式（尾部拼接）和ACM模式。
- **Java**: 使用Javac编译器，标准化类名，使用隔离的临时目录。
- **Python**: 使用Python解释器直接执行。

#### 3.4.2 评测机制
- **C++**: 编译后运行，拼接测试用例代码。
- **Java**: 编译生成class文件，通过java命令运行，支持标准输入输出。
- **Python**: 直接通过python命令运行脚本，通过标准输入传递测试用例。

## 4. 路由设计

### 4.1 主服务器路由

#### 4.1.1 页面路由
- `GET /` - 首页重定向到题目列表
- `GET /all_questions` - 题目列表页面（需要登录）
- `GET /question/{number}` - 题目详情页面（需要登录）
- `GET /login` - 登录页面

#### 4.1.2 API路由
- `POST /api/register` - 用户注册
- `POST /api/login` - 用户登录
- `GET /api/user` - 获取当前用户信息
- `POST /judge/{number}` - 代码评测（需要登录）

#### 4.1.3 管理员路由
- `GET /api/admin/questions` - 获取所有题目列表
- `POST /api/admin/question` - 创建题目
- `POST /api/admin/question/update/{id}` - 更新题目
- `POST /api/admin/question/delete/{id}` - 删除题目

### 4.2 编译服务器路由
- `POST /compile_and_run` - 编译运行服务

## 5. 数据结构设计

### 5.1 题目数据结构

```cpp
struct Question {
    std::string number;    // 题目编号，唯一标识
    std::string title;     // 题目标题
    std::string star;      // 难度等级: 简单/中等/困难
    std::string desc;      // 题目描述
    std::string header;    // 预设代码模板
    std::string tail;      // 测试用例代码
    int cpu_limit;         // CPU时间限制(秒)
    int mem_limit;         // 内存限制(KB)
};
```

### 5.2 用户数据结构

```cpp
struct User {
    std::string id;        // 用户ID
    std::string username;  // 用户名
    std::string password;  // 密码(已哈希)
    std::string email;     // 邮箱地址
    std::string nickname;  // 昵称
    std::string phone;     // 手机号
    std::string created_at;// 注册时间
};

struct Submission {
    std::string id;          // 提交ID
    std::string user_id;     // 用户ID
    std::string question_id; // 题目ID
    std::string result;      // 评测结果状态码
    int cpu_time;            // 运行耗时
    int mem_usage;           // 内存使用
    std::string created_at;  // 提交时间
};
```

### 5.3 会话数据结构

```cpp
struct Session {
    User user;            // 用户信息
    time_t expire_time;   // 过期时间戳
};
```

### 5.4 评测请求数据结构

```json
// 请求格式
{
    "code": "#include <iostream>...",
    "input": "测试输入数据",
    "cpu_limit": 1,
    "mem_limit": 10240
}

// 响应格式
{
    "status": 0,
    "reason": "编译运行成功",
    "stdout": "程序输出",
    "stderr": "错误输出"
}
```

### 5.5 状态码定义

| 状态码 | 含义说明 |
|--------|----------|
| 0 | 编译运行成功 |
| -1 | 提交的代码为空 |
| -2 | 未知错误 |
| -3 | 编译错误 |
| 6 (SIGABRT) | 内存超过限制 |
| 24 (SIGXCPU) | CPU使用超时 |
| 8 (SIGFPE) | 浮点数溢出 |

## 6. 用户认证系统设计

### 6.1 认证流程

#### 6.1.1 用户注册流程
```
1. 用户提交注册表单（用户名、密码、邮箱等）
2. 服务器验证参数合法性
3. 检查用户名是否已存在
4. 对密码进行SHA256哈希处理
5. 将用户信息插入数据库
6. 返回注册结果
```

#### 6.1.2 用户登录流程
```
1. 用户提交登录表单（用户名、密码）
2. 服务器验证用户名和密码
3. 生成会话Token（基于时间戳和随机数）
4. 创建会话信息并存储在内存中
5. 设置Cookie返回给客户端
6. 返回登录成功响应
```

#### 6.1.3 权限检查流程
```
1. 从请求中提取Cookie中的session_id
2. 在内存会话映射中查找对应会话
3. 检查会话是否过期
4. 验证通过后提取用户信息
5. 继续处理业务请求
```

### 6.2 安全设计

#### 6.2.1 密码安全
- 使用SHA256算法对用户密码进行哈希处理
- 不存储明文密码，只存储哈希值
- 密码长度限制：6-30个字符

#### 6.2.2 会话安全
- Token生成基于时间戳和随机数
- 会话过期时间：24小时
- 支持并发访问的线程安全设计

#### 6.2.3 参数验证
- 用户名长度限制：3-20个字符
- 邮箱格式验证
- 输入参数完整性检查

## 7. 前后端交互

### 7.1 请求处理流程

```
用户请求 → Web服务器(httplib) → 路由分发 → 权限检查 → 业务逻辑处理 → 数据模型操作 → 视图渲染 → 响应返回
```

### 7.2 典型交互示例

#### 7.2.1 代码提交流程
```
1. 用户在Web界面编写代码
2. 前端通过AJAX发送POST请求到/judge/{number}
3. 主服务器接收请求，验证用户身份
4. 主服务器从数据库获取题目信息
5. 主服务器拼接用户代码和测试用例
6. 负载均衡器选择最优编译服务器
7. 主服务器发送编译请求到选中的编译服务器
8. 编译服务器执行编译和运行
9. 编译服务器返回结果给主服务器
10. 主服务器格式化结果并返回给前端
11. 前端展示评测结果给用户
```

#### 7.2.2 用户认证流程
```
1. 用户提交登录表单
2. 服务器验证用户名密码
3. 生成会话token和过期时间
4. 将会话信息存储在内存中
5. 设置Cookie返回给客户端
6. 后续请求携带Cookie进行身份验证
7. 服务器从内存中查找会话信息
8. 检查会话是否过期
9. 验证通过后处理业务请求
```

### 7.3 数据传输格式

#### 7.3.1 请求数据格式
```json
{
    "code": "用户提交的代码",
    "input": "用户输入数据",
    "cpu_limit": 时间限制,
    "mem_limit": 内存限制
}
```

#### 7.3.2 响应数据格式
```json
{
    "status": 状态码,
    "reason": "状态描述",
    "stdout": "标准输出",
    "stderr": "标准错误"
}
```

## 8. 模块协作机制

### 8.1 模块关系图

```
oj_server (主服务器)
├── Control (业务控制器)
│   ├── Model (数据模型)
│   ├── View (视图渲染)
│   ├── LoadBlance (负载均衡)
│   └── Session (会话管理)
├── httplib (HTTP服务器)
└── MySQL (数据库)

compile_server (编译服务器)
├── CompileAndRun (编译运行核心)
├── Compiler (编译器封装)
├── Runner (程序运行器)
└── httplib (HTTP服务器)

comm (公共组件)
├── util (工具类)
├── log (日志系统)
└── httplib (HTTP库)
```

### 8.2 消息传递机制

#### 8.2.1 同步HTTP调用
- 主服务器与编译服务器之间采用HTTP协议同步通信
- 请求-响应模式，确保数据一致性
- 超时机制防止长时间等待

#### 8.2.2 负载信息更新
```cpp
// 负载更新流程
1. 主服务器选择编译服务器
2. 增加该服务器的负载计数
3. 发送编译请求
4. 接收响应结果
5. 减少该服务器的负载计数
```

#### 8.2.3 故障检测与恢复
```cpp
// 故障处理流程
1. HTTP请求失败时标记服务器离线
2. 将该服务器从在线列表移除
3. 定期尝试重新连接离线服务器
4. 连接成功后恢复服务器在线状态
```

### 8.3 并发控制

#### 8.3.1 线程安全保护
```cpp
// 关键数据结构保护
std::mutex mtx;                    // 保护负载均衡器数据
std::mutex *mtx;                   // 保护单个服务器负载计数
std::mutex session_mtx_;            // 保护会话数据
```

#### 8.3.2 原子操作
```cpp
// 原子递增ID生成
static std::atomic_uint id(0);
id++;
```

## 9. 前端架构

### 9.1 文件结构
```
oj_server/
├── css/                          # 样式文件
│   ├── index.css                 # 主页样式
│   ├── all_questions.css         # 题目列表样式
│   ├── one_question.css          # 题目详情样式
│   └── login.css                 # 登录页面样式
├── template_html/                # HTML模板
│   ├── all_questions.html        # 题目列表模板
│   ├── one_question.html         # 题目详情模板
│   └── login.html                # 登录页面模板
└── wwwroot/                      # 静态资源
    └── index.html                # 主页
```

### 9.2 设计特点
- **响应式设计**: 适配不同屏幕尺寸
- **现代化界面**: 深色主题，渐变色彩
- **用户友好**: 清晰的导航和反馈
- **代码编辑器**: VS Code风格编辑器，支持语法高亮和智能提示
- **布局优化**: 可调整大小的面板（Resizable Panes）
- **结果展示**: 标签页式结果展示（Tabbed Results）
- **讨论社区 (V0.3.2)**: 基于 SPA 模式的轻量级社区，集成 Markdown 编辑器 (EasyMDE) 和内联评论交互

### 9.3 样式系统
- **CSS变量**: 统一的颜色和主题管理
- **组件化**: 可复用的UI组件
- **动画效果**: 平滑的过渡和交互反馈

## 10. 性能优化

### 10.1 关键性能指标

#### 10.1.1 响应时间
- **题目列表加载**: < 100ms
- **题目详情加载**: < 200ms
- **代码评测响应**: < 5s (包含编译运行时间)
- **用户认证**: < 100ms

#### 10.1.2 并发处理能力
- **单编译服务器**: 支持10个并发评测任务
- **主服务器**: 支持100个并发Web请求
- **系统总吞吐量**: 1000次评测/分钟

### 10.2 性能优化策略

#### 10.2.1 编译服务器优化
```cpp
// 资源限制策略
int cpu_limit = 1;      // CPU时间限制1秒
int mem_limit = 10240;  // 内存限制10MB

// 临时文件快速清理
RemoveTempFile(file_name);  // 及时清理临时文件
```

#### 10.2.2 负载均衡优化
```cpp
// 智能选择算法
// 选择负载最小的服务器处理新请求
uint64_t min_load = machines[online[0]].Load();
for (int i = 1; i < online_num; i++) {
    uint64_t curr_load = machines[online[i]].Load();
    if (min_load > curr_load) {
        min_load = curr_load;
        // 选择当前负载最小的服务器
    }
}
```

#### 10.2.3 数据库查询优化
```cpp
// 题目列表排序优化
sort(all.begin(), all.end(), [](const struct Question &q1, const struct Question &q2){
    return atoi(q1.number.c_str()) < atoi(q2.number.c_str());
});
```

### 10.3 内存管理

#### 10.3.1 临时文件管理
```cpp
// 临时文件生命周期管理
1. 生成唯一文件名
2. 创建临时文件
3. 使用完成后立即删除
4. 异常情况下也要确保清理
```

#### 10.3.2 内存池使用
```cpp
// 避免频繁内存分配
static std::string code;  // 复用字符串缓冲区
static Json::Value value; // 复用JSON对象
```

## 11. 安全设计

### 11.1 代码执行安全

#### 11.1.1 沙箱机制
```cpp
// 资源限制
setrlimit(RLIMIT_CPU, &cpu_limit);    // CPU时间限制
setrlimit(RLIMIT_AS, &mem_limit);     // 内存使用限制

// 权限控制
setuid(nobody_uid);                   // 降权运行
```

#### 11.1.2 输入验证
```cpp
// 代码长度检查
if (code.size() == 0) {
    status_code = -1;  // 拒绝空代码
    return;
}

// 危险关键字过滤
// 禁止system、exec等危险函数调用
```

### 11.2 用户认证安全

#### 11.2.1 密码安全
```cpp
// 密码哈希存储
std::string HashPassword(const std::string &password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*)password.c_str(), password.length(), hash);
    // 转换为十六进制字符串存储
}
```

#### 11.2.2 会话安全
```cpp
// Token生成
std::string GenerateToken() {
    char buf[64];
    snprintf(buf, sizeof(buf), "%lx%lx", 
             (unsigned long)time(nullptr), 
             (unsigned long)rand());
    return std::string(buf);
}

// 会话过期时间
expire_time = time(nullptr) + 86400;  // 1天过期
```

### 11.3 系统安全

#### 11.3.1 文件系统安全
```cpp
// 临时文件隔离
temp_path = "./temp/";  // 独立的临时目录
umask(077);             // 严格的文件权限
```

#### 11.3.2 网络安全
```cpp
// HTTP请求验证
if (req.has_header("Content-Type")) {
    // 验证内容类型
}

// 防止SQL注入
// 使用参数化查询，避免字符串拼接
```

## 12. 异常处理

### 12.1 异常分类

#### 12.1.1 编译时异常
- **语法错误**: 代码语法不正确
- **类型错误**: 变量类型不匹配
- **链接错误**: 缺少必要的库文件

#### 12.1.2 运行时异常
- **内存溢出**: 申请的内存超过限制
- **CPU超时**: 程序执行时间超过限制
- **除零错误**: 数学运算异常
- **段错误**: 非法内存访问

#### 12.1.3 系统异常
- **服务器离线**: 编译服务器不可用
- **数据库错误**: 数据库连接或查询失败
- **文件系统错误**: 临时文件创建失败

### 12.2 异常处理策略

#### 12.2.1 编译异常处理
```cpp
// 编译错误处理
if (!Compiler::Compile(file_name)) {
    status_code = -3;  // 编译错误状态码
    // 读取编译错误信息
    FileUtil::ReadFile(PathUtil::CompilerError(file_name), &desc, true);
    return;
}
```

#### 12.2.2 运行异常处理
```cpp
// 信号处理
switch (code) {
    case SIGABRT:  // 6
        desc = "内存超过范围";
        break;
    case SIGXCPU:  // 24
        desc = "CPU使用超时";
        break;
    case SIGFPE:   // 8
        desc = "浮点数溢出";
        break;
    default:
        desc = "未知错误: " + std::to_string(code);
        break;
}
```

#### 12.2.3 系统异常处理
```cpp
// 服务器故障处理
if (!load_blance_.SmartChoice(&id, &m)) {
    LOG(FATAL) << "所有的后端编译主机已经离线";
    return false;
}

// 自动故障转移
while (true) {
    if (auto res = cli.Post("/compile_and_run", compile_string, "application/json")) {
        if (res->status == 200) {
            // 请求成功
            break;
        }
    } else {
        // 请求失败，标记服务器离线
        load_blance_.OfflineMachine(id);
        // 继续尝试其他服务器
    }
}
```

### 12.3 错误恢复机制

#### 12.3.1 自动重试
```cpp
// HTTP请求重试机制
int retry_count = 3;
while (retry_count-- > 0) {
    if (auto res = cli.Post("/compile_and_run", data, "application/json")) {
        if (res->status == 200) {
            return true;  // 成功
        }
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));  // 等待1秒重试
}
```

#### 12.3.2 服务降级
```cpp
// 当所有编译服务器离线时的处理
if (online_num == 0) {
    // 返回友好的错误信息
    *out_json = R"({"status": -1, "reason": "系统维护中，请稍后重试"})";
    return false;
}
```

## 13. 架构决策记录

### 13.1 ADR-001: 分布式架构选择

**日期**: 2024-01-01  
**状态**: 已接受  
**背景**: 需要支持高并发的代码评测服务

**决策**: 采用主服务器+多编译服务器的分布式架构

**理由**:
- 编译任务CPU密集，需要独立的服务器处理
- 支持水平扩展，可根据负载增加编译服务器
- 故障隔离，单个编译服务器故障不影响整体服务
- 资源隔离，编译任务不会影响Web服务响应

**后果**:
- 增加了系统复杂性，需要处理分布式通信
- 需要实现负载均衡和故障转移机制
- 临时文件管理更加复杂

### 13.2 ADR-002: 负载均衡算法选择

**日期**: 2024-01-02  
**状态**: 已接受  
**背景**: 需要在多个编译服务器之间分配任务

**决策**: 采用最小负载优先算法

**理由**:
- 实现简单，易于理解和维护
- 能够有效避免单个服务器过载
- 适合编译任务这种执行时间相对固定的场景
- 不需要维护复杂的状态信息

**后果**:
- 可能会出现短时间内负载不均衡的情况
- 需要考虑服务器故障时的处理逻辑
- 负载计数需要线程安全保护

### 13.3 ADR-003: 会话管理方案

**日期**: 2024-01-03  
**状态**: 已接受  
**背景**: 需要支持用户登录和身份验证

**决策**: 采用内存中的会话管理，使用Cookie进行身份识别

**理由**:
- 实现简单，不需要额外的存储系统
- 性能高，内存访问速度快
- 适合单机部署的场景
- 易于实现会话过期机制

**后果**:
- 不支持分布式部署，会话无法跨服务器共享
- 服务器重启会导致所有会话失效
- 内存使用量随在线用户数量增长

### 13.4 ADR-004: 用户认证设计

**日期**: 2024-01-04  
**状态**: 已接受  
**背景**: 需要支持用户注册和登录功能

**决策**: 采用SHA256密码哈希 + Token会话认证

**理由**:
- SHA256是广泛使用的安全哈希算法
- Token机制简单有效，易于实现
- 不需要额外的认证服务
- 支持会话过期和权限控制

**后果**:
- 需要安全地存储和管理Token
- 密码找回功能需要额外设计
- 需要考虑Token泄露的风险

### 13.5 ADR-005: 前端技术选择

**日期**: 2024-01-05  
**状态**: 已接受  
**背景**: 需要提供用户友好的Web界面

**决策**: 采用纯HTML/CSS/JS + CTemplate模板引擎

**理由**:
- 与C++后端技术栈保持一致
- 不需要额外的前端构建工具
- 模板引擎简单易用
- 支持动态内容渲染

**后果**:
- 前端交互能力有限
- 需要手动处理AJAX请求
- 样式和脚本管理需要规范

---

**文档版本**: v0.3.2  
**最后更新时间**: 2026-01-27  
**维护团队**: 在线评测系统开发团队